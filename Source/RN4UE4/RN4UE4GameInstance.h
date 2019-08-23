// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RakNetRP.h"
#include "RN4UE4GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class RN4UE4_API URN4UE4GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Levels")
		TArray<FName> GetAllMapNames() const;

	UFUNCTION(BlueprintCallable, Category = "Levels")
		FString RemoveMapPrefix(FString mapName) const;

	UFUNCTION(BlueprintCallable, Category = "Levels")
		void LoadStreamLevelFromIndex(int index);

	UFUNCTION(BlueprintCallable, Category = "Levels")
		void ReloadCurrentStreamLevel();

	void RegisterRakNetManager(ARakNetRP* Manager)
	{
		RakNetManager = Manager;
	}

	UFUNCTION(BlueprintCallable, Category = "RakNet")
		ARakNetRP* GetRakNetManager() const
		{
			return RakNetManager;
		}

private:
	ARakNetRP* RakNetManager;

	// TODO: Store this in a level changer class
	int IndexLevel;
};
