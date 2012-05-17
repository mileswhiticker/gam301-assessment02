#include "NetworkManager.hpp"
//#include "ChatManager.hpp"
#include "Packet.hpp"
#include "Defs.hpp"

#include <iostream>

void NetworkManager::ProcessNetworkIO(double a_DeltaT)
{
	//check to see if there are packets waiting
	fd_set checksockets;
	checksockets.fd_count = 1;
	checksockets.fd_array[0] = m_IOSocket;
	struct timeval t;
	t.tv_sec=0;
	t.tv_usec=0;
	int waiting = select(NULL, &checksockets, NULL, NULL, &t);
	//recieve and process all waiting packets
	while(waiting > 0)
	{
		// read packet
		char buffer[100000];
		sockaddr_in from_address; //temporary sockaddr to get the IP of the sender
		from_address.sin_family = AF_INET;
		from_address.sin_port = htons(1328);
		from_address.sin_addr.s_addr = INADDR_ANY;
		int length = sizeof(from_address); //the size of the INADDR_ANY address for receiving on

		int result = recvfrom(m_IOSocket, buffer, sizeof(buffer), 0, (SOCKADDR*) &from_address, &length);

		Packet* recievedPacket = (Packet*)buffer; //cast into a packet
		waiting--;
		//parse the recieved packet, if it is not empty
		if(recievedPacket->type == PACKET_NETPING)
		{
			//compare the uid, and if it's one of our own then update our internal ip address
			if( ((PacketPing*)recievedPacket)->uid == m_NodeUID)
			{
				m_LocalAddress.sin_addr.s_addr = from_address.sin_addr.s_addr;
			}
			continue;
		}

		//ignore any other packets from self
		if(m_LocalAddress.sin_addr.s_addr == from_address.sin_addr.s_addr)
		{
			continue;
		}
		std::cout << "	Recieving packet " << recievedPacket->type << std::endl;

		//first, handle any packets we want when both serving and not
		bool packet_processed = true;
		switch(recievedPacket->type)
		{
		case(PACKETSERVER_SERVERHERE):
			{
				//check to see if in list of available servers. if not, add it. if so, refresh it
				bool alreadyAdded = false;
				PacketNodeName* recp = (PacketNodeName*)recievedPacket;
				for(unsigned short n=0;n<m_AvailableServers.size();n++)
				{
					if(m_AvailableServers[n].address.sin_addr.S_un.S_addr == from_address.sin_addr.S_un.S_addr)
					{
						//refresh it
						std::cout << "Server: " << inet_ntoa(from_address.sin_addr) << " refreshed." << std::endl;
						alreadyAdded = true;
						m_AvailableServers[n].timeout = 0;
						break;
					}
				}
				if(!alreadyAdded)
				{
					//add it to the list
					std::cout << "Server: " << inet_ntoa(from_address.sin_addr) << " added." << std::endl;
						std::cout << "Server: " << recp->nodename << "/" << recp->nodeUID << " (" << inet_ntoa(from_address.sin_addr) << ") added." << std::endl;
					m_AvailableServers.push_back( NodeInfo(recp->nodeUID,recp->nodename,from_address) );
				}

				//if autoconnect is enabled or we're trying to direct connect, send a trytoconnect unicast packet
				if( (m_IsConnectingToFirstAvailableServer && !m_ConnectedServerUID) || m_TLeftDirectConnectTimeout > 0 )
				{
					//tell the application we are now trying to connect to a server
					m_TargetServerUID = recp->nodeUID;
					m_TLeftConnectTimeout = CONNECT_TIMEOUT;
					m_TLeftConnectPing = CONNECT_RESEND_INTERVAL;
					//send an iamconnecting unicast packet
					PacketNodeName p(m_NodeUID, m_NodeName, PACKETCLIENT_CONNECTTRY);
					sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&from_address,sizeof(from_address));
					std::cout << "Attempting to connect to server..." << std::endl;

					//if we were trying to direct connect, we can stop now
					if(m_TLeftDirectConnectTimeout)
						m_TLeftDirectConnectTimeout = 0;
				}
				break;
			}
		case(PACKET_HEARTBEAT):
			{
				//loop through connected clients to find the sender, and if they're connectet, reset their timeout and send a heartbeat reply back
				//also update the name, if it's changed
				PacketNodeName* recp = (PacketNodeName*)recievedPacket;
				for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
				{
					if(m_ConnectedNodes[n].node_uid == recp->nodeUID)
					{
						//check if their name has changed
						if( m_ConnectedNodes[n].nodeName.compare(recp->nodename) )
						{
							//update it
							if( m_ConnectedNodes[n].nodeName.compare("Unknown") && m_ConnectedNodes[n].nodeName.compare("Unknown[pinged]") )
							{
								//let the user know someone's manually changed their name
								//ChatManager::GetInstance().PrintTextLocal(m_ConnectedNodes[n].nodeName + " has changed their nick to " + recp->nodename + ".", CEGUI::colour(0,255,0));
							}
							m_ConnectedNodes[n].nodeName = recp->nodename;
						}
						m_ConnectedNodes[n].timeout = 0;

						//send a heartbeat reply back
						PacketNodeName p(m_NodeUID, m_NodeName, PACKET_HEARTBEAT_REPLY);
						sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
						break;
					}
				}
				break;
			}
		case(PACKET_HEARTBEAT_REPLY):
			{
				//loop through connected clients to find the sender, and if they're there (connected) then reset their TLastPacket.
				//also update the name, if it's changed
				PacketNodeName* p = (PacketNodeName*)recievedPacket;
				for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
				{
					if(m_ConnectedNodes[n].node_uid == p->nodeUID)
					{
						m_ConnectedNodes[n].timeout = 0;
						break;
					}
				}
				break;
			}
		case(PACKETPEER_CHATMESSAGE):
			{
				//send a reciept packet back so they'll stop spamming us
				PacketChatMsg* recp = (PacketChatMsg*)recievedPacket;
				PacketRcpt p(m_NodeUID, recp->msgTime, PACKETPEER_CHATMESSAGE_RECIEPT);
				sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&from_address,sizeof(from_address));

				//loop through recieved messages to see if we've already recieved this message
				bool alreadyHaveMsg = false;
				for (std::list<RecievedMessage>::iterator iter = m_RecievedMessages.begin(); iter != m_RecievedMessages.end(); iter++)
				{
					//if the current message is a later one than the one they sent (and we still haven't found it) then we don't have it
					if(iter->timeStamp > recp->msgTime)
						break;
					if(iter->timeStamp == recp->msgTime)
					{
						alreadyHaveMsg = true;
						break;
					}
				}

				//if we we dont have it...
				if(!alreadyHaveMsg)
				{
					//check to see if we're the one who said it...
					std::string talkerName = "Unknown";
					bool isHost = false;
					bool isSelf = false;
					if(recp->nodeUID == m_NodeUID)
					{
						isSelf = true;
						talkerName = m_NodeName;
						if(m_IsServing)
							isHost = true;
					}
					else
					{
						//find the talker's name in the connected nodes
						for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
						{
							if(m_ConnectedNodes[n].node_uid == recp->nodeUID)
							{
								talkerName = m_ConnectedNodes[n].nodeName;
								break;
							}
						}
						if(m_ConnectedServerUID == recp->nodeUID)
							isHost = true;
					}
					//dislay it to the user
					//ChatManager::GetInstance().SayText(talkerName, recp->chat_msg, isSelf ? CEGUI::colour(0,0,255) : CEGUI::colour(0,0,0), isHost);
					
					//add a new message
					RecievedMessage newMsg(recp->nodeUID, recp->msgTime);
					m_RecievedMessages.push_back(newMsg);

					//sort the messages, so that they will be faster to parse
					if(m_RecievedMessages.size() > 1)
					{
						std::list<RecievedMessage>::iterator iter = m_RecievedMessages.end();
						iter--;
						while(iter->timeStamp > recp->msgTime)
						{
							m_RecievedMessages.insert(iter, newMsg);
							std::list<RecievedMessage>::iterator iter2 = iter;
							m_RecievedMessages.erase(++iter2);
							iter--;
						}
					}
				}
				break;
			}
		case(PACKETPEER_CHATMESSAGE_RECIEPT):
			{
				//got a message reciept, so we can stop spamming the target client
				PacketRcpt* recp = (PacketRcpt*)recievedPacket;
				//find the dispatch queue for that client, and remove the last message if we're still sending it
				for(unsigned short n=0;n<m_MessageDispatchQueue.size();n++)
				{
					//check if we found the right queue
					if(m_MessageDispatchQueue[n].uid == recp->nodeUID)
					{
						//check to see if we're still trying to send the same message
						if(m_MessageDispatchQueue[n].queuedMessages[0].timeStamp == recp->msgTime)
						{
							//remove the message from the dispatch queue
							m_MessageDispatchQueue[n].queuedMessages.erase( m_MessageDispatchQueue[n].queuedMessages.begin() );
						}
						else if(m_MessageDispatchQueue[n].queuedMessages[0].timeStamp > recp->msgTime)
						{
							//this means the other client is replying to a message that has already timed out for us
						}
						break;
					}
				}
				break;
			}
		case(PACKETPEER_ENTITYPOS):
		case(PACKETPEER_ENTITYMOVE):
		case(PACKETPEER_ENTITYROTATE):
			{
				PacketEntityTransf* recp = (PacketEntityTransf*)recievedPacket;
				std::map<int, Ogre::Vector3> updateTarget = m_EntityMoves;
				bool* pTargetInformer = &m_NewMove;
				//
				if(recievedPacket->type == PACKETPEER_ENTITYROTATE)
				{
					pTargetInformer = &m_NewRot;
					updateTarget = m_EntityRotations;
				}
				else if(recievedPacket->type == PACKETPEER_ENTITYPOS)
				{
					pTargetInformer = &m_NewPos;
					updateTarget = m_EntityPositions;
				}
				//send a reciept packet back so they'll stop spamming us
				/*PacketRcpt p(m_NodeUID, recp->msgTime, PACKETPEER_ENTITYRCPT);
				sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&from_address,sizeof(from_address));*/
				
				//loop through recieved messages to see if we've already recieved this message
				bool alreadyHaveMsg = false;
				for (std::list<RecievedMessage>::iterator iter = m_RecievedMessages.begin(); iter != m_RecievedMessages.end(); iter++)
				{
					//if the current message is a later one than the one they sent (and we still haven't found it) then we don't have it
					if(iter->timeStamp > recp->msgTime)
						break;
					if(iter->timeStamp == recp->msgTime)
					{
						alreadyHaveMsg = true;
						break;
					}
				}
				//if we we don't have it...
				if(!alreadyHaveMsg)
				{
					bool found = false;
					//check to see if there's already an entry in the new entity map
					for(std::map<int, Ogre::Vector3>::iterator iter = updateTarget.begin(); iter != updateTarget.end(); iter++)
					{
						//update the entity transform; the game manager will grab this in it's own time
						if(iter->first == recp->nodeUID)
						{
							//update the stored value
							iter->second = Ogre::Vector3(recp->x, recp->y, recp->z);
						}
					}
					if(!found)
					{
						//add the value in
						*pTargetInformer = true;
						std::pair<int, Ogre::Vector3> newEntry;
						newEntry.first = recp->nodeUID;
						newEntry.second = Ogre::Vector3(recp->x, recp->y, recp->z);
						break;
					}
				}
				break;
			}
		default:
			{
				//deal with any server-only packets
				if(m_IsServing)
					ProcessServerPacket(recievedPacket, from_address);
				//deal with any client-only packets
				else
					ProcessClientPacket(recievedPacket, from_address);
				break;
			}
		}
	}

	//process outward bound packets
	SendPackets(a_DeltaT);

	//see if we're trying to send message/s
	AttemptDispatchMessages(a_DeltaT);

	//update the application time
	//this is only used for unique timestamps, and by this app instance
	m_AppTime += a_DeltaT;
}
