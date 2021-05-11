#ifndef __MESSAGE_TYPE_H__
#define __MESSAGE_TYPE_H__

enum class MessageTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
    UserPosition
};

typedef struct __attribute__ ((packed)) position{
    uint16_t pos_x;
    uint16_t pos_y;
    uint8_t dir;
}Position;


#endif

