// Fill out your copyright notice in the Description page of Project Settings.

#include "RakNetRP.h"
#include <functional>
#include <string>
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Engine.h"
#include "ReplicaBase.h"
#include "RN4UE4GameInstance.h"
#include "Engine/World.h"
#include "ReplicaRigidDynamicClient.h"

using namespace std::placeholders;

DEFINE_LOG_CATEGORY(RakNet_RakNetRP);

class SampleConnection : public Connection_RM3 
{
public:
	SampleConnection(const SystemAddress &_systemAddress, RakNetGUID _guid) : Connection_RM3(_systemAddress, _guid) {}
	virtual ~SampleConnection() {}

	// See documentation - Makes all messages between ID_REPLICA_MANAGER_DOWNLOAD_STARTED and ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE arrive in one tick
	bool QueryGroupDownloadMessages(void) const { return true; }

	virtual Replica3 *AllocReplica(BitStream *allocationId, ReplicaManager3 *replicaManager3)
	{
		ARakNetRP* manager = (ARakNetRP*)replicaManager3;
		RakString typeName;
		allocationId->Read(typeName);
		return manager->GetObjectFromType(typeName);
	}
};

// Sets default values
ARakNetRP::ARakNetRP() : ReplicaManager3()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	initialised = false;
	totalServers = -1;
}

// Called when the game starts or when spawned
void ARakNetRP::BeginPlay()
{
	Super::BeginPlay();

	URN4UE4GameInstance* GameInstance = static_cast<URN4UE4GameInstance*>(GetGameInstance());
	ensureMsgf(GameInstance != nullptr, TEXT("RakNetRP - GameInstance is not of type URN4UE4GameInstance"));
	GameInstance->RegisterRakNetManager(this);

	auto fp = std::bind(&ARakNetRP::CreateBoundarySlot, this, _1, _2);
	rpc.RegisterSlot("CreateBoundary", fp, 0);
	auto deleteFunction = std::bind(&ARakNetRP::DeleteBoundarySlot, this, _1, _2);
	rpc.RegisterSlot("DeleteBoundary", deleteFunction, 0);
	auto getExpectedServersFunction = std::bind(&ARakNetRP::GetExpectedServersSlot, this, _1, _2);
	rpc.RegisterSlot("NumberServers", getExpectedServersFunction, 0);

	allServersChecked = false;
}

