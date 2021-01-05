
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

