#include "GitHubProjectGit.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "SourceControlHelpers.h"

// Keep the early module free of editor/Slate dependencies. Git caches its
// BinaryPath when its Default-phase module loads; the UI loads PostEngineInit.
class FGitHubBridgeBootstrapModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        if (IsRunningCommandlet()) return;
        const FString BridgeIni = FPaths::ProjectSavedDir() / TEXT("Config/GitHubBridge.ini");
        bool bAutoConnect = true;
        GConfig->GetBool(TEXT("GitHubBridge"), TEXT("AutoConnectRevisionControl"), bAutoConnect, BridgeIni);
        if (!bAutoConnect) return;
        FString Git = TEXT("C:/Program Files/Git/cmd/git.exe");
        GConfig->GetString(TEXT("GitHubBridge"), TEXT("Git"), Git, BridgeIni);
        Git = FindGitHubProjectGit(FPaths::ProjectDir(), Git);
        if (!IFileManager::Get().FileExists(*Git)) return;
        const FString ProviderIni = SourceControlHelpers::GetSettingsIni();
        GConfig->SetString(TEXT("GitSourceControl.GitSourceControlSettings"), TEXT("BinaryPath"), *Git, ProviderIni);
        GConfig->Flush(false, ProviderIni);
        UE_LOG(LogTemp, Display, TEXT("GitHubBridge: Prepared the Git revision-control executable path before provider startup."));
    }
    virtual bool SupportsDynamicReloading() override { return false; }
};

IMPLEMENT_MODULE(FGitHubBridgeBootstrapModule, GitHubBridgeBootstrap)
