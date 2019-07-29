// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoundaryManager.generated.h"

class ABoundaryBox;
class ARakNetRP;
UCLASS()
class RN4UE4_API ABoundaryManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoundaryManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SignalBoundariesToServer();

	UPROPERTY(EditAnywhere, Category = "Raknet")
		ARakNetRP*		rakNetManager;
	UPROPERTY(EditAnywhere, Category = "Raknet")
		TArray<ABoundaryBox*> boxes;
	UPROPERTY(EditAnywhere, Category = "Raknet")
		bool multiAuras;

private:
	bool boundariesSent = false;
	
};
