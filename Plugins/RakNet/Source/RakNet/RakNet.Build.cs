// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RakNet : ModuleRules
{
	public RakNet(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable IWYU mode and specify custom PCH
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/RakNetPrivatePCH.h";

		PrivateDefinitions.AddRange(new string[]
		{
			"_CRT_SECURE_NO_WARNINGS",
			"_WINSOCK_DEPRECATED_NO_WARNINGS",
		});
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"RakNet/Private",
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add other private dependencies that you statically link with here ...
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
