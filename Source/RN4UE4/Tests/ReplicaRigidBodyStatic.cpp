// Fill out your copyright notice in the Description page of Project Settings.

#include "ReplicaRigidBodyStatic.h"
#include "RN4UE4.h"
#include "Runtime/Engine/Classes/PhysicsEngine/AggregateGeom.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodySetup.h"
#include "Runtime/Engine/Classes/PhysicsEngine/BodyInstance.h"


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
	
	ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));
}

// Called every frame
void UReplicaRigidBodyStatic::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!registered && ensure(rakNetManager) && rakNetManager->GetInitialised())
	{
		int nbMeshes;
		TArray<FNestedArray> vertices;
		GetAllVertices(nbMeshes, vertices);
		SignalData(nbMeshes, vertices);
		registered = true;
	}
}

void UReplicaRigidBodyStatic::GetAllVertices(int &nbMeshes, TArray<FNestedArray> &vertices)
{
	TArray<UPrimitiveComponent*> comps;
	GetOwner()->GetComponents(comps);
	for (auto Iter = comps.CreateConstIterator(); Iter; ++Iter)
	{
		orionMesh = Cast<UStaticMeshComponent>(*Iter);
		if (orionMesh)
		{
			nbMeshes = orionMesh->GetBodySetup()->AggGeom.ConvexElems.Num();
			for (int i = 0; i < orionMesh->GetBodySetup()->AggGeom.ConvexElems.Num(); i++)
			{
				FNestedArray verticesElem;
				for (FVector vec : orionMesh->GetBodySetup()->AggGeom.ConvexElems[i].VertexData)
				{
					FVector aux;
					aux.X = vec.X;
					aux.Y = vec.Y;
					aux.Z = vec.Z;
					aux = aux / 50.0f;
					FVector ver;
					ver.X = aux.X;
					ver.Y = aux.Z;
					ver.Z = aux.Y;
					verticesElem.Vectors.Push(ver);
				}
				vertices.Push(verticesElem);
			}
		}
	}
}

void UReplicaRigidBodyStatic::SignalData(int &nbMeshes, TArray<FNestedArray> &vertices)
{
	rakNetManager->RPrpcSignalStaticMesh(GetOwner()->GetActorLocation(), GetOwner()->GetActorQuat(), nbMeshes, vertices);
}

