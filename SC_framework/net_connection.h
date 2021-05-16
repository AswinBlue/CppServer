#ifndef __NET_CONNECTION_H__
#define __NET_CONNECTION_H__

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "Logger.h"

namespace net
{
    //Forward declare
    template<typename T>
    class server_interface;

    template<typename T>
    // provide shared pointer rather than raw pointer (this)
    class connection : public std::enable_shared_from_this< connection<T> >
    {
    public:
        enum class owner
        {
            server,
            client
        };
        // parnet : which side of connection is
        // asioContext : context of server/client interface object
        // socket : socket should be unique and wholly owned by connection
        // qIn : incomming queue of client/server interface, connection will send message to it
        connection(owner parent, boost::asio::io_context& asioContext, boost::asio::ip::tcp::socket socket, tsqueue< traced_message<T> >& qIn)
            : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessageIn(qIn), m_DeadlineTimer(asioContext)
        {
            // to make differences with critical fields and less-critical fields,
            // we made disparity
            m_nOwnerType = parent;

            // Construct validation check data
            if (m_nOwnerType == owner::server)
            {
                // if create random data. send it to server when 'WriteValidation()' called
                m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
                m_nHandshakeCheck = scramble(m_nHandshakeOut);
                m_nReadTimer = -1;

                // set read timeout
            }
            else
            {
                // need to wait Server to give 'm_nHandshakeIn'
                m_nHandshakeIn = 0;
                m_nHandshakeOut = 0;
            }

        }
        virtual ~connection() {}
        uint32_t GetID() const
        {
            return id;
        }

    public:
        /* Server Side Functions */
        // called by server interface when server accepted a new client
        bool ConnectToClient(net::server_interface<T>* server, uint32_t uid = 0)
        {
            if (m_nOwnerType == owner::server)
            {
                if (m_socket.is_open())
                {
                    id = uid;

                    // don't read message right after connection
                    // read message after validation
                    //WriteValidation();
                    //ReadValidation(server);

                     ReadHeader(); 
                    /* ReadHeader function is a asio asynchronous funciton
                    * when server connect to server, we must start reading
                    * running ReadHeader function once, we don't have to care about when to read,
                    * it will handle messages by itself whenever client send messages, as it called "asynchronous"
                    */
                }
            }
            else
            {
                LOG_CRITICAL("[{}] Server side function called by Client", id);
            }
        }

        /* Client Side Functions */
        bool ConnectToServer(const boost::asio::ip::tcp::resolver::results_type& endpoints)
        {
            // only relevant to clients
            if (m_nOwnerType == owner::client)
            {
                boost::asio::async_connect(m_socket, endpoints,
                    [this](std::error_code ec, boost::asio::ip::tcp::endpoint endpoint)
                    {
                        if (!ec)
                        {
                            // make asio task to wait and read messages
                            // ReadValidation();
                            ReadHeader();
                        }
                        else
                        {
                            LOG_WARN("[Client] Connect to Server Failed : {}", ec.message());
                        }
                    });
            }
            else
            {
                LOG_CRITICAL("[{}] Client side function called by Server", id);
            }
        }

        /* Common Functions */
        bool Disconnect()
        {
            if (IsConnected())
                // when it's appropriate for asio to do si, close the socket
                // more graceful than just closing by user explicitly
                boost::asio::post(m_asioContext, [this]() {m_socket.close(); });
        }
        // tells if the socket is valid or not
        bool IsConnected() const
        {
            return m_socket.is_open();
        }

        bool Send(const message<T>& msg)
        {
            /* WriteHeader function must be called when user have something to send

            */
            // send asio context a job in a form of a lambda function
            boost::asio::post(m_asioContext,
                [this, msg]()
                {
                    LOG_TRACE("Send {}", msg); 
                    bool bWritingMessage = !m_qMessageOut.empty();
                    // we have to push message in our outgoint message queue first
                    m_qMessageOut.push_back(msg);
                    // not to run multiple writing task, check message out queue to know whether a writing task is already running
                    if (!bWritingMessage) {
                        // make asio to handle message
                        WriteHeader();
                    }
                });
        }

