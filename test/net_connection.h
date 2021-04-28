#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace net
{
    template<typename T>
    // provide shared pointer rather than raw pointer (this)
    class connection : public std::enable_shared_from_this< connection<T> >
    {
    public:
        connection() {}
        virtual ~connection() {}

        bool ConnectToServer();
        bool Disconnect();
        bool IsConnected() const;

        bool Send(const message<T>& msg);

    protected:
        // each connection has a unique socket to a remote
        boost::asio::ip::tcp::socket m_socket
        // connection get context by a client or server interface
        boost::asio::io_context& m_asioContext;

        // connection need a queue for messages going out
        TSQueue< message<T> > m_qMessageOut;

        // reference of the queue in client or server
        // this is where the messages come from
        TSQueue<traced_message>& m_qMessageIn;
    }
}
