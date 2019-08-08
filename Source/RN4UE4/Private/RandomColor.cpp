// Fill out your copyright notice in the Description page of Project Settings.

#include "RandomColor.h"
#include "Engine.h"

// Sets default values
ARandomColor::ARandomColor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMesh"));
}

// Called when the game starts or when spawned
void ARandomColor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARandomColor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UMaterialInstanceDynamic* DynamicMatInstance = StaticMesh->CreateAndSetMaterialInstanceDynamic(0);
	FLinearColor RandomColor;
	RandomColor.R = FMath::RandRange(0, 1);
	RandomColor.G = FMath::RandRange(0, 1);
	RandomColor.B = FMath::RandRange(0, 1);
	RandomColor.A = FMath::RandRange(0, 1);

	//If we have a valid dynamic material instance, modify its parameters
	if (DynamicMatInstance)
	{
		DynamicMatInstance->SetVectorParameterValue(FName("ColorR"), RandomColor);
		DynamicMatInstance->SetScalarParameterValue(FName("MetalParam"), FMath::RandRange(0, 1));
	}
}

