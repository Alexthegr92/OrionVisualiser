// Fill out your copyright notice in the Description page of Project Settings.

#include "BoundaryManager.h"
#include "RN4UE4.h"
#include "EngineUtils.h"
#include "BoundaryBox.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "../RakNetRP.h"


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
	
	ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));
}

// Called every frame
void ABoundaryManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (createCustomBoundariesBoxes)
	{
		if (!boundariesSent && ensure(rakNetManager) && rakNetManager->GetInitialised() && rakNetManager->GetAllServersChecked()) {
			if (CheckServersNumber())
			{
				rakNetManager->SetCustomBoundariesCreated(false);
				SignalBoundariesToServer();
				boundariesSent = true;
			}
		}
	}
}

void ABoundaryManager::SignalBoundariesToServer()
{
	TArray<FVector> pos;
	TArray<FVector> size;
	TArray<int> ranks;
	for (TActorIterator<ABoundaryBox> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (*ActorItr != nullptr)
		{
			ABoundaryBox *box = *ActorItr;
			if (box)
			{
				FVector sizeBox;
				TArray<UPrimitiveComponent*> comps;
				box->GetComponents(comps);
				for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
				{
					UBoxComponent* box = Cast<UBoxComponent>(*Iter);
					if (box)
					{
						sizeBox = box->GetScaledBoxExtent() / 50.0f;
					}
				}
				FVector position = box->GetActorLocation() / 50.0f;
				pos.Add(FVector(position.X,position.Z,position.Y));
				size.Add(FVector(sizeBox.X, sizeBox.Z, sizeBox.Y));
				ranks.Add(box->rank);
			}
		}

	}
	rakNetManager->RPrpcSignalBoundaryBox(pos, size, ranks, multiAuras, errorTolerance);
}

bool ABoundaryManager::CheckServersNumber()
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoundaryBox::StaticClass(), foundActors);
	return rakNetManager->getNumberServers() == foundActors.Num();
}

