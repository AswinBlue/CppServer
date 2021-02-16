#include <mysqlx/xdevapi.h>
#include <string>

#define MODIFY_SET 1
#define MODIFY_UNSET 2
#define MODIFY_APPEND 3

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
    template<typename T>
    mysqlx::CollectionModify modify_set (mysqlx::CollectionModify& r, const char* field, const T value);
    template<typename T>
    mysqlx::CollectionModify modify_arrayAppend (mysqlx::CollectionModify& r, const char* field, const T value);
    mysqlx::CollectionModify modify_unset (mysqlx::CollectionModify& r, const char* field);
    mysqlx::CollectionModify modify (const std::string& database, const std::string& table, const char* query);
};
