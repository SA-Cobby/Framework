using UnrealBuildTool;
using System.IO;

public class GitHubBridgeBootstrap : ModuleRules
{
    public GitHubBridgeBootstrap(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "SourceControl" });
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", "GitHubBridge", "Private"));
    }
}
