// Fill out your copyright notice in the Description page of Project Settings.

#include "ReplicaRigidBodyStatic.h"
#include "RN4UE4.h"
#include "Runtime/Engine/Classes/PhysicsEngine/AggregateGeom.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodySetup.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"
#include "ThirdParty/PhysX3/PhysX_3.4/include/geometry/PxTriangleMesh.h"
#include "Engine/World.h"
#include "RN4UE4GameInstance.h"


// Sets default values
UReplicaRigidBodyStatic::UReplicaRigidBodyStatic()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void UReplicaRigidBodyStatic::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UReplicaRigidBodyStatic::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (rakNetManager == nullptr)
	{
		URN4UE4GameInstance* GameInstance = static_cast<URN4UE4GameInstance*>(GetOwner()->GetGameInstance());
		ensureMsgf(GameInstance != nullptr, TEXT("RakNetRP - GameInstance is not of type URN4UE4GameInstance"));
		rakNetManager = GameInstance->GetRakNetManager();
	}

	if (!registered && ensure(rakNetManager) && rakNetManager->GetAllServersChecked())
	{
		int nbVertices;
		TArray<FVector> vertices;
		int nbIndices;
		TArray<PxU16> indices;
		GetAllVertices(nbVertices, vertices, nbIndices, indices);
		SignalData(nbVertices, vertices, nbIndices, indices);
		registered = true;
	}
}

void UReplicaRigidBodyStatic::GetAllVertices(int &nbVertices, TArray<FVector> &vertices, int &nbIndices, TArray<PxU16> &indices)
{
	TArray<UPrimitiveComponent*> comps;
	GetOwner()->GetComponents(comps);
	for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
	{
		orionMesh = Cast<UStaticMeshComponent>(*Iter);
		if (orionMesh)
		{
			PxTriangleMesh* TempTriMesh = orionMesh->GetBodySetup()->TriMeshes[0];
			check(TempTriMesh);
			int32 TriNumber = TempTriMesh->getNbTriangles();
			nbVertices = TempTriMesh->getNbVertices();
			nbIndices = TempTriMesh->getNbTriangles()*3;
			const PxVec3* PVertices = TempTriMesh->getVertices();
			const void* Triangles = TempTriMesh->getTriangles();

			// Grab triangle indices
			int32 I0, I1, I2;

			for (int32 TriIndex = 0; TriIndex < TriNumber; ++TriIndex)
			{
				if (TempTriMesh->getTriangleMeshFlags() & PxTriangleMeshFlag::e16_BIT_INDICES)
				{
					PxU16* P16BitIndices = (PxU16*)Triangles;
					I0 = P16BitIndices[(TriIndex * 3) + 0];
					I1 = P16BitIndices[(TriIndex * 3) + 1];
					I2 = P16BitIndices[(TriIndex * 3) + 2];
				}
				else
				{
					PxU32* P32BitIndices = (PxU32*)Triangles;
					I0 = P32BitIndices[(TriIndex * 3) + 0];
					I1 = P32BitIndices[(TriIndex * 3) + 1];
					I2 = P32BitIndices[(TriIndex * 3) + 2];
				}

				// Local position
				FVector V0 = FVector(PVertices[I0].x, PVertices[I0].y, PVertices[I0].z);
				FVector V1 = FVector(PVertices[I1].x, PVertices[I1].y, PVertices[I1].z);
				FVector V2 = FVector(PVertices[I2].x, PVertices[I2].y, PVertices[I2].z);
				indices.Push(I0);
				indices.Push(I1);
				indices.Push(I2);
			}
			for (int32 TriIndex = 0; TriIndex < nbVertices; ++TriIndex)
			{
				vertices.Push(FVector(PVertices[TriIndex].x / 50.0f, PVertices[TriIndex].y / 50.0f, PVertices[TriIndex].z / 50.0f));
			}
		}
	}
}

void UReplicaRigidBodyStatic::SignalData(int &nbVertices, TArray<FVector> &vertices, int &nbIndices, TArray<PxU16> &indices)
{
	rakNetManager->RPrpcSignalStaticMesh(GetOwner()->GetActorLocation(), GetOwner()->GetActorQuat(), nbVertices, vertices, nbIndices, indices);
}

