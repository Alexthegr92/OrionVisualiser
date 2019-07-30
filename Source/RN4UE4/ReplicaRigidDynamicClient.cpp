#include "RN4UE4.h"
#include "Runtime/Engine/Classes/PhysicsEngine/AggregateGeom.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodySetup.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"
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
	if (!registered && ensure(rakNetManager) && rakNetManager->GetInitialised())
	{
		rakNetManager->Reference(this);
		registered = true;
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
				data.scale.X = GetOwner()->GetActorScale().X;
				data.scale.Y = GetOwner()->GetActorScale().Z;
				data.scale.Z = GetOwner()->GetActorScale().Y;
				data.mass = vismesh->GetMass();
				data.inertia.X = vismesh->GetInertiaTensor().X;
				data.inertia.Y = vismesh->GetInertiaTensor().Z;
				data.inertia.Z = vismesh->GetInertiaTensor().Y;
				data.angularDamping = vismesh->GetAngularDamping();
				data.linearDamping = vismesh->GetLinearDamping();
				data.gravityEnabled = vismesh->IsGravityEnabled();

				if (vismesh->GetBodySetup()->AggGeom.BoxElems.Num()>0) {
					data.geom = 3;
				}
				else if (vismesh->GetBodySetup()->AggGeom.SphylElems.Num()) {
					data.geom = 2;
				}
				else if (vismesh->GetBodySetup()->AggGeom.SphereElems.Num()) {
					data.geom = 1;
				}
				//if mesh
				else if (vismesh->GetBodySetup()->AggGeom.ConvexElems.Num() > 0) {
					data.geom = 4;
					data.numVertex = vismesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData.Num();
					for (FVector vec : vismesh->GetBodySetup()->AggGeom.ConvexElems[0].VertexData)
					{
						FVector aux = vec / 50.0f;
						Vec3 ver;
						ver.X = aux.X;
						ver.Y = aux.Z;
						ver.Z = aux.Y;
						data.vertexData.push_back(ver);
					}
				}

				FVector centerMass = vismesh->GetCenterOfMass();
				centerMass = centerMass / 50.0f;
				data.centerMass.X = centerMass.X;
				data.centerMass.Y = centerMass.Z;
				data.centerMass.Z = centerMass.Y;
				data.MaxAngularVelocity = vismesh->GetBodyInstance()->MaxAngularVelocity;
				data.typeName = TCHAR_TO_UTF8(*vismesh->GetBodySetup()->GetName());
				data.restitution = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitution();
				PxCombineMode::Enum restituCombineMode = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getRestitutionCombineMode();
				data.dynamicFriction = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getDynamicFriction();
				data.staticFriction = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getStaticFriction();
				PxCombineMode::Enum frictionCombineMode = vismesh->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFrictionCombineMode();
				//PxFlags<PxMaterialFlag::Enum, PxU16> flags = vismesh->GetStaticMeshComponent()->GetBodySetup()->GetPhysMaterial()->GetPhysXMaterial()->getFlags();
				if (restituCombineMode == PxCombineMode::eAVERAGE)
					data.restitutionCombineMode = 0;
				else if (restituCombineMode == PxCombineMode::eMIN)
					data.restitutionCombineMode = 1;
				else if (restituCombineMode == PxCombineMode::eMULTIPLY)
					data.restitutionCombineMode = 2;
				else if (restituCombineMode == PxCombineMode::eMAX)
					data.restitutionCombineMode = 3;
				if (frictionCombineMode == PxCombineMode::eAVERAGE)
					data.frictionCombineModeInt = 0;
				else if (frictionCombineMode == PxCombineMode::eMIN)
					data.frictionCombineModeInt = 1;
				else if (frictionCombineMode == PxCombineMode::eMULTIPLY)
					data.frictionCombineModeInt = 2;
				else if (frictionCombineMode == PxCombineMode::eMAX)
					data.frictionCombineModeInt = 3;
				//	constructionBitstream->Write<PxMaterialFlags>(flags);
				vismesh->SetSimulatePhysics(false);
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
	GetOwner()->SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}