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
#include "Utils.h"

#include <string>
#include <sstream>
#include <boost/thread.hpp>

class Server : public net::server_interface <MessageTypes>
{
private:
    boost::thread thread_map_broadcasting;
    bool flag_thread_map_broadcasting = false;
    std::vector<UserData> m_totalUser;
    std::vector<bool> m_totalUserUsage;

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
                m_totalUserUsage[client->GetID() - UID_START_NUMBER] = true;
            }
            else
            {
                std::cout << "[SERVER] spawn index : " << idx << "\n";
            }
        }

        net::message<MessageTypes> msg;
        msg.header.id = MessageTypes::ClientSendUserID;
        msg << client->GetID();
        MessageClient(client, msg);
    }

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr< net::connection<MessageTypes> > client)
	{
        // if using Map
        if (flag_thread_map_broadcasting)
        {
            m_totalUserUsage[client->GetID()] = false;
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
        case MessageTypes::UserPositionUpdate:
        {
            if (flag_thread_map_broadcasting)
            {
                if ((client->GetID() - UID_START_NUMBER) < m_totalUser.size())
                {
                    //static_assert(std::is_trivially_copyable<Position>::value,
                    //    "'Position' must be trivially copyable");

                    UserData user;
                    //std::copy(msg.header.body.data(), msg.header.size, &pos); // memcpy
                    // msg >> p.pos_x >> p.pos_y >> p.dir;
                    msg >> user;

                    std:: cout << "[SERVER] Position Message came : " << user << "\n";
                    m_totalUser[client->GetID() - UID_START_NUMBER] = user;
                }
                else
                {
                    std::cout << "[SERVER] out of range client id :" << client->GetID() << ", limit :" << m_totalUser.size() << "\n";
                }
            }
            else
            {
                std::cout << "[SERVER] Map is not running now\n";
            }
           break;
        }
        case MessageTypes::ServerSendUserID:
        {
            // TODO : verify ID and resend ID
            break;
        }
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
            msg.header.id = MessageTypes::UserPositionUpdate;
            for (int i = 0; i < MAX_USER_ON_MAP; ++i)
            {
                // Client 'i' exist in map
                if (m_totalUserUsage[i])
                {
                    std::cout << i << ": " << m_totalUser[i] << "\n";
                    // msg << m_totalUser[i].pos_x << m_totalUser[i].pos_y << m_totalUser[i].dir;
                    msg << m_totalUser[i];
                }
            }
            MessageAllClients(msg);
            std::cout << "[SERVER] broadcast map\n";
        }
    }

    void SpawnPlayer(std::shared_ptr< net::connection<MessageTypes> > client)
    {
        net::message<MessageTypes> msg;
        msg.header.id = MessageTypes::UserPositionUpdate;
        UserData user;
        // int to string
        // IntToArray(client->GetID(), (user.ID), USER_ID_LEN);
        std::stringstream ss;
        //std::to_string(client->GetID());
        ss << std::setw(USER_ID_LEN) << std::setfill('0') << client->GetID();
        std::string s = ss.str();
        std::memcpy(user.ID, reinterpret_cast<const uint8_t*>(s.c_str()), USER_ID_LEN);
        user.pos = {0, 0, 0};
        std::cout << "[SERVER] User spaned : " << user << "\n";

        m_totalUser[client->GetID() - UID_START_NUMBER] = user;

        msg << user;
        MessageClient(client, msg);
    }
    public:
    // start thread
    void StartMap()
    {
        m_totalUser.resize(MAX_USER_ON_MAP);
        m_totalUserUsage.resize(MAX_USER_ON_MAP, false);
        //thread_map_broadcasting = boost::thread(boost::bind(&Server::ThreadBroadcastMap));
        flag_thread_map_broadcasting = true;
        std::cout << "[SERVER] Start World, size : " << MAX_USER_ON_MAP << "\n";
        ThreadBroadcastMap();
    }

};

#endif
