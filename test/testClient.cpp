#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"
#include "net_client.h"
#include "net_server.h"
#include "net_connection.h"
#include "MessageTypes.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string>
#include <boost/thread.hpp>

class CustomClient : public net::client_interface<MessageTypes>
{
public:
	void PingServer()	
	{
		net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::ServerPing;

		// Caution with this...
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();		

		msg << timeNow;
        std::cout << "Ping server : " << msg << "\n";
		Send(msg);
	}

	void MessageAll()
	{
		net::message<MessageTypes> msg;
		msg.header.id = MessageTypes::MessageAll;		
		Send(msg);
        std::cout << "Message All" << msg << "\n";
	}

    void SendMessage(std::string str)
    {
		net::message<MessageTypes> msg;
        msg << str;
		msg.header.id = MessageTypes::MessageAll;
        Send(msg);
        std::cout << "Send Message " << str << "\n";
    }

};

void keyboardInput(CustomClient* c, bool* bQuit)
{
    std::string key;
    do {
        boost::this_thread::sleep(boost::posix_time::millisec(100));
        std::cin >> key;
        std::cout << ">>" << key << "\n" << std::flush;
        fflush(stdin);
        if (!key.compare("1")) {
            c->PingServer();
        }
        else if (!key.compare("2")) {
            c->MessageAll();
        }
        else if (!key.compare("3")) {
            *bQuit = true;
            std::cout << "disconnect\n";
        }
        else {
            c->SendMessage(key);
        }
        fflush(stdout);
    } while(!(*bQuit));
}

int main()
{
    CustomClient c;
    c.Connect("127.0.0.1", 3600);
    bool bQuit = false;

    boost::thread th1 = boost::thread(boost::bind(&keyboardInput, &c, &bQuit));

    while (!bQuit)
    {
        // asio control

        if (c.IsConnected())
		{
			if (!c.Incoming().empty())
			{
				auto msg = c.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case MessageTypes::ServerAccept:
				{
					// Server has responded to a ping request				
					std::cout << "Server Accepted Connection\n" << std::flush;
				}
				break;


				case MessageTypes::ServerPing:
				{
					// Server has responded to a ping request
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n" << std::flush;
				}
				break;

				case MessageTypes::ServerMessage:
				{
					// Server has responded to a ping request	
					uint32_t clientID;
					msg >> clientID;
					std::cout << "Hello from [" << clientID << "]\n" << std::flush;
				}
				break;
                default:
                {
                    std::cout << "unknown type message came\n";
                }
				} // -> switch
			}
		}
		else
		{
			std::cout << "Server Downed\n" << std::flush;
			bQuit = true;
		}
        fflush(stdout);
    } // -> while

    th1.join();

	return 0;
}
