#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "Defs.hpp"
#include <vector>

#define NOMINMAX
#include "WinSock2.h"
#include <vector>
#include <string>
#include <list>
#include <map>

#include <Ogre/OgreVector3.h>
struct Packet;

namespace CEGUI
{
	class colour;
	class Listbox;
	class Vector3;
};

class NetworkManager
{
public:
	static NetworkManager& GetInstance()
	{
		static NetworkManager instance;
		return instance;
	};
	void SetPeerList(CEGUI::Listbox* a_pPeerList);
	bool InitialiseNetworking();
	void EnqueueChatMessage(std::string a_Text, int a_SourceUID = 0);
	void AttemptDispatchMessages(double a_DeltaT);
	//
	void UpdateServerList();
	std::vector<NodeInfo> GetServerList();
	//
	void UpdateAll();
	void Heartbeat(double a_DeltaT);
	void ProcessNetworkIO(double a_DeltaT);
	//
	void AttemptConnect(double a_DeltaT);
	void ConnectToServer(std::string a_ServerIP);
	void ConnectToFirstAvailableServer();
	//
	bool DefaultServe();
	void Disconnect();
	void ServerDisconnectClient(int a_UID);
	void ClientDisconnectServer();
	//
	bool CheckIsServing();
	int& GetNodeUID();
	std::string m_NodeName;
	//
	bool HaveEntitiesMoved();
	bool HaveEntitiesRotated();
	bool HaveEntitiesUpdatedPos();
	void SetOwnEntityMove(Ogre::Vector3 a_NewTransform);
	void SetOwnEntityRotate(Ogre::Vector3 a_NewTransform);
	void SetOwnEntityPos(Ogre::Vector3 a_NewTransform);
	std::map<int, Ogre::Vector3> NetworkManager::GetNewEntityMoves();
	std::map<int, Ogre::Vector3> GetNewEntityRotations();
	std::map<int, Ogre::Vector3> GetNewEntityPos();
	//
	bool HaveNewPlayersConnected();
	NodeInfo GetNewConnectee();
	//
	bool IsConnectedToServer();
	//
private:
	bool m_Connected;
	NetworkManager();
	void Ping();
	bool Serve();
	bool StopServe();
	void DisconnectClient(int a_UID);
	//
	bool ProcessClientPacket(Packet* a_pRecievedPacket, sockaddr_in a_SourceAddress);
	bool ProcessServerPacket(Packet* a_pRecievedPacket, sockaddr_in a_SourceAddress);
	void SendPackets(double a_DeltaT);
	//
	std::vector<NodeInfo> m_NewConnectees;
	std::map<int, Ogre::Vector3> m_EntityMoves;
	std::map<int, Ogre::Vector3> m_EntityRotations;
	std::map<int, Ogre::Vector3> m_EntityPositions;
	bool m_NewPos;
	bool m_NewRot;
	bool m_NewMove;
	double tLeftTransmitPos;
	double tNextTransmitPos;
	double tLeftTransmitRot;
	double tNextTransmitRot;
	double tLeftTransmitMove;
	double tNextTransmitMove;
	Ogre::Vector3 m_OwnEntityMoving;
	Ogre::Vector3 m_OwnEntityRotating;
	Ogre::Vector3 m_OwnEntityPos;
	//
	double tLeftGlobalPosUpdate;
	//
	bool m_IsServing;
	bool m_IsConnectingToFirstAvailableServer;
	//
	std::vector<NodeInfo> m_ConnectedNodes;
	std::vector<NodeInfo> m_AvailableServers;
	CEGUI::Listbox* m_pPeerList;
	//
	double m_TLeftDirectConnectTimeout;
	double m_TLeftDirectConnectPing;
	sockaddr_in m_DirectConnectTarget;
	//
	int m_NodeUID;
	int m_ConnectedServerUID;
	int m_TargetServerUID;
	//
	double m_TLeftConnectTimeout;
	double m_TLeftConnectPing;
	double m_TLeftFindServers;
	double m_TLeftPingServers;
	//
	SOCKET m_IOSocket;
	sockaddr_in m_LocalAddress;
	sockaddr_in m_BroadcastAddress;
	//
	double m_AppTime;
	std::list<RecievedMessage> m_RecievedMessages;
	//std::vector<QueuedMessage> m_QueuedClientMessages;
	std::vector<DispatchMessageQueue> m_MessageDispatchQueue;
};

#endif	//NETWORK_HPP