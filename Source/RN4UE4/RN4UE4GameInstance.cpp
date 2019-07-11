// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "RN4UE4GameInstance.h"




TArray<FName> URN4UE4GameInstance::GetAllMapNames() {
	auto ObjectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, true);
	ObjectLibrary->LoadAssetDataFromPath(TEXT("/Game/Maps"));
	TArray<FAssetData> AssetDatas;
	ObjectLibrary->GetAssetDataList(AssetDatas);
	UE_LOG(LogTemp, Warning, TEXT("Found maps: %d"), AssetDatas.Num());

	TArray<FName> Names = TArray<FName>();

	for (int32 i = 0; i < AssetDatas.Num(); ++i)
	{
		FAssetData& AssetData = AssetDatas[i];
		Names.Add(AssetData.AssetName);
	}
	levelNames = Names;
	return Names;
}