#include "GitHubGit.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"

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
#endif