// Called every frame
void ARakNetRP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (rakPeer == nullptr)	{	return;	}

	for (p = rakPeer->Receive(); p; rakPeer->DeallocatePacket(p), p = rakPeer->Receive())
	{
		switch (p->data[0])
		{
		case ID_CONNECTION_ATTEMPT_FAILED:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_CONNECTION_ATTEMPT_FAILED\n"));
			rakPeer->CloseConnection(p->systemAddress, false);
			//rakPeer->GetConnectionList(SystemAddress *remoteSystems, unsigned short *numberOfSystems)
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_NO_FREE_INCOMING_CONNECTIONS\n"));
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_CONNECTION_REQUEST_ACCEPTED\n"));
			if (newConnectionCallback != nullptr)
			{
				newConnectionCallback(p->systemAddress);
			}
			break;
		case ID_NEW_INCOMING_CONNECTION:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_NEW_INCOMING_CONNECTION from %s\n"), p->systemAddress.ToString());
			if (newConnectionCallback != nullptr)
			{
				newConnectionCallback(p->systemAddress);
			}
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_DISCONNECTION_NOTIFICATION\n"));
			DroppedConnection(p->systemAddress.GetPort());
			break;
		case ID_CONNECTION_LOST:
			UE_LOG(RakNet_RakNetRP, Log, TEXT("ID_CONNECTION_LOST\n"));
			DroppedConnection(p->systemAddress.GetPort());
			break;
		case ID_ADVERTISE_SYSTEM:
			// The first conditional is needed because ID_ADVERTISE_SYSTEM may be from a system we are connected to, but replying on a different address.
			// The second conditional is because AdvertiseSystem also sends to the loopback
			if (rakPeer->GetSystemAddressFromGuid(p->guid) == UNASSIGNED_SYSTEM_ADDRESS &&
				rakPeer->GetMyGUID() != p->guid)
			{
				UE_LOG(RakNet_RakNetRP, Log, TEXT("Connecting to %s\n"), p->systemAddress.ToString(true));
				rakPeer->Connect(p->systemAddress.ToString(false), p->systemAddress.GetPort(), 0, 0);
			}
			break;
		case ID_SND_RECEIPT_LOSS:
		case ID_SND_RECEIPT_ACKED:
		{
			uint32_t msgNumber;
			memcpy(&msgNumber, p->data + 1, 4);

			DataStructures::List<Replica3*> replicaListOut;
			GetReplicasCreatedByMe(replicaListOut);
			unsigned int idx;
			for (idx = 0; idx < replicaListOut.Size(); idx++)
			{
				static_cast<ReplicaBase*>(replicaListOut[idx])->NotifyReplicaOfMessageDeliveryStatus(p->guid, msgNumber, p->data[0] == ID_SND_RECEIPT_ACKED);
			}
		}
		break;
		}
	}

	for (int i = 0; i < 11; i++)
	{
		if (rakPeer->GetInternalID(UNASSIGNED_SYSTEM_ADDRESS, 0).GetPort() != SERVER_PORT + i)
			rakPeer->AdvertiseSystem("255.255.255.255", SERVER_PORT + i, 0, 0, 0);
	}

	// TODO: Handle servers disconnecting
	if (!allServersChecked && initialised)
	{
		DataStructures::List<RakNet::SystemAddress> addresses;
		DataStructures::List<RakNet::RakNetGUID> guids;
		rakPeer->GetSystemList(addresses, guids);
		if (totalServers == addresses.Size())
		{
			allServersChecked = true;
		}
	}
}

void ARakNetRP::RPStartup()
{
	UE_LOG(RakNet_RakNetRP, Log, TEXT("ARakNetRP::RPStartup"));

	rakPeer = RakPeerInterface::GetInstance();
	SocketDescriptor socketDescriptor(0, 0);
	rakPeer->Startup(32, &socketDescriptor, 1);

	UE_LOG(RakNet_RakNetRP, Log, TEXT("ARakNetRP::RPStartup - passed startup"));

	// Start RakNet, up to 32 connections if the server
	rakPeer->AttachPlugin(this);
	SetNetworkIDManager(&networkIdManager);
	rakPeer->SetMaximumIncomingConnections(32);

	rakPeer->AllowConnectionResponseIPMigration(false);
	rakPeer->AttachPlugin(&rpc);

	UE_LOG(RakNet_RakNetRP, Log, TEXT("ARakNetRP::RPStartup - plugin attached"));

	initialised = true;
}

void ARakNetRP::RPDisconnect()
{
	if (rakPeer != nullptr)
	{
		rakPeer->Shutdown(100, 0);
		RakPeerInterface::DestroyInstance(rakPeer);
	}

	delete p;
	p = nullptr;
}

void ARakNetRP::RPConnect(const FString& host, const int port)
{
	rakPeer->Connect(TCHAR_TO_ANSI(*host), port, nullptr, 0);
}

void ARakNetRP::RPrpcSpawn(FVector pos, FVector dir)
{
	RakNet::BitStream testBs;
	testBs.WriteVector<float>(pos.X, pos.Y, pos.Z);
	testBs.WriteVector<float>(dir.X, dir.Y, dir.Z);

	DataStructures::List<RakNet::SystemAddress> addresses;
	DataStructures::List<RakNet::RakNetGUID> guids;
	rakPeer->GetSystemList(addresses, guids);

	for (unsigned int i = 0; i < addresses.Size(); ++i)
	{
		rpc.Signal("Spawn", &testBs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addresses[i], false, false);
	}
}

