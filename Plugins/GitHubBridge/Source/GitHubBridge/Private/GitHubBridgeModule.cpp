#include "GitHubGit.h"
#include "GitHubRevisionControl.h"
#include "GitHubSubmitBridge.h"
#include "Async/Async.h"
#include "Containers/Ticker.h"
#include "FileHelpers.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Misc/CoreDelegates.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "SourceControlOperations.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "GitHubBridge"

class FGitHubBridgeModule : public IModuleInterface
{
    FGitHubConfig Config;
    FString RevisionControlStatus = TEXT("Waiting for project repository detection.");
    bool bRevisionControlConnecting = false;
    bool bEditorInitialized = false;
    bool bDetectingProject = false;
    TSharedPtr<int32> CallbackLifetime = MakeShared<int32>(0);
    TUniquePtr<FGitHubSubmitBridge> SubmitBridge;
    struct FQueuedSubmitPush { FGitHubConfig Config; FString Commit; };
    TArray<FQueuedSubmitPush> SubmitPushQueue;
    bool bNotifyPushResult = false;
    FString DiffOutput = TEXT("Choose Pending changes or Latest commit to review a diff.");
    bool bDiffOperation = false;
    FString Message;
    FString Output = TEXT("Configure your existing Git checkout, apply settings, then refresh status.");
    TFuture<FGitHubResult> Work;
    bool bBusy = false;
    bool bExitOperation = false;
    bool bExitAllowed = false;
    bool bExitDialogOpen = false;
    TWeakPtr<SWindow> ExitWindow;
    FDelegateHandle CloseHandle;
    FDelegateHandle ModalTickHandle;
    FTSTicker::FDelegateHandle TickHandle;
    const FName TabName = TEXT("GitHubBridge");
    const FName DiffTabName = TEXT("GitHubBridgeDiff");

