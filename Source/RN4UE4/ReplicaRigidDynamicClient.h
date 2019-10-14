// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ReplicaRigidDynamic.h"
#include "RakNetRP.h"
#include "ReplicaRigidDynamicClient.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RN4UE4_API UReplicaRigidDynamicClient : public USceneComponent, public ReplicaRigidDynamic
{
	GENERATED_BODY()

public:	
	// FIXME: This suppresses warnings about hiding an inherited function; we un-hide it here
	using UActorComponent::Serialize;

	// Sets default values for this component's properties
	UReplicaRigidDynamicClient();

	virtual RakString GetName() const { return RakString("ReplicaRigidDynamic"); }
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		return RM3SR_DO_NOT_SERIALIZE;
	}
	virtual RM3ConstructionState QueryConstruction(Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ClientConstruction(destinationConnection, false);
	}
	virtual bool QueryRemoteConstruction(Connection_RM3 *sourceConnection) {
		return false;
	}
	virtual RM3QuerySerializationResult QuerySerialization(Connection_RM3 *destinationConnection) {
		return QuerySerialization_ServerSerializable(destinationConnection, false);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_Client(droppedConnection);
	}
	virtual void DeallocReplica(Connection_RM3 *sourceConnection) {
		GetOwner()->Destroy();
	}

	void GetParentComponent();
	void CenterToMesh(RigidDynamicConstructionData& data);
	void ReadPhysicValues(RigidDynamicConstructionData& data);
	virtual RigidDynamicConstructionData GetConstructionData() override;
	virtual void Deserialize(DeserializeParameters* deserializeParameters) override;
	void OnPoppedConnection(Connection_RM3* droppedConnection) override;
	void UpdateTransform();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	bool registered;
	UStaticMeshComponent* orionMesh = nullptr;
	FTransform relativePos;
	FVector centerMass;
	bool attached = false;
	ARakNetRP*		rakNetManager;
};
