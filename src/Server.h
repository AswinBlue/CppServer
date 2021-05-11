#ifndef __SERVER_H__
#define __SERVER_H__

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_client.h"
#include "net_server.h"
#include "net_connection.h"
#include "MessageTypes.h"
#include "Define.h"

#include <boost/thread.hpp>

class Server : public net::server_interface <MessageTypes>
{
private:
    boost::thread thread_map_broadcasting;
    bool flag_thread_map_broadcasting = false;
    std::vector<Position> globalMap;
    std::vector<bool> globalMapUsage;

public:
	Server(uint16_t nPort) : net::server_interface<MessageTypes>(nPort)
	{} 
    ~Server()
    {
        if (flag_thread_map_broadcasting)
            thread_map_broadcasting.join();
    }

protected:
	virtual bool OnClientConnect(std::shared_ptr< net::connection<MessageTypes> > client)
	{
		//net::message<MessageTypes> msg;
		//msg.header.id = MessageTypes::ServerAccept;
		//client->Send(msg);

		return true;
	}
    virtual void PostClientConnected(std::shared_ptr< net::connection<MessageTypes> > client)
    {
        if (flag_thread_map_broadcasting)
        {
            SpawnPlayer(client);
            int idx = client->GetID() - UID_START_NUMBER;
            if (idx >= 0 && idx < MAX_USER_ON_MAP)
            {
                globalMapUsage[client->GetID() - UID_START_NUMBER] = true;
            }
            else
            {
                std::cout << "[SERVER] spawn index : " << idx << "\n";
            }
        }
    }

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr< net::connection<MessageTypes> > client)
	{
        // if using Map
        if (flag_thread_map_broadcasting)
        {
            globalMapUsage[client->GetID()] = false;
        }
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
            MessageClient(client, msg);
            break;

		case MessageTypes::MessageAll:
        {
			std::cout << "[" << client->GetID() << "]: Message All\n";

			// Construct a new message and send it to all clients
			net::message<MessageTypes> msg_all;
			msg_all.header.id = MessageTypes::ServerMessage;
			msg_all << client->GetID();
            std::cout << msg_all;
			MessageAllClients(msg_all, client);
            break;
        }
        case MessageTypes::UserPosition:
        {
            if (flag_thread_map_broadcasting)
            {
                if ((client->GetID() - UID_START_NUMBER) < globalMap.size())
                {
                    //static_assert(std::is_trivially_copyable<Position>::value,
                    //    "'Position' must be trivially copyable");

                    Position p;
                    //std::copy(msg.header.body.data(), msg.header.size, &pos); // memcpy
                    // msg >> p.pos_x >> p.pos_y >> p.dir;
                    msg >> p;
                    std:: cout << "[SERVER] Position Message " << p.pos_x << " " << p.pos_y << " " << p.dir << "\n";
                    globalMap[client->GetID() - UID_START_NUMBER] = p;
                }
                else
                {
                    std::cout << "[SERVER] out of range client id :" << client->GetID() << " mapSize :" << globalMap.size() << "\n";
                }
            }
            else
            {
                std::cout << "[SERVER] Map is not running now\n";
            }
        }
           break;
        default:
			std::cout << "[" << client->GetID() << "]: Echo Client\n";
            // echo
            MessageClient(client, msg);
            // do nothing
            break;
		}
        fflush(stdout);
	}

    // thread function : manage user position and broadcasting in 
    void ThreadBroadcastMap ()
    {
        while (true) {
            boost::this_thread::sleep(boost::posix_time::millisec(5000));
            net::message<MessageTypes> msg;
            msg.header.id = MessageTypes::UserPosition;
            for (int i = 0; i < MAX_USER_ON_MAP; ++i)
            {
                // Client 'i' exist in map
                if (globalMapUsage[i])
                {
                    std::cout << i << ": " << globalMap[i].pos_x << " " << globalMap[i].pos_y << " " << globalMap[i].dir << ";\n";
                    // msg << globalMap[i].pos_x << globalMap[i].pos_y << globalMap[i].dir;
                    msg << globalMap[i];
                }
            }
            MessageAllClients(msg);
            std::cout << "[SERVER] broadcast map\n";
        }
    }

    void SpawnPlayer(std::shared_ptr< net::connection<MessageTypes> > client)
    {
        client->GetID();
        net::message<MessageTypes> msg;
        msg.header.id = MessageTypes::UserPosition;
        Position pos = {0, 0, 0};
        msg << pos;
        MessageClient(client, msg);
    }
    public:
    // start thread
    void StartMap()
    {
        std::cout << "[SERVER] --- mapsize: " << globalMap.size() << "\n";
        globalMap.resize(MAX_USER_ON_MAP);
        std::cout << "[SERVER] --- mapsize: " << globalMap.size() << "\n";
        globalMapUsage.resize(MAX_USER_ON_MAP, false);
        //thread_map_broadcasting = boost::thread(boost::bind(&Server::ThreadBroadcastMap));
        flag_thread_map_broadcasting = true;
        std::cout << "[SERVER] Start Map, size : " << MAX_USER_ON_MAP << "\n";
        ThreadBroadcastMap();
    }

};

#endif
