// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "RN4UE4GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "ChangeLevelsComponent.h"


// Sets default values for this component's properties
UChangeLevelsComponent::UChangeLevelsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UChangeLevelsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UChangeLevelsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	URN4UE4GameInstance * instance = dynamic_cast<URN4UE4GameInstance*>(GetWorld()->GetGameInstance());
	if (instance)
	{
		if (instance->timeToChangeLevel > 0)
		{
			if (currentTime > instance->timeToChangeLevel)
			{
				if (instance->indexLevel < instance->levelNames.Num()-1)
				{
					FLatentActionInfo LatentInfo1;
					UGameplayStatics::UnloadStreamLevel(this, instance->levelNames[instance->indexLevel], LatentInfo1);
					instance->indexLevel++;
					FLatentActionInfo LatentInfo2;
					UGameplayStatics::LoadStreamLevel(this, instance->levelNames[instance->indexLevel], true, true, LatentInfo2);
				}
			}
		}
		if (instance->indexLevel >= instance->levelNames.Num())
			currentTime += DeltaTime;
	}
}

