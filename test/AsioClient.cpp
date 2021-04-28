#include <iostream>
#include <chrono>
#include "../src/Packet.h"

#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

std::vector<char> vBuffer(10 * BUFSIZ);

// read data until there is no data to read
void readAsynchronous(boost::asio::ip::tcp::socket& socket) {
    socket.async_read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()),
    /* when data is available to read in socket, activate lambda function below */
    [&](std::error_code ec, std::size_t length) {
        if (!ec) {
            std::cout << "\n\nRead " << length << " bytes\n\n";

            for (int i = 0; i < length; i++) {
                std::cout << vBuffer[i];
            }
            readAsynchronous(socket);
        }
        else {
            std::cout << ec << std::endl;
        }
    });
}

void readSynchronous(boost::asio::ip::tcp::socket& socket) {
    socket.wait(socket.wait_read); 
    /* wait untill socket has data available to read 
    * when the data come, start reading*/
    
    size_t bytes = socket.available(); 
    /* we don't know how long the data is. we can't decide when to stop
    * server may not send all of the data in once
    * so we can possibly read only partial data in this way*/
    std::cout << "bytes received : " << bytes << std::endl;

    if (bytes > 0) {
        std::vector<char> vBuffer(bytes);
        socket.read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()));

        for (auto c : vBuffer)
            std::cout << c;
    }
}

int main (int argc, char* argv[]) {
    boost::asio::io_context context; // space where asio can do its work
    // boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(argv[1]), atoi(argv[2]));
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("51.38.81.49"), 80);
    boost::asio::ip::tcp::socket socket(context);

    boost::asio::io_context::work idleWork(context); // give fake work to asio so the program don't exit immediately
    std::thread contextThread = std::thread([&]() {context.run();}); // run in thread. don't block main process

    socket.connect(endpoint);

    if (socket.is_open()) {
        // 1-1. read asynchronousll
        readAsynchronous(socket);

        std::string sRequest = 
            "GET /index.html HTTP/1.1\r\n"
            "Host: google.com\r\n"
            "Connection: close\r\n\r\n";
        std::cout << sRequest << std::endl;

        socket.write_some(boost::asio::buffer(sRequest.data(), sRequest.size()));
        /* after sending data, we need to wait untill respond come */

        // 1-2. wait for a long time
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(30000ms); // dummy process in main thread(just waiting)
        context.stop();
        if (contextThread.joinable()) contextThread.join();
        /* when using asynchronous read, you need to wait main thread until the async function operates 
        * you can't assure when it starts */

        // 2-1. read synchronously
        // readSynchronous(socket);
    }
    return 0;
}
