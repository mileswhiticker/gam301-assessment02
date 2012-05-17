#ifndef PACKET_HPP
#define PACKET_HPP

enum PACKET_TYPE
{
	PACKET_INVALID = 0,
	PACKET_NETPING,
	//
	PACKET_HEARTBEAT,
	PACKET_HEARTBEAT_REPLY,
	//
	PACKETCLIENT_FINDSERVER,
	PACKETSERVER_SERVERHERE,
	PACKETCLIENT_GETCLIENTLIST,
	//
	PACKETCLIENT_CONNECTTRY,
	PACKETSERVER_CONNECTCANCELLED,
	PACKETSERVER_CONNECTSUCCESS,
	PACKETSERVER_CLIENTCONNECTED,
	PACKETCLIENT_CLIENTCONNECTED_RECIEPT,
	//
	PACKETPEER_CHATMESSAGE,
	PACKETPEER_CHATMESSAGE_RECIEPT,
	PACKETPEER_ENTITYNEW,
	PACKETPEER_ENTITYMOVE,
	PACKETPEER_ENTITYROTATE,
	PACKETPEER_ENTITYPOS,
	PACKETPEER_ENTITYRCPT,
	//
	PACKETSERVER_DISCONNECTING,
	PACKETCLIENT_DISCONNECTING
	//
};

struct Packet
{
	Packet(PACKET_TYPE a_type = PACKET_INVALID)
	{
		type = a_type;
	}
	PACKET_TYPE type;
};

struct PacketPing : public Packet
{
	PacketPing(int a_uid, PACKET_TYPE a_type = PACKET_INVALID)
	:	Packet(a_type)
	,	uid(a_uid)
	{
		//
	}
	int uid;
};

struct PacketNodeName : public Packet
{
	PacketNodeName(int a_nodeuid, std::string a_nodeName, PACKET_TYPE a_type = PACKET_INVALID)
	:	Packet(a_type)
	,	nodeUID(a_nodeuid)
	{
		for(unsigned short i=0;i<a_nodeName.size() && i < 24;i++)
		{
			nodename[i] = a_nodeName[i];
		};
		nodename[a_nodeName.size()] = '\0';
	};
	void SetNodeName(std::string a_nodeName)
	{
		for(unsigned short i=0;i<a_nodeName.size() && i < 24;i++)
		{
			nodename[i] = a_nodeName[i];
		};
		nodename[a_nodeName.size()] = '\0';
	};
	//
	int nodeUID;
	char nodename[25];
};

struct PacketNodeAddress : public Packet
{
	PacketNodeAddress(int a_nodeuid, sockaddr_in a_node_address, PACKET_TYPE a_type = PACKET_INVALID)
	:	Packet(a_type)
	,	nodeUID(a_nodeuid)
	,	node_address(a_node_address)
	{
		//
	};
	//
	int nodeUID;
	sockaddr_in node_address;
};

struct PacketChatMsg : public Packet
{
	PacketChatMsg(int a_nodeuid, std::string chat_msg_in, double a_msgTime, PACKET_TYPE a_type = PACKET_INVALID)
	:	Packet(a_type)
	,	nodeUID(a_nodeuid)
	,	msgTime(a_msgTime)
	{
		for(unsigned short i=0;i<chat_msg_in.size() && i < 99;i++)
		{
			chat_msg[i] = chat_msg_in[i];
		};
		chat_msg[chat_msg_in.size()] = '\0';
	}
	void SetChatMsg(std::string chat_msg_in)
	{
		for(unsigned short i=0;i<chat_msg_in.size() && i < 99;i++)
		{
			chat_msg[i] = chat_msg_in[i];
		};
		chat_msg[chat_msg_in.size()] = '\0';
	}
	char chat_msg[100];
	int nodeUID;
	double msgTime;
};

struct PacketRcpt : public Packet
{
	PacketRcpt(int a_nodeuid, double a_msgTime, PACKET_TYPE a_type = PACKET_INVALID)
	:	Packet(a_type)
	,	nodeUID(a_nodeuid)
	,	msgTime(a_msgTime)
	{
		//
	}
	int nodeUID;
	double msgTime;
};

/*
	PACKETPEER_ENTITYNEW,
	PACKETPEER_ENTITYMOVE,
	PACKETPEER_ENTITYROTATE,
*/

struct PacketEntityTransf : public Packet
{
	PacketEntityTransf(int a_nodeuid,  double a_msgTime, float a_x, float a_y, float a_z, PACKET_TYPE a_type = PACKET_INVALID)
	:	Packet(a_type)
	,	nodeUID(a_nodeuid)
	,	msgTime(a_msgTime)
		//
	,	x(a_x)
	,	y(a_y)
	,	z(a_z)
	{
		//
	}
	float x;
	float y;
	float z;
	int nodeUID;
	double msgTime;
};

#endif	//PACKET_HPP