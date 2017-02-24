// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "Replica.h"

DEFINE_LOG_CATEGORY(RakNet_Replica);


// Sets default values
AReplica::AReplica()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AReplica::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AReplica::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AReplica::DeserializeConstruction(BitStream *constructionBitstream, Connection_RM3 *sourceConnection)
{
	constructionBitstream->Read(geom);
	AActor* visual = nullptr;

	switch (geom)
	{
	case PxGeometryType::eSPHERE:
	{
		if (sphereBP == nullptr) break;

		visual = GetWorld()->SpawnActor<AActor>(sphereBP, GetTransform(), FActorSpawnParameters());
	}
		break;
	case PxGeometryType::ePLANE:
		break;
	case PxGeometryType::eCAPSULE:
	{
		if (capsuleBP == nullptr) break;

		visual = GetWorld()->SpawnActor<AActor>(capsuleBP, GetTransform(), FActorSpawnParameters());
	}
		break;
	case PxGeometryType::eBOX:
	{
		if (boxBP == nullptr) break;

		visual = GetWorld()->SpawnActor<AActor>(boxBP, GetTransform(), FActorSpawnParameters());
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

	if (visual != nullptr) visual->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true));

	constructionBitstream->Read(posX);
	constructionBitstream->Read(posY);
	constructionBitstream->Read(posZ);
	constructionBitstream->Read(rotX);
	constructionBitstream->Read(rotY);
	constructionBitstream->Read(rotZ);
	constructionBitstream->Read(rotW);
	UpdateTransform();

	return SampleReplica::DeserializeConstruction(constructionBitstream, sourceConnection);
}

void AReplica::Deserialize(DeserializeParameters *deserializeParameters)
{
	SampleReplica::Deserialize(deserializeParameters);
	UpdateTransform();
}

void AReplica::UpdateTransform()
{
	float	matrixElements[16] = {
		1,  0, 0, 0,
		0,  0, 1, 0,
		0,  1, 0, 0,
		0,  0, 0, 1
	};

	FMatrix conversionMatrix = FMatrix();
	memcpy(conversionMatrix.M, matrixElements, 16 * sizeof(float));

	FQuat rot = FQuat(rotX, rotY, rotZ, rotW);
	FVector pos = FVector(posX, posY, posZ) * 50.0f;
	FTransform transform = FTransform(FRotator(rot), pos, FVector(1, 1, 1));
	transform *= FTransform(conversionMatrix);
	SetActorTransform(transform, false, nullptr, ETeleportType::TeleportPhysics);
}
