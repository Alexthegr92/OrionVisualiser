// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RandomColor.generated.h"

UCLASS()
class RN4UE4_API ARandomColor : public AActor
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* StaticMesh;
public:	
	// Sets default values for this actor's properties
	ARandomColor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
