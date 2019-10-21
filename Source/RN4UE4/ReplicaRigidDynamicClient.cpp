#include "ReplicaRigidDynamicClient.h"
#include "Engine/StaticMesh.h"
#include "RN4UE4GameInstance.h"
#include "Util.h"

UReplicaRigidDynamicClient::UReplicaRigidDynamicClient()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReplicaRigidDynamicClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (rakNetManager == nullptr)
	{
		URN4UE4GameInstance* GameInstance = static_cast<URN4UE4GameInstance*>(GetOwner()->GetGameInstance());
		ensureMsgf(GameInstance != nullptr, TEXT("RakNetRP - GameInstance is not of type URN4UE4GameInstance"));
		rakNetManager = GameInstance->GetRakNetManager();
	}
	
	if (!WasReferenced() && ensure(rakNetManager) && rakNetManager->GetInitialised())
	{
		rakNetManager->Reference(this);
	}
}

void UReplicaRigidDynamicClient::OnConstruction(const RigidDynamicConstructionData& Data)
{
	const PxGeometryType::Enum GeomType = static_cast<PxGeometryType::Enum>(Data.geom);
	SetVisual(GeomType);

	pos = Data.pos;
	rot = Data.rot;

	UpdateTransform();
}

RigidDynamicConstructionData UReplicaRigidDynamicClient::GetConstructionData()
{
	FTransform actorTransform = Util::UnrealToPhysX(GetOwner()->GetActorTransform());

	RigidDynamicConstructionData data;
	data.geom = 0;
	data.pos = Util::ToVec3(actorTransform.GetLocation());
	data.rot = Util::ToQuat(actorTransform.GetRotation());

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
	DestroyThis();
}

void UReplicaRigidDynamicClient::UpdateTransform()
{
	FTransform transform = FTransform(FRotator(Util::ToFQuat(rot)), Util::ToFVector(pos));
	GetOwner()->SetActorTransform(Util::PhysXToUnreal(transform), false, nullptr, ETeleportType::TeleportPhysics);
}

bool UReplicaRigidDynamicClient::DeserializeDestruction(BitStream *DestructionBitstream, Connection_RM3 *SourceConnection)
{
	DestroyThis();

	return true;
}

void UReplicaRigidDynamicClient::DestroyThis() const
{
	if (visual != nullptr)
	{
		visual->Destroy();
	}

	GetOwner()->Destroy();
}

void UReplicaRigidDynamicClient::SetMaterial(int32 ElementIndex, UMaterialInterface* InMaterial) const
{
	TArray<UStaticMeshComponent*> Components;
	visual->GetComponents<UStaticMeshComponent>(Components);
	for (int32 i = 0; i < Components.Num(); i++)
	{
		UStaticMeshComponent* StaticMeshComponent = Components[i];
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetMaterial(ElementIndex, InMaterial);
		}
	}
}

void UReplicaRigidDynamicClient::PostDeserializeConstruction(BitStream* ConstructionBitstream, Connection_RM3* SourceConnection)
{
	check(Server0Material != nullptr);
	check(Server1Material != nullptr);
	check(Server2Material != nullptr);
	check(Server3Material != nullptr);
	check(UnknownMaterial != nullptr);
	
	const unsigned short Port = SourceConnection->GetSystemAddress().GetPort();
	const int Rank = Port - 12345;

	switch (Rank)
	{
	case 0:
		SetMaterial(0, Server0Material);
		break;
	case 1:
		SetMaterial(0, Server1Material);
		break;
	case 2:
		SetMaterial(0, Server2Material);
		break;
	case 3:
		SetMaterial(0, Server3Material);
		break;
	default:
		SetMaterial(0, UnknownMaterial);
		break;
	}
}


void UReplicaRigidDynamicClient::SetVisual(const PxGeometryType::Enum GeomType)
{
	check(SphereBP != nullptr);
	check(CapsuleBP != nullptr);
	check(BoxBP != nullptr);
	
	AStaticMeshActor* Shape = nullptr;

	switch (GeomType)
	{
	case PxGeometryType::eSPHERE:
	{
		if (SphereBP == nullptr) break;

		Shape = Cast<AStaticMeshActor>(SphereBP->GetDefaultObject());
	}
	break;
	case PxGeometryType::ePLANE:
		break;
	case PxGeometryType::eCAPSULE:
	{
		if (CapsuleBP == nullptr) break;

		Shape = Cast<AStaticMeshActor>(CapsuleBP->GetDefaultObject());
	}
	break;
	case PxGeometryType::eBOX:
	{
		if (BoxBP == nullptr) break;
		Shape = Cast<AStaticMeshActor>(BoxBP->GetDefaultObject());
	}
	break;
	case PxGeometryType::eCONVEXMESH:
		break;
	case PxGeometryType::eTRIANGLEMESH:
		break;
	case PxGeometryType::eHEIGHTFIELD:
		break;
	case PxGeometryType::eGEOMETRY_COUNT:
		break;
	case PxGeometryType::eINVALID:
		break;
	default:
		break;
	}

	checkf(Shape != nullptr, TEXT("UReplicaRigidDynamicClient::SetVisual() - geometry type not supported: %s"), GeomType);

	FActorSpawnParameters Parameters = FActorSpawnParameters();
	FTransform SpawnTransform = FTransform();
	Parameters.Template = Shape;
	visual = GetWorld()->SpawnActor(Shape->GetClass(), &SpawnTransform, Parameters);

	check(visual != nullptr);
	
	visual->AttachToActor(GetOwner(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));
}

