#include "ReplicaRigidDynamicClient.h"
#include "RN4UE4.h"
#include "Runtime/Engine/Classes/PhysicsEngine/AggregateGeom.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodySetup.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"
#include "Runtime/Engine/Classes/PhysicalMaterials/PhysicalMaterial.h"
#include "ThirdParty/PhysX3/PhysX_3.4/Include/PxRigidDynamic.h"
#include "ThirdParty/PhysX3/PhysX_3.4/Include/PxMaterial.h"
#include "PhysXIncludes.h" 
#include "PhysXPublic.h"
#include "PhysicsPublic.h"
#include "Engine/World.h"
#include "RN4UE4GameInstance.h"
#include "Engine/World.h"
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

void UReplicaRigidDynamicClient::ReadPhysicValues(RigidDynamicConstructionData& data)
{
	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
	PxScene* SyncScene = PhysScene->GetPhysXScene(PST_Sync);
	SCENE_LOCK_READ(SyncScene);
	PxRigidDynamic * rigid = orionMesh->GetBodyInstance()->GetPxRigidDynamic_AssumesLocked();
	if (rigid != nullptr)
	{
		data.mass = rigid->getMass() / 50.0f;
		data.angularDamping = rigid->getAngularDamping();
		data.linearDamping = rigid->getLinearDamping();
		data.gravityEnabled = orionMesh->IsGravityEnabled();
		FVector centerMass = FVector(rigid->getCMassLocalPose().p.x, rigid->getCMassLocalPose().p.y, rigid->getCMassLocalPose().p.z);
		data.centerMass = Vec3(centerMass.X, centerMass.Y, centerMass.Z);
		FQuat centerMassRot = FQuat(rigid->getCMassLocalPose().q.x, rigid->getCMassLocalPose().q.y, rigid->getCMassLocalPose().q.z, rigid->getCMassLocalPose().q.w);
		data.centerMassRot = Quat(centerMassRot.X, centerMassRot.Y, centerMassRot.Z, centerMassRot.W);
		data.maxAngularVelocity = rigid->getMaxAngularVelocity();
		data.maxDepenetrationVelocity = rigid->getMaxDepenetrationVelocity();
		data.minCCDAdvanceCoefficient = rigid->getMinCCDAdvanceCoefficient();
		data.isSleeping = rigid->isSleeping();
		data.wakeCounter = rigid->getWakeCounter();
		data.maxContactImpulse = rigid->getMaxContactImpulse();
		data.linearVelocity = Vec3(rigid->getLinearVelocity().x, rigid->getLinearVelocity().z, rigid->getLinearVelocity().y);
		data.angularVelocity = Vec3(rigid->getAngularVelocity().x, rigid->getAngularVelocity().z, rigid->getAngularVelocity().y);
		PxU32 velCounts;
		PxU32 posCounts;
		rigid->getSolverIterationCounts(posCounts, velCounts);
		data.posCounts = posCounts;
		data.velCounts = velCounts;
		data.disableGravity = rigid->getActorFlags().isSet(PxActorFlag::eDISABLE_GRAVITY);
		data.disableSimulation = rigid->getActorFlags().isSet(PxActorFlag::eDISABLE_SIMULATION);
		data.sendSleepNotes = rigid->getActorFlags().isSet(PxActorFlag::eSEND_SLEEP_NOTIFIES);
		data.isVisualization = rigid->getActorFlags().isSet(PxActorFlag::eVISUALIZATION);
		data.isEnableCCD = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eENABLE_CCD);
		data.isEnableCCDFriction = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eENABLE_CCD_FRICTION);
		data.isEnableCCDMaxContact = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eENABLE_CCD_MAX_CONTACT_IMPULSE);
		data.isPoseIntegration = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eENABLE_POSE_INTEGRATION_PREVIEW);
		//data.isKinematic = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC);
		//data.isKinematicScene = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES);
		data.isKinematic = false;
		data.isKinematicScene = false;
		data.isAngX = rigid->getRigidDynamicLockFlags().isSet(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X);
		data.isAngZ = rigid->getRigidDynamicLockFlags().isSet(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y);
		data.isAngY = rigid->getRigidDynamicLockFlags().isSet(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);
		data.isLinX = rigid->getRigidDynamicLockFlags().isSet(PxRigidDynamicLockFlag::eLOCK_LINEAR_X);
		data.isLinZ = rigid->getRigidDynamicLockFlags().isSet(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y);
		data.isLinY = rigid->getRigidDynamicLockFlags().isSet(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z);
		data.restitution = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitution();
		PxCombineMode::Enum restituCombineMode = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitutionCombineMode();
		data.dynamicFriction = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getDynamicFriction();
		data.staticFriction = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getStaticFriction();
		PxCombineMode::Enum frictionCombineMode = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFrictionCombineMode();
		data.isDisableFriction = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags().isSet(physx::PxMaterialFlag::eDISABLE_FRICTION);
		data.isDisableStrongFriction = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags().isSet(physx::PxMaterialFlag::eDISABLE_STRONG_FRICTION);
		switch (restituCombineMode)
		{
		case PxCombineMode::eAVERAGE:
			data.restitutionCombineMode = 0;
			break;
		case PxCombineMode::eMIN:
			data.restitutionCombineMode = 1;
			break;
		case PxCombineMode::eMULTIPLY:
			data.restitutionCombineMode = 2;
			break;
		case PxCombineMode::eMAX:
			data.restitutionCombineMode = 3;
			break;
		default:
			break;
		}
		switch (frictionCombineMode)
		{
		case PxCombineMode::eAVERAGE:
			data.frictionCombineModeInt = 0;
			break;
		case PxCombineMode::eMIN:
			data.frictionCombineModeInt = 1;
			break;
		case PxCombineMode::eMULTIPLY:
			data.frictionCombineModeInt = 2;
			break;
		case PxCombineMode::eMAX:
			data.frictionCombineModeInt = 3;
			break;
		default:
			break;
		}
	}
	SCENE_UNLOCK_READ(SyncScene);
	orionMesh->SetSimulatePhysics(false);
}

