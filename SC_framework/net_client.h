#ifndef __NET_CLIENT_H__
#define __NET_CLIENT_H__

#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

namespace net
{
    template <typename T>
    class client_interface
    {
    public:
        client_interface()
        {
            // TODO : Initialise the socket with the io context
        }
        virtual ~client_interface()
        {
            // shutdown
            Disconnect();
        }

    public:
        bool Connect(const std::string& host, const uint16_t port)
        {
            try
            {
                // Resolve address into tangible physical address
                // rather than using direct ipaddress, asio provides 'resolver' which transfer url into address
                boost::asio::ip::tcp::resolver resolver(m_context);
                // if resolver can't produce ip address, it will throw exception
                boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                // Create connection
                m_connection = std::make_unique< connection<T> >(
                    connection<T>::owner::client,
                    m_context,
                    boost::asio::ip::tcp::socket(m_context),
                    m_qMessageIn);

                // Connect to server physically
                m_connection -> ConnectToServer(endpoints);

                // Start context thread
                thrContext = std::thread([this]() { m_context.run(); });
            }
            catch (std::exception& e)
            {
                std::cerr << "Client Exception: " << e.what() << "\n";
                return false;
            }
            return true;
        }

        void Disconnect()
        {
            // check if the connection is actually connected
            if (IsConnected())
            {
                // disconnect from server gracefully
                m_connection->Disconnect();
            }
            // either way, asio context isn't running, stop context
            m_context.stop();
            // stop the thread too
            if (thrContext.joinable())
                thrContext.join();

            // we're done with the connection. release the unique pointer
            m_connection.release();
        }

        bool IsConnected()
        {
            if (m_connection) {
                return m_connection->IsConnected();
            }
            else {
                return false;
            }
        }

        // send message to server
        void Send(const message<T>& msg)
        {
            if (IsConnected())
                m_connection->Send(msg);
        }

        // get message from server
        tsqueue< traced_message<T> >& Incoming()
        {
            return m_qMessageIn;
        }

    protected:
        // asio object to handle data transmission
        boost::asio::io_context m_context;
        // thread for asio to work
        std::thread thrContext;
        std::unique_ptr< connection<T> > m_connection;
    private:
        // thread safe queue of incoming messages
        tsqueue< traced_message<T> > m_qMessageIn;
    };
}
#endif