    FString SettingsFile() const { return FPaths::ProjectSavedDir() / TEXT("Config/GitHubBridge.ini"); }
    void Load()
    {
        Config.Root = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
        const FString File = SettingsFile();
        const TCHAR* Section = TEXT("GitHubBridge");
        GConfig->GetString(Section, TEXT("Git"), Config.Git, File);
        GConfig->GetString(Section, TEXT("Root"), Config.Root, File);
        GConfig->GetString(Section, TEXT("Remote"), Config.Remote, File);
        GConfig->GetString(Section, TEXT("Branch"), Config.Branch, File);
        GConfig->GetString(Section, TEXT("Username"), Config.Username, File);
        GConfig->GetString(Section, TEXT("Name"), Config.Name, File);
        GConfig->GetString(Section, TEXT("Email"), Config.Email, File);
        GConfig->GetBool(Section, TEXT("PromptOnExit"), Config.bPromptOnExit, File);
        GConfig->GetBool(Section, TEXT("AutoConnectRevisionControl"), Config.bAutoConnectRevisionControl, File);
        GConfig->GetBool(Section, TEXT("PushAfterUnrealSubmit"), Config.bPushAfterUnrealSubmit, File);
        Config.Git = GitHubGit::FindProjectGit(FPaths::ProjectDir(), Config.Git);
    }
    void Save()
    {
        const FString File = SettingsFile();
        const TCHAR* Section = TEXT("GitHubBridge");
        GConfig->SetString(Section, TEXT("Git"), *Config.Git, File);
        GConfig->SetString(Section, TEXT("Root"), *Config.Root, File);
        GConfig->SetString(Section, TEXT("Remote"), *Config.Remote, File);
        GConfig->SetString(Section, TEXT("Branch"), *Config.Branch, File);
        GConfig->SetString(Section, TEXT("Username"), *Config.Username, File);
        GConfig->SetString(Section, TEXT("Name"), *Config.Name, File);
        GConfig->SetString(Section, TEXT("Email"), *Config.Email, File);
        GConfig->SetBool(Section, TEXT("PromptOnExit"), Config.bPromptOnExit, File);
        GConfig->SetBool(Section, TEXT("AutoConnectRevisionControl"), Config.bAutoConnectRevisionControl, File);
        GConfig->SetBool(Section, TEXT("PushAfterUnrealSubmit"), Config.bPushAfterUnrealSubmit, File);
        GConfig->Flush(false, File);
    }
    bool Tick(float)
    {
        if (bBusy && Work.IsReady())
        {
            const FGitHubResult R = Work.Get();
            const bool bWasDetection = bDetectingProject;
            bDetectingProject = false;
            if (R.bSuccess && R.DetectedConfig.IsSet())
            {
                Config = R.DetectedConfig.GetValue();
                Save();
            }
            Output = (R.bSuccess ? TEXT("SUCCESS\n") : TEXT("FAILED\n")) + R.Output;
            bBusy = false;
            if (bDiffOperation)
            {
                DiffOutput = R.Output;
                bDiffOperation = false;
            }
            if (bNotifyPushResult)
            {
                Notify(R.bSuccess ? (R.Output.Contains(TEXT("Rebased onto")) ? TEXT("Rebased and pushed. Restart Unreal before editing assets changed on disk.") : TEXT("GitHub push completed."))
                    : TEXT("Rebase or push failed. Local history is retained. Open GitHub Bridge for details."), !R.bSuccess);
                bNotifyPushResult = false;
            }
            if (bExitOperation && R.bSuccess)
            {
                bExitAllowed = true;
                if (auto W = ExitWindow.Pin()) W->RequestDestroyWindow();
            }
            bExitOperation = false;
            if (R.bSuccess && R.DetectedConfig.IsSet()) ConnectRevisionControl();
            else if (bWasDetection && !R.bSuccess)
                RevisionControlStatus = Config.bAutoConnectRevisionControl
                    ? TEXT("Not connected: project repository detection failed. See the details below.")
                    : TEXT("Automatic connection is off. Unreal's current provider is left unchanged.");
            else if (R.bSuccess && Config.bAutoConnectRevisionControl && ISourceControlModule::Get().GetProvider().GetName() == FName(TEXT("Git")))
                ISourceControlModule::Get().QueueStatusUpdate(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()));
        }
        if (!bBusy && !bRevisionControlConnecting && !SubmitPushQueue.IsEmpty())
        {
            const FQueuedSubmitPush Pending = SubmitPushQueue[0];
            SubmitPushQueue.RemoveAt(0);
            if (Config.bPushAfterUnrealSubmit)
            {
                bNotifyPushResult = true;
                Start([Pending] { return GitHubGit::PushCommitted(Pending.Config, Pending.Commit); });
            }
        }
        return true;
    }
    void Notify(const FString& Text, bool bFailed = false)
    {
        UE_LOG(LogTemp, Display, TEXT("GitHubBridge: %s"), *Text);
        if (FApp::IsUnattended() || !FSlateApplication::IsInitialized()) return;
        FNotificationInfo Info(FText::FromString(Text));
        Info.ExpireDuration = bFailed ? 15.0f : 7.0f;
        if (auto Item = FSlateNotificationManager::Get().AddNotification(Info))
            Item->SetCompletionState(bFailed ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success);
    }
    void OnUnrealCommitted()
    {
        const auto ProviderStatus = ISourceControlModule::Get().GetProvider().GetStatus();
        const FString* ProviderRoot = ProviderStatus.Find(ISourceControlProvider::EStatus::Repository);
        if (!ProviderRoot || !FPaths::IsSamePath(*ProviderRoot, Config.Root))
        {
            Output = TEXT("Unreal committed locally, but its checkout differs from GitHub Bridge's configured repository. Detect the project repository, then use Push existing commits.");
            Notify(Output, true);
            return;
        }
        const auto Commit = GitHubGit::Run(Config, { TEXT("rev-parse"), TEXT("--verify"), TEXT("HEAD") }, 10);
        if (!Commit.bSuccess)
        {
            Output = TEXT("Unreal committed locally, but the new commit could not be identified. Refresh status and use Push existing commits.");
            Notify(Output, true);
            return;
        }
        SubmitPushQueue.Add({ Config, Commit.Output.TrimStartAndEnd() });
        Notify(TEXT("Unreal submit committed locally. GitHub push queued."));
    }
    void Start(TFunction<FGitHubResult()> Job, bool bForExit = false)
    {
        if (bBusy) return;
        Save();
        bBusy = true;
        bExitOperation = bForExit;
        Output = TEXT("Working… Network commands have a three-minute timeout. Authentication may open your browser.");
        Work = Async(EAsyncExecution::ThreadPool, [Job = MoveTemp(Job)]() { return Job(); });
    }
    void ConnectRevisionControl()
    {
        if (!Config.bAutoConnectRevisionControl)
        {
            RevisionControlStatus = TEXT("Automatic connection is off. Unreal's current provider is left unchanged.");
            return;
        }
        if (!GitHubRevisionControl::PrepareGitPath(true, Config.Git))
        {
            RevisionControlStatus = TEXT("Git path saved for Unreal's provider. Restart Unreal to use this path, or check that the executable exists.");
            return;
        }
        if (!GitHubRevisionControl::SelectGitProvider(true, RevisionControlStatus)) return;
        bRevisionControlConnecting = true;
        const TWeakPtr<int32> Lifetime = CallbackLifetime;
        const auto Operation = ISourceControlOperation::Create<FConnect>();
        const ECommandResult::Type Started = ISourceControlModule::Get().GetProvider().Execute(Operation, EConcurrency::Asynchronous,
            FSourceControlOperationComplete::CreateLambda([this, Lifetime](const FSourceControlOperationRef&, ECommandResult::Type Result)
            {
                if (!Lifetime.IsValid()) return;
                bRevisionControlConnecting = false;
                RevisionControlStatus = Result == ECommandResult::Succeeded
                    ? TEXT("Connected to Unreal's Git revision control. Asset status and revision-control menus are available.")
                    : TEXT("Unreal's Git status refresh failed. Check the editor Revision Control log and repository settings.");
                UE_LOG(LogTemp, Display, TEXT("GitHubBridge revision control: %s"), *RevisionControlStatus);
            }));
        if (Started != ECommandResult::Succeeded)
        {
            bRevisionControlConnecting = false;
            RevisionControlStatus = TEXT("Unreal could not start the Git connection. Check the Revision Control log.");
        }
    }
    bool SavePackages()
    {
        if (!FEditorFileUtils::SaveDirtyPackages(false, true, true, false, false, false))
        {
            Output = TEXT("Asset saving was cancelled or failed. Commit/push cancelled; the editor remains open.");
            return false;
        }
        return true;
    }
    FReply Commit(bool bForExit)
    {
        if (!SavePackages()) return FReply::Handled();
        const FGitHubConfig Copy = Config;
        const FString CommitMessage = Message;
        Start([Copy, CommitMessage] { return GitHubGit::CommitPush(Copy, CommitMessage); }, bForExit);
        return FReply::Handled();
    }
    TSharedRef<SWidget> Field(const FString& Label, FString FGitHubConfig::* Member)
    {
        return SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 2)
            [ SNew(STextBlock).Text(FText::FromString(Label)) ]
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(SEditableTextBox)
                .IsEnabled_Lambda([this] { return !bBusy && !bExitDialogOpen; })
                .Text_Lambda([this, Member] { return FText::FromString(Config.*Member); })
                .OnTextChanged_Lambda([this, Member](const FText& Text) { Config.*Member = Text.ToString(); })
                .OnTextCommitted_Lambda([this](const FText&, ETextCommit::Type) { Save(); }) ];
    }
    TSharedRef<SWidget> Button(const FString& Label, TFunction<FReply()> Action)
    {
        return SNew(SButton).Text(FText::FromString(Label))
            .IsEnabled_Lambda([this] { return !bBusy && !bRevisionControlConnecting; })
            .OnClicked_Lambda([Action] { return Action(); });
    }
    TSharedRef<SWidget> OutputBox()
    {
        return SNew(SMultiLineEditableTextBox).IsReadOnly(true)
            .Text_Lambda([this] { return FText::FromString(Output); });
    }
    TSharedRef<SWidget> MessageField()
    {
        return SNew(SEditableTextBox).HintText(LOCTEXT("Message", "Commit message"))
            .IsEnabled_Lambda([this] { return !bBusy; })
            .Text_Lambda([this] { return FText::FromString(Message); })
            .OnTextChanged_Lambda([this](const FText& Text) { Message = Text.ToString(); });
    }
    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs&)
    {
        TSharedRef<SVerticalBox> Fields = SNew(SVerticalBox);
        Fields->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
        [ Button(TEXT("Review changes / diff"), [this]
            {
                FGlobalTabmanager::Get()->TryInvokeTab(DiffTabName);
                RequestDiff(false);
                return FReply::Handled();
            }) ];
        Fields->AddSlot().AutoHeight().Padding(0, 0, 0, 6)
        [ Button(TEXT("Detect project repository"), [this]
            {
                if (!bExitDialogOpen) DetectProjectRepository();
                return FReply::Handled();
            }) ];
        Fields->AddSlot().AutoHeight()[Field(TEXT("Git executable (project/git checked first during detection)"), &FGitHubConfig::Git)];
        Fields->AddSlot().AutoHeight()[Field(TEXT("Existing local repository root"), &FGitHubConfig::Root)];
        Fields->AddSlot().AutoHeight()[Field(TEXT("GitHub clone URL (origin)"), &FGitHubConfig::Remote)];
        Fields->AddSlot().AutoHeight()[Field(TEXT("Working branch"), &FGitHubConfig::Branch)];
        Fields->AddSlot().AutoHeight()[Field(TEXT("GitHub username (HTTPS browser sign-in)"), &FGitHubConfig::Username)];
        Fields->AddSlot().AutoHeight()[Field(TEXT("Commit author name"), &FGitHubConfig::Name)];
        Fields->AddSlot().AutoHeight()[Field(TEXT("Commit author email"), &FGitHubConfig::Email)];
        Fields->AddSlot().AutoHeight().Padding(0, 8)
        [ SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("Auth", "HTTPS uses Git Credential Manager and browser sign-in. SSH uses your existing keys/agent. Tokens and passwords are never stored by this plugin.")) ];
        Fields->AddSlot().AutoHeight()
        [ SNew(SCheckBox)
            .IsEnabled_Lambda([this] { return !bBusy && !bExitDialogOpen && !bRevisionControlConnecting; })
            .IsChecked_Lambda([this] { return Config.bAutoConnectRevisionControl ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State)
            {
                Config.bAutoConnectRevisionControl = State == ECheckBoxState::Checked;
                Save();
                if (Config.bAutoConnectRevisionControl) DetectProjectRepository();
                else RevisionControlStatus = TEXT("Automatic connection is off. Unreal's current provider is left unchanged.");
            })
            [ SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("AutoRevisionControl", "Automatically connect Unreal's Git revision control (selects Git as the active provider)")) ] ];
        Fields->AddSlot().AutoHeight().Padding(0, 4, 0, 8)
        [ SNew(STextBlock).AutoWrapText(true).Text_Lambda([this] { return FText::FromString(RevisionControlStatus); }) ];
        Fields->AddSlot().AutoHeight()
        [ SNew(SCheckBox)
            .IsEnabled_Lambda([this] { return !bBusy && !bExitDialogOpen && !bRevisionControlConnecting; })
            .IsChecked_Lambda([this] { return Config.bPushAfterUnrealSubmit ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State)
            {
                Config.bPushAfterUnrealSubmit = State == ECheckBoxState::Checked;
                if (!Config.bPushAfterUnrealSubmit) SubmitPushQueue.Empty();
                if (SubmitBridge) SubmitBridge->SetEnabled(Config.bPushAfterUnrealSubmit);
                Save();
            })
            [ SNew(STextBlock).Text(LOCTEXT("PushNativeSubmit", "Rebase and push to GitHub after a successful Unreal Git submit")) ] ];
        Fields->AddSlot().AutoHeight().Padding(0, 4, 0, 8)
        [ SNew(STextBlock).AutoWrapText(true).Text_Lambda([this] { return FText::FromString(SubmitBridge ? SubmitBridge->GetStatus() : TEXT("Submit integration not initialized.")); }) ];
        Fields->AddSlot().AutoHeight()
        [ SNew(SCheckBox)
            .IsEnabled_Lambda([this] { return !bBusy && !bExitDialogOpen; })
            .IsChecked_Lambda([this] { return Config.bPromptOnExit ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { Config.bPromptOnExit = State == ECheckBoxState::Checked; Save(); })
            [ SNew(STextBlock).Text(LOCTEXT("Prompt", "Prompt on exit only when assets need saving or changes need pushing")) ] ];
        auto Operation = [this](int32 Which)
        {
            const FGitHubConfig Copy = Config;
            Start([Copy, Which]
            {
                switch (Which)
                {
                    case 0: return GitHubGit::Configure(Copy);
                    case 1: return GitHubGit::Login(Copy);
                    case 2: return GitHubGit::Status(Copy);
                    case 3: return GitHubGit::SwitchBranch(Copy);
                    default: return GitHubGit::Pull(Copy);
                }
            });
            return FReply::Handled();
        };
        const TCHAR* Labels[] = { TEXT("Apply origin + author settings"), TEXT("Sign in to GitHub (browser)"), TEXT("Refresh status"), TEXT("Fetch + switch branch"), TEXT("Pull (fast-forward only)") };
        for (int32 Index = 0; Index < 5; ++Index)
            Fields->AddSlot().AutoHeight().Padding(0, 3)
            [ Button(Labels[Index], [this, Operation, Index]
                {
                    if (bExitDialogOpen) return FReply::Handled();
                    if ((Index == 3 || Index == 4) &&
                        FMessageDialog::Open(EAppMsgType::OkCancel, LOCTEXT("Reload", "This changes files on disk. Close asset editors first; restart Unreal afterwards to reload changed assets. Continue?")) != EAppReturnType::Ok)
                        return FReply::Handled();
                    if ((Index == 3 || Index == 4) && !SavePackages()) return FReply::Handled();
                    return Operation(Index);
                }) ];
        Fields->AddSlot().AutoHeight().Padding(0, 8)[MessageField()];
        Fields->AddSlot().AutoHeight().Padding(0, 3)
        [ Button(TEXT("Rebase + push existing commits"), [this]
            {
                if (bExitDialogOpen) return FReply::Handled();
                const FGitHubConfig Copy = Config;
                bNotifyPushResult = true;
                Start([Copy] { return GitHubGit::PushCommitted(Copy); });
                return FReply::Handled();
            }) ];
        Fields->AddSlot().AutoHeight()
        [ Button(TEXT("Save assets, commit ALL changes + push"), [this]
            {
                if (bExitDialogOpen) return FReply::Handled();
                if (FMessageDialog::Open(EAppMsgType::OkCancel, LOCTEXT("All", "Save assets, stage ALL non-ignored changes in the repository (including deletions), commit, and push to the configured GitHub branch? Review status and .gitignore first.")) != EAppReturnType::Ok)
                    return FReply::Handled();
                return Commit(false);
            }) ];
        Fields->AddSlot().AutoHeight().Padding(0, 6)
        [ SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("Scope", "Commits include the entire repository. Review .gitignore and configure Git LFS before adding Unreal binary assets. Pull/switch require a clean working tree.")) ];
        Fields->AddSlot().AutoHeight().Padding(0, 6)[SNew(SBox).MinDesiredHeight(180)[OutputBox()]];
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)
            [ SNew(SBorder).Padding(16)[ SNew(SScrollBox) + SScrollBox::Slot()[Fields] ] ];
    }
    void RequestDiff(bool bLatest)
    {
        if (bBusy || bExitDialogOpen || bRevisionControlConnecting) return;
        const FGitHubConfig Copy = Config;
        DiffOutput = TEXT("Loading diff…");
        bDiffOperation = true;
        Start([Copy, bLatest] { return GitHubGit::ReviewDiff(Copy, bLatest); });
    }
    TSharedRef<SDockTab> SpawnDiffTab(const FSpawnTabArgs&)
    {
        return SNew(SDockTab).TabRole(ETabRole::NomadTab)
        [ SNew(SBorder).Padding(12)
            [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                [ SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                    [ Button(TEXT("Pending changes vs last commit"), [this] { RequestDiff(false); return FReply::Handled(); }) ]
                    + SHorizontalBox::Slot().AutoWidth()
                    [ Button(TEXT("Latest commit vs previous commit"), [this] { RequestDiff(true); return FReply::Handled(); }) ] ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
                [ SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("DiffHelp", "Read-only review: '-' lines were removed; '+' lines were added. Pending view includes staged, unstaged and untracked files on disk. Binary assets are listed without a text patch; use Unreal's asset Diff against Depot for supported assets. Save assets and refresh to include their latest edits.")) ]
                + SVerticalBox::Slot().FillHeight(1)
                [ SNew(SMultiLineEditableTextBox).IsReadOnly(true).AutoWrapText(false).AlwaysShowScrollbars(true)
                    .Font(FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>(TEXT("MonospacedText")).Font)
                    .Text_Lambda([this] { return FText::FromString(DiffOutput); }) ] ] ];
    }
    bool CanClose()
    {
        if (FModuleManager::GetModuleChecked<IMainFrameModule>(TEXT("MainFrame")).IsRecreatingDefaultMainFrame()) return true;
        if (bBusy || bExitDialogOpen || bRevisionControlConnecting || !SubmitPushQueue.IsEmpty()) return false;
        if (!Config.bPromptOnExit) return true;
        TArray<UPackage*> DirtyPackages;
        FEditorFileUtils::GetDirtyPackages(DirtyPackages);
        const FGitHubExitState ExitState = GitHubGit::InspectExitState(Config);
        if (!ExitState.ShouldPrompt(DirtyPackages.Num())) return true;
        Output = FString::Printf(TEXT("Unsaved Unreal packages: %d\n"), DirtyPackages.Num()) + ExitState.Summary;
        bExitAllowed = false;
        bExitDialogOpen = true;
        TSharedRef<SWindow> Window = SNew(SWindow)
            .Title(LOCTEXT("ExitTitle", "GitHub — before closing Unreal"))
            .ClientSize(FVector2D(720, 460)).SupportsMinimize(false).SupportsMaximize(false);
        ExitWindow = Window;
        Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&)
        {
            // Closing the dialog during an operation cancels the exit, never the Git operation.
            if (bBusy) { bExitOperation = false; bExitAllowed = false; }
        }));
        Window->SetContent(SNew(SBorder).Padding(18)
        [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
            [ SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(
                TEXT("There are changes to save or push, or their status could not be verified.\nSave assets, commit all non-ignored repository changes, and push before closing?\nRepository: ") + Config.Root + TEXT("\nBranch: ") + Config.Branch + TEXT("\nPush failures keep Unreal open. Closing this dialog cancels exit."))) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)[MessageField()]
            + SVerticalBox::Slot().FillHeight(1)[OutputBox()]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
            [ SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ Button(TEXT("Save, commit + push, then close"), [this] { return Commit(true); }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ Button(TEXT("Close without pushing"), [this]
                    {
                        bExitAllowed = true;
                        if (auto W = ExitWindow.Pin()) W->RequestDestroyWindow();
                        return FReply::Handled();
                    }) ]
                + SHorizontalBox::Slot().AutoWidth()
                [ Button(TEXT("Cancel exit"), [this]
                    {
                        if (auto W = ExitWindow.Pin()) W->RequestDestroyWindow();
                        return FReply::Handled();
                    }) ] ] ]);
        auto& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
        FSlateApplication::Get().AddModalWindow(Window, MainFrame.GetParentWindow());
        bExitDialogOpen = false;
        ExitWindow.Reset();
        return bExitAllowed;
    }
    void RegisterMenus()
    {
        FToolMenuOwnerScoped Owner(this);
        UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Window"));
        Menu->FindOrAddSection(TEXT("WindowLayout")).AddMenuEntry(
            TEXT("OpenGitHubBridge"), LOCTEXT("Tab", "GitHub Bridge"),
            LOCTEXT("Tooltip", "Configure GitHub and synchronize this repository"), FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([this] { FGlobalTabmanager::Get()->TryInvokeTab(TabName); })));
        Menu->FindOrAddSection(TEXT("WindowLayout")).AddMenuEntry(
            TEXT("OpenGitHubBridgeDiff"), LOCTEXT("DiffMenu", "GitHub Bridge Diff"),
            LOCTEXT("DiffTooltip", "Review pending changes or compare the latest commit with its parent"), FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([this] { FGlobalTabmanager::Get()->TryInvokeTab(DiffTabName); RequestDiff(false); })));
    }
    void DetectProjectRepository()
    {
        if (bBusy || bRevisionControlConnecting) return;
        bDetectingProject = true;
        Config.Git = GitHubGit::FindProjectGit(FPaths::ProjectDir(), Config.Git);
        const FGitHubConfig Copy = Config;
        const FString Project = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
        Start([Copy, Project] { return GitHubGit::DetectRepository(Copy, Project); });
    }
