#include"sqliteUtil.h"
#include<memory>
namespace jxtd{
	namespace misc{
		namespace dboperator{
			sqliteUtil& sqliteUtil::getInstance() {
				static sqliteUtil instance;
				return instance;
			}

			std::unique_ptr<sqliteConnector> sqliteUtil::open(const std::string database) {
				sqlite3* db;
				int rc = sqlite3_open_v2(database.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
				if (rc != SQLITE_OK)
				{
				return nullptr;
				}
				std::unique_ptr<sqliteConnector> connector;
				connector = std::make_unique<sqliteConnector>(db);
				return connector;
			}
        }
    }
}
