#ifndef __NET_SERVER_H__
#define __NET_SERVER_H__

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"
#include "Define.h"

namespace net
{
    template<typename T>
    class server_interface
    {
    public:
        server_interface(uint16_t port)
            // initialize m_asioAcceptor, endpoint means the address of the server which will listen the connection
            : m_asioAcceptor(m_asioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
        {
            std::cout << "[SERVER] Opening Server with port " << port << "\n";
        }

        virtual ~server_interface()
        {
            Stop();
        }

        bool Start()
        {
            std::cout << "[SERVER] Start server\n";
            try
            {
                // work with asio context
                WaitForClientConnection();
                m_threadContext = std::thread([this]() { m_asioContext.run(); });
            }
            catch (std::exception& e) 
            {
                // when something prohibited the server from listening
                std::cerr << "[SERVER] Exception: " << e.what() << "\n";
                return false;
            }

            std::cout << "[SERVER] Started\n";
            return true;
        }
        
        void Stop()
        {
            // stop asio context
            m_asioContext.stop();

            // wait for thread
            if (m_threadContext.joinable()) m_threadContext.join();
            std::cout << "[SERVER] Stopped\n";
        }

        // ASIO:: task for asio context
        void WaitForClientConnection()
        {
            // wait for client connect, listening
            m_asioAcceptor.async_accept(
                [this] (std::error_code ec, boost::asio::ip::tcp::socket socket)
                {
                    if (!ec)
                    {
                        // client connected successfully
                        // remote_endpoint return ip address
                        std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
                        // temporarily make connection
                        std::shared_ptr< connection<T> > new_conn = 
                            // allocate object
                            std::make_shared< connection<T> > (connection<T>::owner::server,
                                m_asioContext, std::move(socket), m_qMessageIn);
                            // pass by reference the message queue of server
                            // message queue of server is unique but shared across all of the connections
                            // it needs to be thread safe

                        // when server needs to deny connection
                        if (OnClientConnect(new_conn))
                        {
                            // push connection to deque
                            m_deqConnections.push_back(std::move(new_conn));
                            m_deqConnections.back() -> ConnectToClient(this, nIDCounter++); // allocate an id to connection
                            std::cout << "[" << m_deqConnections.back()->GetID() << "] Connection Accomplished\n";
                            PostClientConnected(m_deqConnections.back());
                        }
                        else
                        {
                            // new_conn is smart pointer, there's no reference to it. so it will automatically deleted
                            std::cout << "[-----] Connection Denied\n";
                        }
                    }
                    else
                    {
                        // display error
                        std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
                    }
                    // wait for another client connection. just re-register another asynchronous task
                    WaitForClientConnection();
                });
        }
        // ASIO
        void MessageClient(std::shared_ptr< connection<T> > client, const message<T>& msg)
        {
            // check 'client' is not null and isConnected
            if (client && client->IsConnected())
            {
                std::cout <<"DEBUG: MessageClient send message\n";
                client->Send(msg);
            }
            else
            {
                // clients can be disconnected in many reasons
                // when client is disconnected, we need to cut it out
                OnClientDisconnect(client);
                client.reset();
                // 'erase' operation is high-cost operation when there are many clients.
                m_deqConnections.erase(
                    std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());

            }
        }
        // ASIO
        void MessageAllClients(const message<T>& msg, std::shared_ptr< connection<T> > pIgnoreClient = nullptr)
        {
            bool bInvalidClientExists = false;

            for (auto& client : m_deqConnections) 
            {
                if (client && client->IsConnected())
                {
                    // ignore specific  client
                    if (client != pIgnoreClient) {
                        std::cout << "DEBUG: MessageAllClients send message\n";
                        client->Send(msg);
                    }
                }
                else
                {
                    // release client and make it null
                    OnClientDisconnect(client);
                    client.reset();

                    // don't erase from deque in loop. it will mass up the loop.
                    bInvalidClientExists = true;
                }
            }
            // to call high-cost operation once, use boolean
            if (bInvalidClientExists)
            {
                m_deqConnections.erase(
                    std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
            }
        }

        // allow users to decide when is the most appropriate time to handle messages
        // nMaxMessage : -1 means maximum
        // bWait : wait for client's next message or sleep
        void Update(size_t nMaxMessages = -1, bool bWait = false)
        {
            /* when server gets busy transfering messages, this function never returns 
             * and constantly process messages
             * if so, the serverside application logic wouldn't be update at all
             * so we give max size constrain to prevent this happening
             */
            if (bWait) m_qMessageIn.wait(); // wait until new message come, save CPU resources

            size_t nMessageCount = 0;
            while (nMessageCount < nMaxMessages && !m_qMessageIn.empty())
            {
                // pop message out from trhe queue
                auto msg = m_qMessageIn.pop_front();
                // pass it to the message handler
                // shared pointer of message client, and message itself
                OnMessage(msg.remote, msg.msg);
                nMessageCount++;
            }

        }
    protected:
        // can be overwrited by developer
        virtual bool OnClientConnect(std::shared_ptr< connection<T> > client)
        {
            // TODO : filter connections we don't expected
            // return false on default. developer must override this
            // decide whether accept new client or decline request
            return false;
        }
        virtual void PostClientConnected(std::shared_ptr< connection<T> > client)
        {
            // after client connected
        }
        virtual void OnClientDisconnect(std::shared_ptr< connection<T> > client)
        {
        }
        // allow server to deal with specific message
        virtual void OnMessage(std::shared_ptr< connection<T> > client, message<T>& msg)
        {
            /* if you want to make all things asynchronous, use OnMessage function
             * call this function in message arrive handler function
             * but we choosed to manage all the connections into serialized queue 
             * it's your choice, it's just frame work decision
             */
        }
    public:
        // in server-side, called when a client is validated
        virtual void OnClientValidated(std::shared_ptr< connection<T> > client)
        {
        }

        // thread safe queue for incoming messages
        tsqueue< traced_message<T> > m_qMessageIn;
        // container to manage connections
        std::deque< std::shared_ptr< connection<T> > > m_deqConnections;

        /* order of declaration is important, it is also the order of initialization */
        boost::asio::io_context m_asioContext;
        boost::asio::ip::tcp::acceptor m_asioAcceptor;
        std::thread m_threadContext;

        // clients will get unique id, which server can manage with
        uint32_t nIDCounter = UID_START_NUMBER;
    };
}

#endif
