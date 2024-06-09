#pragma once
#include"sqlite3.h"
#include"sqliteConnector.h"
#include<memory>

namespace jxtd{
	namespace misc{
		namespace dboperator{
            class sqliteUtil{
				public:
					static sqliteUtil& getInstance();
					sqliteUtil(const sqliteUtil&) = delete;
					sqliteUtil& operator=(const sqliteUtil&) = delete;
					std::unique_ptr<sqliteConnector> open(const std::string);
				private:
					sqliteUtil() = default;
			};
        }
    }
}
