// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "BoundaryBox.h"
#include "BoundaryManager.h"


// Sets default values
ABoundaryManager::ABoundaryManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoundaryManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABoundaryManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABoundaryManager::SignalBOundariesToServer()
{
	for (TActorIterator<ABoundaryBox> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		if (*ActorItr != nullptr)
		{
			ABoundaryBox *box = *ActorItr;
			break;
		}

	}
}

