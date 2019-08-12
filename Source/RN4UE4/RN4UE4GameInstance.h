// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
};
