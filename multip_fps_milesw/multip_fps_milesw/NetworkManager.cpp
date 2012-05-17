#include "NetworkManager.hpp"
//#include "WindowManager.hpp"
//#include "ChatManager.hpp"
//#include "SceneManager.hpp"
#include "Packet.hpp"

#include <CEGUI/CEGUIColour.h>

#define NOMINMAX
#include "ws2tcpip.h"
#include "mswsock.h"
#include "WinSock2.h"
#include <iostream>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib") 

NetworkManager::NetworkManager()
:	m_IsServing(false)
,	m_NodeUID(rand())
,	m_IsConnectingToFirstAvailableServer(false)
,	m_AppTime(0)
,	m_ConnectedServerUID(0)
,	m_TargetServerUID(0)
,	m_TLeftConnectTimeout(0)
,	m_TLeftConnectPing(0)
,	m_TLeftFindServers(SEARCH_MAX)
,	m_TLeftPingServers(0)
,	m_pPeerList(NULL)
,	m_TLeftDirectConnectTimeout(0)
	//
,	m_NewPos(true)
,	m_NewRot(false)
,	m_OwnEntityMoving(Ogre::Vector3(0,0,0))
,	m_OwnEntityRotating(Ogre::Vector3(0,0,0))
	//
,	tLeftTransmitPos(0)
,	tNextTransmitPos(0)
,	tLeftTransmitRot(0)
,	tNextTransmitRot(0)
,	tLeftGlobalPosUpdate(0)
{
	int num = rand()%9999;
	char* suffix = new char[5];
	_itoa_s(num, suffix, 5, 10);
	m_NodeName = "DefaultUser";
	m_NodeName += suffix;
}

void NetworkManager::SetPeerList(CEGUI::Listbox* a_pPeerList)
{
	m_pPeerList = a_pPeerList;
}

bool NetworkManager::InitialiseNetworking()
{
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
	{
		std::cout << "WSAStartup failed." << std::endl;
		return false;
	}
	m_IOSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_IOSocket == SOCKET_ERROR)
	{
	   std::cout << "Error Opening socket: Error " << WSAGetLastError() << std::cout;
	   return false;
	}

	//setup the local (server) address the socket will listen on
	m_LocalAddress.sin_family = AF_INET;
	m_LocalAddress.sin_addr.s_addr = INADDR_ANY;//inet_addr("10.40.61.9")
	m_LocalAddress.sin_port = htons(1328);
	
	//setup the broadcast address
	m_BroadcastAddress.sin_family = AF_INET;
	m_BroadcastAddress.sin_port = htons(1328);
	m_BroadcastAddress.sin_addr.S_un.S_addr = inet_addr("255.255.255.255");

	//bind the socket (so that it listens)
	if(bind(m_IOSocket, (SOCKADDR*) &m_LocalAddress, sizeof(m_LocalAddress)) == SOCKET_ERROR) 
	{
		std::cout << "bind() failed: Error " << WSAGetLastError() << std::cout;
	}
	//automatically reset the socket connection if it exits prematurely
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	WSAIoctl(m_IOSocket,SIO_UDP_CONNRESET,&bNewBehavior,sizeof(bNewBehavior),NULL,0,&dwBytesReturned,NULL,NULL);
	
	//enable broadcasting on this socket
	BOOL bOptVal = TRUE;
	setsockopt(m_IOSocket,SOL_SOCKET,SO_BROADCAST,(const char *)&bOptVal,sizeof(BOOL));
	
	//join multicast group
	/*
	ip_mreq mreq;
	mreq.imr_multiaddr.S_un.S_addr = inet_addr("224.3.2.1");
	mreq.imr_interface.S_un.S_addr = INADDR_ANY;
	setsockopt(m_IOSocket,IPPROTO_IP,IP_ADD_MEMBERSHIP,(const char *)&mreq,sizeof(ip_mreq));
	*/

	//ping the local net a few times
	Ping();
	Ping();
	Ping();
	return true;
}

std::vector<NodeInfo> NetworkManager::GetServerList()
{
	return m_AvailableServers;
}

bool NetworkManager::Serve()
{
	//this one will generally allow a few user-defined options to be set
	return DefaultServe();
}

bool NetworkManager::DefaultServe()
{
	if(m_IsServing)
	{
		return false;
	}
	else
	{
		//setup server here
		std::string out = "Server created successfully using default configurations. Users can direct connect using IP: ";
		out += inet_ntoa(m_LocalAddress.sin_addr);
		std::cout << out << std::endl;
		//ChatManager::GetInstance().PrintTextLocal(out, CEGUI::colour(0,0,255));
		m_IsServing = true;
	}
	return true;
}

bool NetworkManager::CheckIsServing()
{
	return m_IsServing;
}

int& NetworkManager::GetNodeUID()
{
	return m_NodeUID;
}

void NetworkManager::ConnectToFirstAvailableServer()
{
	std::cout << "Connecting to first available server..." << std::endl;
	m_IsConnectingToFirstAvailableServer = true;
	m_TLeftFindServers = 0;		//next time networkProcessIO() calls, it will immediately ping for new servers
	//if there is a pre-existing list of servers to connect to, try the first one now
	if(m_AvailableServers.size())
	{
		//it will immediately attempt to connect to this server next time ProcessNetworkIO() calls
		m_TargetServerUID = m_AvailableServers[0].node_uid;
		m_TLeftConnectTimeout = 0;
		m_TLeftConnectPing = CONNECT_RESEND_INTERVAL;
	}
}

