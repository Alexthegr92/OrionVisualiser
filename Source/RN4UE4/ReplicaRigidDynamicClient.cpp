#include "RN4UE4.h"
#include "Runtime/Engine/Classes/PhysicsEngine/AggregateGeom.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodySetup.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"
#include "ThirdParty/PhysX3/PhysX_3.4/Include/PxRigidDynamic.h"
#include "ThirdParty/PhysX3/PhysX_3.4/Include/PxMaterial.h"
#include "PhysXIncludes.h" 
#include "PhysXPublic.h"
#include "PhysicsPublic.h"
#include "ReplicaRigidDynamicClient.h"

UReplicaRigidDynamicClient::UReplicaRigidDynamicClient()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReplicaRigidDynamicClient::BeginPlay()
{
	Super::BeginPlay();

	//ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));
	registered = false;
}

void UReplicaRigidDynamicClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (rakNetManager != nullptr)
	{
		if (!registered && ensure(rakNetManager) && rakNetManager->GetInitialised())
		{
			rakNetManager->Reference(this);
			registered = true;
		}
	}
	else
	{
		for (TActorIterator<ARakNetRP> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
			if (*ActorItr != nullptr)
			{
				ARakNetRP *rak = *ActorItr;
				rakNetManager = rak;
				break;
			}
		}
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
		data.centerMass.X = centerMass.X;
		data.centerMass.Y = centerMass.Y;
		data.centerMass.Z = centerMass.Z;
		FQuat centerMassRot = FQuat(rigid->getCMassLocalPose().q.x, rigid->getCMassLocalPose().q.y, rigid->getCMassLocalPose().q.z, rigid->getCMassLocalPose().q.w);
		data.centerMassRot.X = centerMassRot.X;
		data.centerMassRot.Y = centerMassRot.Y;
		data.centerMassRot.Z = centerMassRot.Z;
		data.centerMassRot.W = centerMassRot.W;
		data.maxAngularVelocity = rigid->getMaxAngularVelocity();
		data.maxDepenetrationVelocity = rigid->getMaxDepenetrationVelocity();
		data.typeName = rigid->getConcreteTypeName();
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
		data.isKinematic = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC);
		data.isKinematicScene = rigid->getRigidBodyFlags().isSet(PxRigidBodyFlag::eUSE_KINEMATIC_TARGET_FOR_SCENE_QUERIES);
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
		//PxFlags<PxMaterialFlag::Enum, PxU16> flags = vismesh->GetStaticMeshComponent()->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags();
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

void UReplicaRigidDynamicClient::FindNearestStaticMesh()
{
	float distance;
	TArray<UPrimitiveComponent*> comps;
	GetOwner()->GetComponents(comps);
	for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
	{
		UStaticMeshComponent* vismesh = Cast<UStaticMeshComponent>(*Iter);
		if (vismesh != nullptr && vismesh->GetStaticMesh() != nullptr && orionMesh != vismesh)
		{
			orionMesh = vismesh;
			relativePos = orionMesh->GetComponentTransform();
			relativePos.SetToRelativeTransform(GetComponentTransform());
			distance = FVector::Distance(GetComponentTransform().GetLocation(), orionMesh->GetRelativeTransform().GetLocation());
			TArray<UPrimitiveComponent*> comps2;
			GetOwner()->GetComponents(comps2);
			for (auto Iter2 = comps2.CreateConstIterator(); Iter2; ++Iter2)
			{
				UStaticMeshComponent* vismesh2 = Cast<UStaticMeshComponent>(*Iter2);
				if (vismesh2 != nullptr && orionMesh != vismesh2)
				{
					float distance2 = FVector::Distance(GetComponentTransform().GetLocation(), vismesh2->GetRelativeTransform().GetLocation());
					if (distance2 < distance)
					{
						orionMesh = vismesh2;
						relativePos = orionMesh->GetRelativeTransform();
						relativePos.SetToRelativeTransform(GetComponentTransform());
						distance = distance2;
					}
				}
			}
		}
		break;
	}
}

void UReplicaRigidDynamicClient::CenterToMesh(RigidDynamicConstructionData & data)
{
	SetRelativeLocation(FVector(data.centerMass.X, data.centerMass.Y, data.centerMass.Z));
	relativePos.SetLocation(FVector(data.centerMass.X, -data.centerMass.Z, data.centerMass.Y));
	FRotator relRot = FRotator(0, 0, 0);
	relRot.Add(0, 180.0f, -90.0f);
	relativePos.SetRotation(relRot.Quaternion());
}

RigidDynamicConstructionData UReplicaRigidDynamicClient::GetConstructionData()
{
	RigidDynamicConstructionData data;

	FindNearestStaticMesh();
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

	data.clientCreated = true;
	data.pos = Vec3(actorTransform.GetLocation().X, actorTransform.GetLocation().Y, actorTransform.GetLocation().Z);
	data.rot = Quat(actorTransform.GetRotation().X, actorTransform.GetRotation().Y, actorTransform.GetRotation().Z, actorTransform.GetRotation().W);
	data.scale = Vec3(actorTransform.GetScale3D().X, actorTransform.GetScale3D().Y, actorTransform.GetScale3D().Z);
	data.numVertex = 0;

	if (orionMesh->GetBodySetup()->AggGeom.BoxElems.Num() > 0) {
		data.geom = 3;
		data.scale.X = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].X / 2.0f / 50.0f * data.scale.X;
		data.scale.Y = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].Z / 2.0f / 50.0f * data.scale.Z;
		data.scale.Z = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].Y / 2.0f / 50.0f * data.scale.Y;
	}
	else if (orionMesh->GetBodySetup()->AggGeom.SphylElems.Num() > 0) {
		data.geom = 2;
		data.scale.Y = orionMesh->GetBodySetup()->AggGeom.SphylElems[0].Length / 2.0f / 50.0f * data.scale.Z;
		data.scale.X = orionMesh->GetBodySetup()->AggGeom.SphylElems[0].Radius / 50.0f * data.scale.X;
	}
	else if (orionMesh->GetBodySetup()->AggGeom.SphereElems.Num() > 0) {
		data.geom = 0;
		data.scale.X = orionMesh->GetBodySetup()->AggGeom.SphereElems[0].Radius / 50.0f * data.scale.X;
	}
	//if mesh
	else if (orionMesh->GetBodySetup()->AggGeom.ConvexElems.Num() > 0) {
		data.geom = 4;
		data.numVertex = orionMesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData.Num();
		for (FVector vec : orionMesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData)
		{
			FVector aux = vec - FVector(data.centerMass.X, data.centerMass.Y, data.centerMass.Z);
			aux = aux / 50.0f;
			Vec3 ver;
			ver = Vec3(aux.X, aux.Z, aux.Y);
			data.vertexData.push_back(ver);
		}
	}
	FVector centerLoc = centerMassTransform.GetLocation() / 50.0f;
	data.centerMass = Vec3(centerLoc.X, centerLoc.Y, centerLoc.Z);
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
	FQuat	newRotQuat = FQuat(rot.X, rot.Y, rot.Z, rot.W);
	FVector newPos = FVector(pos.X, pos.Y, pos.Z) * 50.0f;
	FVector scale = FVector(-1, 1, 1);// X will get negated, so set scale to -1, so final result is 1
	FRotator newRot = FRotator(newRotQuat);
	FTransform transform = FTransform(newRot, newPos, scale);
	transform *= FTransform(conversionMatrix);
	orionMesh->SetWorldTransform(relativePos * transform, false, nullptr, ETeleportType::TeleportPhysics);
}