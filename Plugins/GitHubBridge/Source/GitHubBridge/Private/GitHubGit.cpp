#include "GitHubGit.h"
#include "GitHubProjectGit.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "Misc/Guid.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"

namespace
{
    bool HasPendingIntegration(const FGitHubConfig& C)
    {
        for (const TCHAR* Marker : { TEXT("rebase-merge"), TEXT("rebase-apply"), TEXT("MERGE_HEAD"), TEXT("CHERRY_PICK_HEAD"), TEXT("REVERT_HEAD") })
        {
            const auto R = GitHubGit::Run(C, { TEXT("rev-parse"), TEXT("--git-path"), Marker }, 10);
            if (!R.bSuccess) return true;
            FString Path = R.Output.TrimStartAndEnd();
            if (FPaths::IsRelative(Path)) Path = C.Root / Path;
            if (IFileManager::Get().DirectoryExists(*Path) || IFileManager::Get().FileExists(*Path)) return true;
        }
        return false;
    }
    // Windows argv quoting, including trailing backslashes and embedded quotes.
    FString Quote(const FString& Value)
    {
        FString Result = TEXT("\"");
        int32 Slashes = 0;
        for (TCHAR C : Value)
        {
            if (C == TEXT('\\')) { ++Slashes; continue; }
            if (C == TEXT('"'))
            {
                Result += FString::ChrN(Slashes * 2 + 1, TEXT('\\'));
                Result += C;
            }
            else { Result += FString::ChrN(Slashes, TEXT('\\')); Result += C; }
            Slashes = 0;
        }
        return Result + FString::ChrN(Slashes * 2, TEXT('\\')) + TEXT("\"");
    }
    FGitHubResult Fail(const FString& Text) { return { false, Text }; }
    bool ValidRemote(const FString& Url)
    {
        // No credentials in URLs; authentication belongs to GCM or SSH.
        FString Path;
        if (Url.StartsWith(TEXT("https://github.com/"))) Path = Url.Mid(19);
        else if (Url.StartsWith(TEXT("git@github.com:"))) Path = Url.Mid(15);
        else return false;
        TArray<FString> Parts;
        Path.ParseIntoArray(Parts, TEXT("/"), false);
        if (Parts.Num() != 2 || Parts[0].IsEmpty() || Parts[1].IsEmpty()) return false;
        for (TCHAR C : Path)
            if (!(FChar::IsAlnum(C) || C == '-' || C == '_' || C == '.' || C == '/')) return false;
        return true;
    }
}

FGitHubResult GitHubGit::Run(const FGitHubConfig& Config, const TArray<FString>& Args, double Timeout)
{
    FString Command = TEXT("--no-pager -c color.ui=false -c core.quotepath=false -c core.askPass= -C ") + Quote(Config.Root);
    for (const FString& Arg : Args) Command += TEXT(" ") + Quote(Arg);
    void* Read = nullptr; void* Write = nullptr;
    void* InputRead = nullptr; void* InputWrite = nullptr;
    if (!FPlatformProcess::CreatePipe(Read, Write) || !FPlatformProcess::CreatePipe(InputRead, InputWrite, true))
    {
        FPlatformProcess::ClosePipe(Read, Write);
        return Fail(TEXT("Could not create Git process pipes."));
    }
    FProcHandle Process = FPlatformProcess::CreateProc(*Config.Git, *Command, false, true, true,
        nullptr, 0, *Config.Root, Write, InputRead, Write);
    // Close our copies of child handles. EOF prevents hidden terminal password prompts.
    FPlatformProcess::ClosePipe(nullptr, Write);
    FPlatformProcess::ClosePipe(InputRead, InputWrite);
    if (!Process.IsValid())
    {
        FPlatformProcess::ClosePipe(Read, nullptr);
        return Fail(TEXT("Cannot start Git. Check the Git executable and repository folder."));
    }
    FString Output;
    const double Start = FPlatformTime::Seconds();
    bool bTimeout = false;
    while (FPlatformProcess::IsProcRunning(Process))
    {
        Output += FPlatformProcess::ReadPipe(Read);
        if (FPlatformTime::Seconds() - Start > Timeout)
        {
            FPlatformProcess::TerminateProc(Process, true);
            bTimeout = true;
            break;
        }
        FPlatformProcess::Sleep(0.02f);
    }
    FPlatformProcess::WaitForProc(Process);
    Output += FPlatformProcess::ReadPipe(Read);
    int32 Code = -1;
    FPlatformProcess::GetProcReturnCode(Process, &Code);
    FPlatformProcess::CloseProc(Process);
    FPlatformProcess::ClosePipe(Read, nullptr);
    if (bTimeout) Output += TEXT("\nGit timed out. Check authentication/network and retry. Any completed local commit is retained.");
    return { Code == 0 && !bTimeout, Output };
}

