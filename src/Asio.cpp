#include "Asio.h"
#include "Logger.h"
#include <string>

bool Asio::connect(const std::string& ip, const int& port) {
    std::error_code ec;
    // create a context
    boost::asio::io_context context;
    // set endpoint to connect
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(ip), port);
    // create a socket, context will deliver the implementation
    this->socket = new boost::asio::ip::tcp::socket(context);

    // connect
    this->socket->connect(endpoint);

    if (!ec) {
        LOG_DEBUG("Connected to endpoint");
    }
    else {
        LOG_ERROR("Failed to connect to address", ec.message());
    }
}
