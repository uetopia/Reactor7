// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Comp : ModuleRules
{
	public Comp(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay","Json",
                "JsonUtilities",
                "HTTP",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "LoginFlow",
                "UMG",
                "Slate",
                "SlateCore"
         });
        PrivateDependencyModuleNames.AddRange(new string[] { "LoginFlow" });
    }
}
