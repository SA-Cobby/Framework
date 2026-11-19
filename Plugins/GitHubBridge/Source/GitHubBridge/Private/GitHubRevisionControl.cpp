#include "GitHubRevisionControl.h"
#include "HAL/FileManager.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "SourceControlHelpers.h"

namespace GitHubRevisionControl
{
    // The built-in provider caches its binary path at module startup. Keep the
    // first observed path rather than assuming later ini writes update that cache.
    static TOptional<FString> LoadedBinaryPath;

    bool PrepareGitPath(bool bEnabled, const FString& GitPath)
    {
        if (!bEnabled || !IFileManager::Get().FileExists(*GitPath)) return false;
        const FString Ini = SourceControlHelpers::GetSettingsIni();
        const TCHAR* Section = TEXT("GitSourceControl.GitSourceControlSettings");
        FString Existing;
        GConfig->GetString(Section, TEXT("BinaryPath"), Existing, Ini);
        const bool bLoaded = FModuleManager::Get().IsModuleLoaded(TEXT("GitSourceControl"));
        if (bLoaded && !LoadedBinaryPath.IsSet()) LoadedBinaryPath = Existing;
        GConfig->SetString(Section, TEXT("BinaryPath"), *GitPath, Ini);
        GConfig->Flush(false, Ini);
        if (!bLoaded)
        {
            LoadedBinaryPath = GitPath;
            return true;
        }
        return LoadedBinaryPath.IsSet() && FPaths::IsSamePath(LoadedBinaryPath.GetValue(), GitPath);
    }

    bool SelectGitProvider(bool bEnabled, FString& OutStatus)
    {
        if (!bEnabled)
        {
            OutStatus = TEXT("Automatic connection is off. Unreal's current provider is left unchanged.");
            return false;
        }
        if (!FModuleManager::Get().LoadModulePtr<IModuleInterface>(TEXT("GitSourceControl")))
        {
            OutStatus = TEXT("Unreal's Git provider is unavailable. Enable the engine Git plugin and restart Unreal.");
            return false;
        }
        ISourceControlModule& SourceControl = ISourceControlModule::Get();
        TArray<FName> Providers;
        SourceControl.GetProviderNames(Providers);
        if (!Providers.Contains(FName(TEXT("Git"))))
        {
            OutStatus = TEXT("No Git revision-control provider is registered.");
            return false;
        }
        SourceControl.SetProvider(TEXT("Git"));
        if (SourceControl.GetProvider().GetName() != FName(TEXT("Git")))
        {
            OutStatus = TEXT("Unreal could not select the Git provider.");
            return false;
        }
        SourceControl.GetProvider().Init(false);
        if (!SourceControl.GetProvider().IsAvailable())
        {
            OutStatus = TEXT("Git is selected, but Unreal could not open the project repository. Check its Git path and restart Unreal.");
            return false;
        }
        OutStatus = TEXT("Unreal's Git provider is selected. Refreshing project file status…");
        return true;
    }
}
