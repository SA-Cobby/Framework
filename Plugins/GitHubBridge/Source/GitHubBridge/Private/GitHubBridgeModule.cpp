#include "GitHubGit.h"
#include "Async/Async.h"
#include "Containers/Ticker.h"
#include "FileHelpers.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Docking/TabManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
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

#define LOCTEXT_NAMESPACE "GitHubBridge"

class FGitHubBridgeModule : public IModuleInterface
{
    FGitHubConfig Config;
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
        GConfig->Flush(false, File);
    }
    bool Tick(float)
    {
        if (bBusy && Work.IsReady())
        {
            const FGitHubResult R = Work.Get();
            if (R.bSuccess && R.DetectedConfig.IsSet())
            {
                Config = R.DetectedConfig.GetValue();
                Save();
            }
            Output = (R.bSuccess ? TEXT("SUCCESS\n") : TEXT("FAILED\n")) + R.Output;
            bBusy = false;
            if (bExitOperation && R.bSuccess)
            {
                bExitAllowed = true;
                if (auto W = ExitWindow.Pin()) W->RequestDestroyWindow();
            }
            bExitOperation = false;
        }
        return true;
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
            .IsEnabled_Lambda([this] { return !bBusy; })
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
            .IsEnabled_Lambda([this] { return !bBusy && !bExitDialogOpen; })
            .IsChecked_Lambda([this] { return Config.bPromptOnExit ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { Config.bPromptOnExit = State == ECheckBoxState::Checked; Save(); })
            [ SNew(STextBlock).Text(LOCTEXT("Prompt", "Prompt to commit and push when exiting Unreal")) ] ];
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
    bool CanClose()
    {
        if (FModuleManager::GetModuleChecked<IMainFrameModule>(TEXT("MainFrame")).IsRecreatingDefaultMainFrame()) return true;
        if (bBusy || bExitDialogOpen) return false;
        if (!Config.bPromptOnExit) return true;
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
                TEXT("Save assets, commit all non-ignored repository changes, and push before closing?\nRepository: ") + Config.Root + TEXT("\nBranch: ") + Config.Branch + TEXT("\nPush failures keep Unreal open. Closing this dialog cancels exit."))) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)[MessageField()]
            + SVerticalBox::Slot().FillHeight(1)[OutputBox()]
            + SVerticalBox::Slot().AutoHeight().Padding(0, 12, 0, 0)
            [ SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
                [ Button(TEXT("Commit + push, then close"), [this] { return Commit(true); }) ]
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
    }
    void DetectProjectRepository()
    {
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
        FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName,
            FOnSpawnTab::CreateRaw(this, &FGitHubBridgeModule::SpawnTab))
            .SetDisplayName(LOCTEXT("TabName", "GitHub Bridge")).SetMenuType(ETabSpawnerMenuType::Hidden);
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
        // A worker uses module code: do not unload until it finishes.
        if (Work.IsValid()) Work.Wait();
        FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
        if (FSlateApplication::IsInitialized()) FSlateApplication::Get().GetOnModalLoopTickEvent().Remove(ModalTickHandle);
        if (auto* MainFrame = FModuleManager::GetModulePtr<IMainFrameModule>(TEXT("MainFrame"))) MainFrame->UnregisterCanCloseEditor(CloseHandle);
        UToolMenus::UnRegisterStartupCallback(this);
        UToolMenus::UnregisterOwner(this);
        if (auto Tab = FGlobalTabmanager::Get()->FindExistingLiveTab(TabName)) Tab->RequestCloseTab();
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
    }
    virtual bool SupportsDynamicReloading() override { return false; }
};

IMPLEMENT_MODULE(FGitHubBridgeModule, GitHubBridge)
#undef LOCTEXT_NAMESPACE
