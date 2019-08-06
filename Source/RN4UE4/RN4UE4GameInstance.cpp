// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "RN4UE4GameInstance.h"




TArray<FName> URN4UE4GameInstance::GetAllMapNames() {
	TArray<FName> Names = TArray<FName>();

	for (int32 i = 0; i < GetWorld()->StreamingLevels.Num(); ++i)
	{
		Names.Add(FPackageName::GetShortFName(GetWorld()->StreamingLevels[i]->GetWorldAssetPackageFName()));
	}
	levelNames = Names;
	return Names;
}