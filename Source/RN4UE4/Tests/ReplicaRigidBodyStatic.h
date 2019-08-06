// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RakNetRP.h"
#include "ReplicaRigidBodyStatic.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RN4UE4_API UReplicaRigidBodyStatic : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UReplicaRigidBodyStatic();

	UPROPERTY(EditAnywhere, Category = "Raknet")
		ARakNetRP*		rakNetManager;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GetAllVertices(int &nbMeshes, TArray<FNestedArray> &vertices);
	void SignalData(int &nbMeshes, TArray<FNestedArray> &vertices);

private:
	bool registered = false;
	UStaticMeshComponent* orionMesh = nullptr;
	
};
