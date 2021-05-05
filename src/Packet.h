#include "PacketCommon.h"

namespace olc {
    namespace net {
        // when passing messages between different architecture, you may consider byte orders 
        template <typename T>
        struct header {
            T mid{};
            uint32_t size = 0;
        };

        template <typename T>
        struct packet {
            header<T> header{};
            std::vector<uint8_t> body;

            size_t size() const
            {
                return sizeof(header<T>) + body.size();
            }

            // override for std::cout
            friend std::ostream& operator << (std::ostream& os, const packet<T>& pkt) {
                os << "MID: " << int(pkt.header.id) << " Size:" << pkt.header.size;
                return os;
            }

            template<typename <DataType>
            friend packet<T>& operator >> (packet<T>& pkt, const DataType& data) {
                // pop data like stack not to realloc memory whenever you pop data
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to serialize");
                size_t i = pkt.body.size() - sizeof(DataType);
                std::memcpy(&data, pkt.body.data() + i, sizeof(DataType));
                pkt.body.resize(i);
                pkt.header.size = pkt.size();
                return pkt;
            }
        };
    }
}

typedef struct packet{
    int opcode;
    char data[1024];
}Packet;

typedef struct position{
    int userId;
    int x;
    int y;
    int heading;
    int action;
}Position;

// opcode names
enum {
    USER_ID = 1,
    POSITION_INFO = 2
};

// action names
enum {
    MOVE = 1,
    TELEPORT = 2
};

