#ifndef __MESSAGE_TYPE_H__
#define __MESSAGE_TYPE_H__

#include "Define.h"
#include <iostream>
#include <string>
enum class MessageTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    UserPositionUpdate,
    ServerSendUserID,
    ClientSendUserID
};

typedef struct __attribute__ ((packed)) Position {
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t dir;

    friend std::ostream& operator << (std::ostream& os, const Position& pos) {
        // print 'dir' as HEX
        os << "pos_x: " << int(pos.pos_x) << " pos_y: " << int(pos.pos_y) << " dir: 0x" << std::hex << unsigned(pos.dir) << std::dec;
        return os;
    }
}Position;

typedef struct __attribute__ ((packed)) UserData {
    uint8_t ID[USER_ID_LEN];
    Position pos;
    friend std::ostream& operator << (std::ostream& os, const UserData& user) {
        os << "ID: " << (char*)user.ID << " " << user.pos;
        return os;
    }
}UserData;


#endif

