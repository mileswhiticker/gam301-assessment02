#include "NetworkManager.hpp"
#include "Packet.hpp"
#include "Defs.hpp"

#include <iostream>

void NetworkManager::AttemptConnect(double a_DeltaT)
{
	//check to see if we're trying to connect to a server
	if(m_TargetServerUID)
	{
		if(m_ConnectedServerUID)
		{
			m_TargetServerUID = 0;
			return;
		}
		m_TLeftConnectTimeout -= a_DeltaT;
		m_TLeftConnectPing -= a_DeltaT;
		if(m_TLeftConnectTimeout <= 0)
		{
			//the connect timed out
			m_TLeftConnectTimeout = CONNECT_TIMEOUT;
			std::cout << "Could not connect to server - timed out!" << std::endl;

			//remove that server from the available server list, and try to connect to another (if there are any others)
			for(unsigned short n=0;n<m_AvailableServers.size();n++)
			{
				if(m_AvailableServers[n].node_uid == m_TargetServerUID)
				{
					m_AvailableServers.erase(m_AvailableServers.begin() + n);
					break;
				}
			}
			//try to connect to the next server in line
			if(m_IsConnectingToFirstAvailableServer && m_AvailableServers.size())
			{
				m_TLeftConnectTimeout = CONNECT_TIMEOUT;
				m_TLeftConnectPing = CONNECT_RESEND_INTERVAL;
				m_TargetServerUID = m_AvailableServers[0].node_uid;
			}
			else
			{
				m_TargetServerUID = 0;
			}
		}
		//see if we want to ping the target server again
		else if(m_TLeftConnectPing <= 0 && m_TargetServerUID)
		{
			//time to ping the server again
			m_TLeftConnectPing = CONNECT_RESEND_INTERVAL;
			//find the target server from the list of available servers
			for(unsigned short n=0;n<m_AvailableServers.size();n++)
			{
				if(m_AvailableServers[n].node_uid == m_TargetServerUID)
				{
					PacketNodeName p(m_TargetServerUID, m_NodeName, PACKETCLIENT_CONNECTTRY);
					sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_AvailableServers[n].address,sizeof(m_AvailableServers[n].address));
					std::cout << "Attempting to connect to server..." << std::endl;
					break;
				}
			}
		}
	}
}