FString GitHubGit::FindProjectGit(const FString& ProjectDirectory, const FString& Fallback)
{
    return FindGitHubProjectGit(ProjectDirectory, Fallback);
}

FGitHubResult GitHubGit::DetectRepository(const FGitHubConfig& Config, const FString& ProjectDirectory)
{
    FString Directory = FPaths::ConvertRelativePathToFull(ProjectDirectory);
    FPaths::NormalizeDirectoryName(Directory);
    bool bFoundMarker = false;
    while (!Directory.IsEmpty())
    {
        const FString Marker = Directory / TEXT(".git");
        if (IFileManager::Get().DirectoryExists(*Marker) || IFileManager::Get().FileExists(*Marker))
        {
            bFoundMarker = true;
            break;
        }
        const FString Parent = FPaths::GetPath(Directory);
        if (Parent == Directory) break;
        Directory = Parent;
    }
    if (!bFoundMarker)
        return Fail(TEXT("No .git folder or worktree file found in the project folder or its parents. Existing settings were kept."));

    FGitHubConfig Detected = Config;
    Detected.Root = FPaths::ConvertRelativePathToFull(ProjectDirectory);
    auto Root = Run(Detected, { TEXT("rev-parse"), TEXT("--show-toplevel") }, 10);
    if (!Root.bSuccess)
        return Fail(TEXT("A .git entry was found, but Git could not read the checkout. Check the Git executable, repository permissions, and worktree metadata. Existing settings were kept."));
    Detected.Root = Root.Output.TrimStartAndEnd();
    FPaths::NormalizeDirectoryName(Detected.Root);
    Detected.Remote.Empty();
    Detected.Branch.Empty();
    Detected.Name.Empty();
    Detected.Email.Empty();
    FString Notes;
    auto Remote = Run(Detected, { TEXT("remote"), TEXT("get-url"), TEXT("origin") }, 10);
    if (Remote.bSuccess && ValidRemote(Remote.Output.TrimStartAndEnd()))
        Detected.Remote = Remote.Output.TrimStartAndEnd();
    else
        Notes += TEXT("\nOrigin is missing or is not a supported credential-free GitHub URL. Enter the GitHub clone URL manually.");
    auto Branch = Run(Detected, { TEXT("symbolic-ref"), TEXT("--short"), TEXT("HEAD") }, 10);
    if (Branch.bSuccess) Detected.Branch = Branch.Output.TrimStartAndEnd();
    else Notes += TEXT("\nHEAD is detached. Select an existing branch before committing or pushing.");
    auto Name = Run(Detected, { TEXT("config"), TEXT("--get"), TEXT("user.name") }, 10);
    if (Name.bSuccess) Detected.Name = Name.Output.TrimStartAndEnd();
    auto Email = Run(Detected, { TEXT("config"), TEXT("--get"), TEXT("user.email") }, 10);
    if (Email.bSuccess) Detected.Email = Email.Output.TrimStartAndEnd();
    FGitHubResult Result { true, TEXT("Detected project repository: ") + Detected.Root +
        TEXT("\nLoaded origin, current branch, and available Git author identity. No repository settings were changed.") + Notes };
    Result.DetectedConfig = MoveTemp(Detected);
    return Result;
}

