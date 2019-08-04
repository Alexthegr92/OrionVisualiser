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

	ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));

	registered = false;
}

void UReplicaRigidDynamicClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!physicsCopied) {
		GetNearestStaticMesh();
		ReadPhysicValues();
	}

		if (!registered && ensure(rakNetManager) && rakNetManager->GetInitialised())
		{
			rakNetManager->Reference(this);
			registered = true;
		}
}

void UReplicaRigidDynamicClient::ReadPhysicValues()
{
	FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
	PxScene* SyncScene = PhysScene->GetPhysXScene(PST_Sync);
	SCENE_LOCK_READ(SyncScene);
	PxRigidDynamic * rigid = orionMesh->GetBodyInstance()->GetPxRigidDynamic_AssumesLocked();
	if (rigid != nullptr) {
		mass = rigid->getMass();
		inertia = FVector(rigid->getMassSpaceInertiaTensor().x, rigid->getMassSpaceInertiaTensor().y, rigid->getMassSpaceInertiaTensor().z);
		inertia = inertia;
		inertiaInv = FVector(rigid->getMassSpaceInvInertiaTensor().x, rigid->getMassSpaceInvInertiaTensor().y, rigid->getMassSpaceInvInertiaTensor().z);
		inertiaInv = inertiaInv / 50.0f;
		angularDamping = rigid->getAngularDamping();
		linearDamping = rigid->getLinearDamping();
		isGravity = orionMesh->IsGravityEnabled();
		centerMass = FVector(rigid->getCMassLocalPose().p.x, rigid->getCMassLocalPose().p.y, rigid->getCMassLocalPose().p.z);
		centerMassRot = FQuat(rigid->getCMassLocalPose().q.x, rigid->getCMassLocalPose().q.y, rigid->getCMassLocalPose().q.z, rigid->getCMassLocalPose().q.w);
		MaxAngularVelocity = rigid->getMaxAngularVelocity();
		MaxDepenetrationVelocity = rigid->getMaxDepenetrationVelocity();
		typeName = rigid->getConcreteTypeName();
		restitution = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitution();
		PxCombineMode::Enum restituCombineMode = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitutionCombineMode();
		dynamicFriction = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getDynamicFriction();
		staticFriction = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getStaticFriction();
		PxCombineMode::Enum frictionCombineMode = orionMesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFrictionCombineMode();
		//PxFlags<PxMaterialFlag::Enum, PxU16> flags = vismesh->GetStaticMeshComponent()->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags();
		if (restituCombineMode == PxCombineMode::eAVERAGE)
			restitutionCombineMode = 0;
		else if (restituCombineMode == PxCombineMode::eMIN)
			restitutionCombineMode = 1;
		else if (restituCombineMode == PxCombineMode::eMULTIPLY)
			restitutionCombineMode = 2;
		else if (restituCombineMode == PxCombineMode::eMAX)
			restitutionCombineMode = 3;
		if (frictionCombineMode == PxCombineMode::eAVERAGE)
			frictionCombineModeInt = 0;
		else if (frictionCombineMode == PxCombineMode::eMIN)
			frictionCombineModeInt = 1;
		else if (frictionCombineMode == PxCombineMode::eMULTIPLY)
			frictionCombineModeInt = 2;
		else if (frictionCombineMode == PxCombineMode::eMAX)
			frictionCombineModeInt = 3;
		//	constructionBitstream->Write<PxMaterialFlags>(flags);
	}
	SCENE_UNLOCK_READ(SyncScene);
	orionMesh->SetSimulatePhysics(false);
	physicsCopied = true;
}

