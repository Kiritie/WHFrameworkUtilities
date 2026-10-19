// Some copyright should be here...

using UnrealBuildTool;

public class WHFrameworkUtilities : ModuleRules
{
	public WHFrameworkUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		bLegacyPublicIncludePaths = false;

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UMG",
				"CommonUI",
				"GameplayTags",
				"WHFramework",
				"WHFrameworkCore"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"InputCore",
				"CommonUI",
				"GameplayTags",
				"EnhancedInput",
				"WHFramework",
				"WHFrameworkCore",
				"DBTween"
			});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"AssetRegistry",
					"ContentBrowser",
					"UnrealEd"
				});
		}
	}
}
