#include"sqliteConnector.h"
namespace jxtd{
	namespace misc{
		namespace dboperator{
			sqliteConnector::sqliteConnector() {
				db = nullptr;
			}
			sqliteConnector::sqliteConnector(sqlite3* sq) {
				this->db = sq;
			}

			sqliteConnector::~sqliteConnector() {
				this->close();
			}

			void sqliteConnector::close() {
				if (db) {
					sqlite3_close(db);
					db = nullptr;
				}
			}

			int sqliteConnector::beginTranscation() {
				int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr); 
				if (rc != SQLITE_OK) {
					return 0;
				}
				return 1;
			}

			int sqliteConnector::commitTranscation() {
				int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
				if (rc == SQLITE_OK)
					return 1;
				else
					return 0;
			}

			int sqliteConnector::rollbackTranscation() {
				int rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
				if (rc == SQLITE_OK)
					return 1;
				else
					return 0;
			}

			int sqliteConnector::excuteQuery(const std::string sql) {
				sqlite3_stmt* stmt = nullptr;
				int result = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &stmt, NULL);
				if (result != SQLITE_OK) {
					sqlite3_finalize(stmt);
					return 0;
				}
				result = sqlite3_step(stmt);
				if (result != SQLITE_DONE) {
					sqlite3_finalize(stmt);
					return 0;
				}
				sqlite3_finalize(stmt);
				return 1;
			}
		}
	}
}
