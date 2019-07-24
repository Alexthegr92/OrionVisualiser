#include "ReplicaRigidDynamicClient.h"
#include "Replica.h"
#include "PhysXIncludes.h" 

UReplicaRigidDynamicClient::UReplicaRigidDynamicClient()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReplicaRigidDynamicClient::BeginPlay()
{
	Super::BeginPlay();

	registered = false;
}

void UReplicaRigidDynamicClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!registered && rakNetManager!=nullptr && rakNetManager->GetInitialised() && !spawned)
	{
		rakNetManager->Reference(this);
		registered = true;
	}
}

void UReplicaRigidDynamicClient::OnConstruction(const RigidDynamicConstructionData & data)
{
	physx::PxGeometryType::Enum geomType = static_cast<physx::PxGeometryType::Enum>(data.geom);
	AReplica* rep = dynamic_cast<AReplica*>(GetOwner());
	if(rep)
		rep->SetVisual(geomType);

	pos = data.pos;
	rot = data.rot;

	UpdateTransform();
}

RigidDynamicConstructionData UReplicaRigidDynamicClient::GetConstructionData()
{
	FTransform actorTransform = GetOwner()->GetActorTransform();

	// Conversion matrix from PhysX to Unreal
	float	matrixElements[16] = {
		1,  0, 0, 0,
		0,  0, 1, 0,
		0,  1, 0, 0,
		0,  0, 0, 1
	};

	FMatrix conversionMatrix = FMatrix();
	memcpy(conversionMatrix.M, matrixElements, 16 * sizeof(float));

	actorTransform *= FTransform(conversionMatrix.Inverse());
	actorTransform.ScaleTranslation(1 / 50.0f);

	RigidDynamicConstructionData data;
	data.geom = 0;
	data.pos.X = -actorTransform.GetLocation().X;
	data.pos.Y = actorTransform.GetLocation().Y;
	data.pos.Z = actorTransform.GetLocation().Z;
	data.rot.X = actorTransform.GetRotation().X;
	data.rot.Y = actorTransform.GetRotation().Y;
	data.rot.Z = actorTransform.GetRotation().Z;
	data.rot.W = actorTransform.GetRotation().W;

	return data;
}

void UReplicaRigidDynamicClient::Deserialize(DeserializeParameters* deserializeParameters)
{
	ReplicaRigidDynamic::Deserialize(deserializeParameters);
	UpdateTransform();
}

void UReplicaRigidDynamicClient::OnPoppedConnection(Connection_RM3 * droppedConnection)
{
	ReplicaBase::OnPoppedConnection(droppedConnection);
	GetOwner()->Destroy();
}

void UReplicaRigidDynamicClient::UpdateTransform()
{
	// Conversion matrix from PhysX to Unreal
	float	matrixElements[16] = {
		1,  0, 0, 0,
		0,  0, 1, 0,
		0,  1, 0, 0,
		0,  0, 0, 1
	};

	FMatrix conversionMatrix = FMatrix();
	memcpy(conversionMatrix.M, matrixElements, 16 * sizeof(float));

	FQuat	newRot = FQuat(rot.X, rot.Y, rot.Z, rot.W);
	FVector newPos = FVector(pos.X, pos.Y, pos.Z) * 50.0f;
	FVector scale = FVector(-1, 1, 1);					// X will get negated, so set scale to -1, so final result is 1
	FTransform transform = FTransform(FRotator(newRot), newPos, scale);
	transform *= FTransform(conversionMatrix);
	GetOwner()->SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}

bool UReplicaRigidDynamicClient::DeserializeDestruction(BitStream *destructionBitstream, Connection_RM3 *sourceConnection)
{
	GetOwner()->Destroy();
	return true;
}

void UReplicaRigidDynamicClient::SetSpawned(bool spa)
{
	spawned = spa;
}

void UReplicaRigidDynamicClient::SetMaterial(int32 elementIndex, UMaterialInterface* inMaterial)
{
	TArray<UStaticMeshComponent*> components;
	
	AReplica* rep = dynamic_cast<AReplica*>(GetOwner());
	if (rep) {
		rep->visual->GetComponents<UStaticMeshComponent>(components);
		for (int32 i = 0; i < components.Num(); i++)
		{
			UStaticMeshComponent* StaticMeshComponent = components[i];
			if (StaticMeshComponent)
			{
				StaticMeshComponent->SetMaterial(elementIndex, inMaterial);
			}
		}
	}
}

void UReplicaRigidDynamicClient::PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection)
{
	unsigned short port = sourceConnection->GetSystemAddress().GetPort();
	int rank = port - 12345;

	switch (rank)
	{
	case 0:
		SetMaterial(0, server0Material);
		break;
	case 1:
		SetMaterial(0, server1Material);
		break;
	case 2:
		SetMaterial(0, server2Material);
		break;
	case 3:
		SetMaterial(0, server3Material);
		break;
	default:
		SetMaterial(0, unknownMaterial);
		break;
	}
}
