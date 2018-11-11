// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class SpriteStudio6 : ModuleRules
	{
		public SpriteStudio6(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivatePCHHeaderFile = "Private/SpriteStudio6PrivatePCH.h";

			DynamicallyLoadedModuleNames.AddRange(
				new string[] {
				}
				);

			PublicIncludePaths.AddRange(
				new string[] {
					ModuleDirectory + "/Public",
					ModuleDirectory + "/Public/Actor",
					ModuleDirectory + "/Public/Component",
					ModuleDirectory + "/Public/Data",
					ModuleDirectory + "/Public/Player",
					ModuleDirectory + "/Public/Render",
					ModuleDirectory + "/Public/UMG",
					ModuleDirectory + "/Public/Misc",
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"SpriteStudio6/Private",
					"SpriteStudio6/Private/SSSDK",
					"SpriteStudio6/Private/Actor",
					"SpriteStudio6/Private/Component",
					"SpriteStudio6/Private/Data",
					"SpriteStudio6/Private/Player",
					"SpriteStudio6/Private/Render",
					"SpriteStudio6/Private/UMG",
					"SpriteStudio6/Private/Misc",
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputCore",
					"RHI",
					"RenderCore",
					"ShaderCore",
					"UtilityShaders",
					"SlateCore",
					"SlateRHIRenderer",
					"Slate",
					"UMG",
					"Projects",
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Renderer",
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
				}
				);
		}
	}
}