FGitHubResult GitHubGit::Validate(const FGitHubConfig& C, bool bCheckBranch)
{
    if (!ValidRemote(C.Remote)) return Fail(TEXT("Use https://github.com/OWNER/REPO.git or git@github.com:OWNER/REPO.git (without a token)."));
    auto R = Run(C, { TEXT("rev-parse"), TEXT("--show-toplevel") });
    if (!R.bSuccess) return R;
    FString Actual = R.Output.TrimStartAndEnd();
    FString Expected = FPaths::ConvertRelativePathToFull(C.Root);
    FPaths::NormalizeDirectoryName(Actual); FPaths::NormalizeDirectoryName(Expected);
    if (!FPaths::IsSamePath(Actual, Expected)) return Fail(TEXT("Select the repository root, not a subfolder. Operations cover the whole repository."));
    R = Run(C, { TEXT("check-ref-format"), TEXT("--branch"), C.Branch });
    if (!R.bSuccess || C.Branch.StartsWith(TEXT("-"))) return Fail(TEXT("Enter a valid branch name."));
    R = Run(C, { TEXT("remote"), TEXT("get-url"), TEXT("origin") });
    if (!R.bSuccess || R.Output.TrimStartAndEnd() != C.Remote) return Fail(TEXT("Origin differs from the configured GitHub repository. Use Apply repository settings first."));
    // Refuse a separate push destination that could silently receive the upload.
    R = Run(C, { TEXT("remote"), TEXT("get-url"), TEXT("--push"), TEXT("--all"), TEXT("origin") });
    if (!R.bSuccess || R.Output.TrimStartAndEnd() != C.Remote) return Fail(TEXT("Origin has a different or multiple push URLs. Correct these in Git before pushing."));
    if (bCheckBranch)
    {
        R = Run(C, { TEXT("symbolic-ref"), TEXT("--short"), TEXT("HEAD") });
        if (!R.bSuccess || R.Output.TrimStartAndEnd() != C.Branch) return Fail(TEXT("Current branch differs from the configured branch (or HEAD is detached). Use Switch branch."));
    }
    return { true, TEXT("") };
}

FGitHubResult GitHubGit::Status(const FGitHubConfig& C)
{
    auto R = Validate(C);
    if (!R.bSuccess) return R;
    return Run(C, { TEXT("status"), TEXT("--short"), TEXT("--branch"), TEXT("--untracked-files=all") });
}

