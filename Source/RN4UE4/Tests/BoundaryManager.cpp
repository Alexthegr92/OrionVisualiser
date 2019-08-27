// Fill out your copyright notice in the Description page of Project Settings.

#include "BoundaryManager.h"
#include "RN4UE4.h"
#include "EngineUtils.h"
#include "BoundaryBox.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "../RakNetRP.h"
#include "RN4UE4GameInstance.h"

using namespace std::placeholders;

// Sets default values
ABoundaryManager::ABoundaryManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called every frame
void ABoundaryManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (rakNetManager == nullptr)
	{
		URN4UE4GameInstance* GameInstance = static_cast<URN4UE4GameInstance*>(GetGameInstance());
		ensureMsgf(GameInstance != nullptr, TEXT("RakNetRP - GameInstance is not of type URN4UE4GameInstance"));
		rakNetManager = GameInstance->GetRakNetManager();
		ensure(rakNetManager);

		const auto newConnection = std::bind(&ABoundaryManager::SignalBoundariesToServer, this, _1);
		rakNetManager->SetNewConnectionCallback(newConnection);
	}

	if (createCustomBoundariesBoxes && (!boundariesSent && rakNetManager->GetAllServersChecked()))
	{
		ensureMsgf(CheckServersNumber(), TEXT("Number of servers connected and boundaries boxes created aren't the same"));
		ensureMsgf(CheckBoxesHaveDifferentRanks(), TEXT("There are more than one box using the same rank value"));
		boundariesSent = true;
	}
}

void ABoundaryManager::SignalBoundariesToServer(const RakNet::SystemAddress address)
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

	RPrpcSignalBoundaryBox(pos, size, ranks, address);
}

void ABoundaryManager::RPrpcSignalBoundaryBox(const TArray<FVector> pos, const TArray<FVector> size, const TArray<int> ranks, const RakNet::SystemAddress address)
{
	RakNet::BitStream testBs;
	testBs.Write<int>(pos.Num());

	for (int i = 0; i < pos.Num(); i++)
	{
		testBs.WriteVector<float>(pos[i].X, pos[i].Y, pos[i].Z);
		testBs.WriteVector<float>(size[i].X, size[i].Y, size[i].Z);
		testBs.Write<int>(ranks[i]);
	}

	rakNetManager->GetRpc()->Signal("CreateBoundaryVisualizer", &testBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false, false);
}

bool ABoundaryManager::CheckServersNumber()
{
	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoundaryBox::StaticClass(), foundActors);
	return rakNetManager->getNumberServers() == foundActors.Num();
}

bool ABoundaryManager::CheckBoxesHaveDifferentRanks()
{
	for (TActorIterator<ABoundaryBox> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ABoundaryBox *box = *ActorItr;
		if (box)
		{
			for (TActorIterator<ABoundaryBox> ActorItr2(GetWorld()); ActorItr2; ++ActorItr2)
			{
				ABoundaryBox *box2 = *ActorItr2;
				if (box2)
				{
					if (box != box2)
					{
						if (box->rank == box2->rank)
							return false;
					}
				}
			}
		}
	}
	return true;
}

