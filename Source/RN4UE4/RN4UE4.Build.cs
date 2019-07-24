// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class RN4UE4 : ModuleRules
{
	public RN4UE4(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable IWYU mode
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RakNet", "Replicas", "PhysX", "APEX" });
	}
}
