#include "NetworkManager.hpp"
//#include "SceneManager.hpp"
//#include "WindowManager.hpp"
//#include "ChatManager.hpp"
#include "Packet.hpp"

#include <iostream>

bool NetworkManager::ProcessClientPacket(Packet* a_pRecievedPacket, sockaddr_in a_SourceAddress)
{
	bool processed = true;
	switch(a_pRecievedPacket->type)
	{
	case(PACKETSERVER_DISCONNECTING):
		{
			//disconnect from the server, clientside
			ClientDisconnectServer();
			break;
		}
	case(PACKETCLIENT_DISCONNECTING):
		{
			//another client is disconnecting, find them and clear their data
			PacketPing* recp = (PacketPing*)a_pRecievedPacket;
			for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
			{
				if(m_ConnectedNodes[n].node_uid == recp->uid)
				{
					std::string out = "";
					out += m_ConnectedNodes[n].nodeName;
					out += " has disconnected from the server.";
					std::cout << out << std::endl;
					//ChatManager::GetInstance().PrintTextLocal(out, CEGUI::colour(255,0,0));
					break;
				}
			}
			break;
		}
	case(PACKETSERVER_CONNECTSUCCESS):
		{
			//we were successfully added to the server
			m_ConnectedServerUID = m_TargetServerUID;
			m_TargetServerUID = 0;
			//SceneManager::GetInstance().GotoChatClient();
			m_TLeftFindServers = 0;
			//find the selected server node
			std::string out = "Connect to ";
			std::string serverName = "Unknown";
			std::string ipString = "x.x.x.x";
			for(unsigned short n=0;n<m_AvailableServers.size();n++)
			{
				if(m_AvailableServers[n].node_uid == m_ConnectedServerUID)
				{
					m_ConnectedNodes.push_back( NodeInfo(m_ConnectedServerUID, m_AvailableServers[n].nodeName, m_AvailableServers[n].address ) );
					serverName = m_AvailableServers[n].nodeName;
					ipString = inet_ntoa(m_AvailableServers[n].address.sin_addr);
					break;
				}
			}
			out += ipString;
			out += " successful. This server is hosted by: ";
			out += serverName;
			std::cout << out << std::endl;
			//ChatManager::GetInstance().PrintTextLocal(out,CEGUI::colour(0,0,255));
			//ChatManager::GetInstance().AddClientName(serverName,false,true);

			break;
		}
	case(PACKETSERVER_CLIENTCONNECTED):
		{
			//add the other client to this program's list of connected nodes
			PacketNodeAddress* recp = (PacketNodeAddress*)a_pRecievedPacket;
			m_ConnectedNodes.push_back( NodeInfo(recp->nodeUID, "Unknown[pinged]", recp->node_address ) );
			//set time_since_last_heartbeat to HEARTBEAT_INTERVAL so that ProcessNetworkIO() will immediately ping the new peer next time it runs
			m_ConnectedNodes.back().time_since_last_heartbeat = HEARTBEAT_INTERVAL;
			m_ConnectedNodes.back().timeout = 0;
			//ChatManager::GetInstance().AddClientName("Unknown[pinged]");
			break;
		}
	default:
		{
			processed = false;
			break;
		}
	}
	return processed;
}

bool NetworkManager::IsConnectedToServer()
{
	if(m_IsServing || m_ConnectedServerUID)
		return true;
	return false;
}