FGitHubExitState GitHubGit::InspectExitState(const FGitHubConfig& C)
{
    FGitHubExitState State;
    auto FailCheck = [&State](const FString& Reason)
    {
        State.Summary = TEXT("Could not confirm that everything is pushed.\n") + Reason;
        return State;
    };
    auto R = Validate(C);
    if (!R.bSuccess) return FailCheck(R.Output);
    R = Run(C, { TEXT("status"), TEXT("--porcelain"), TEXT("--untracked-files=normal") }, 10);
    if (!R.bSuccess) return FailCheck(R.Output);
    State.bWorkingTreeDirty = !R.Output.TrimStartAndEnd().IsEmpty();
    R = Run(C, { TEXT("rev-parse"), TEXT("--verify"), TEXT("--quiet"), TEXT("HEAD^{commit}") }, 10);
    if (!R.bSuccess)
    {
        // A valid symbolic branch with no commit yet has no history to upload.
        if (!R.Output.TrimStartAndEnd().IsEmpty()) return FailCheck(R.Output);
        State.bKnown = true;
        State.Summary = State.bWorkingTreeDirty ? TEXT("Uncommitted repository changes need to be committed and pushed.")
            : TEXT("No local commits or file changes to push.");
        return State;
    }
    const FString Head = R.Output.TrimStartAndEnd();
    const FString RemoteRef = TEXT("refs/remotes/origin/") + C.Branch;
    R = Run(C, { TEXT("show-ref"), TEXT("--verify"), TEXT("--quiet"), RemoteRef }, 10);
    const bool bHaveRemoteRef = R.bSuccess;
    if (!bHaveRemoteRef && !R.Output.TrimStartAndEnd().IsEmpty()) return FailCheck(R.Output);
    // Compare against the configured destination branch, not an unrelated upstream.
    // This is a local check: closing the editor never requires a fetch/login.
    R = Run(C, { TEXT("rev-list"), TEXT("--count"), bHaveRemoteRef ? RemoteRef + TEXT("..") + Head : Head }, 10);
    if (!R.bSuccess || !LexTryParseString(State.UnpushedCommits, *R.Output.TrimStartAndEnd())) return FailCheck(R.Output);
    State.bKnown = true;
    State.Summary = FString::Printf(TEXT("Uncommitted repository changes: %s\nLocal commits awaiting push: %d"),
        State.bWorkingTreeDirty ? TEXT("yes") : TEXT("no"), State.UnpushedCommits);
    if (!bHaveRemoteRef) State.Summary += TEXT("\nThe destination branch has no local remote-tracking reference; its commits are treated as awaiting push.");
    return State;
}

FGitHubResult GitHubGit::Configure(const FGitHubConfig& C)
{
    if (!ValidRemote(C.Remote)) return Fail(TEXT("Enter a GitHub HTTPS or SSH clone URL without credentials."));
    auto R = Run(C, { TEXT("rev-parse"), TEXT("--show-toplevel") });
    if (!R.bSuccess) return Fail(TEXT("Select an existing local Git repository. Clone or initialize it with Git first."));
    if (!FPaths::IsSamePath(R.Output.TrimStartAndEnd(), FPaths::ConvertRelativePathToFull(C.Root)))
        return Fail(TEXT("The repository folder must be the Git root."));
    R = Run(C, { TEXT("remote"), TEXT("get-url"), TEXT("origin") });
    R = Run(C, { TEXT("remote"), R.bSuccess ? TEXT("set-url") : TEXT("add"), TEXT("origin"), C.Remote });
    if (!R.bSuccess) return R;
    if (C.Name.IsEmpty() || C.Email.IsEmpty()) return Fail(TEXT("Origin applied. Enter a commit author name and email, then apply again."));
    R = Run(C, { TEXT("config"), TEXT("--local"), TEXT("user.name"), C.Name });
    if (!R.bSuccess) return R;
    R = Run(C, { TEXT("config"), TEXT("--local"), TEXT("user.email"), C.Email });
    if (R.bSuccess) R.Output = TEXT("Origin and local commit identity applied. Review the branch, then sign in or use your SSH agent.");
    return R;
}

FGitHubResult GitHubGit::SwitchBranch(const FGitHubConfig& C)
{
    auto R = Validate(C, false); if (!R.bSuccess) return R;
    R = Run(C, { TEXT("status"), TEXT("--porcelain") });
    if (!R.bSuccess || !R.Output.TrimStartAndEnd().IsEmpty()) return Fail(TEXT("Commit or stash all changes before switching branches."));
    R = Run(C, { TEXT("fetch"), TEXT("origin") }); if (!R.bSuccess) return R;
    return Run(C, { TEXT("switch"), C.Branch });
}

FGitHubResult GitHubGit::Pull(const FGitHubConfig& C)
{
    auto R = Validate(C); if (!R.bSuccess) return R;
    R = Run(C, { TEXT("status"), TEXT("--porcelain") });
    if (!R.bSuccess || !R.Output.TrimStartAndEnd().IsEmpty()) return Fail(TEXT("Commit or stash all changes before pulling. Pull only allows a clean working tree."));
    return Run(C, { TEXT("pull"), TEXT("--ff-only"), TEXT("--no-rebase"), TEXT("origin"), C.Branch });
}

