// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "BoundaryBox.h"
#include "../RakNetRP.h"
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
	if(rakNetManager != nullptr)
		if (!boundariesSent && rakNetManager->GetAllServersChecked()) {
			rakNetManager->SetCustomBoundariesCreated(false);
			SignalBoundariesToServer();
		}
	else
	{
		for (TActorIterator<ARakNetRP> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
			if (*ActorItr != nullptr)
			{
				ARakNetRP *rak = *ActorItr;
				rakNetManager = rak;
				break;
			}

		}
	}
}

void ABoundaryManager::SignalBoundariesToServer()
{
	TArray<FVector> pos;
	TArray<FVector> size;
	for (TActorIterator<ABoundaryBox> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (*ActorItr != nullptr)
		{
			ABoundaryBox *box = *ActorItr;
			if (box)
			{
				FVector position = box->GetActorLocation() / 50.0f;
				FVector sizeBox = box->GetActorScale3D();
				pos.Add(FVector(position.X,position.Z,position.Y));
				size.Add(FVector(sizeBox.X, sizeBox.Z, sizeBox.Y));
			}
		}

	}
	rakNetManager->RPrpcSignalBoundaryBox(pos, size, multiAuras);
	boundariesSent = true;
}

