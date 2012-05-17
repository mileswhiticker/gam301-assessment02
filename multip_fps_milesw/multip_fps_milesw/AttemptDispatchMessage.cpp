#include "NetworkManager.hpp"
//#include "ChatManager.hpp"
#include "Packet.hpp"
#include "Defs.hpp"

#include <iostream>

void NetworkManager::EnqueueChatMessage(std::string a_Text, int a_SourceUID)
{
	if(!a_SourceUID)
		a_SourceUID = m_NodeUID;
	//add the message to the dispatch queue for all connected clients
	for(unsigned short n=0; n<m_ConnectedNodes.size(); n++)
	{
		bool foundClientDispatchQueue = false;
		for(unsigned short m=0; m<m_MessageDispatchQueue.size(); m++)
		{
			if(m_MessageDispatchQueue[m].uid == m_ConnectedNodes[n].node_uid)
			{
				foundClientDispatchQueue = true;
				//add the messasge to the dispatch queue for that client
				//the network manager will automatically resend it to the client until it gets a reciept packet from the client for that message
				m_MessageDispatchQueue[m].queuedMessages.push_back( QueuedMessage(a_SourceUID, a_Text, m_AppTime) );
				break;
			}
		}
		//couldnt find the dispatch queue for that client, better add it
		if(!foundClientDispatchQueue)
		{
			m_MessageDispatchQueue.push_back( DispatchMessageQueue(m_ConnectedNodes[n].node_uid) );
			m_MessageDispatchQueue.back().queuedMessages.push_back( QueuedMessage(a_SourceUID, a_Text, m_AppTime) );
		}
	}
	//display the message
	//ChatManager::GetInstance().SayText(m_NodeName, a_Text, CEGUI::colour(0,0,255), true);
}

void NetworkManager::AttemptDispatchMessages(double a_DeltaT)
{
	//loop through the message dispatch queues
	for(unsigned short n=0; n<m_MessageDispatchQueue.size(); n++)
	{
		//if there are no waiting messages for that queue, continue
		if(!m_MessageDispatchQueue[n].queuedMessages.size())
			continue;
		//only attempt to send the earliest waiting message, so that messages will always be sent in order
		m_MessageDispatchQueue[n].queuedMessages[0].timeout += a_DeltaT;
		m_MessageDispatchQueue[n].queuedMessages[0].time_left_resend -= a_DeltaT;

		//if the message timed out, dont send the message
		if(m_MessageDispatchQueue[n].queuedMessages[0].timeout >= MESSAGE_TIMEOUT)
		{
			//ServerDisconnectClient(m_MessageDispatchQueue[n].uid);
			m_MessageDispatchQueue[n].queuedMessages.erase(m_MessageDispatchQueue[n].queuedMessages.begin());
			continue;
		}

		//is it time to resend the message?
		if(m_MessageDispatchQueue[n].queuedMessages[0].time_left_resend <= 0)
		{
			//reset the resend_timer
			m_MessageDispatchQueue[n].queuedMessages[0].time_left_resend = MESSAGE_RESEND_INTERVAL;

			//find the client which we want to send to
			for(unsigned short i=0; i<m_ConnectedNodes.size(); i++)
			{
				if(m_ConnectedNodes[i].node_uid == m_MessageDispatchQueue[n].uid)
				{
					PacketChatMsg p(m_NodeUID, m_MessageDispatchQueue[n].queuedMessages[0].text, m_MessageDispatchQueue[n].queuedMessages[0].timeStamp, PACKETPEER_CHATMESSAGE);
					sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[i].address,sizeof(m_ConnectedNodes[i].address));
					break;
				}
			}
		}
	}
}
