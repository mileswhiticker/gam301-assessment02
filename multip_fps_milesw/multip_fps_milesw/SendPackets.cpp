#include "NetworkManager.hpp"
#include "Packet.hpp"
#include "Defs.hpp"

#include <iostream>

void NetworkManager::SendPackets(double a_DeltaT)
{
	//ping connected peers, update timeouts etc
	Heartbeat(a_DeltaT);

	//see if we're trying to connect to a server
	if(m_TargetServerUID)
		AttemptConnect(a_DeltaT);
	else if(m_TLeftDirectConnectTimeout > 0)
	{
		m_TLeftDirectConnectPing -= a_DeltaT;
		m_TLeftDirectConnectTimeout += a_DeltaT;
		if(m_TLeftDirectConnectPing <= 0)
		{
			//send an ruthere unicast packet
			m_TLeftDirectConnectPing = CONNECT_RESEND_INTERVAL;
			Packet p(PACKETCLIENT_FINDSERVER);
			sendto(m_IOSocket, (const char*)&p, sizeof(p), 0, (SOCKADDR*)&m_DirectConnectTarget, sizeof(m_DirectConnectTarget));
		}
  		if(m_TLeftDirectConnectTimeout <= 0)
		{
			m_TLeftDirectConnectTimeout = 0;
		}
	}
	if(m_TLeftFindServers < SEARCH_MAX)
	{
		//if we go for a timeout period without finding any servers, stop automatically trying to connect
		m_TLeftFindServers += a_DeltaT;
		m_TLeftPingServers -= a_DeltaT;
		if(m_TLeftPingServers <= 0)
		{
			std::cout << "Pinging for available servers..." << std::endl;
			m_TLeftPingServers = SEARCH_PING_INTERVAL;
			UpdateServerList();
		}
		if(m_TLeftFindServers >= SEARCH_MAX)
		{
			//if we've searched this long and not found anything, give up
			m_IsConnectingToFirstAvailableServer = false;
			m_TLeftFindServers = SEARCH_MAX;
			std::cout << "Finished searching for servers" << std::endl;
		}
	}
	
	//----------------------------- see if we need to update our rotation
	if(m_NewRot)
	{
		m_NewRot = false;
		tLeftTransmitRot = UPDATE_TIMEOUT;
		//note: this will reset the timer if it was in the process of sending an old update
	}

	//see if we are in the process of updating our rotation
	if(tLeftTransmitRot > 0)
	{
		//update timers
		tNextTransmitRot -= a_DeltaT;
		tLeftTransmitRot -= a_DeltaT;

		//it's time to send a new update packet!
		if(tNextTransmitPos < 0)
		{
			tNextTransmitRot = MIN_UPDATE_WAIT;
			//ping dem
			for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
			{
				PacketEntityTransf p(m_NodeUID, m_AppTime, m_OwnEntityRotating.x, m_OwnEntityRotating.y, m_OwnEntityRotating.z, PACKETPEER_ENTITYROTATE);
				sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
				std::cout << "Sending new movement data from this client." << std::endl;
			}
		}
	}
	else
		tLeftTransmitRot = 0;
	
	//----------------------------- see if we need to update our movement
	if(m_NewMove)
	{
		m_NewMove = false;
		tLeftTransmitMove = UPDATE_TIMEOUT;
		//note: this will reset the timer if it was in the process of sending an old update
	}

	//see if we are in the process of updating our movement
	if(tLeftTransmitMove > 0)
	{
		//update timers
		tNextTransmitMove -= a_DeltaT;
		tLeftTransmitMove -= a_DeltaT;

		//it's time to send a new update packet!
		if(tNextTransmitMove < 0)
		{
			tNextTransmitMove = MIN_UPDATE_WAIT;
			//ping dem
			for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
			{
				PacketEntityTransf p(m_NodeUID, m_AppTime, m_OwnEntityMoving.x, m_OwnEntityMoving.y, m_OwnEntityMoving.z, PACKETPEER_ENTITYMOVE);
				sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
				std::cout << "Sending new movement data from this client." << std::endl;
			}
		}
	}
	else
		tLeftTransmitRot = 0;
	
	//----------------------------- see if it's time to update everyone with our position
	tLeftGlobalPosUpdate -= a_DeltaT;
	if(tLeftGlobalPosUpdate < 0)
	{
		tLeftGlobalPosUpdate = GLOBAL_POS_UPDATE;
		//it's time to send a new update packet!
		//send it to this connected client
		for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
		{
			PacketEntityTransf p(m_NodeUID, m_AppTime, m_OwnEntityPos.x, m_OwnEntityPos.y, m_OwnEntityPos.z, PACKETPEER_ENTITYPOS);
			sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
			std::cout << "Sending new position data from this client." << std::endl;
		}
	}
}