void UReplicaRigidDynamicClient::GetNearestStaticMesh()
{

	float distance;
	TArray<UPrimitiveComponent*> comps;
	GetOwner()->GetComponents(comps);
	for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
	{
		UStaticMeshComponent* vismesh = Cast<UStaticMeshComponent>(*Iter);
		if (vismesh)
		{
			if (vismesh->GetStaticMesh() != nullptr) {
				if (orionMesh != vismesh) {
					orionMesh = vismesh;
					relativePos = orionMesh->GetRelativeTransform();
					relativePos.SetToRelativeTransform(GetComponentTransform());
					distance = FVector::Distance(GetComponentTransform().GetLocation(), orionMesh->GetRelativeTransform().GetLocation());
					TArray<UPrimitiveComponent*> comps2;
					GetOwner()->GetComponents(comps2);
					for (auto Iter2 = comps2.CreateConstIterator(); Iter2; ++Iter2)
					{
						UStaticMeshComponent* vismesh2 = Cast<UStaticMeshComponent>(*Iter2);
						if (vismesh2)
						{
							if (orionMesh != vismesh2)
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
				}
			}
		}
		break;
	}
}

RigidDynamicConstructionData UReplicaRigidDynamicClient::GetConstructionData()
{
	FTransform actorTransform = GetOwner()->GetActorTransform();

	// Conversion matrix from PhysX to Unreal
/*	float	matrixElements[16] = {
		1,  0, 0, 0,
		0,  0, 1, 0,
		0,  1, 0, 0,
		0,  0, 0, 1
	};

	FMatrix conversionMatrix = FMatrix();
	memcpy(conversionMatrix.M, matrixElements, 16 * sizeof(float));

	actorTransform *= FTransform(conversionMatrix.Inverse());
	actorTransform.ScaleTranslation(1 / 50.0f);*/

	RigidDynamicConstructionData data;
	data.clientCreated = true;
	FVector position = GetComponentLocation();
	FQuat rot = GetComponentQuat();
	position = position / 50.0f;
	data.pos.X = position.X;
	data.pos.Y = position.Z;
	data.pos.Z = position.Y;
	data.rot.X = rot.X;
	data.rot.Y = rot.Z;
	data.rot.Z = rot.Y;
	data.rot.W = rot.W;
	data.numVertex = 0;
	data.scale.X = GetOwner()->GetActorScale().X;
	data.scale.Y = GetOwner()->GetActorScale().Z;
	data.scale.Z = GetOwner()->GetActorScale().Y;
	data.mass = mass;
	data.inertia.X = inertia.X;
	data.inertia.Y = inertia.Z;
	data.inertia.Z = inertia.Y;
	data.inertiaInv.X = inertiaInv.X;
	data.inertiaInv.Z = inertiaInv.Z;
	data.inertiaInv.Y = inertiaInv.Y;
	data.angularDamping = angularDamping;
	data.linearDamping = linearDamping;
	data.gravityEnabled = isGravity;

	if (orionMesh->GetBodySetup()->AggGeom.BoxElems.Num() > 0) {
		data.geom = 3;
		data.scale.X = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].X / 2.0f / 50.0f * data.scale.X;
		data.scale.Y = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].Z / 2.0f / 50.0f * data.scale.Y;
		data.scale.Z = orionMesh->GetBodySetup()->AggGeom.BoxElems[0].Y / 2.0f / 50.0f * data.scale.Z;
	}
	else if (orionMesh->GetBodySetup()->AggGeom.SphylElems.Num() > 0) {
		data.geom = 2;
		data.scale.Y = orionMesh->GetBodySetup()->AggGeom.SphylElems[0].Length / 2.0f / 50.0f * data.scale.Y;
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
			FVector aux;
			aux.X = vec.X - centerMass.X;
			aux.Y = vec.Y - centerMass.Y;
			aux.Z = vec.Z - centerMass.Z;
			aux = aux / 50.0f;
			Vec3 ver;
			ver.X = aux.X;
			ver.Y = aux.Z;
			ver.Z = aux.Y;
			data.vertexData.push_back(ver);
		}
	}
	centerMass = centerMass / 50.0f;
	data.centerMass.X = centerMass.X;
	data.centerMass.Y = centerMass.Z;
	data.centerMass.Z = centerMass.Y;
	data.centerMassRot.X = centerMassRot.X;
	data.centerMassRot.Y = centerMassRot.Z;
	data.centerMassRot.Z = centerMassRot.Y;
	data.centerMassRot.W = centerMassRot.W;
	data.MaxAngularVelocity = MaxAngularVelocity;
	data.MaxDepenetrationVelocity = MaxDepenetrationVelocity;
	data.typeName = typeName;
	data.restitution = restitution;
	data.dynamicFriction = dynamicFriction;
	data.staticFriction = staticFriction;
	//PxFlags<PxMaterialFlag::Enum, PxU16> flags = vismesh->GetStaticMeshComponent()->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags();
	data.restitutionCombineMode = restitutionCombineMode;
	data.frictionCombineModeInt = frictionCombineModeInt;
	//	constructionBitstream->Write<PxMaterialFlags>(flags);
	//vismesh->SetSimulatePhysics(false);
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
	if (!attached)
	{
		orionMesh->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		orionMesh->SetRelativeTransform(relativePos);
		attached = true;
	}
	SetWorldTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}