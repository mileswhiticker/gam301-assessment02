#ifndef DEFS_HPP
#define DEFS_HPP

#define NOMINMAX
#include "WinSock2.h"
#include <string>
#include <vector>

#define HEARTBEAT_TIMEOUT 120
#define HEARTBEAT_INTERVAL 10
#define HEARTBEAT_RESEND_INTERVAL 2

#define CONNECT_TIMEOUT 5
#define CONNECT_RESEND_INTERVAL 0.1

#define SEARCH_MAX 7					//time to search for servers
#define SEARCH_PING_INTERVAL 2			//time inbetween pings searching for servers

#define MESSAGE_TIMEOUT 5
#define MESSAGE_RESEND_INTERVAL 0.5		//time between resending the earliest chat message the client hasn't recieved

#define MIN_UPDATE_WAIT 0.5				//minimum waiting time between entity transform updates
#define UPDATE_TIMEOUT 5				//waiting time for new transform packets before timeout

#define GLOBAL_POS_UPDATE 3				//time in between position updates

struct NodeInfo
{
	NodeInfo(int a_uid, std::string a_nodeName, sockaddr_in a_address)
	:	time_since_last_heartbeat(HEARTBEAT_INTERVAL)
	,	timeout(0)
	,	node_uid(a_uid)
	,	nodeName(a_nodeName)
	,	address(a_address)
	{
		//
	}
	int node_uid;
	std::string nodeName;
	sockaddr_in address;
	double timeout;
	double time_since_last_heartbeat;
};

//only need the source of the message and the timestamp - the timestamp functions as the message's unique id
struct RecievedMessage
{
	RecievedMessage(int a_uid, double a_timeStamp)
	:	uid(a_uid)
	,	timeStamp(a_timeStamp)
	{
		//
	};
	int uid;
	double timeStamp;
};

struct QueuedMessage
{
	QueuedMessage(int a_uid, std::string a_text, double a_timeSent)
	:	text(a_text)
	,	timeStamp(a_timeSent)
	,	timeout(0)
	,	time_left_resend(0)		//0 so that the message will be sent immediately next time ProcessNetworkIO() calls
	,	uid(a_uid)				//the originating client uid
	{
		//
	}
	std::string text;
	int uid;
	double timeStamp;
	double time_left_resend;
	double timeout;
};

struct DispatchMessageQueue
{
	DispatchMessageQueue(int a_uid)
	:	uid(a_uid)
	{
		//
	}
	int uid;
	std::vector<QueuedMessage> queuedMessages;
};

#endif	//DEFS_HPP