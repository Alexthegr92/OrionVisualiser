// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReplicaRigidDynamic.h"
#include "RakNetRP.h"
#include "ReplicaRigidDynamicClient.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RN4UE4_API UReplicaRigidDynamicClient : public UActorComponent, public ReplicaRigidDynamic
{
	GENERATED_BODY()

public:	
	// FIXME: This suppresses warnings about hiding an inherited function; we un-hide it here
	using UActorComponent::Serialize;

	// Sets default values for this component's properties
	UReplicaRigidDynamicClient();

	UPROPERTY(EditAnywhere, Category = "Raknet")
		ARakNetRP*		rakNetManager;

	void OnConstruction(const RigidDynamicConstructionData& data);

	UPROPERTY(EditAnywhere, Category = "Server0Material")
		UMaterial* server0Material;

	UPROPERTY(EditAnywhere, Category = "Server1Material")
		UMaterial* server1Material;

	UPROPERTY(EditAnywhere, Category = "Server2Material")
		UMaterial* server2Material;

	UPROPERTY(EditAnywhere, Category = "Server3Material")
		UMaterial* server3Material;

	UPROPERTY(EditAnywhere, Category = "UnknownMaterial")
		UMaterial* unknownMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "SphereBP")
		TSubclassOf<AStaticMeshActor> sphereBP;

	UPROPERTY(EditDefaultsOnly, Category = "BoxBP")
		TSubclassOf<AStaticMeshActor> boxBP;

	UPROPERTY(EditDefaultsOnly, Category = "CapsuleBP")
		TSubclassOf<AStaticMeshActor> capsuleBP;

	UPROPERTY(EditAnywhere, Category = "VisualRepresentation")
		AActor* visual = nullptr;

	virtual RakString GetName() const { return RakString("ReplicaRigidDynamic"); }
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		return RM3SR_DO_NOT_SERIALIZE;
	}
	virtual RM3ConstructionState QueryConstruction(Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ClientConstruction(destinationConnection, false);
	}
	virtual bool QueryRemoteConstruction(Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_ClientConstruction(sourceConnection, false);
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

	virtual RigidDynamicConstructionData GetConstructionData() override;
	virtual void Deserialize(DeserializeParameters* deserializeParameters) override;

	void OnPoppedConnection(Connection_RM3* droppedConnection) override;

	virtual bool DeserializeDestruction(BitStream *destructionBitstream, Connection_RM3 *sourceConnection);

	void UpdateTransform();
		void SetVisual(physx::PxGeometryType::Enum geomType);
	void SetSpawned(bool spa);
	void SetMaterial(int32 elementIndex, UMaterialInterface* inMaterial);
	virtual void PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	bool registered;
	bool spawned = false;
};