FGitHubResult GitHubGit::CommitPush(const FGitHubConfig& C, const FString& Message)
{
    auto R = Validate(C); if (!R.bSuccess) return R;
    if (HasPendingIntegration(C)) return Fail(TEXT("Finish or abort the existing rebase/merge/cherry-pick before committing and pushing."));
    R = Run(C, { TEXT("diff"), TEXT("--name-only"), TEXT("--diff-filter=U") });
    if (!R.bSuccess || !R.Output.TrimStartAndEnd().IsEmpty()) return Fail(TEXT("Resolve merge conflicts before committing."));
    R = Run(C, { TEXT("status"), TEXT("--porcelain") }); if (!R.bSuccess) return R;
    if (!R.Output.TrimStartAndEnd().IsEmpty())
    {
        if (Message.TrimStartAndEnd().IsEmpty()) return Fail(TEXT("Enter a commit message."));
        R = Run(C, { TEXT("add"), TEXT("--all"), TEXT("--"), TEXT(".") }); if (!R.bSuccess) return R;
        // Hooks run normally. A failed hook leaves changes staged for review/retry.
        R = Run(C, { TEXT("commit"), TEXT("-m"), Message }); if (!R.bSuccess) return R;
    }
    R = PushCommitted(C);
    if (R.bSuccess)
    {
        auto Check = Run(C, { TEXT("status"), TEXT("--porcelain") });
        if (!Check.bSuccess || !Check.Output.TrimStartAndEnd().IsEmpty())
            return Fail(TEXT("Push completed, but the working tree still has changes (possibly from a hook). Review status before closing.\n") + Check.Output);
        R.Output += TEXT("\nPush completed.");
    }
    else R.Output += TEXT("\nPush failed. Local commits are safe; correct the problem and retry. No force push was used.");
    return R;
}

