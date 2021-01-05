#include <mysqlx/xdevapi.h>

class MySQL {
private:
    const char *URL = "mysqlx://RPG@127.0.0.1";
    Session session;
public:
    MySQL();
};
