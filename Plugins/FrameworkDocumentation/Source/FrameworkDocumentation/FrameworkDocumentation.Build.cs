using UnrealBuildTool;

public class FrameworkDocumentation : ModuleRules
{
    public FrameworkDocumentation(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG", "Blutility" });
        PrivateDependencyModuleNames.AddRange(new[] { "Slate", "SlateCore", "UnrealEd", "ToolMenus", "MainFrame", "Projects" });
        RuntimeDependencies.Add("$(PluginDir)/Resources/Fonts/Montserrat-Regular.ttf", StagedFileType.NonUFS);
        RuntimeDependencies.Add("$(PluginDir)/Resources/Fonts/Montserrat-Bold.ttf", StagedFileType.NonUFS);
        RuntimeDependencies.Add("$(PluginDir)/Resources/Fonts/OFL.txt", StagedFileType.NonUFS);
    }
}