void UReplicaRigidDynamicClient::GetParentComponent()
{
	bool isChild;
	USceneComponent * parentComps = GetAttachParent();
	UStaticMeshComponent* meshComp = dynamic_cast<UStaticMeshComponent*>(parentComps);
	if (meshComp)
	{
		orionMesh = meshComp;
		isChild = true;
		TArray<USceneComponent*> meshChildrenComps;
		orionMesh->GetChildrenComponents(false, meshChildrenComps);
		for (auto Iter = meshChildrenComps.CreateConstIterator(); Iter; ++Iter)
		{
			UReplicaRigidDynamicClient* replicaClientComp = Cast<UReplicaRigidDynamicClient>(*Iter);
			if (replicaClientComp != nullptr)
			{
					ensureMsgf(replicaClientComp == this, TEXT("There can only be one ReplicaRigidDynamicComponent for each Static Mesh Component"));
			}
		}
	}
	ensureMsgf(isChild, TEXT("ReplicaRigidDynamicComponents is not a childcomponent of a static mesh component"));
}

void UReplicaRigidDynamicClient::CenterToMesh(RigidDynamicConstructionData & data)
{
	SetRelativeLocation(FVector(data.centerMass.X, data.centerMass.Y, data.centerMass.Z));
	relativePos.SetLocation(FVector(data.centerMass.X, -data.centerMass.Z, data.centerMass.Y));
	FRotator relRot = FRotator(0, 0, 0);
	relRot.Add(0, 180.0f, -90.0f);
	relativePos.SetRotation(relRot.Quaternion());
	relativePos.SetScale3D(orionMesh->GetComponentScale());
}

bool UReplicaRigidDynamicClient::OnConstruction(const RigidDynamicConstructionData& Data)
{
	const PxGeometryType::Enum GeomType = static_cast<PxGeometryType::Enum>(Data.geom);
	SetVisual(GeomType);

	pos = Data.pos;
	rot = Data.rot;

	UpdateTransform();

	return true;
}

