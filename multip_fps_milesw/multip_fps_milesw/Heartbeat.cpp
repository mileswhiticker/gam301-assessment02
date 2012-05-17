#include "NetworkManager.hpp"
#include "Defs.hpp"
#include "Packet.hpp"

void NetworkManager::UpdateAll()
{
	//set all connected nodes to be immediately pinged next update
	for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
	{
		m_ConnectedNodes[n].timeout = HEARTBEAT_INTERVAL;
		m_ConnectedNodes[n].time_since_last_heartbeat = HEARTBEAT_RESEND_INTERVAL;
	}
}

void NetworkManager::Heartbeat(double a_DeltaT)
{
	//loop through and check to see if any clients need to be pinged / are about to timeout
	for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
	{
		m_ConnectedNodes[n].timeout += a_DeltaT;
		m_ConnectedNodes[n].time_since_last_heartbeat += a_DeltaT;
		//
		if(m_ConnectedNodes[n].timeout > HEARTBEAT_INTERVAL && m_ConnectedNodes[n].time_since_last_heartbeat > HEARTBEAT_RESEND_INTERVAL)
		{
			//ping that client
			PacketNodeName p(m_NodeUID, m_NodeName, PACKET_HEARTBEAT);
			sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
			m_ConnectedNodes[n].time_since_last_heartbeat = 0;
		}
	}
}
