// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "PhysXIncludes.h" 
#include "ReplicaRigidDynamic.h"
#include "RakNetRP.h"
#include "Materials/Material.h"
#include "ReplicaRigidDynamicClient.generated.h"


UCLASS( Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RN4UE4_API UReplicaRigidDynamicClient : public UActorComponent, public ReplicaRigidDynamic
{
	GENERATED_BODY()

public:	
	// FIXME: This suppresses warnings about hiding an inherited function; we un-hide it here
	using UActorComponent::Serialize;

	// Sets default values for this component's properties
	UReplicaRigidDynamicClient();

	void OnConstruction(const RigidDynamicConstructionData& Data) override;

	UPROPERTY(EditAnywhere, Category = "ServerMaterials")
		UMaterial* Server0Material;

	UPROPERTY(EditAnywhere, Category = "ServerMaterials")
		UMaterial* Server1Material;

	UPROPERTY(EditAnywhere, Category = "ServerMaterials")
		UMaterial* Server2Material;

	UPROPERTY(EditAnywhere, Category = "ServerMaterials")
		UMaterial* Server3Material;

	UPROPERTY(EditAnywhere, Category = "ServerMaterials")
		UMaterial* UnknownMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "ReplicaActors")
		TSubclassOf<AStaticMeshActor> SphereBP;

	UPROPERTY(EditDefaultsOnly, Category = "ReplicaActors")
		TSubclassOf<AStaticMeshActor> BoxBP;

	UPROPERTY(EditDefaultsOnly, Category = "ReplicaActors")
		TSubclassOf<AStaticMeshActor> CapsuleBP;

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
	void UpdateTransform();

	bool DeserializeDestruction(BitStream *DestructionBitstream, Connection_RM3 *SourceConnection) override;

	void SetVisual(const PxGeometryType::Enum GeomType);
	void SetMaterial(int32 ElementIndex, UMaterialInterface* InMaterial) const;
	void PostDeserializeConstruction(BitStream *ConstructionBitstream, Connection_RM3 *SourceConnection) override;
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Used so server created replicas don't get referenced twice
	void MarkAsReferenced() { registered = true; }
	
private:
	void DestroyThis() const;

	bool registered;

	ARakNetRP*		rakNetManager;
	
	AActor* visual = nullptr;
};
