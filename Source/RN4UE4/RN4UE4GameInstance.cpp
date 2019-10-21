// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4GameInstance.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"

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

void URN4UE4GameInstance::LoadStreamLevelFromIndex(int index)
{
	TArray<FName> AllLevels = GetAllMapNames();

	if (AllLevels.IsValidIndex(IndexLevel))
	{
		FName CurrentlyLoadedLevel = AllLevels[IndexLevel];
		FLatentActionInfo info;
		info.UUID = 0;
		UGameplayStatics::UnloadStreamLevel(this, CurrentlyLoadedLevel, info);
	}

	if (AllLevels.IsValidIndex(index))
	{
		FName NewLevel = AllLevels[index];
		FLatentActionInfo info;
		info.UUID = 1;
		UGameplayStatics::LoadStreamLevel(this, NewLevel, true, true, info);
	}

	IndexLevel = index;
}

void URN4UE4GameInstance::ReloadCurrentStreamLevel()
{
	LoadStreamLevelFromIndex(IndexLevel);
}