FGitHubResult GitHubGit::PushCommitted(const FGitHubConfig& C, const FString& Commit)
{
    auto R = Validate(C); if (!R.bSuccess) return R;
    if (HasPendingIntegration(C)) return Fail(TEXT("Finish or abort the existing rebase/merge/cherry-pick before pushing."));
    R = Run(C, { TEXT("rev-parse"), TEXT("--verify"), TEXT("--end-of-options"), Commit + TEXT("^{commit}") });
    if (!R.bSuccess) return { false, TEXT("No valid local commit to push.\n") + R.Output };
    FString Object = R.Output.TrimStartAndEnd();
    R = Run(C, { TEXT("rev-parse"), TEXT("HEAD") });
    if (!R.bSuccess || R.Output.TrimStartAndEnd() != Object)
        return Fail(TEXT("The branch changed after this submit was queued. No rebase or push was performed. Review it, then use Push existing commits."));
    R = Run(C, { TEXT("ls-remote"), TEXT("--heads"), TEXT("origin"), TEXT("refs/heads/") + C.Branch });
    if (!R.bSuccess) return R;
    FString Details;
    if (!R.Output.TrimStartAndEnd().IsEmpty())
    {
        const FString RemoteRef = TEXT("refs/remotes/origin/") + C.Branch;
        R = Run(C, { TEXT("fetch"), TEXT("--no-tags"), TEXT("origin"), TEXT("+refs/heads/") + C.Branch + TEXT(":") + RemoteRef });
        if (!R.bSuccess) return R;
        R = Run(C, { TEXT("merge-base"), TEXT("--is-ancestor"), RemoteRef, Object });
        if (!R.bSuccess)
        {
            R = Run(C, { TEXT("merge-base"), Object, RemoteRef });
            if (!R.bSuccess) return Fail(TEXT("Local and remote histories have no common ancestor. Resolve this with Git before pushing."));
            R = Run(C, { TEXT("status"), TEXT("--porcelain"), TEXT("--untracked-files=no") });
            if (!R.bSuccess || !R.Output.TrimStartAndEnd().IsEmpty())
                return Fail(TEXT("The remote has new commits, but tracked files have uncommitted/staged changes. Commit or stash them before rebasing. Nothing was pushed; no automatic stash was created."));
            const FString Backup = TEXT("refs/github-bridge/pre-rebase/") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
            R = Run(C, { TEXT("update-ref"), Backup, Object });
            if (!R.bSuccess) return R;
            Details = TEXT("\nOriginal local tip preserved at ") + Backup;
            R = Run(C, { TEXT("-c"), TEXT("rebase.autoStash=false"), TEXT("-c"), TEXT("rebase.updateRefs=false"),
                TEXT("rebase"), TEXT("--rebase-merges"), TEXT("--no-autostash"), RemoteRef });
            if (!R.bSuccess)
            {
                R.Output += TEXT("\nRebase did not complete; nothing was pushed.") + Details;
                if (HasPendingIntegration(C)) R.Output += TEXT("\nResolve conflicts and run git rebase --continue, or use git rebase --abort in your Git tool. Unreal stays open.");
                return R;
            }
            R = Run(C, { TEXT("rev-parse"), TEXT("HEAD") });
            if (!R.bSuccess) return R;
            Object = R.Output.TrimStartAndEnd();
            Details += TEXT("\nRebased onto the fetched destination branch. Restart Unreal before editing assets changed on disk.");
        }
        else Details = TEXT("\nFetched destination branch; local commits are already based on it.");
    }
    else Details = TEXT("\nThe remote branch does not exist yet; creating it without a rebase.");
    R = Validate(C); if (!R.bSuccess) return R;
    R = Run(C, { TEXT("-c"), TEXT("push.followTags=false"), TEXT("push"), TEXT("--set-upstream"), TEXT("origin"), Object + TEXT(":refs/heads/") + C.Branch });
    R.Output += Details;
    R.Output += R.bSuccess ? TEXT("\nCommitted changes pushed. Uncommitted files were left untouched.")
        : TEXT("\nPush failed. Your local commit is retained. Correct the problem, then use Push existing commits to retry. No force push was used.");
    return R;
}

