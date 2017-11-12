// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class Ss6PlayerExamplesEditorTarget : TargetRules
{
	public Ss6PlayerExamplesEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "Ss6PlayerExamples" } );
	}
}
