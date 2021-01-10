#include "MySQL.h"
#include "Logger.h"

MySQL::MySQL(const std::string URI) {
// "mysqlx://RPG:pswd@127.0.0.1";
    MySQL::URI = URI;
    mysqlx::Session session(URI);
    LOG_INFO("MySQL: session created");
}

MySQL::~MySQL() {
    mysqlx::getSession(MySQL::URI).close();
    LOG_INFO("MySQL: session closed");
}

bool MySQL::find (const std::string& table, const std::string& key) {
    mysqlx::Schema sch = mysqlx::getSession(MySQL::URI).getSchema(table);
    mysqlx::Collection coll = sch.createCollection(key);
}
