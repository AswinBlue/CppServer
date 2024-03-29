#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "net_common.h"
#include <type_traits>

namespace net
{
    // type 'T' could be enum class 
    // 'T' can be used to ensure the message are valid at complie time
    template <typename T>
    struct message_header
    {
        T id{};
        uint32_t size = 0;
    };

    template <typename T>
    struct message
    {
        message_header<T> header{};
        std::vector<uint8_t> body; // arrays of 8byte data

        // return size of message
        size_t size() const
        {
            return body.size();
        }

        // friend function, treated as global function(not a member function)
        friend std::ostream& operator << (std::ostream& os, const message<T>& msg) {
            os << "MID: " << int(msg.header.id) << " Size: " << msg.header.size;
            // std::copy(msg.body.begin(), msg.body.end(), std::ostream_iterator<uint8_t>(std::cout, ""));
            return os;
        }

        // operation << : push data into message
        template <typename DataType>
        friend message<T>& operator << (message<T>& msg, const DataType& data)
        {
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");
            /* if the 'DataType' has static variable, or complex arrangements of pointer,
             * it cannot be serialized */

            size_t i = msg.body.size(); // get current body size
            msg.body.resize(i + sizeof(DataType)); // resize vector
            std::memcpy(msg.body.data() + i, &data, sizeof(DataType)); // physically write data
            msg.header.size = msg.size(); // update size of message
            return msg; // return itself
        }
        /* this function is flexible. but resizing memory costs a lot*/

        // operation >> : pop data into message
        // treating vector as stack, so that we don't have to reallocate and copy everytime we pop data
        template <typename DataType>
        friend message<T>& operator >> (message<T>& msg, DataType& data)
        {
            static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");
            size_t i = msg.body.size() - sizeof(DataType); // get size of body after pop
            std::memcpy(&data, msg.body.data() + i, sizeof(DataType)); // physically write data
            msg.body.resize(i); // shrink the vector, but 
            msg.header.size = msg.size(); // update size of message
            return msg; // return itself
        }

        /*
        * ========== For 'vector', 'string', make template specification ==========
        */
        friend message<T>& operator << (message<T>& msg, const std::string& data)
        {
            size_t i = msg.body.size(); // get current body size
            msg.body.resize(i + data.size()); // resize vector
            //std::memcpy(msg.body.data(), reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
            std::copy(data.begin(), data.end(), msg.body.begin());
            msg.header.size = msg.size(); // update size of message
            return msg; // return itself
        }
        friend message<T>& operator >> (message<T>& msg, std::vector<T>& data)
        {
            size_t i = msg.body.size() - data.size(); // get size of body after pop
            // std::memcpy(data.data(), msg.body.data() + i, data.size()); // physically write data
            std::copy(msg.body.begin(), msg.body.end(), data.begin());
            msg.body.resize(i); // shrink the vector, but 
            msg.header.size = msg.size(); // update size of message
            return msg; // return itself
        }

        template <typename U>
        friend message<T>& operator << (message<T>& msg, const std::vector<U>& data)
        {
            size_t i = msg.body.size(); // get current body size
            msg.body.resize(i + data.size()); // resize vector
            std::memcpy(msg.body.data() + i, data.data(), data.size()); // physically write data
            // std::copy(data.begin(), data.end(), msg.body.begin());
            msg.header.size = msg.size(); // update size of message
            return msg; // return itself
        }
        template <typename U>
        friend message<T>& operator >> (message<T>& msg, std::vector<U>& data)
        {
            size_t i = msg.body.size() - data.size(); // get size of body after pop
            std::memcpy(data.data(), msg.body.data() + i, data.size()); // physically write data
            // std::copy(msg.body.begin(), msg.body.end(), data.begin());
            msg.body.resize(i); // shrink the vector, but 
            msg.header.size = msg.size(); // update size of message
            return msg; // return itself
        }
        
    };

    // Forward declare the 'connection' class
    template <typename T> // 'T' is the enum class that defines message
    class connection;

    // messages having connection information, so that we can findout where to send respond
    template <typename T>
    struct traced_message
    {
        std::shared_ptr< connection<T> > remote = nullptr;
        message<T> msg;

        // friend property string maker
        friend std::ostream& operator << (std::ostream& os, const traced_message<T>& message)
        {
            os << message.msg;
            return os;
        }
    };
}

#endif
