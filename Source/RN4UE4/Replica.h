// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RakNet.h"
#include "RakNetTypes.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "ReplicaManager3.h"
#include "NetworkIDManager.h"
#include "VariableDeltaSerializer.h"
#include "GetTime.h"
#include "Rand.h"

#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "ReplicaRigidDynamic.h"
#include "Replica.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(RakNet_Replica, Log, All);

using namespace RakNet;

UCLASS()
class RN4UE4_API AReplica : public AActor, public ReplicaRigidDynamic
{
	GENERATED_BODY()

public:
	// FIXME: This suppresses warnings about hiding inherited functions; we un-hide them here
	using AActor::OnConstruction;
	using AActor::Serialize;

	// Sets default values for this actor's properties
	AReplica();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	void OnConstruction(const RigidDynamicConstructionData& data);

	//UPROPERTY(VisibleAnywhere)
	//	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "SphereBP")
		TSubclassOf<AStaticMeshActor> sphereBP;

	UPROPERTY(EditDefaultsOnly, Category = "BoxBP")
		TSubclassOf<AStaticMeshActor> boxBP;

	UPROPERTY(EditDefaultsOnly, Category = "CapsuleBP")
		TSubclassOf<AStaticMeshActor> capsuleBP;

	UPROPERTY(EditDefaultsOnly, Category = "Server0Material")
		UMaterial* server0Material;

	UPROPERTY(EditDefaultsOnly, Category = "Server1Material")
		UMaterial* server1Material;

	UPROPERTY(EditDefaultsOnly, Category = "Server2Material")
		UMaterial* server2Material;

	UPROPERTY(EditDefaultsOnly, Category = "Server3Material")
		UMaterial* server3Material;

	UPROPERTY(EditDefaultsOnly, Category = "UnknownMaterial")
		UMaterial* unknownMaterial;

	virtual RakString GetName(void) const { return RakString("ReplicaRigidDynamic"); }
	virtual RM3SerializationResult Serialize(SerializeParameters *serializeParameters)
	{
		return RM3SR_DO_NOT_SERIALIZE;
	}
	virtual RM3ConstructionState QueryConstruction(Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {
		return QueryConstruction_ServerConstruction(destinationConnection, false);
	}
	virtual bool QueryRemoteConstruction(Connection_RM3 *sourceConnection) {
		return QueryRemoteConstruction_ServerConstruction(sourceConnection, false);
	}
	virtual RM3QuerySerializationResult QuerySerialization(Connection_RM3 *destinationConnection) {
		return QuerySerialization_ServerSerializable(destinationConnection, false);
	}
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection(Connection_RM3 *droppedConnection) const {
		return QueryActionOnPopConnection_Server(droppedConnection);
	}
	virtual void DeallocReplica(Connection_RM3 *sourceConnection) {
		Destroy();
	}

	virtual void Deserialize(DeserializeParameters *deserializeParameters);
	virtual bool DeserializeDestruction(BitStream *destructionBitstream, Connection_RM3 *sourceConnection);

	void UpdateTransform();
	void SetMaterial(int32 elementIndex, UMaterialInterface* inMaterial);


	virtual void PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) override;

private:
	AActor* visual = nullptr;
};
