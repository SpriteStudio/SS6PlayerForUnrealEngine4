// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class Ss6PlayerExamplesTarget : TargetRules
{
	public Ss6PlayerExamplesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "Ss6PlayerExamples" } );
	}
}
