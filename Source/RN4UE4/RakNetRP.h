// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkIDManager.h"
#include "RakPeerInterface.h"
#include "ReplicaManager3.h"
#include "RPC4Plugin.h"
#include "RakNetRP.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllocReplica, FString, receivedString);


DECLARE_LOG_CATEGORY_EXTERN(RakNet_RakNetRP, Log, All);

// ReplicaManager3 is in the namespace RakNet
using namespace RakNet;

class ReplicaManager3Sample;

class UReplicaRigidDynamicClient;
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

	UReplicaRigidDynamicClient* GetObjectFromType(RakString TypeName) const;

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

	int GetExpectedNumberOfServers() const;

	UPROPERTY(EditAnywhere, Category = "ReplicaComponent")
		TSubclassOf<UReplicaRigidDynamicClient> ReplicaComponent;
	
private:

	void ConnectToIP(const FString& address);

	RakPeerInterface*		rakPeer				= nullptr;
	NetworkIDManager		networkIdManager;	// ReplicaManager3 requires NetworkIDManager to lookup pointers from numbers.
	Packet*					p					= nullptr;// Holds packets

	std::function<void(SystemAddress address)> newConnectionCallback;
	
	RPC4 rpc;

	bool initialised;

	static const int SERVER_PORT = 12345;
	int						totalServers;
	bool					allServersChecked;
};
