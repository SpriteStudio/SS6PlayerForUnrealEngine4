namespace UnrealBuildTool.Rules
{
	public class SpriteStudio6Ed : ModuleRules
	{
		public SpriteStudio6Ed(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivatePCHHeaderFile = "Private/SpriteStudio6EdPrivatePCH.h";

			DynamicallyLoadedModuleNames.AddRange(
				new string[] {
					"PropertyEditor",
				}
				);

			PublicIncludePaths.AddRange(
				new string[] {
					ModuleDirectory + "/Public",
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"SpriteStudio6Ed/Private",
					"SpriteStudio6Ed/Private/ActorFactory",
					"SpriteStudio6Ed/Private/AssetFactory",
					"SpriteStudio6Ed/Private/Loader",
					"SpriteStudio6Ed/Private/Loader/babel",
					"SpriteStudio6Ed/Private/Loader/tinyxml2",
					"SpriteStudio6Ed/Private/Viewer",
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"UnrealEd",
					"AssetTools",
					"Slate",
					"SlateCore",
					"EditorStyle",
					"AppFramework",
					"MessageLog",
					"ApplicationCore",

					"SpriteStudio6",
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
				}
				);

			bUseUnity = false;
		}
	}
}