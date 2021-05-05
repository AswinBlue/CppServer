#ifndef __SERVER_H__
#define __SERVER_H__

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_client.h"
#include "net_server.h"
#include "net_connection.h"
#include "MessageTypes.h"


class Server : public net::server_interface <MessageTypes>
{
public:
	Server(uint16_t nPort) : net::server_interface<MessageTypes>(nPort)
	{} 

protected:
	virtual bool OnClientConnect(std::shared_ptr< net::connection<MessageTypes> > client)
	{
		net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr< net::connection<MessageTypes> > client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr< net::connection<MessageTypes> > client, net::message<MessageTypes>& msg)
	{
		switch (msg.header.id)
		{
		case MessageTypes::ServerPing:
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			// Simply bounce message back to client
			client->Send(msg);
            break;

		case MessageTypes::MessageAll:
			std::cout << "[" << client->GetID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			net::message<MessageTypes> msg;
			msg.header.id = MessageTypes::ServerMessage;
			msg << client->GetID();
			MessageAllClients(msg, client);
            break;
		}
        fflush(stdout);
	}
};

#endif