RigidDynamicConstructionData UReplicaRigidDynamicClient::GetConstructionData()
{
	RigidDynamicConstructionData data;

	GetParentComponent();
	AttachToComponent(orionMesh, FAttachmentTransformRules::KeepWorldTransform);

	ReadPhysicValues(data);
	CenterToMesh(data);
	FRotator rotOffset = orionMesh->GetComponentRotation();
	rotOffset.Add(0, 180, -90);
	orionMesh->SetWorldRotation(rotOffset.Quaternion());
	FTransform actorTransform = orionMesh->GetComponentTransform();
	centerMass = FVector(data.centerMass.X, data.centerMass.Y, data.centerMass.Z);
	// Conversion matrix from PhysX to Unreal
	float    matrixElements[16] = {
		1,  0, 0, 0,
		0,  0, 1, 0,
		0,  1, 0, 0,
		0,  0, 0, 1
	};

	FMatrix conversionMatrix = FMatrix();
	memcpy(conversionMatrix.M, matrixElements, 16 * sizeof(float));
	
	actorTransform.SetScale3D(FVector(-actorTransform.GetScale3D().X, actorTransform.GetScale3D().Y, actorTransform.GetScale3D().Z));
	actorTransform *= FTransform(conversionMatrix.Inverse());
	actorTransform.ScaleTranslation(1 / 50.0f);

	FTransform centerMassTransform = FTransform(FQuat(data.centerMassRot.X, data.centerMassRot.Y, data.centerMassRot.Z, data.centerMassRot.W),
		FVector(data.centerMass.X, data.centerMass.Y, data.centerMass.Z), FVector(-1.0f, 1.0f, 1.0f));
	centerMassTransform *= FTransform(conversionMatrix.Inverse());
	centerMassTransform.ScaleTranslation(1 / 50.0f);

	data.pos = Util::ToVec3(actorTransform.GetLocation());
	data.rot = Util::ToQuat(actorTransform.GetRotation());
	data.scale = Vec3(actorTransform.GetScale3D().X, actorTransform.GetScale3D().Y, actorTransform.GetScale3D().Z);
	data.numVertex = 0;

	if (orionMesh->GetBodySetup()->AggGeom.BoxElems.Num() > 0) {
		data.geom = 3;
		data.scale.X = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].X / 2.0f / 50.0f * data.scale.X;
		data.scale.Y = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].Z / 2.0f / 50.0f * data.scale.Z;
		data.scale.Z = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].Y / 2.0f / 50.0f * data.scale.Y;
		data.centerMass = Vec3(0.0f,0.0f,0.0f);
	}
	else if (orionMesh->GetBodySetup()->AggGeom.SphylElems.Num() > 0) {
		data.geom = 2;
		data.scale.Y = orionMesh->GetBodySetup()->AggGeom.SphylElems[0].Length / 2.0f / 50.0f * data.scale.Z;
		data.scale.X = orionMesh->GetBodySetup()->AggGeom.SphylElems[0].Radius / 50.0f * data.scale.X;
		relativePos.SetLocation(FVector(data.centerMass.Z, data.centerMass.X, data.centerMass.Y));
		FRotator relRot = FRotator(0, 0, 0);
		relRot.Add(90.0f, 0.0f, 0.0f);
		relativePos.SetRotation(relRot.Quaternion());
		data.centerMass = Vec3(0.0f, 0.0f, 0.0f);
	}
	else if (orionMesh->GetBodySetup()->AggGeom.SphereElems.Num() > 0) {
		data.geom = 0;
		data.scale.X = orionMesh->GetBodySetup()->AggGeom.SphereElems[0].Radius / 50.0f * data.scale.X;
		data.centerMass = Vec3(0.0f, 0.0f, 0.0f);
	}
	//if mesh
	else if (orionMesh->GetBodySetup()->AggGeom.ConvexElems.Num() > 0) {
		data.geom = 4;
		data.numVertex = orionMesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData.Num();
		for (FVector vec : orionMesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData)
		{
			FVector aux = vec * orionMesh->GetComponentScale() - FVector(data.centerMass.X, data.centerMass.Y, data.centerMass.Z);
			aux = aux / 50.0f;
			Vec3 ver;
			ver = Vec3(aux.X, aux.Z, aux.Y);
			data.vertexData.push_back(ver);
		}
		data.centerMass = Vec3(0.0f, 0.0f, 0.0f);
	}
	data.centerMassRot = Quat(centerMassTransform.GetRotation().X, centerMassTransform.GetRotation().Y, centerMassTransform.GetRotation().Z, centerMassTransform.GetRotation().W);
	FVector comLoc = GetComponentLocation() / 50.0f;
	data.pos = Vec3(comLoc.X, comLoc.Z, comLoc.Y);
	rotOffset = orionMesh->GetComponentRotation();
	rotOffset.Add(0, -180, 90);
	orionMesh->SetWorldRotation(rotOffset.Quaternion());
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
	orionMesh->SetWorldTransform(Util::PhysXToUnreal(transform), false, nullptr, ETeleportType::TeleportPhysics);
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
