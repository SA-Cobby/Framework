using UnrealBuildTool;

public class GitHubBridge : ModuleRules
{
    public GitHubBridge(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] {
            "Core", "CoreUObject", "Engine", "Slate", "SlateCore",
            "UnrealEd", "MainFrame", "ToolMenus", "SourceControl", "SourceControlWindows", "Virtualization"
        });
    }
}
