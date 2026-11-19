#pragma once
#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

inline FString FindGitHubProjectGit(const FString& ProjectDirectory, const FString& Fallback)
{
    const FString GitDirectory = FPaths::ConvertRelativePathToFull(ProjectDirectory) / TEXT("git");
    const TCHAR* Candidates[] = { TEXT("cmd/git.exe"), TEXT("bin/git.exe"), TEXT("git.exe"), TEXT("mingw64/bin/git.exe") };
    for (const TCHAR* Relative : Candidates)
    {
        const FString Candidate = GitDirectory / Relative;
        if (IFileManager::Get().FileExists(*Candidate)) return Candidate;
    }
    return Fallback;
}
