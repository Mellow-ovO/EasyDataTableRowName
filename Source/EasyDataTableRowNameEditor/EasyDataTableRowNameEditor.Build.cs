using UnrealBuildTool;

public class EasyDataTableRowNameEditor : ModuleRules
{
    public EasyDataTableRowNameEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine",
                "Slate",
                "SlateCore",
                "PropertyEditor",
                "GraphEditor",
                "SlateCore",
                "UnrealEd",
                "AssetRegistry",
                "BlueprintGraph",
                "ApplicationCore",
                "UnrealEd",
                "Kismet",
                "EditorWidgets"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "PropertyEditor",
                "GraphEditor",
                "InputCore"
            }
        );
    }
}