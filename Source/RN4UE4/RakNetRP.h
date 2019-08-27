// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "RakNet.h"
#include "RakNetTypes.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "ReplicaManager3.h"
#include "NetworkIDManager.h"
#include "VariableDeltaSerializer.h"
#include "GetTime.h"

#include "Replica.h"

//#include <stdio.h>
//#include "Kbhit.h"
//#include <string.h>
//#include <stdlib.h>
//#include "RakSleep.h"
//#include "Gets.h"

#include "GameFramework/Actor.h"
#include "RPC4Plugin.h"
#include "RakNetRP.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllocReplica, FString, receivedString);


DECLARE_LOG_CATEGORY_EXTERN(RakNet_RakNetRP, Log, All);

// ReplicaManager3 is in the namespace RakNet
using namespace RakNet;

class ReplicaManager3Sample;

UCLASS()
class RN4UE4_API ARakNetRP : public AActor, public ReplicaManager3
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARakNetRP();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPStartup();

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPDisconnect();

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPConnect(const FString& host, const int port);

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPrpcSpawn(FVector pos, FVector dir);

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPrpcSignalAllServers(const FString& sharedIdentifier);

	UFUNCTION(BlueprintCallable, Category = "RakNet|RakNetRP")
		void RPrpcSignalBoundaryBox(const TArray<FVector> pos, const TArray<FVector> size, const TArray<int> ranks,bool multiAuras,float errorTolerance);

	UPROPERTY(EditDefaultsOnly, Category = "Object to spawn")
		TSubclassOf<AReplica> objectToSpawn;

	AReplica* GetObjectFromType(RakString typeName);

	void CustomCreatedBoundarySlot(RakNet::BitStream * bitStream, Packet * packet);

	void CreateBoundarySlot(RakNet::BitStream * bitStream, Packet * packet);

	void DeleteBoundarySlot(RakNet::BitStream * bitStream, Packet * packet);

	void GetExpectedServersSlot(RakNet::BitStream * bitStream, Packet * packet);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RakNet|RakNetRP")
		void DeleteBoundaryBox(int rank);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RakNet|RakNetRP")
		void CreateBoundaryBox(int rank, FVector pos, FVector size);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RakNet|RakNetRP")
		void CreateBoundaryPlane(int rank, FVector pos, FVector normal);

	virtual Connection_RM3* AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const;
	virtual void DeallocConnection(Connection_RM3 *connection) const;

	void DroppedConnection(unsigned short Port);

	void SetNewConnectionCallback(const std::function<void(SystemAddress address)>& fun)
	{
		newConnectionCallback = fun;
	}

	RPC4* GetRpc() { return &rpc; }

	bool GetInitialised() const { return initialised; }

	bool GetAllServersChecked() const;

	bool IsCustomBoundariesCreated() const;

	void SetCustomBoundariesCreated(bool boundariesCreated);
private:

	void ConnectToIP(const FString& address);

	RakPeerInterface*		rakPeer				= nullptr;
	NetworkIDManager		networkIdManager;	// ReplicaManager3 requires NetworkIDManager to lookup pointers from numbers.
	Packet*					p					= nullptr;// Holds packets

	std::function<void(SystemAddress address)> newConnectionCallback;
	
	RPC4 rpc;

	bool initialised;
	bool customBoundariesCreated = true;
	static const int SERVER_PORT = 12345;
	int						totalServers;
	bool					allServersChecked;
};
