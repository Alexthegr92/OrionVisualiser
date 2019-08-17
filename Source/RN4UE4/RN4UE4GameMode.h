// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "RN4UE4GameMode.generated.h"

class ARakNetRP;
/**
 * 
 */
UCLASS()
class RN4UE4_API ARN4UE4GameMode : public AGameMode
{
	GENERATED_BODY()
	
public:

	void RegisterRakNetManager(ARakNetRP* Manager)
	{
		RakNetManager = Manager;
	}

	ARakNetRP* GetRakNetManager() const
	{
		return RakNetManager;
	}

private:
		ARakNetRP* RakNetManager;	
	
};
