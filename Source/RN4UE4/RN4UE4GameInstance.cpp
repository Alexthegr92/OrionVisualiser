// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4GameInstance.h"
#include "RN4UE4.h"
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"

TArray<FName> URN4UE4GameInstance::GetAllMapNames() const
{
	TArray<FName> Names = TArray<FName>();

	for (ULevelStreaming* level : GetWorld()->StreamingLevels)
	{
		Names.Add(FPackageName::GetShortFName(level->GetWorldAssetPackageFName()));
	}

	return Names;
}

FString URN4UE4GameInstance::RemoveMapPrefix(FString mapName) const
{
	mapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	return mapName;
}
