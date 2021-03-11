#ifndef _ASIO_H_
#define _ASIO_H_

#define ASIO_STANDALONE
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

class Asio {
private:
    boost::asio::ip::tcp::socket* socket;
public:
    Asio();
    bool connect(const std::string& ip, const int& port);

};
#endif