FGitHubResult GitHubGit::ReviewDiff(const FGitHubConfig& C, bool bLatestCommit)
{
    auto Root = Run(C, { TEXT("rev-parse"), TEXT("--show-toplevel") }, 10);
    if (!Root.bSuccess) return Root;
    if (!FPaths::IsSamePath(Root.Output.TrimStartAndEnd(), FPaths::ConvertRelativePathToFull(C.Root)))
        return Fail(TEXT("Detect the repository root before reviewing changes."));
    auto Head = Run(C, { TEXT("rev-parse"), TEXT("--verify"), TEXT("--quiet"), TEXT("HEAD") }, 10);
    FString Output;
    if (bLatestCommit)
    {
        if (!Head.bSuccess) return Fail(TEXT("This repository has no commit to compare yet. Use Pending changes."));
        auto Parent = Run(C, { TEXT("rev-parse"), TEXT("--verify"), TEXT("--quiet"), TEXT("HEAD^") }, 10);
        auto Patch = Parent.bSuccess
            ? Run(C, { TEXT("diff"), TEXT("--no-ext-diff"), TEXT("--no-textconv"), TEXT("--find-renames"), TEXT("HEAD^"), TEXT("HEAD"), TEXT("--") }, 30)
            : Run(C, { TEXT("show"), TEXT("--root"), TEXT("--format="), TEXT("--no-ext-diff"), TEXT("--no-textconv"), TEXT("HEAD"), TEXT("--") }, 30);
        if (!Patch.bSuccess) return Patch;
        auto Info = Run(C, { TEXT("log"), TEXT("-1"), TEXT("--format=Commit %H%nAuthor: %an%nDate: %aI%n%n%B") }, 10);
        Output = TEXT("LATEST COMMIT VS ") + FString(Parent.bSuccess ? TEXT("FIRST PARENT") : TEXT("EMPTY TREE (INITIAL COMMIT)")) + TEXT("\n\n") + Info.Output + TEXT("\n") + Patch.Output;
    }
    else
    {
        Output = TEXT("PENDING CHANGES VS LAST COMMIT\nIncludes staged and unstaged file changes on disk. Unsaved Unreal assets are not yet included.\n\n");
        if (Head.bSuccess)
        {
            auto Patch = Run(C, { TEXT("diff"), TEXT("--no-ext-diff"), TEXT("--no-textconv"), TEXT("--find-renames"), TEXT("HEAD"), TEXT("--") }, 30);
            if (!Patch.bSuccess) return Patch;
            Output += Patch.Output.IsEmpty() ? TEXT("No tracked-file differences.\n") : Patch.Output;
        }
        else
        {
            auto Staged = Run(C, { TEXT("diff"), TEXT("--cached"), TEXT("--no-ext-diff"), TEXT("--no-textconv"), TEXT("--") }, 30);
            auto Unstaged = Run(C, { TEXT("diff"), TEXT("--no-ext-diff"), TEXT("--no-textconv"), TEXT("--") }, 30);
            if (!Staged.bSuccess) return Staged;
            if (!Unstaged.bSuccess) return Unstaged;
            Output += TEXT("No previous commit. Staged additions:\n") + Staged.Output + TEXT("\nUnstaged edits to those additions:\n") + Unstaged.Output;
        }
        auto Untracked = Run(C, { TEXT("ls-files"), TEXT("--others"), TEXT("--exclude-standard") }, 10);
        if (!Untracked.bSuccess) return Untracked;
        TArray<FString> Files;
        Untracked.Output.ParseIntoArrayLines(Files, true);
        for (int32 Index = 0; Index < Files.Num(); ++Index)
        {
            if (Index >= 50 || Output.Len() > 2 * 1024 * 1024)
            {
                Output += TEXT("\nMore untracked files omitted. Use your Git client to review the full set.\n");
                break;
            }
            auto Patch = Run(C, { TEXT("diff"), TEXT("--no-index"), TEXT("--no-ext-diff"), TEXT("--no-textconv"), TEXT("--"), TEXT("/dev/null"), Files[Index] }, 10);
            // --no-index uses exit code 1 for an ordinary difference.
            Output += TEXT("\nUNTRACKED: ") + Files[Index] + TEXT("\n") + Patch.Output;
        }
    }
    if (Output.Len() > 2 * 1024 * 1024) Output = Output.Left(2 * 1024 * 1024) + TEXT("\n[Preview truncated; review the complete diff in Git.]\n");
    return { true, Output };
}

FGitHubResult GitHubGit::Login(const FGitHubConfig& C)
{
    if (C.Username.IsEmpty() || C.Username.StartsWith(TEXT("-"))) return Fail(TEXT("Enter your GitHub username."));
    auto Probe = Run(C, { TEXT("credential-manager"), TEXT("--help") }, 10);
    FString Helper = TEXT("credential-manager");
    if (!Probe.bSuccess || !Probe.Output.Contains(TEXT("github")))
    {
        Helper = TEXT("credential-manager-core");
        Probe = Run(C, { Helper, TEXT("--help") }, 10);
    }
    if (!Probe.bSuccess || !Probe.Output.Contains(TEXT("github")))
        return Fail(TEXT("Browser login requires a newer Git Credential Manager. Update Git for Windows with GCM enabled, or use an SSH clone URL and your existing SSH agent."));
    auto R = Run(C, { Helper, TEXT("github"), TEXT("login"), TEXT("--username"), C.Username }, 180);
    if (!R.bSuccess) R.Output += TEXT("\nInstall/update Git for Windows with Git Credential Manager, or configure SSH externally. GitHub account passwords cannot authenticate Git HTTPS.");
    return R;
}