    private:
        // ASIO : prime context ready to read a message header
        void ReadHeader()
        {
            // get message from m_socket, save header data in asio buffer
            boost::asio::async_read(m_socket, boost::asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        LOG_TRACE("ReadHeader {}", m_msgTemporaryIn);
                        // check it has appropriate length
                        if (m_msgTemporaryIn.header.size > 0)
                        {
                            // allocate enough space for messae body
                            m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                            ReadBody(); // connect next asynchronous task
                        }
                        else
                        {
                            // no body to handle. just add it to queue
                            AddToIncomingMessageQueue();
                        }
                    }
                    else
                    {
                        // dump error and manually force close socket
                        LOG_WARN("[{}] Read Header Failed", id);
                        m_socket.close();
                        // if we close socket, interface which try to communicate with this socket in the future will recognize it and tidy up its deque
                    }
                });
        }

        void ReadBody()
        {
            // call async read function on the same socket again to read rest of the message, which will be the body data
            boost::asio::async_read(m_socket, boost::asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                [this](std::error_code ec, std::size_t length) 
                {
                    if (!ec)
                    {
                        LOG_TRACE("ReadBody {}", m_msgTemporaryIn);
                        AddToIncomingMessageQueue();
                    }
                    else
                    {
                        // dump error and manually force close socket
                        LOG_WARN("[{}] Read Body Failed", id);
                        m_socket.close();
                    }
                });
        }

        void WriteHeader()
        {
            boost::asio::async_write(m_socket, boost::asio::buffer(&m_qMessageOut.front().header, sizeof(message_header<T>)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        // if we have body to send
                        // refer size of body directly or use 'size' field in header. your choice
                        if (m_qMessageOut.front().body.size() > 0)
                        {
                            WriteBody();
                        }
                        else
                        {
                            // we're done with the message. remove it
                            m_qMessageOut.pop_front();

                            // check if there are more messages to send, link asio context to do next
                            if (!m_qMessageOut.empty())
                            {
                                WriteHeader();
                            }
                        }
                    }
                    else
                    {
                        LOG_WARN("[{}] Write Header Failed", id);
                        m_socket.close();
                    }
                });
        }

        void WriteBody()
        {
             boost::asio::async_write(m_socket, boost::asio::buffer(m_qMessageOut.front().body.data(), m_qMessageOut.front().body.size()),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        // we're done with the message. remove it
                        m_qMessageOut.pop_front();

                        // check if there are more messages to send, link asio context to do next
                        if (!m_qMessageOut.empty())
                        {
                            WriteHeader();
                        }
                    }
                    else
                    {
                        LOG_WARN("[{}] Write Body Failed", id);
                        m_socket.close();
                    }
                });
        }
        void AddToIncomingMessageQueue()
        {
            if (m_nOwnerType == owner::server)
                // make message into traced_message and send it to server interface
                // connectionh<T>& + message = traced_message
                m_qMessageIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
            else // m_nOwnerType == owner::client
                // client don't need to manage where the messages are from
                m_qMessageIn.push_back({ nullptr, m_msgTemporaryIn});
                /* we decided to force clients to only have one connection(server),
                * in the client interface, the connections will be stored as a single unique pointer
                */

            // when after read message, we need to link another task for asio context to perform
            // this case, wait for another header
            ReadHeader();
        }

        /* 
        * server need to validate whether connected client is one we need to business with
        * send random number and get result, check if it is expected one
        * validate with crypt function
        */
        uint64_t scramble(uint64_t nInput)
        {
            uint64_t out = nInput ^ 0x10904EFD665530C0;
            out = (out & 0xFFFF0000FFFF0000) >> 4 | (out & 0xFFFF0000F0F0F0F0) >> 4;
            return out ^ 0xCBCBDEFAACCBDBD;
        }

        void WriteValidation()
        {
            boost::asio::async_write(m_socket, boost::asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
                [this](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        // after validation, wait for server
                        // if server shut down connection, client close connection in next ReadHeader()
                        if (m_nOwnerType == owner::client) {
                            ReadHeader();
                        }
                    }
                    else
                    {
                        m_socket.close();
                    }
                });
        }
        void ReadValidation(net::server_interface<T>* server = nullptr)
        {
            boost::asio::async_read(m_socket, boost::asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
                [this, server](std::error_code ec, std::size_t length)
                {
                    if (!ec)
                    {
                        if (m_nOwnerType == owner::client)
                        {
                            // use 'scramble' function to make crypto data
                            m_nHandshakeOut = scramble(m_nHandshakeIn);
                            // send back to server
                            WriteValidation();
                        }
                        else if (m_nOwnerType == owner::server)
                        {
                            if (m_nHandshakeIn == m_nHandshakeCheck)
                            {
                                LOG_DEBUG("[SERVER] Client Validated");
                                server->OnClientValidated(this->shared_from_this());
                                // let server read client's header
                                ReadHeader();
                            }
                            else
                            {
                                // client failed to respond crypto
                                LOG_DEBUG("Disconnect Suspicious Client");
                                // TODO : list up this client on black-list 
                                m_socket.close();
                            }
                        }
                        else
                        {
                            // wrong case, do nothing
                        }

                        // after validation, wait for server
                        // if server shut down connection, we close connection in next ReadHeader()
                        if (m_nOwnerType == owner::client)
                            ReadHeader();
                    }
                    else
                    {
                        LOG_WARN("[SERVER] Unsolicited Client Disconnected"); 
                        m_socket.close();
                    }
                });
        }

    protected:
        // each connection has a unique socket to a remote
        boost::asio::ip::tcp::socket m_socket;
        // connection get context by a client or server interface
        boost::asio::io_context& m_asioContext;

        // connection need a queue for messages going out
        tsqueue< message<T> > m_qMessageOut;

        // reference of the queue in client or server
        // this is where the messages come from
        tsqueue< traced_message<T> >& m_qMessageIn;
        message<T> m_msgTemporaryIn;

        // type of interface
        owner m_nOwnerType = owner::server; // default server
        uint32_t id = 0;

        // for Handshake Validation
        uint64_t m_nHandshakeOut = 0;
        uint64_t m_nHandshakeIn = 0;
        uint64_t m_nHandshakeCheck = 0; // for server to compare client's result with answer

        // check read timer
        // to cut out non-working client in server side
        // client should ping periodically
        boost::asio::deadline_timer m_DeadlineTimer;
        uint32_t m_nReadTimer;

   };
}

#endif

