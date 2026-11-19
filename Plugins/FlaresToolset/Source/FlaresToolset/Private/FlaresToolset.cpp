#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/CoreDelegates.h"
#include "ShaderCore.h"
#include "FlaresToolsetViewExtension.h"

class FFlaresToolsetModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        const FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("FlaresToolset"))->GetBaseDir(), TEXT("Shaders"));
        AddShaderSourceDirectoryMapping(TEXT("/Plugin/FlaresToolset"), PluginShaderDir);

        InitHandle = FCoreDelegates::OnPostEngineInit.AddLambda(
            [this]()
            {
                Extension = FSceneViewExtensions::NewExtension<FFlaresToolsetViewExtension>();
            });
    }

    virtual void ShutdownModule() override
    {
        FCoreDelegates::OnPostEngineInit.Remove(InitHandle);
        FlushRenderingCommands();
        Extension.Reset();
    }

private:
    TSharedPtr<FFlaresToolsetViewExtension, ESPMode::ThreadSafe> Extension;
    FDelegateHandle InitHandle;
};

IMPLEMENT_MODULE(FFlaresToolsetModule, FlaresToolset)
