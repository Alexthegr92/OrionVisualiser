// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "PhysXPublic.h"
#include "Runtime/Engine/Classes/PhysicsEngine/AggregateGeom.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodySetup.h"
#include "ClientCreatedReplica.h"


// Sets default values for this component's properties
UClientCreatedReplica::UClientCreatedReplica()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UClientCreatedReplica::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UClientCreatedReplica::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UClientCreatedReplica::SerializeConstruction(BitStream * constructionBitstream, Connection_RM3 * destinationConnection)
{
	// variableDeltaSerializer is a helper class that tracks what variables were sent to what remote system
	// This call adds another remote system to track
	FVector pos(GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Z, GetOwner()->GetActorLocation().Y);
	pos = pos / 50.0f;
	FQuat rot = GetOwner()->GetActorRotation().Quaternion();
	AStaticMeshActor * vismesh = dynamic_cast<AStaticMeshActor*>(GetOwner());
	if (vismesh != nullptr) {
	constructionBitstream->Write<int>(typeMesh);
	constructionBitstream->WriteVector<float>(pos.X, pos.Z, pos.Y);
	constructionBitstream->WriteVector<float>(vismesh->GetActorForwardVector().X, vismesh->GetActorForwardVector().Z, vismesh->GetActorForwardVector().Y);
	constructionBitstream->WriteVector<float>(vismesh->GetActorScale().X, vismesh->GetActorScale().Z, vismesh->GetActorScale().Y);
	constructionBitstream->WriteVector<float>(rot.X, rot.Z, rot.Y);
	constructionBitstream->Write<float>(rot.W);
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetMass());
	constructionBitstream->WriteVector<float>(vismesh->GetStaticMeshComponent()->GetInertiaTensor().X, vismesh->GetStaticMeshComponent()->GetInertiaTensor().Z, vismesh->GetStaticMeshComponent()->GetInertiaTensor().Y);
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetAngularDamping());
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetLinearDamping());
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetAngularDamping());
	constructionBitstream->Write<bool>(vismesh->GetStaticMeshComponent()->IsGravityEnabled());
	//if box
	if (typeMesh == 0) {
		FRotator rotAux = GetOwner()->GetActorRotation();
		FVector ext;
		FBoxSphereBounds sph;
		FTransform tr;
		GetOwner()->SetActorRotation(rotAux);
		vismesh->GetStaticMeshComponent()->GetBodySetup()->AggGeom.CalcBoxSphereBounds(sph, tr);
		ext = sph.BoxExtent;
		ext = ext * 0.5;
		constructionBitstream->WriteVector<float>(ext.X, ext.Z, ext.Y);
	}
	
	//if sphere
	else if (typeMesh == 1) {
		float radius;
		FBoxSphereBounds sph;
		FTransform tr;
		vismesh->GetStaticMeshComponent()->GetBodySetup()->AggGeom.CalcBoxSphereBounds(sph, tr);
		radius = sph.SphereRadius;
		constructionBitstream->Write<float>(radius);
	}
	//if capsule
	else if (typeMesh == 2) {
		FRotator rotAux = GetOwner()->GetActorRotation();
		FVector ext;
		FBoxSphereBounds sph;
		FTransform tr;
		GetOwner()->SetActorRotation(FQuat(0, 0, 0, 1));
		vismesh->GetStaticMeshComponent()->GetBodySetup()->AggGeom.CalcBoxSphereBounds(sph, tr);
		ext = sph.BoxExtent;
		ext = ext * 0.5f;
		constructionBitstream->Write<float>(ext.Y - ext.X);
		constructionBitstream->Write<float>(ext.X);
		GetOwner()->SetActorRotation(rotAux);
	}
	//if mesh
	else if (typeMesh == 3) {
		constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetBodySetup()->AggGeom.ConvexElems[0].VertexData.Num());
		for (FVector vec : vismesh->GetStaticMeshComponent()->GetBodySetup()->AggGeom.ConvexElems[0].VertexData)
		{
			vec = vec / 50.0f;
			constructionBitstream->WriteVector<float>(vec.X, vec.Z, vec.Y);
		}
	}
	SampleReplica::SerializeConstruction(constructionBitstream, destinationConnection);
	}
}

void UClientCreatedReplica::Deserialize(DeserializeParameters * deserializeParameters)
{
}

bool UClientCreatedReplica::DeserializeDestruction(BitStream * destructionBitstream, Connection_RM3 * sourceConnection)
{
	return false;
}
