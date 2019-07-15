// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
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

void ABoundaryBox::setBoundary()
{
	// Box trigger for self
	FVector pos = GetActorLocation();
	FVector extents = BoxComponent->GetUnscaledBoxExtent();
	extents = extents / 2.0f;

	//PxRigidStatic* rankZero = helloWorld->createBoundaryTriggerBox(posZero, halfExtents);

	/*helloWorld->createBoundaryTriggerForSelf(rankZero);
	helloWorld->setCellPos(posZero);
	helloWorld->createBoundaryTriggerForRank(rankOne, 1);
	PxRigidStatic* rankTwo = helloWorld->createBoundaryTriggerPlane(posTwo, planeNormal);
	helloWorld->createBoundaryTriggerForRank(rankTwo, 2);
	helloWorld->addAuraExchange(1, 1);
	helloWorld->addAuraExchange(2, 2);

	if (multiAuraSend)
	{
		helloWorld->addAuraExchange(1, 2);
		helloWorld->addAuraExchange(2, 1);
	}*/


}

// Called every frame
void ABoundaryBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

