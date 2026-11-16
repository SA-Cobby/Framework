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
};

struct FGitHubResult
{
    bool bSuccess = false;
    FString Output;
    TOptional<FGitHubConfig> DetectedConfig;
};

namespace GitHubGit
{
    FString FindProjectGit(const FString& ProjectDirectory, const FString& Fallback);
    FGitHubResult DetectRepository(const FGitHubConfig& Config, const FString& ProjectDirectory);
    FGitHubResult Run(const FGitHubConfig& Config, const TArray<FString>& Args, double Timeout = 180.0);
    FGitHubResult Validate(const FGitHubConfig& Config, bool bCheckBranch = true);
    FGitHubResult Status(const FGitHubConfig& Config);
    FGitHubResult Configure(const FGitHubConfig& Config);
    FGitHubResult SwitchBranch(const FGitHubConfig& Config);
    FGitHubResult Pull(const FGitHubConfig& Config);
    FGitHubResult CommitPush(const FGitHubConfig& Config, const FString& Message);
    FGitHubResult Login(const FGitHubConfig& Config);
}
