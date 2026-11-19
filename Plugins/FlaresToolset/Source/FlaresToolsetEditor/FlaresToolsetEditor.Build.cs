using UnrealBuildTool;

public class FlaresToolsetEditor : ModuleRules
{
    public FlaresToolsetEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "FlaresToolset",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "ToolMenus",
            "PropertyEditor",
            "DeveloperSettings"
        });
    }
}
