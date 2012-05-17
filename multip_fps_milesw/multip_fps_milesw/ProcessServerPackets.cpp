#include "NetworkManager.hpp"
//#include "WindowManager.hpp"
//#include "ChatManager.hpp"
#include "Packet.hpp"

#include <iostream>

bool NetworkManager::ProcessServerPacket(Packet* a_pRecievedPacket, sockaddr_in a_SourceAddress)
{
	bool processed = true;
	switch(a_pRecievedPacket->type)
	{
	case(PACKETCLIENT_FINDSERVER):
		{
			//send an iamhere unicast packet in reply
			PacketNodeName p(m_NodeUID, m_NodeName, PACKETSERVER_SERVERHERE);
			sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&a_SourceAddress,sizeof(a_SourceAddress));
			break;
		}
	case(PACKETCLIENT_GETCLIENTLIST):
		{
			//tell the client about all the other clients
			PacketNodeName newClientP(0, "", PACKETSERVER_CLIENTCONNECTED);
			for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
			{
				newClientP.nodeUID = m_ConnectedNodes[n].node_uid;
				newClientP.SetNodeName(m_ConnectedNodes[n].nodeName);
				sendto(m_IOSocket,(char*)&newClientP,sizeof(newClientP),0,(SOCKADDR*)&a_SourceAddress,sizeof(a_SourceAddress));
			}
			break;
		}
	case(PACKETCLIENT_CONNECTTRY):
		{
			//send an addingyoul unicast packet in reply
			//sending all the node info, because the client will be forced to connect to this server
			//this is so that in case of packet error, the client will only connect to one server... even if it's the wrong one :P
			//the alternative is connecting to multiple servers and breaking.
			PacketNodeName p(m_NodeUID, m_NodeName, PACKETSERVER_CONNECTSUCCESS);
			sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&a_SourceAddress,sizeof(a_SourceAddress));

			//tell all other connected nodes about the new client, then add the new client to the connected nodes list
			PacketNodeName* recp = (PacketNodeName*)a_pRecievedPacket;
			std::cout << "Client connected: " << recp->nodename << "/" << recp->nodeUID << std::endl;
			for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
			{
				PacketNodeName p(recp->nodeUID, recp->nodename, PACKETSERVER_CLIENTCONNECTED);
				sendto(m_IOSocket,(char*)&p,sizeof(p),0,(SOCKADDR*)&m_ConnectedNodes[n].address,sizeof(m_ConnectedNodes[n].address));
			}
			
			//tell the new client about all the peers
			PacketNodeAddress newClientP(0, sockaddr_in(), PACKETSERVER_CLIENTCONNECTED);
			for(unsigned short n=0;n<m_ConnectedNodes.size();n++)
			{
				newClientP.nodeUID = m_ConnectedNodes[n].node_uid;
				newClientP.node_address = m_ConnectedNodes[n].address;
				sendto(m_IOSocket,(char*)&newClientP,sizeof(newClientP),0,(SOCKADDR*)&a_SourceAddress,sizeof(a_SourceAddress));
			}

			//output to the chat window that a new client has joined
			std::string out = recp->nodename;
			out += " has joined the server.";
			std::cout << out << std::endl;
			//ChatManager::GetInstance().PrintTextLocal(out, CEGUI::colour(0,255,0));

			//add the client to the server's connected nodes
			m_ConnectedNodes.push_back( NodeInfo(recp->nodeUID, recp->nodename, a_SourceAddress) );

			//add to the list of new players
			m_NewConnectees.push_back(m_ConnectedNodes.back());
			
			break;
		}
	case(PACKETCLIENT_DISCONNECTING):
		{
			//disconnect the client, serverside
			ServerDisconnectClient( ((PacketPing*)a_pRecievedPacket)->uid );
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
