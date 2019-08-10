#include "ReplicaRigidDynamicClient.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

UReplicaRigidDynamicClient::UReplicaRigidDynamicClient()
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FClassFinder<AStaticMeshActor> SphereBlueprint(TEXT("/Game/Blueprints/Sphere"));
	if (SphereBlueprint.Succeeded()) {
		sphereBP = SphereBlueprint.Class;
	}
	static ConstructorHelpers::FClassFinder<AStaticMeshActor> BoxBlueprint(TEXT("/Game/Blueprints/Box"));
	if (BoxBlueprint.Succeeded()) {
		boxBP = BoxBlueprint.Class;
	}
	static ConstructorHelpers::FClassFinder<AStaticMeshActor> CapsuleBlueprint(TEXT("/Game/Blueprints/Capsule"));
	if (CapsuleBlueprint.Succeeded()) {
		capsuleBP = CapsuleBlueprint.Class;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MatFinder0(TEXT("Material'/Game/Materials/Server0'"));
	if (MatFinder0.Succeeded())
	{
		server0Material = MatFinder0.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterial> MatFinder1(TEXT("Material'/Game/Materials/Server1'"));
	if (MatFinder1.Succeeded())
	{
		server1Material = MatFinder1.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterial> MatFinder2(TEXT("Material'/Game/Materials/Server2'"));
	if (MatFinder2.Succeeded())
	{
		server2Material = MatFinder2.Object;
	}
	static ConstructorHelpers::FObjectFinder<UMaterial> MatFinder3(TEXT("Material'/Game/Materials/Server3'"));
	if (MatFinder3.Succeeded())
	{
		server3Material = MatFinder3.Object;
	}
}

void UReplicaRigidDynamicClient::BeginPlay()
{
	Super::BeginPlay();

	registered = false;
}

void UReplicaRigidDynamicClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));
	if (!registered && ensure(rakNetManager) && rakNetManager->GetInitialised() && !spawned)
	{
		rakNetManager->Reference(this);
		registered = true;
	}
}

void UReplicaRigidDynamicClient::OnConstruction(const RigidDynamicConstructionData & data)
{
	physx::PxGeometryType::Enum geomType = static_cast<physx::PxGeometryType::Enum>(data.geom);
	SetVisual(geomType);

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
	visual->GetComponents<UStaticMeshComponent>(components);
	for (int32 i = 0; i < components.Num(); i++)
	{
		UStaticMeshComponent* StaticMeshComponent = components[i];
		if (StaticMeshComponent)
		{
			StaticMeshComponent->SetMaterial(elementIndex, inMaterial);
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


void UReplicaRigidDynamicClient::SetVisual(physx::PxGeometryType::Enum geomType)
{
	FActorSpawnParameters Parameters = FActorSpawnParameters();
	FTransform SpawnTransform = FTransform();
	AStaticMeshActor* shape = nullptr;

	switch (geomType)
	{
	case physx::PxGeometryType::eSPHERE:
	{
		if (sphereBP == nullptr) break;

		shape = Cast<AStaticMeshActor>(sphereBP->GetDefaultObject());
	}
	break;
	case physx::PxGeometryType::ePLANE:
		break;
	case physx::PxGeometryType::eCAPSULE:
	{
		if (capsuleBP == nullptr) break;

		shape = Cast<AStaticMeshActor>(capsuleBP->GetDefaultObject());
	}
	break;
	case physx::PxGeometryType::eBOX:
	{
		if (boxBP == nullptr) break;
		shape = Cast<AStaticMeshActor>(boxBP->GetDefaultObject());
	}
	break;
	case physx::PxGeometryType::eCONVEXMESH:
		break;
	case physx::PxGeometryType::eTRIANGLEMESH:
		break;
	case physx::PxGeometryType::eHEIGHTFIELD:
		break;
	case physx::PxGeometryType::eGEOMETRY_COUNT:
		break;
	case physx::PxGeometryType::eINVALID:
		break;
	default:
		break;
	}
	AActor * ac = GetOwner();
	if (shape != nullptr)
	{
		Parameters.Template = shape;
		visual = GetWorld()->SpawnActor(shape->GetClass(), &SpawnTransform, Parameters);
	}

	if (visual != nullptr)
	{
		
		visual->AttachToActor(ac, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));
	}
}

