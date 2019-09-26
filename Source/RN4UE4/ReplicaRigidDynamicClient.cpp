#include "ReplicaRigidDynamicClient.h"
#include "RN4UE4GameInstance.h"
#include "Engine/World.h"

UReplicaRigidDynamicClient::UReplicaRigidDynamicClient()
{
	PrimaryComponentTick.bCanEverTick = true;
	registered = false;
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
	
	if (!registered && ensure(rakNetManager) && rakNetManager->GetInitialised())
	{
		rakNetManager->Reference(this);
		registered = true;
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
	DestroyThis();
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

