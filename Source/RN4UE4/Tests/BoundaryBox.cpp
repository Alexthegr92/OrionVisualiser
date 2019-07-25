// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "RakNetRP.h"
#include "BoundaryBox.h"


// Sets default values
ABoundaryBox::ABoundaryBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
}

// Called when the game starts or when spawned
void ABoundaryBox::BeginPlay()
{
	Super::BeginPlay();
}
/*
void ABoundaryBox::SetBoundary()
{
	// Box trigger for self
	FVector pos = GetActorLocation();
	FVector size = GetActorScale3D();
	rakNetManager->RPrpcSignalBoundaryBox(FVector(pos.X, pos.Z, pos.Y), FVector(size.X, size.Z, size.Y), rank);
}*/

// Called every frame
void ABoundaryBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

