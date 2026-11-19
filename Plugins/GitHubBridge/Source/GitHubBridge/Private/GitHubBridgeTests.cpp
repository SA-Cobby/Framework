#include "GitHubGit.h"
#include "GitHubRevisionControl.h"
#include "GitHubSubmitBridge.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "SourceControlHelpers.h"
#include "SourceControlOperations.h"
#include "Misc/ScopeExit.h"
#include "FileHelpers.h"
#include "UObject/Package.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGitHubBridgeRepositoryTest,
    "GitHubBridge.RepositoryGuardsAndProcessArguments",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGitHubBridgeRepositoryTest::RunTest(const FString&)
{
    FGitHubConfig C;
    C.Root = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() /
        TEXT("Automation/GitHubBridge") / FGuid::NewGuid().ToString() / TEXT("Repo with spaces"));
    C.Remote = TEXT("https://github.com/example/fixture.git");
    C.Branch = TEXT("main");
    C.Name = TEXT("Test Author");
    C.Email = TEXT("test@example.invalid");
    IFileManager::Get().MakeDirectory(*C.Root, true);
    auto Check = [this](const TCHAR* Label, const FGitHubResult& R)
    {
        if (!R.bSuccess) AddError(FString(Label) + TEXT(": ") + R.Output);
        return R.bSuccess;
    };
    if (!Check(TEXT("Initialize"), GitHubGit::Run(C, { TEXT("init"), TEXT("-b"), TEXT("main") }))) return false;
    if (!Check(TEXT("Configure"), GitHubGit::Configure(C))) return false;
    const FString NestedProject = C.Root / TEXT("Games/NestedProject");
    IFileManager::Get().MakeDirectory(*NestedProject, true);
    FGitHubConfig Stale;
    Stale.Root = TEXT("Z:/Missing old checkout");
    Stale.Remote = TEXT("https://github.com/old/repository.git");
    Stale.Branch = TEXT("old-branch");
    Stale.Username = TEXT("retained-account");
    auto Detection = GitHubGit::DetectRepository(Stale, NestedProject);
    if (!Check(TEXT("Detect ancestor repository"), Detection) || !Detection.DetectedConfig.IsSet()) return false;
    TestTrue(TEXT("Detected root is ancestor checkout"), FPaths::IsSamePath(Detection.DetectedConfig->Root, C.Root));
    TestEqual(TEXT("Origin imported"), Detection.DetectedConfig->Remote, C.Remote);
    TestEqual(TEXT("Unborn branch imported"), Detection.DetectedConfig->Branch, C.Branch);
    TestEqual(TEXT("Author imported"), Detection.DetectedConfig->Name, C.Name);
    TestEqual(TEXT("Email imported"), Detection.DetectedConfig->Email, C.Email);
    TestEqual(TEXT("Account preserved"), Detection.DetectedConfig->Username, Stale.Username);
    TestTrue(TEXT("Unborn branch validates"), GitHubGit::Validate(C).bSuccess);
    const FString Asset = C.Root / TEXT("Example asset.txt");
    FFileHelper::SaveStringToFile(TEXT("first revision"), *Asset);
    TestFalse(TEXT("Dirty pull refused before networking"), GitHubGit::Pull(C).bSuccess);
    TestFalse(TEXT("Dirty switch refused before networking"), GitHubGit::SwitchBranch(C).bSuccess);
    TestFalse(TEXT("Empty commit message refused"), GitHubGit::CommitPush(C, TEXT("  ")).bSuccess);
    Check(TEXT("Stage"), GitHubGit::Run(C, { TEXT("add"), TEXT("--all") }));
    const FString Message = TEXT("Quoted \"message\" & literal $() and trailing slash\\");
    Check(TEXT("Commit argument quoting"), GitHubGit::Run(C, { TEXT("commit"), TEXT("-m"), Message }));
    auto Log = GitHubGit::Run(C, { TEXT("log"), TEXT("-1"), TEXT("--format=%B") });
    TestEqual(TEXT("Exact message survived process argv"), Log.Output.TrimStartAndEnd(), Message);
    TestTrue(TEXT("Clean status"), GitHubGit::Status(C).bSuccess);
    const FString Worktree = FPaths::GetPath(C.Root) / TEXT("Linked worktree");
    if (Check(TEXT("Create linked worktree"), GitHubGit::Run(C, { TEXT("worktree"), TEXT("add"), TEXT("-b"), TEXT("linked"), Worktree })))
    {
        auto Linked = GitHubGit::DetectRepository(Stale, Worktree);
        TestTrue(TEXT("Git file worktree detected"), Linked.bSuccess && Linked.DetectedConfig.IsSet());
        if (Linked.DetectedConfig.IsSet())
        {
            TestTrue(TEXT("Worktree root detected"), FPaths::IsSamePath(Linked.DetectedConfig->Root, Worktree));
            TestEqual(TEXT("Worktree branch imported"), Linked.DetectedConfig->Branch, FString(TEXT("linked")));
        }
    }
    Check(TEXT("Detach checkout"), GitHubGit::Run(C, { TEXT("switch"), TEXT("--detach") }));
    auto Detached = GitHubGit::DetectRepository(C, NestedProject);
    TestTrue(TEXT("Detached checkout still detected"), Detached.bSuccess && Detached.DetectedConfig.IsSet());
    if (Detached.DetectedConfig.IsSet()) TestTrue(TEXT("Detached branch is empty"), Detached.DetectedConfig->Branch.IsEmpty());
    Check(TEXT("Reattach checkout"), GitHubGit::Run(C, { TEXT("switch"), TEXT("main") }));
    C.Branch = TEXT("other");
    TestFalse(TEXT("Branch mismatch refused"), GitHubGit::Validate(C).bSuccess);
    C.Branch = TEXT("main");
    GitHubGit::Run(C, { TEXT("config"), TEXT("remote.origin.pushurl"), TEXT("https://github.com/other/repo.git") });
    TestFalse(TEXT("Alternate push destination refused"), GitHubGit::Validate(C).bSuccess);
    GitHubGit::Run(C, { TEXT("config"), TEXT("--unset-all"), TEXT("remote.origin.pushurl") });
    C.Remote = TEXT("https://token@github.com/example/fixture.git");
    TestFalse(TEXT("Credential-bearing URL refused"), GitHubGit::Configure(C).bSuccess);
    GitHubGit::Run(C, { TEXT("remote"), TEXT("set-url"), TEXT("origin"), C.Remote });
    auto CredentialDetection = GitHubGit::DetectRepository(C, NestedProject);
    TestTrue(TEXT("Checkout with unsupported origin still detected"), CredentialDetection.bSuccess);
    if (CredentialDetection.DetectedConfig.IsSet())
        TestTrue(TEXT("Credential-bearing origin not imported"), CredentialDetection.DetectedConfig->Remote.IsEmpty());
    C.Remote = TEXT("https://github.com/example/fixture.git");
    GitHubGit::Run(C, { TEXT("remote"), TEXT("set-url"), TEXT("origin"), C.Remote });
    GitHubGit::Run(C, { TEXT("remote"), TEXT("remove"), TEXT("origin") });
    auto MissingOrigin = GitHubGit::DetectRepository(C, NestedProject);
    TestTrue(TEXT("Checkout without origin detected"), MissingOrigin.bSuccess);
    if (MissingOrigin.DetectedConfig.IsSet()) TestTrue(TEXT("Missing origin clears stale URL"), MissingOrigin.DetectedConfig->Remote.IsEmpty());
    GitHubGit::Run(C, { TEXT("remote"), TEXT("add"), TEXT("origin"), C.Remote });
    C.Branch = TEXT("-invalid");
    TestFalse(TEXT("Option-like branch refused"), GitHubGit::Validate(C).bSuccess);
    C.Branch = TEXT("main");
    const FString Root = C.Root;
    C.Root = Root / TEXT("Child");
    IFileManager::Get().MakeDirectory(*C.Root, true);
    TestFalse(TEXT("Subdirectory root refused"), GitHubGit::Validate(C).bSuccess);
    FFileHelper::SaveStringToFile(TEXT("gitdir: missing-worktree-metadata"), *(C.Root / TEXT(".git")));
    auto Broken = GitHubGit::DetectRepository(C, C.Root);
    TestFalse(TEXT("Invalid git marker refused"), Broken.bSuccess);
    TestFalse(TEXT("Failed detection cannot replace config"), Broken.DetectedConfig.IsSet());
    AddInfo(TEXT("Fixture retained for inspection: ") + Root);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGitHubBridgeProjectGitTest,
    "GitHubBridge.ProjectLocalGitDiscovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGitHubBridgeProjectGitTest::RunTest(const FString&)
{
    const FString Project = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() /
        TEXT("Automation/GitHubBridge") / FGuid::NewGuid().ToString() / TEXT("Game with spaces"));
    IFileManager::Get().MakeDirectory(*Project, true);
    const FString Fallback = TEXT("C:/Custom Git/cmd/git.exe");
    TestEqual(TEXT("No local Git preserves configured executable"), GitHubGit::FindProjectGit(Project, Fallback), Fallback);
    // Empty markers exercise location selection only; no untrusted executables are run.
    const TCHAR* Layouts[] = { TEXT("mingw64/bin/git.exe"), TEXT("git.exe"), TEXT("bin/git.exe"), TEXT("cmd/git.exe") };
    for (const TCHAR* Layout : Layouts)
    {
        const FString Candidate = Project / TEXT("git") / Layout;
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Candidate), true);
        FFileHelper::SaveStringToFile(TEXT("location-only test fixture"), *Candidate);
        TestEqual(FString(TEXT("Project Git selected: ")) + Layout,
            GitHubGit::FindProjectGit(Project, Fallback), Candidate);
    }
    TestEqual(TEXT("cmd launcher preferred when multiple layouts exist"), GitHubGit::FindProjectGit(Project, Fallback),
        Project / TEXT("git/cmd/git.exe"));
    const FString EmptyProject = Project / TEXT("AnotherProject");
    IFileManager::Get().MakeDirectory(*(EmptyProject / TEXT("git")), true);
    TestEqual(TEXT("Empty git folder preserves configured executable"), GitHubGit::FindProjectGit(EmptyProject, Fallback), Fallback);
    AddInfo(TEXT("Location fixtures retained: ") + Project);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGitHubBridgeRevisionControlTest,
    "GitHubBridge.UnrealGitProviderIntegration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGitHubBridgeRevisionControlTest::RunTest(const FString&)
{
    ISourceControlModule& Module = ISourceControlModule::Get();
    const FName OriginalProvider = Module.GetProvider().GetName();
    bool bAutoConnect = true;
    GConfig->GetBool(TEXT("GitHubBridge"), TEXT("AutoConnectRevisionControl"), bAutoConnect,
        FPaths::ProjectSavedDir() / TEXT("Config/GitHubBridge.ini"));
    if (bAutoConnect) TestEqual(TEXT("Startup automation selected Git"), OriginalProvider, FName(TEXT("Git")));
    const FString Ini = SourceControlHelpers::GetSettingsIni();
    FString OriginalPath;
    GConfig->GetString(TEXT("GitSourceControl.GitSourceControlSettings"), TEXT("BinaryPath"), OriginalPath, Ini);
    FString Status;
    TestFalse(TEXT("Disabled setting does not select a provider"), GitHubRevisionControl::SelectGitProvider(false, Status));
    TestEqual(TEXT("Disabled setting leaves provider unchanged"), Module.GetProvider().GetName(), OriginalProvider);
    TestFalse(TEXT("Disabled setting does not prepare a Git path"), GitHubRevisionControl::PrepareGitPath(false, TEXT("Z:/MustNotWrite/git.exe")));
    FString PathAfterDisabled;
    GConfig->GetString(TEXT("GitSourceControl.GitSourceControlSettings"), TEXT("BinaryPath"), PathAfterDisabled, Ini);
    TestEqual(TEXT("Disabled setting leaves provider binary setting unchanged"), PathAfterDisabled, OriginalPath);
    FGitHubConfig Project;
    Project.Root = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    auto Repo = GitHubGit::Run(Project, { TEXT("rev-parse"), TEXT("--show-toplevel") });
    if (!Repo.bSuccess)
    {
        AddError(TEXT("Run this integration test from an isolated Git-backed test project."));
        return false;
    }
    const bool bSelected = GitHubRevisionControl::SelectGitProvider(true, Status);
    TestTrue(*Status, bSelected);
    if (bSelected)
    {
        TestEqual(TEXT("Built-in Git provider selected"), Module.GetProvider().GetName(), FName(TEXT("Git")));
        TestTrue(TEXT("Provider opened the project repository"), Module.GetProvider().IsAvailable());
        const auto ProviderStatus = Module.GetProvider().GetStatus();
        const FString* Root = ProviderStatus.Find(ISourceControlProvider::EStatus::Repository);
        TestTrue(TEXT("Provider uses the same project checkout"), Root && FPaths::IsSamePath(*Root, Repo.Output.TrimStartAndEnd()));
        TestEqual(TEXT("Built-in Git Connect operation succeeds"),
            Module.GetProvider().Execute(ISourceControlOperation::Create<FConnect>(), EConcurrency::Synchronous), ECommandResult::Succeeded);
        TestFalse(TEXT("Turning automation off does not disconnect Git"), GitHubRevisionControl::SelectGitProvider(false, Status));
        TestEqual(TEXT("Active provider retained after opt-out"), Module.GetProvider().GetName(), FName(TEXT("Git")));
    }
    Module.SetProvider(OriginalProvider);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGitHubBridgeNativeSubmitTest,
    "GitHubBridge.NativeSubmitSelectedFiles",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGitHubBridgeNativeSubmitTest::RunTest(const FString&)
{
    auto& Module = ISourceControlModule::Get();
    const FName OriginalProvider = Module.GetProvider().GetName();
    auto& Windows = ISourceControlWindowsModule::Get();
    const auto PreviousOverride = Windows.SubmitOverrideDelegate;
    Windows.SubmitOverrideDelegate.Unbind();
    ON_SCOPE_EXIT { Windows.SubmitOverrideDelegate = PreviousOverride; Module.SetProvider(OriginalProvider); };
    FString Status;
    if (!GitHubRevisionControl::SelectGitProvider(true, Status)) { AddError(Status); return false; }
    auto& Provider = Module.GetProvider();
    int32 Requests = 0;
    bool bCanSubmit = true;
    FGitHubSubmitBridge Bridge(FSimpleDelegate::CreateLambda([&Requests] { ++Requests; }), [&bCanSubmit] { return bCanSubmit; });
    Bridge.SetEnabled(true);
    const FString Id = FGuid::NewGuid().ToString();
    const FString Selected = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / (TEXT("NativeSubmit-") + Id + TEXT(".txt")));
    const FString Unselected = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / (TEXT("Unselected-") + Id + TEXT(".txt")));
    FFileHelper::SaveStringToFile(TEXT("selected change"), *Selected);
    FFileHelper::SaveStringToFile(TEXT("unselected change"), *Unselected);
    SSubmitOverrideParameters Parameters;
    Parameters.Description = TEXT("Native Git submit fixture ") + Id;
    Parameters.ToSubmit.SetSubtype<TArray<FString>>({ Selected });
    TestEqual(TEXT("Mark selected file for add"), Provider.Execute(ISourceControlOperation::Create<FMarkForAdd>(), TArray<FString>{Selected}), ECommandResult::Succeeded);
    Provider.Close();
    TestEqual(TEXT("Failed native check-in does not claim success"), Bridge.Submit(Parameters), FSubmitOverrideReply::Error);
    TestEqual(TEXT("Failed native check-in queues no push"), Requests, 0);
    Provider.Init(false);
    TestEqual(TEXT("Native dialog override commits"), Windows.SubmitOverrideDelegate.Execute(Parameters), FSubmitOverrideReply::Handled);
    TestEqual(TEXT("Successful native check-in queues exactly one push"), Requests, 1);
    FGitHubConfig Project;
    Project.Root = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    auto Description = GitHubGit::Run(Project, { TEXT("log"), TEXT("-1"), TEXT("--format=%B") });
    TestTrue(TEXT("Native submit preserves description"), Description.bSuccess && Description.Output.Contains(Parameters.Description));
    auto Tree = GitHubGit::Run(Project, { TEXT("ls-tree"), TEXT("--name-only"), TEXT("HEAD"), TEXT("--"), Unselected });
    TestTrue(TEXT("Unselected file was not committed"), Tree.bSuccess && Tree.Output.TrimStartAndEnd().IsEmpty());
    Bridge.SetEnabled(false);
    TestEqual(TEXT("Opt-out falls through to native behavior"), Bridge.Submit(Parameters), FSubmitOverrideReply::ProviderNotSupported);
    TestEqual(TEXT("Opt-out queues no extra push"), Requests, 1);
    Windows.SubmitOverrideDelegate.BindLambda([](SSubmitOverrideParameters) { return FSubmitOverrideReply::ProviderNotSupported; });
    Bridge.SetEnabled(true);
    TestFalse(TEXT("Other extension's override is not replaced"), Windows.SubmitOverrideDelegate.IsBoundToObject(&Bridge));
    TestEqual(TEXT("Conflict queues no extra push"), Requests, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGitHubBridgePushOnlyTest,
    "GitHubBridge.PushOnlyPreservesUncommittedFiles",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGitHubBridgePushOnlyTest::RunTest(const FString&)
{
    const FString Fixture = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Automation/GitHubBridge") /
        FGuid::NewGuid().ToString() / TEXT("Push fixture with spaces"));
    const FString RemoteRoot = Fixture / TEXT("Remote.git");
    FGitHubConfig C;
    C.Root = Fixture / TEXT("Local");
    C.Remote = TEXT("git@github.com:fixture/local-only.git");
    C.Name = TEXT("Fixture Author"); C.Email = TEXT("fixture@example.invalid");
    IFileManager::Get().MakeDirectory(*C.Root, true);
    auto Require = [this](const TCHAR* Label, const FGitHubResult& R) { if (!R.bSuccess) AddError(FString(Label) + TEXT(": ") + R.Output); return R.bSuccess; };
    if (!Require(TEXT("Initialize local"), GitHubGit::Run(C, {TEXT("init"), TEXT("-b"), TEXT("main")}))) return false;
    if (!Require(TEXT("Initialize local bare remote"), GitHubGit::Run(C, {TEXT("init"), TEXT("--bare"), RemoteRoot}))) return false;
    if (!Require(TEXT("Configure fixture"), GitHubGit::Configure(C))) return false;
    auto EmptyExit = GitHubGit::InspectExitState(C);
    TestTrue(TEXT("Empty unborn repository can close without a push prompt"), EmptyExit.bKnown && !EmptyExit.ShouldPrompt(0));
    // SSH is replaced with a local receive-pack process. No GitHub connection or
    // credentials are used, while validation still exercises a supported clone URL.
    const FString Script = Fixture / TEXT("local-ssh.sh");
    FFileHelper::SaveStringToFile(TEXT("#!/bin/sh\ncase \"$*\" in\n*git-upload-pack*) exec git upload-pack '") + RemoteRoot +
        TEXT("';;\n*git-receive-pack*) exec git receive-pack '") + RemoteRoot + TEXT("';;\n*) exit 1;;\nesac\n"),
        *Script, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const FString Shell = FPaths::GetPath(FPaths::GetPath(C.Git)) / TEXT("bin/sh.exe");
    if (!Require(TEXT("Local SSH transport"), GitHubGit::Run(C, { TEXT("config"), TEXT("core.sshCommand"), TEXT("\"") + Shell + TEXT("\" \"") + Script + TEXT("\"") }))) return false;
    GitHubGit::Run(C, {TEXT("config"), TEXT("ssh.variant"), TEXT("simple")});
    const FString Selected = C.Root / TEXT("Selected.txt");
    const FString Unselected = C.Root / TEXT("Not committed.txt");
    FFileHelper::SaveStringToFile(TEXT("committed"), *Selected);
    GitHubGit::Run(C, {TEXT("add"), TEXT("--"), Selected});
    if (!Require(TEXT("Create commit"), GitHubGit::Run(C, {TEXT("commit"), TEXT("-m"), TEXT("Push fixture")}))) return false;
    auto BeforePushExit = GitHubGit::InspectExitState(C);
    TestTrue(TEXT("Committed but unpublished branch requires exit prompt"), BeforePushExit.bKnown && BeforePushExit.UnpushedCommits > 0 && BeforePushExit.ShouldPrompt(0));
    const auto Before = GitHubGit::Run(C, {TEXT("rev-parse"), TEXT("HEAD")});
    FFileHelper::SaveStringToFile(TEXT("keep this local"), *Unselected);
    if (!Require(TEXT("Push existing commit"), GitHubGit::PushCommitted(C, Before.Output.TrimStartAndEnd()))) return false;
    FGitHubConfig Remote = C; Remote.Root = RemoteRoot;
    const auto RemoteCommit = GitHubGit::Run(Remote, {TEXT("rev-parse"), TEXT("refs/heads/main")});
    TestEqual(TEXT("Exact submitted commit reached remote"), RemoteCommit.Output.TrimStartAndEnd(), Before.Output.TrimStartAndEnd());
    auto Untracked = GitHubGit::Run(C, {TEXT("ls-files"), TEXT("--others"), TEXT("--exclude-standard")});
    TestTrue(TEXT("Push did not stage unrelated file"), Untracked.Output.Contains(TEXT("Not committed.txt")));
    auto DirtyExit = GitHubGit::InspectExitState(C);
    TestTrue(TEXT("Untracked files still require exit prompt after push"), DirtyExit.bKnown && DirtyExit.bWorkingTreeDirty && DirtyExit.ShouldPrompt(0));
    IFileManager::Get().Delete(*Unselected);
    auto SyncedExit = GitHubGit::InspectExitState(C);
    TestTrue(TEXT("Successful push plus clean tree skips exit prompt"), SyncedExit.bKnown && !SyncedExit.ShouldPrompt(0));
    FFileHelper::SaveStringToFile(TEXT("tracked edit"), *Selected);
    TestTrue(TEXT("Tracked edits require exit prompt"), GitHubGit::InspectExitState(C).ShouldPrompt(0));
    GitHubGit::Run(C, {TEXT("add"), TEXT("--"), Selected});
    TestTrue(TEXT("Staged edits require exit prompt"), GitHubGit::InspectExitState(C).ShouldPrompt(0));
    GitHubGit::Run(C, {TEXT("restore"), TEXT("--staged"), TEXT("--"), Selected});
    GitHubGit::Run(C, {TEXT("restore"), TEXT("--"), Selected});
    FFileHelper::SaveStringToFile(TEXT("keep this local"), *Unselected);
    const FString OtherRoot = Fixture / TEXT("Other");
    if (!Require(TEXT("Clone competing checkout"), GitHubGit::Run(C, {TEXT("clone"), TEXT("-b"), TEXT("main"), RemoteRoot, OtherRoot}))) return false;
    FGitHubConfig Other = C; Other.Root = OtherRoot;
    GitHubGit::Run(Other, {TEXT("config"), TEXT("user.name"), C.Name});
    GitHubGit::Run(Other, {TEXT("config"), TEXT("user.email"), C.Email});
    FFileHelper::SaveStringToFile(TEXT("remote advance"), *(OtherRoot / TEXT("Other.txt")));
    GitHubGit::Run(Other, {TEXT("add"), TEXT("Other.txt")});
    if (!Require(TEXT("Competing commit"), GitHubGit::Run(Other, {TEXT("commit"), TEXT("-m"), TEXT("Remote advance")}))) return false;
    if (!Require(TEXT("Advance local remote"), GitHubGit::Run(Other, {TEXT("push"), TEXT("origin"), TEXT("HEAD:main")}))) return false;
    // Bring only the local remote-tracking ref up to date, without changing HEAD.
    if (!Require(TEXT("Update fixture tracking ref"), GitHubGit::Run(C, {TEXT("fetch"), RemoteRoot, TEXT("refs/heads/main:refs/remotes/origin/main")}))) return false;
    IFileManager::Get().Delete(*Unselected);
    auto BehindExit = GitHubGit::InspectExitState(C);
    TestTrue(TEXT("Behind-only clean branch has nothing to push"), BehindExit.bKnown && !BehindExit.ShouldPrompt(0));
    FFileHelper::SaveStringToFile(TEXT("local divergence"), *Selected);
    GitHubGit::Run(C, {TEXT("add"), TEXT("--"), Selected});
    if (!Require(TEXT("Create divergent local commit"), GitHubGit::Run(C, {TEXT("commit"), TEXT("-m"), TEXT("Local divergence")}))) return false;
    auto DivergedExit = GitHubGit::InspectExitState(C);
    TestTrue(TEXT("Diverged branch with local commits requires exit prompt"), DivergedExit.bKnown && DivergedExit.UnpushedCommits > 0 && DivergedExit.ShouldPrompt(0));
    const auto LocalBeforeRebase = GitHubGit::Run(C, {TEXT("rev-parse"), TEXT("HEAD")});
    FFileHelper::SaveStringToFile(TEXT("temporary tracked edit"), *Selected);
    TestFalse(TEXT("Dirty tracked changes block a required rebase"), GitHubGit::PushCommitted(C).bSuccess);
    FString PreservedEdit; FFileHelper::LoadFileToString(PreservedEdit, *Selected);
    TestEqual(TEXT("Blocked rebase preserves tracked edit"), PreservedEdit, FString(TEXT("temporary tracked edit")));
    GitHubGit::Run(C, {TEXT("restore"), TEXT("--"), Selected});
    FFileHelper::SaveStringToFile(TEXT("keep this local"), *Unselected);
    if (!Require(TEXT("Rebase divergent local commit and push"), GitHubGit::PushCommitted(C))) return false;
    const auto Rebased = GitHubGit::Run(C, {TEXT("rev-parse"), TEXT("HEAD")});
    TestNotEqual(TEXT("Rebase rewrites the local commit"), Rebased.Output.TrimStartAndEnd(), LocalBeforeRebase.Output.TrimStartAndEnd());
    const auto RemoteAfterRebase = GitHubGit::Run(Remote, {TEXT("rev-parse"), TEXT("refs/heads/main")});
    TestEqual(TEXT("Rebased tip reached remote"), RemoteAfterRebase.Output.TrimStartAndEnd(), Rebased.Output.TrimStartAndEnd());
    TestTrue(TEXT("Fetched remote changes exist locally"), IFileManager::Get().FileExists(*(C.Root / TEXT("Other.txt"))));
    FString Remaining; FFileHelper::LoadFileToString(Remaining, *Unselected);
    TestEqual(TEXT("Rebase preserves unrelated untracked edits"), Remaining, FString(TEXT("keep this local")));
    const auto Backups = GitHubGit::Run(C, {TEXT("for-each-ref"), TEXT("--format=%(objectname)"), TEXT("refs/github-bridge/pre-rebase/")});
    TestTrue(TEXT("Original commit is protected by a backup ref"), Backups.Output.Contains(LocalBeforeRebase.Output.TrimStartAndEnd()));
    const FString Hooks = Fixture / TEXT("RejectHooks");
    IFileManager::Get().MakeDirectory(*Hooks, true);
    FFileHelper::SaveStringToFile(TEXT("#!/bin/sh\nexit 1\n"), *(Hooks / TEXT("pre-receive")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    GitHubGit::Run(Remote, {TEXT("config"), TEXT("core.hooksPath"), Hooks});
    FFileHelper::SaveStringToFile(TEXT("local next"), *Selected);
    GitHubGit::Run(C, {TEXT("add"), TEXT("--"), Selected});
    if (!Require(TEXT("New local commit"), GitHubGit::Run(C, {TEXT("commit"), TEXT("-m"), TEXT("Local next")}))) return false;
    const auto BeforeRejected = GitHubGit::Run(C, {TEXT("rev-parse"), TEXT("HEAD")});
    TestFalse(TEXT("Remote hook rejection is reported"), GitHubGit::PushCommitted(C).bSuccess);
    const auto AfterRejected = GitHubGit::Run(C, {TEXT("rev-parse"), TEXT("HEAD")});
    TestEqual(TEXT("Rejected push retains the local tip"), AfterRejected.Output.TrimStartAndEnd(), BeforeRejected.Output.TrimStartAndEnd());
    GitHubGit::Run(Remote, {TEXT("config"), TEXT("--unset"), TEXT("core.hooksPath")});
    if (!Require(TEXT("Update competing checkout"), GitHubGit::Run(Other, {TEXT("pull"), TEXT("--ff-only"), TEXT("origin"), TEXT("main")}))) return false;
    FFileHelper::SaveStringToFile(TEXT("remote conflicting edit"), *(OtherRoot / TEXT("Selected.txt")));
    GitHubGit::Run(Other, {TEXT("add"), TEXT("Selected.txt")});
    if (!Require(TEXT("Commit remote conflict"), GitHubGit::Run(Other, {TEXT("commit"), TEXT("-m"), TEXT("Remote conflict")}))) return false;
    if (!Require(TEXT("Push remote conflict"), GitHubGit::Run(Other, {TEXT("push"), TEXT("origin"), TEXT("HEAD:main")}))) return false;
    const auto ConflictRemote = GitHubGit::Run(Remote, {TEXT("rev-parse"), TEXT("refs/heads/main")});
    const auto Conflict = GitHubGit::PushCommitted(C);
    TestFalse(TEXT("Conflicting rebase stops push"), Conflict.bSuccess);
    const auto ConflictedFiles = GitHubGit::Run(C, {TEXT("diff"), TEXT("--name-only"), TEXT("--diff-filter=U")});
    TestTrue(TEXT("Conflict remains available to resolve"), ConflictedFiles.Output.Contains(TEXT("Selected.txt")));
    const auto RemoteAfterConflict = GitHubGit::Run(Remote, {TEXT("rev-parse"), TEXT("refs/heads/main")});
    TestEqual(TEXT("Conflicted rebase never changes the remote"), RemoteAfterConflict.Output.TrimStartAndEnd(), ConflictRemote.Output.TrimStartAndEnd());
    TestFalse(TEXT("Commit/push refuses an ongoing rebase"), GitHubGit::CommitPush(C, TEXT("Must not commit")).bSuccess);
    if (!Require(TEXT("Abort fixture rebase"), GitHubGit::Run(C, {TEXT("rebase"), TEXT("--abort")}))) return false;
    const auto Aborted = GitHubGit::Run(C, {TEXT("rev-parse"), TEXT("HEAD")});
    TestEqual(TEXT("Abort restores original local tip"), Aborted.Output.TrimStartAndEnd(), BeforeRejected.Output.TrimStartAndEnd());
    C.Branch = TEXT("wrong-branch");
    auto UnknownExit = GitHubGit::InspectExitState(C);
    TestTrue(TEXT("Unverified configuration cannot silently skip prompt"), !UnknownExit.bKnown && UnknownExit.ShouldPrompt(0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGitHubBridgeUnsavedExitTest,
    "GitHubBridge.UnsavedAssetsRequireExitPrompt",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGitHubBridgeUnsavedExitTest::RunTest(const FString&)
{
    FGitHubExitState Synced;
    Synced.bKnown = true;
    TestFalse(TEXT("Synced state without unsaved assets skips prompt"), Synced.ShouldPrompt(0));
    UPackage* Package = CreatePackage(*(TEXT("/Game/GitHubBridgeExit_") + FGuid::NewGuid().ToString()));
    ON_SCOPE_EXIT { Package->SetDirtyFlag(false); };
    Package->SetDirtyFlag(true);
    TArray<UPackage*> Dirty;
    FEditorFileUtils::GetDirtyPackages(Dirty);
    TestTrue(TEXT("Unreal detects the dirty project package"), Dirty.Contains(Package));
    TestTrue(TEXT("Unsaved package requires prompt even when Git is synced"), Synced.ShouldPrompt(Dirty.Num()));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGitHubBridgeDiffTest,
    "GitHubBridge.ReadOnlyCommitDiff",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGitHubBridgeDiffTest::RunTest(const FString&)
{
    FGitHubConfig C;
    C.Root = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Automation/GitHubBridge") /
        FGuid::NewGuid().ToString() / TEXT("Diff fixture with spaces"));
    IFileManager::Get().MakeDirectory(*C.Root, true);
    auto Require = [this](const TCHAR* Label, const FGitHubResult& R) { if (!R.bSuccess) AddError(FString(Label) + TEXT(": ") + R.Output); return R.bSuccess; };
    if (!Require(TEXT("Initialize diff fixture"), GitHubGit::Run(C, {TEXT("init"), TEXT("-b"), TEXT("main")}))) return false;
    GitHubGit::Run(C, {TEXT("config"), TEXT("user.name"), TEXT("Diff Fixture")});
    GitHubGit::Run(C, {TEXT("config"), TEXT("user.email"), TEXT("fixture@example.invalid")});
    const FString A = C.Root / TEXT("Changed file.txt");
    const FString Deleted = C.Root / TEXT("Deleted.txt");
    FFileHelper::SaveStringToFile(TEXT("before\n"), *A);
    auto Unborn = GitHubGit::ReviewDiff(C);
    TestTrue(TEXT("Unborn repository previews new files"), Unborn.bSuccess && Unborn.Output.Contains(TEXT("+before")));
    TestFalse(TEXT("Latest commit view explains absent history"), GitHubGit::ReviewDiff(C, true).bSuccess);
    FFileHelper::SaveStringToFile(TEXT("removed\n"), *Deleted);
    FFileHelper::SaveStringToFile(TEXT("Ignored.txt\n"), *(C.Root / TEXT(".gitignore")));
    GitHubGit::Run(C, {TEXT("add"), TEXT("--all")});
    if (!Require(TEXT("Initial diff commit"), GitHubGit::Run(C, {TEXT("commit"), TEXT("-m"), TEXT("Baseline")}))) return false;
    auto Initial = GitHubGit::ReviewDiff(C, true);
    TestTrue(TEXT("Initial commit compares against empty tree"), Initial.bSuccess && Initial.Output.Contains(TEXT("EMPTY TREE")) && Initial.Output.Contains(TEXT("+before")));
    GitHubGit::Run(C, {TEXT("config"), TEXT("diff.external"), TEXT("Z:/MustNotRunDiff.exe")});
    FFileHelper::SaveStringToFile(TEXT("after\n"), *A);
    IFileManager::Get().Delete(*Deleted);
    FFileHelper::SaveStringToFile(TEXT("staged\n"), *(C.Root / TEXT("Staged.txt")));
    GitHubGit::Run(C, {TEXT("add"), TEXT("Staged.txt")});
    FFileHelper::SaveStringToFile(TEXT("untracked\n"), *(C.Root / TEXT("New file.txt")));
    FFileHelper::SaveStringToFile(TEXT("ignored\n"), *(C.Root / TEXT("Ignored.txt")));
    TArray<uint8> Bytes { 0, 1, 2, 0, 3 };
    FFileHelper::SaveArrayToFile(Bytes, *(C.Root / TEXT("Binary.uasset")));
    const auto BeforeStatus = GitHubGit::Run(C, {TEXT("status"), TEXT("--porcelain")});
    const auto BeforeIndex = GitHubGit::Run(C, {TEXT("diff"), TEXT("--cached"), TEXT("--no-ext-diff")});
    auto Pending = GitHubGit::ReviewDiff(C);
    if (!Require(TEXT("Pending diff"), Pending)) return false;
    TestTrue(TEXT("Working-tree modification shown"), Pending.Output.Contains(TEXT("-before")) && Pending.Output.Contains(TEXT("+after")));
    TestTrue(TEXT("Staged addition shown"), Pending.Output.Contains(TEXT("+staged")));
    TestTrue(TEXT("Untracked addition shown"), Pending.Output.Contains(TEXT("+untracked")));
    TestTrue(TEXT("Deletion shown"), Pending.Output.Contains(TEXT("-removed")));
    TestTrue(TEXT("Binary change identified"), Pending.Output.Contains(TEXT("Binary files")));
    TestFalse(TEXT("Ignored untracked file excluded"), Pending.Output.Contains(TEXT("UNTRACKED: Ignored.txt")));
    const auto AfterStatus = GitHubGit::Run(C, {TEXT("status"), TEXT("--porcelain")});
    const auto AfterIndex = GitHubGit::Run(C, {TEXT("diff"), TEXT("--cached"), TEXT("--no-ext-diff")});
    TestEqual(TEXT("Review does not change repository status"), AfterStatus.Output, BeforeStatus.Output);
    TestEqual(TEXT("Review does not change staging"), AfterIndex.Output, BeforeIndex.Output);
    GitHubGit::Run(C, {TEXT("add"), TEXT("--all")});
    if (!Require(TEXT("Second diff commit"), GitHubGit::Run(C, {TEXT("commit"), TEXT("-m"), TEXT("New changes")}))) return false;
    auto Latest = GitHubGit::ReviewDiff(C, true);
    TestTrue(TEXT("Latest commit compares against previous commit"), Latest.bSuccess && Latest.Output.Contains(TEXT("-before")) && Latest.Output.Contains(TEXT("+after")));
    return true;
}
#endif
