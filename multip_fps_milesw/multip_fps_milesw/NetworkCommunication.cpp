#include "NetworkManager.hpp"
#include "ChatManager.hpp"
#include "Packet.hpp"

void NetworkManager::ConnectToServer(std::string a_ServerIP)
{
	//set these, and update will periodically ping it to see if a server is running
	m_DirectConnectTarget.sin_addr.s_addr = inet_addr(a_ServerIP.c_str());
	m_TLeftDirectConnectTimeout = CONNECT_TIMEOUT;
	m_TLeftFindServers = 0;		//don't want to be scanning for other random servers
}

void NetworkManager::Ping()
{
	PacketPing p(m_NodeUID,PACKET_NETPING);
	sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_BroadcastAddress,sizeof(m_BroadcastAddress));
};

void NetworkManager::UpdateServerList()
{
	//send an ruthere broadcast packet, any servers reached will send a reply back
	Packet p(PACKETCLIENT_FINDSERVER);
	sendto(m_IOSocket, (const char*)&p, sizeof(p), 0, (SOCKADDR*)&m_BroadcastAddress, sizeof(m_BroadcastAddress));
}
