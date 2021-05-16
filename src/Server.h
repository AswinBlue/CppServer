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
#include "Logger.h"

#include <string>
#include <sstream>
#include <boost/thread.hpp>

class Server : public net::server_interface <MessageTypes>
{
private:
// thread list
    bool m_isUserDataBroadcasting = false;
    bool m_isSocketListening = false;
    std::vector<std::thread> m_threadList;
// UserData list
    std::vector<UserData> m_totalUser;
    std::vector<bool> m_totalUserUsage;

public:
	Server(uint16_t nPort) : net::server_interface<MessageTypes>(nPort)
	{} 
    ~Server()
    {
        m_isUserDataBroadcasting = false;
        m_isSocketListening = false;
        for (std::vector<std::thread> ::iterator t = m_threadList.begin(); t != m_threadList.end(); t++)
        {
            t->join();
        }
        Stop();
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
        // assign client's own ID and send it to client
        net::message<MessageTypes> msg;
        msg.header.id = MessageTypes::ClientSendUserID;
        // int to string
        std::stringstream ss;
        ss << std::setw(USER_ID_LEN) << std::setfill('0') << client->GetID();
        std::string s = ss.str();
        /* 
        * copy char* data into uint8_t*
        * WARN : if you don't use '<<' operator, you have to resize msg.body first
        */
        // case 1. use memcpy
        // msg.body.resize(msg.body.size() + s.size());
        // std::memcpy(msg.body.data(), reinterpret_cast<const uint8_t*>(s.c_str()), USER_ID_LEN);
        // std::copy(s.begin(), s.end(), msg.body.begin());
        msg << s;

        MessageClient(client, msg);

        if (m_isUserDataBroadcasting)
        {
            SpawnPlayer(client);
            int idx = client->GetID() - UID_START_NUMBER;
            if (idx >= 0 && idx < MAX_USER_ON_MAP)
            {
                m_totalUserUsage[client->GetID() - UID_START_NUMBER] = true;
            }
            else
            {
                LOG_DEBUG("[SERVER] spawn index : {}", idx);
            }
        }

    }

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr< net::connection<MessageTypes> > client)
	{
        // if using Map
        if (m_isUserDataBroadcasting)
        {
            m_totalUserUsage[client->GetID() - UID_START_NUMBER] = false;
        }
		LOG_DEBUG("[{}] Removing client", client->GetID());
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr< net::connection<MessageTypes> > client, net::message<MessageTypes>& msg)
	{
		switch (msg.header.id)
		{
		case MessageTypes::ServerPing:
			LOG_TRACE("[{}] Server Ping", client->GetID());

			// Simply bounce message back to client
            MessageClient(client, msg);
            break;

		case MessageTypes::MessageAll:
        {

			// Construct a new message and send it to all clients
			net::message<MessageTypes> msg_all;
			msg_all.header.id = MessageTypes::ServerMessage;
			msg_all << client->GetID();
			LOG_TRACE("[{}] Message All : {}", client->GetID(), msg_all);
			MessageAllClients(msg_all, client);
            break;
        }
        case MessageTypes::UserPositionUpdate:
        {
            if (m_isUserDataBroadcasting)
            {
                if ((client->GetID() - UID_START_NUMBER) < m_totalUser.size())
                {
                    //static_assert(std::is_trivially_copyable<Position>::value,
                    //    "'Position' must be trivially copyable");

                    UserData user;
                    //std::copy(msg.header.body.data(), msg.header.size, &pos); // memcpy
                    // msg >> p.pos_x >> p.pos_y >> p.dir;
                    msg >> user;

                    LOG_TRACE("[SERVER] Position Message came : {}", user);
                    m_totalUser[client->GetID() - UID_START_NUMBER].pos = user.pos;
                }
                else
                {
                    LOG_WARN("[SERVER] out of range client id : {}, limit : {}", client->GetID(), m_totalUser.size());
                }
            }
            else
            {
                LOG_WARN("[SERVER] Map is not running now");
            }
           break;
        }
        case MessageTypes::ServerSendUserID:
        {
            // TODO : verify ID and resend ID
            break;
        }
        default:
			LOG_WARN("[{}] Wrong Message Type", client->GetID());
            // TODO : client sent wrong message type, consider the client as suspicious attacker
            break;
		}
        fflush(stdout);
	}

    void SpawnPlayer(std::shared_ptr< net::connection<MessageTypes> > client)
    {
        net::message<MessageTypes> msg;
        msg.header.id = MessageTypes::UserPositionUpdate;
        UserData user;
        // int to string
        std::stringstream ss;
        ss << std::setw(USER_ID_LEN) << std::setfill('0') << client->GetID();
        std::string s = ss.str();
        // case 1. copy string to uint8_t[]
        // std::memcpy(user.ID, reinterpret_cast<const uint8_t*>(s.c_str()), USER_ID_LEN);
        // case 2. copy string to vector<uint8_t>
        std::memcpy(user.ID, s.c_str(), USER_ID_LEN);
        user.pos = {0, 0, 0};
        LOG_DEBUG("[SERVER] User Spawned :: {}", user);
        m_totalUser[client->GetID() - UID_START_NUMBER] = user;

        msg << user;
        MessageClient(client, msg);
    }

    // thread function : manage user position and broadcasting in 
    void ThreadBroadcastUserData()
    {
        while (m_isUserDataBroadcasting) {
            boost::this_thread::sleep(boost::posix_time::millisec(5000));
            net::message<MessageTypes> msg;
            msg.header.id = MessageTypes::UserPositionUpdate;
            for (int i = 0; i < MAX_USER_ON_MAP; ++i)
            {
                // Client 'i' exist in map
                if (m_totalUserUsage[i])
                {
                    LOG_TRACE("{}: {} ", i, m_totalUser[i]);
                    msg << m_totalUser[i];
                }
            }
            MessageAllClients(msg);
            LOG_TRACE("[SERVER] broadcast userdata map\n");
        }
    }

    public:
    // start thread
    bool StartUserDataBroadcasting()
    {
        m_totalUser.resize(MAX_USER_ON_MAP);
        m_totalUserUsage.resize(MAX_USER_ON_MAP, false);
        m_threadList.push_back(std::thread(&Server::ThreadBroadcastUserData, this));
        m_isUserDataBroadcasting = true;
        LOG_INFO("[SERVER] ---Start World, size : {}", MAX_USER_ON_MAP);
        return true;
    }
    
    // start server-client socket communication thread
    bool StartServerSocket(size_t maxMessage, bool wait)
    {
        if (!Start()) {
            return false;
        } 
        m_isSocketListening = true;

        m_threadList.push_back(std::thread(
            [this](size_t maxMessage, bool wait){
                while (m_isSocketListening) {
                    Update(maxMessage, wait);
                }
            }, maxMessage, wait));
        LOG_INFO("[SERVER] --- Start Server Socket");
        return true;
    }

};

#endif