public:
    virtual void StartupModule() override
    {
        if (IsRunningCommandlet()) return;
        Load();
        InitializeEditor();
    }
    void InitializeEditor()
    {
        UE_LOG(LogTemp, Display, TEXT("GitHubBridge: Initializing editor integration."));
        bEditorInitialized = true;
        SubmitBridge = MakeUnique<FGitHubSubmitBridge>(FSimpleDelegate::CreateRaw(this, &FGitHubBridgeModule::OnUnrealCommitted),
            [this] { return !bBusy && !bRevisionControlConnecting; });
        SubmitBridge->SetEnabled(Config.bPushAfterUnrealSubmit);
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName,
            FOnSpawnTab::CreateRaw(this, &FGitHubBridgeModule::SpawnTab))
            .SetDisplayName(LOCTEXT("TabName", "GitHub Bridge")).SetMenuType(ETabSpawnerMenuType::Hidden);
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(DiffTabName,
            FOnSpawnTab::CreateRaw(this, &FGitHubBridgeModule::SpawnDiffTab))
            .SetDisplayName(LOCTEXT("DiffTabName", "GitHub Bridge Diff")).SetMenuType(ETabSpawnerMenuType::Hidden);
        UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGitHubBridgeModule::RegisterMenus));
        auto& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
        CloseHandle = MainFrame.RegisterCanCloseEditor(IMainFrameModule::FMainFrameCanCloseEditor::CreateRaw(this, &FGitHubBridgeModule::CanClose));
        TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FGitHubBridgeModule::Tick));
        ModalTickHandle = FSlateApplication::Get().GetOnModalLoopTickEvent().AddLambda([this](float Delta) { Tick(Delta); });
        DetectProjectRepository();
    }
    virtual void ShutdownModule() override
    {
        if (IsRunningCommandlet()) return;
        CallbackLifetime.Reset();
        SubmitBridge.Reset();
        if (!bEditorInitialized) return;
        // A worker uses module code: do not unload until it finishes.
        if (Work.IsValid()) Work.Wait();
        FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
        if (FSlateApplication::IsInitialized()) FSlateApplication::Get().GetOnModalLoopTickEvent().Remove(ModalTickHandle);
        if (auto* MainFrame = FModuleManager::GetModulePtr<IMainFrameModule>(TEXT("MainFrame"))) MainFrame->UnregisterCanCloseEditor(CloseHandle);
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (auto Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabName)) Tab->RequestCloseTab();
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
        if (auto Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(DiffTabName)) Tab->RequestCloseTab();
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DiffTabName);
    }
    virtual bool SupportsDynamicReloading() override { return false; }
};

IMPLEMENT_MODULE(FGitHubBridgeModule, GitHubBridge)
#undef LOCTEXT_NAMESPACE
