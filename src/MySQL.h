#include <mysqlx/xdevapi.h>
#include <string>

class MySQL {
private:
    std::string URI;
    std::string host;
    int port = 33060;
    std::string user;
    std::string password;

public:
    MySQL(const std::string& host, const int port, const std::string& user, const std::string& password);
    ~MySQL();
    mysqlx::DocResult find(const std::string& database, const std::string& table, const char* query);
    mysqlx::Result upload(const std::string& database, const std::string& table, std::list<std::string> docs);
    mysqlx::Result remove(const std::string& database, const std::string& table, const char* query);
};
