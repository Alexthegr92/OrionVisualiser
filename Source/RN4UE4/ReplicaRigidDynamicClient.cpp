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

	if (!physicsCopied) {
		TArray<UPrimitiveComponent*> comps;
		GetOwner()->GetComponents(comps);
		for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
		{
			UStaticMeshComponent* vismesh = Cast<UStaticMeshComponent>(*Iter);
			if (vismesh)
			{
				if (vismesh->GetStaticMesh() != nullptr) {
					orionMesh = vismesh;
					relativePos = orionMesh->GetRelativeTransform();
					if(GetNumChildrenComponents()==0)
						relativePos.SetToRelativeTransform(GetOwner()->GetActorTransform());
					FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
					PxScene* SyncScene = PhysScene->GetPhysXScene(PST_Sync);
					SCENE_LOCK_READ(SyncScene);
					PxRigidDynamic * rigid = vismesh->GetBodyInstance()->GetPxRigidDynamic_AssumesLocked();
					if (rigid != nullptr) {
						mass = rigid->getMass() / 50.0f;
						rigid->getMassSpaceInertiaTensor();
						inertia = FVector(rigid->getMassSpaceInertiaTensor().x, rigid->getMassSpaceInertiaTensor().z, rigid->getMassSpaceInertiaTensor().y);
						inertia = inertia / 50.0f;
						angularDamping = rigid->getAngularDamping();
						linearDamping = rigid->getLinearDamping();
						isGravity = vismesh->IsGravityEnabled();
						centerMass = FVector(rigid->getCMassLocalPose().p.x, rigid->getCMassLocalPose().p.z, rigid->getCMassLocalPose().p.y);
						centerMass = centerMass / 50.0f;
						MaxAngularVelocity = rigid->getMaxAngularVelocity();
						typeName = rigid->getConcreteTypeName();
						restitution = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitution();
						PxCombineMode::Enum restituCombineMode = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitutionCombineMode();
						dynamicFriction = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getDynamicFriction();
						staticFriction = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getStaticFriction();
						PxCombineMode::Enum frictionCombineMode = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFrictionCombineMode();
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
					vismesh->SetSimulatePhysics(false);
				}
			}
		}
		physicsCopied = true;
	}

	if (rakNetManager != nullptr)
	{
		if (!registered && rakNetManager != nullptr && rakNetManager->GetInitialised())
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
	FVector position = GetOwner()->GetActorLocation();
	FQuat rot = GetOwner()->GetActorRotation().Quaternion();
	position = position / 50.0f;
	data.pos.X = position.X;
	data.pos.Y = position.Z;
	data.pos.Z = position.Y;
	data.rot.X = rot.X;
	data.rot.Y = rot.Z;
	data.rot.Z = rot.Y;
	data.rot.W = rot.W;
	data.numVertex = 0;
	TArray<UPrimitiveComponent*> comps;
	GetOwner()->GetComponents(comps);
	for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
	{
		UStaticMeshComponent* vismesh = Cast<UStaticMeshComponent>(*Iter);
		if (vismesh)
		{
			if (vismesh->GetStaticMesh() != nullptr) {
				data.scale.X = GetOwner()->GetActorScale().X;
				data.scale.Y = GetOwner()->GetActorScale().Z;
				data.scale.Z = GetOwner()->GetActorScale().Y;
				data.mass = mass;
				data.inertia.X = inertia.X;
				data.inertia.Y = inertia.Z;
				data.inertia.Z = inertia.Y;
				data.angularDamping = angularDamping;
				data.linearDamping = linearDamping;
				data.gravityEnabled = isGravity;

				if (vismesh->GetBodySetup()->AggGeom.BoxElems.Num() > 0) {
					data.geom = 3;
					data.scale.X = vismesh->GetBodySetup()->AggGeom.BoxElems[0].X/ 2.0f / 50.0f * data.scale.X;
					data.scale.Y = vismesh->GetBodySetup()->AggGeom.BoxElems[0].Z / 2.0f / 50.0f * data.scale.Y;
					data.scale.Z = vismesh->GetBodySetup()->AggGeom.BoxElems[0].Y / 2.0f / 50.0f * data.scale.Z;
				}
				else if (vismesh->GetBodySetup()->AggGeom.SphylElems.Num() > 0) {
					data.geom = 2;
					data.scale.Y = vismesh->GetBodySetup()->AggGeom.SphylElems[0].Length / 2.0f / 50.0f * data.scale.Y;
					data.scale.X = vismesh->GetBodySetup()->AggGeom.SphylElems[0].Radius / 50.0f * data.scale.X;
				}
				else if (vismesh->GetBodySetup()->AggGeom.SphereElems.Num() > 0) {
					data.geom = 0;
					data.scale.X = vismesh->GetBodySetup()->AggGeom.SphereElems[0].Radius / 50.0f * data.scale.X;
				}
				//if mesh
				else if (vismesh->GetBodySetup()->AggGeom.ConvexElems.Num() > 0) {
					data.geom = 4;
					data.numVertex = vismesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData.Num();
					for (FVector vec : vismesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData)
					{
						FVector aux = vec;
						aux = aux / 50.0f;
						Vec3 ver;
						ver.X = aux.X;
						ver.Y = aux.Z;
						ver.Z = aux.Y;
						data.vertexData.push_back(ver);
					}
				}

				data.centerMass.X = centerMass.X;
				data.centerMass.Y = centerMass.Z;
				data.centerMass.Z = centerMass.Y;
				data.MaxAngularVelocity = MaxAngularVelocity;
				data.typeName = typeName;
				data.restitution = restitution;
				data.dynamicFriction = dynamicFriction;
				data.staticFriction = staticFriction;
				//PxFlags<PxMaterialFlag::Enum, PxU16> flags = vismesh->GetStaticMeshComponent()->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags();
				data.restitutionCombineMode = restitutionCombineMode;
				data.frictionCombineModeInt = frictionCombineModeInt;
				//	constructionBitstream->Write<PxMaterialFlags>(flags);
				//vismesh->SetSimulatePhysics(false);
			}
		}
	}
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
	GetOwner()->SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}