#include <mysqlx/xdevapi.h>
#include <string>

class MySQL {
private:
    std::string URI = NULL;
public:
    MySQL();
    MySQL(const std::string URI);
    ~MySQL();
    bool find(const std::string& table, const std::string& key);
};
