#pragma once
#include "CoreMinimal.h"

namespace GitHubRevisionControl
{
    // Returns true if the provider can use this path in the current editor session.
    bool PrepareGitPath(bool bEnabled, const FString& GitPath);
    // Does not change the active provider when disabled or when Git is unavailable.
    bool SelectGitProvider(bool bEnabled, FString& OutStatus);
}
