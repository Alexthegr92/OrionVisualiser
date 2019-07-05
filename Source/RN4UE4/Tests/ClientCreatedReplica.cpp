// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "PhysXIncludes.h" 
#include "ClientCreatedReplica.h"


// Sets default values for this component's properties
UClientCreatedReplica::UClientCreatedReplica()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	m_registered = false;
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

	if (!m_registered && rakNetManager->getAllServersChecked())
	{
		m_registered = true;
		rakNetManager->Reference(this);
	}
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
	constructionBitstream->WriteVector<float>(pos.X, pos.Y, pos.Z);
	constructionBitstream->WriteVector<float>(vismesh->GetActorForwardVector().X, vismesh->GetActorForwardVector().Z, vismesh->GetActorForwardVector().Y);
	constructionBitstream->WriteVector<float>(vismesh->GetActorScale().X, vismesh->GetActorScale().Z, vismesh->GetActorScale().Y);
	constructionBitstream->WriteVector<float>(rot.X, rot.Y, rot.Z);
	constructionBitstream->Write<float>(rot.W);
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetMass());
	constructionBitstream->WriteVector<float>(vismesh->GetStaticMeshComponent()->GetInertiaTensor().X, vismesh->GetStaticMeshComponent()->GetInertiaTensor().Z, vismesh->GetStaticMeshComponent()->GetInertiaTensor().Y);
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetAngularDamping());
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetLinearDamping());
	constructionBitstream->Write<float>(vismesh->GetStaticMeshComponent()->GetAngularDamping());
	constructionBitstream->Write<bool>(vismesh->GetStaticMeshComponent()->IsGravityEnabled());
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
