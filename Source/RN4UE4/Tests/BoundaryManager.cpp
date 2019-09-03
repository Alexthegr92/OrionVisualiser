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

	if (createCustomBoundariesBoxes && (!BoundariesChecked && rakNetManager->GetAllServersChecked()))
	{
		ensureMsgf(CheckServersNumber(), TEXT("Number of servers connected and boundaries boxes created aren't the same"));
		ensureMsgf(CheckBoxesHaveDifferentRanks(), TEXT("There are more than one box using the same rank value"));
		BoundariesChecked = true;
	}
}

void ABoundaryManager::SignalBoundariesToServer(const SystemAddress address) const
{
	TArray<FVector> Positions;
	TArray<FVector> Sizes;
	TArray<int>     Ranks;

	for (TActorIterator<ABoundaryBox> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (*ActorItr == nullptr) continue;

		ABoundaryBox* BoundaryBox = *ActorItr;
		if (BoundaryBox == nullptr) continue;

		TArray<UBoxComponent*> Components;
		BoundaryBox->GetComponents<UBoxComponent>(Components);

		ensureMsgf(Components.Num() == 1, TEXT("BoundaryManager - Boundary box without exactly one box component on it"));

		UBoxComponent* BoxComponent = Components[0];
		const FVector  BoxSize      = BoxComponent->GetScaledBoxExtent() / 50.0f;

		const FVector ThisBoxComponentPosition = BoundaryBox->GetActorLocation() / 50.0f;
		Positions.Add(FVector(ThisBoxComponentPosition.X, ThisBoxComponentPosition.Z, ThisBoxComponentPosition.Y));
		Sizes.Add(FVector(BoxSize.X, BoxSize.Z, BoxSize.Y));
		Ranks.Add(BoundaryBox->Rank);
	}

	RPrpcSignalBoundaryBox(Positions, Sizes, Ranks, address);
}

void ABoundaryManager::RPrpcSignalBoundaryBox(const TArray<FVector>& pos, const TArray<FVector>& size, const TArray<int>& ranks,
											const SystemAddress      address) const
{
	BitStream testBs;
	testBs.Write<int>(pos.Num());

	for (int i = 0; i < pos.Num(); i++)
	{
		testBs.WriteVector<float>(pos[i].X, pos[i].Y, pos[i].Z);
		testBs.WriteVector<float>(size[i].X, size[i].Y, size[i].Z);
		testBs.Write<int>(ranks[i]);
	}

	rakNetManager->GetRpc()->Signal("CreateBoundaryVisualizer", &testBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, address, false,
									false);
}

bool ABoundaryManager::CheckServersNumber() const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoundaryBox::StaticClass(), FoundActors);

	return rakNetManager->getNumberServers() == FoundActors.Num();
}

bool ABoundaryManager::CheckBoxesHaveDifferentRanks() const
{
	UWorld* world = GetWorld();
	for (TActorIterator<ABoundaryBox> ActorItr(world); ActorItr; ++ActorItr)
	{
		ABoundaryBox* Box = *ActorItr;
		if (Box == nullptr) continue;

		for (TActorIterator<ABoundaryBox> ActorItr2(world); ActorItr2; ++ActorItr2)
		{
			ABoundaryBox* Box2 = *ActorItr2;
			if (Box2 == nullptr || Box == Box2 || Box->Rank != Box2->Rank) continue;

			return false;
		}
	}

	return true;
}