void ARakNetRP::RPrpcSignalAllServers(const FString& sharedIdentifier)
{
	DataStructures::List<RakNet::SystemAddress> addresses;
	DataStructures::List<RakNet::RakNetGUID> guids;
	rakPeer->GetSystemList(addresses, guids);

	const char* signalString = TCHAR_TO_ANSI(*sharedIdentifier);

	for (unsigned int i = 0; i < addresses.Size(); ++i)
	{
		rpc.Signal(signalString, nullptr, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addresses[i], false, false);
	}
}

void ARakNetRP::DroppedConnection(unsigned short Port)
{
	int rank = Port - 12345;
	DeleteBoundaryBox(rank);
}

UReplicaRigidDynamicClient* ARakNetRP::GetObjectFromType(RakString typeName)
{
	if (typeName == "ReplicaRigidDynamic") 
	{
		// spawn the object 
		FActorSpawnParameters Parameters;
		AActor* newReplica = (AActor*)GetWorld()->SpawnActor(AActor::StaticClass(), new FTransform(), Parameters);
		USceneComponent * RootSceneComponent = NewObject<USceneComponent>(newReplica,TEXT("RootScene"));
		newReplica->SetRootComponent(RootSceneComponent);
		UReplicaRigidDynamicClient* replicaClient = NewObject<UReplicaRigidDynamicClient>(newReplica);
		replicaClient->MarkAsReferenced();
		replicaClient->RegisterComponentWithWorld(GetWorld());
		return replicaClient;
	}

	return nullptr;
}

void ARakNetRP::CreateBoundarySlot(RakNet::BitStream * bitStream, Packet * packet)
{
	int rank;
	bitStream->Read<int>(rank);

	int geomType;
	bitStream->Read<int>(geomType);

	FVector pos;
	bitStream->ReadVector<float>(pos.X, pos.Y, pos.Z);

	switch (geomType)
	{
	case 1:
	{
		FVector normal;
		bitStream->ReadVector<float>(normal.X, normal.Y, normal.Z);
		CreateBoundaryPlane(rank, pos, normal);
	}
	break;
	case 3:
	{
		FVector size;
		bitStream->ReadVector<float>(size.X, size.Y, size.Z);
		CreateBoundaryBox(rank, pos, size);
	}
	break;
	default:
		break;
	}
}

void ARakNetRP::DeleteBoundarySlot(RakNet::BitStream * bitStream, Packet * packet)
{
	int rank;
	bitStream->Read<int>(rank);
	DeleteBoundaryBox(rank);
}

void ARakNetRP::GetExpectedServersSlot(RakNet::BitStream * bitStream, Packet * packet)
{
	int number;
	bitStream->Read<int>(number);
	totalServers = number;
}

void ARakNetRP::DeleteBoundaryBox_Implementation(int rank)
{
}

void ARakNetRP::CreateBoundaryBox_Implementation(int rank, FVector pos, FVector size)
{
}

void ARakNetRP::CreateBoundaryPlane_Implementation(int rank, FVector pos, FVector normal)
{
}

Connection_RM3* ARakNetRP::AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const {
	return new SampleConnection(systemAddress, rakNetGUID);
}

void ARakNetRP::DeallocConnection(Connection_RM3 *connection) const {
	delete connection;
}

bool ARakNetRP::GetAllServersChecked() const
{
	return allServersChecked;
}

int ARakNetRP::GetExpectedNumberOfServers() const
{
	return totalServers;
}

void ARakNetRP::ConnectToIP(const FString& address)
{
	FString host;
	FString port;
	address.Split(":", &host, &port);
	int portNumber = FCString::Atoi(*port);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 120.0f, FColor::Yellow, FString("ARakNetRP::ConnectToIP() Connecting to " + host + ":" + port));
	}

	RPConnect(host, portNumber);
}
