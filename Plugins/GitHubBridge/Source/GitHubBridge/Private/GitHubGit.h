#pragma once
#include "CoreMinimal.h"

struct FGitHubConfig
{
    FString Git = TEXT("C:/Program Files/Git/cmd/git.exe");
    FString Root;
    FString Remote;
    FString Branch = TEXT("main");
    FString Username;
    FString Name;
    FString Email;
    bool bPromptOnExit = true;
    bool bAutoConnectRevisionControl = true;
    bool bPushAfterUnrealSubmit = true;
};

struct FGitHubResult
{
    bool bSuccess = false;
    FString Output;
    TOptional<FGitHubConfig> DetectedConfig;
};

struct FGitHubExitState
{
    bool bKnown = false;
    bool bWorkingTreeDirty = false;
    int32 UnpushedCommits = 0;
    FString Summary;
    bool ShouldPrompt(int32 UnsavedPackages) const
    {
        return UnsavedPackages > 0 || !bKnown || bWorkingTreeDirty || UnpushedCommits > 0;
    }
};

namespace GitHubGit
{
    FString FindProjectGit(const FString& ProjectDirectory, const FString& Fallback);
    FGitHubResult DetectRepository(const FGitHubConfig& Config, const FString& ProjectDirectory);
    FGitHubResult Run(const FGitHubConfig& Config, const TArray<FString>& Args, double Timeout = 180.0);
    FGitHubResult Validate(const FGitHubConfig& Config, bool bCheckBranch = true);
    FGitHubResult Status(const FGitHubConfig& Config);
    FGitHubExitState InspectExitState(const FGitHubConfig& Config);
    FGitHubResult ReviewDiff(const FGitHubConfig& Config, bool bLatestCommit = false);
    FGitHubResult Configure(const FGitHubConfig& Config);
    FGitHubResult SwitchBranch(const FGitHubConfig& Config);
    FGitHubResult Pull(const FGitHubConfig& Config);
    FGitHubResult CommitPush(const FGitHubConfig& Config, const FString& Message);
    FGitHubResult PushCommitted(const FGitHubConfig& Config, const FString& Commit = TEXT("HEAD"));
    FGitHubResult Login(const FGitHubConfig& Config);
}
