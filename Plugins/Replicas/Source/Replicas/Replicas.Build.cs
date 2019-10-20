// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Replicas : ModuleRules
{
	public Replicas(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable IWYU mode
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// Disable unity build
		bFasterWithoutUnity = true;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "RakNet" });

		PublicIncludePaths.Add(Path.GetFullPath(Path.Combine(ModuleDirectory, "../../../../PhysX-3.4/PhysX_3.4/Samples/Replicas/Include")));
	}
}
