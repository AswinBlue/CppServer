#ifndef __MESSAGE_TYPE_H__
#define __MESSAGE_TYPE_H__

enum class MessageTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};


#endif
