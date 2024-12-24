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
				"Core"
			});
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"CommonUI",
				"GameplayTags",
				"WHFramework",
				"DBTween"
			});
	}
}
