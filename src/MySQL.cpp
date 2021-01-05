#include "MySQL.h"
#include "Logger.h"

MySQL::MySQL() {
    session(MySQL::URL);
    LOG_INFO("MySQL: session created");
}

MySQL::find (char& table, char& key) {
    Schema sch = session.getSchema(table);
    Collection coll = sch.createCollection(key, true);

}