void NetworkManager::Disconnect()
{
	if(m_IsServing)
	{
		for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
		{
			ServerDisconnectClient(m_ConnectedNodes[n].node_uid);
		}
		//clear server / chat data, etc
		//SceneManager::GetInstance().GotoMainMenu();
		m_ConnectedNodes.erase(m_ConnectedNodes.begin(), m_ConnectedNodes.end());
		m_IsServing = false;
		//ChatManager::GetInstance().ClearChatInput();
		//ChatManager::GetInstance().ClearChatOutput();
	}
	else
	{
		ClientDisconnectServer();
		//SceneManager::GetInstance().GotoMainMenu();
	}
}

void NetworkManager::ServerDisconnectClient(int a_UID)
{
	//remove them from the connected nodes
	for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
	{
		if(m_ConnectedNodes[n].node_uid == a_UID)
		{
			//remove them and send a disconnect packet
			PacketPing recp(m_ConnectedNodes[n].node_uid, PACKETSERVER_DISCONNECTING);
			sendto(m_IOSocket,(char*)&recp,sizeof(recp),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
			m_ConnectedNodes.erase(m_ConnectedNodes.begin() + n);

			//now loop through the remaining client list and say that their friend dc'd
			for(unsigned short i=0;i<m_ConnectedNodes.size();i++)
			{
				PacketPing p(a_UID, PACKETCLIENT_DISCONNECTING);
				sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[i].address,sizeof(m_ConnectedNodes[i].address));
			}
			break;
		}
	}

	//remove their dispatch queue
	for(unsigned short n=0; n<m_MessageDispatchQueue.size(); n++)
	{
		if(m_MessageDispatchQueue[n].uid == a_UID)
		{
			m_MessageDispatchQueue.erase(m_MessageDispatchQueue.begin() + n);
			break;
		}
	}

	//print out to the local chatwindow
	for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
	{
		if(m_ConnectedNodes[n].node_uid == a_UID)
		{
			std::string out = "";
			out += m_ConnectedNodes[n].nodeName;
			out += " has disconnected from the server.";
			std::cout << out << std::endl;
			//ChatManager::GetInstance().PrintTextLocal(out, CEGUI::colour(255,0,0));
			break;
		}
	}
}

void NetworkManager::ClientDisconnectServer()
{
	//find the server details
	for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
	{
		if(m_ConnectedNodes[n].node_uid == m_ConnectedServerUID)
		{
			//send the server a packet telling them we're leaving
			PacketPing recp(m_NodeUID, PACKETCLIENT_DISCONNECTING);
			sendto(m_IOSocket,(char*)&recp,sizeof(recp),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
			break;
		}
	}

	//clear server / chat data, etc
	//SceneManager::GetInstance().GotoMainMenu();
	m_ConnectedNodes.erase(m_ConnectedNodes.begin(), m_ConnectedNodes.end());
	m_ConnectedServerUID = 0;
	//ChatManager::GetInstance().ClearChatInput();
	//ChatManager::GetInstance().ClearChatOutput();

	//clear the message dispatch queues
	m_MessageDispatchQueue.erase(m_MessageDispatchQueue.begin(), m_MessageDispatchQueue.end());

}

bool NetworkManager::HaveEntitiesMoved()
{
	if(m_EntityMoves.size())
		return true;
	return false;
}

bool NetworkManager::HaveEntitiesRotated()
{
	if(m_EntityRotations.size())
		return true;
	return false;
}

bool NetworkManager::HaveEntitiesUpdatedPos()
{
	if(m_EntityPositions.size())
		return true;
	return false;
}

std::map<int, Ogre::Vector3> NetworkManager::GetNewEntityMoves()
{
	m_NewPos = false;
	std::map<int, Ogre::Vector3> out;
	while(m_EntityMoves.size())
	{
		m_EntityMoves.erase(m_EntityMoves.begin());
	}
	return out;
}

std::map<int, Ogre::Vector3> NetworkManager::GetNewEntityRotations()
{
	m_NewRot = false;
	std::map<int, Ogre::Vector3> out = m_EntityRotations;
	while(m_EntityRotations.size())
	{
		m_EntityRotations.erase(m_EntityRotations.begin());
	}
	return m_EntityRotations;
}

std::map<int, Ogre::Vector3> NetworkManager::GetNewEntityPos()
{
	m_NewRot = false;
	std::map<int, Ogre::Vector3> out = m_EntityPositions;
	while(m_EntityPositions.size())
	{
		m_EntityPositions.erase(m_EntityPositions.begin());
	}
	return out;
}

void NetworkManager::SetOwnEntityMove(Ogre::Vector3 a_NewTransform)
{
	m_NewPos = true;
	m_OwnEntityMoving = a_NewTransform;
}

void NetworkManager::SetOwnEntityRotate(Ogre::Vector3 a_NewTransform)
{
	m_NewRot = true;
	m_OwnEntityRotating = a_NewTransform;
}

NodeInfo NetworkManager::GetNewConnectee()
{
	NodeInfo out = *m_NewConnectees.begin();
	m_NewConnectees.erase(m_NewConnectees.begin());
	return out;
}

bool NetworkManager::HaveNewPlayersConnected()
{
	if(m_NewConnectees.size())
		return true;
	return false;
}
