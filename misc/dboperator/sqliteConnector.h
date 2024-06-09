#pragma once
#include<sqlite3.h>
#include<vector>
#include<tuple>
#include<string>
#include<type_traits>
namespace jxtd{
	namespace misc{
		namespace dboperator{
			template<typename T>
			struct is_string_type {
				static constexpr bool value = std::is_same<typename std::decay<T>::type, std::string>::value
					|| std::is_same<typename std::decay<T>::type, char*>::value;
			};

			template<typename Single>
			Single getCellValue(sqlite3_stmt* stmt, int i) {
				if constexpr(std::is_integral<Single>::value) {
					return static_cast<Single>(sqlite3_column_int(stmt, i));
				}
				else if constexpr(std::is_floating_point<Single>::value) {
					return static_cast<Single>(sqlite3_column_double(stmt, i));
				}
				else {
					static_assert(is_string_type<Single>::value,
					"Unsupported type");
					const unsigned char* value = sqlite3_column_text(stmt, i);
					return reinterpret_cast<const char*>(value);
				}
			}

			template<typename... Args> class ResultSetHelper {};	//主模板

			template<> class ResultSetHelper<> {
				public:
					static std::tuple<> getResultCell(sqlite3_stmt* stmt, int i) {
						return std::make_tuple();
					}
			};

			template<typename Single, typename... Args>
			class ResultSetHelper<Single, Args...>: private ResultSetHelper<Args...>
			{
				public:
					ResultSetHelper() {
					}
					static std::tuple<Single, Args...> getResultCell(sqlite3_stmt* stmt, int i) {
						return std::tuple_cat(std::make_tuple(getCellValue<Single>(stmt, i)), ResultSetHelper<Args...>::getResultCell(stmt, i + 1));
					}
			};

			template<typename... Args>
			std::tuple<Args...> getResultRow(sqlite3_stmt* stmt) {
				return ResultSetHelper<Args...>::getResultCell(stmt, 0);
			}

			class sqliteConnector  {
				public:
					sqliteConnector();
					explicit sqliteConnector(sqlite3* sq);
					~sqliteConnector();
					int beginTranscation();
					int commitTranscation();
					int rollbackTranscation();
					int excuteQuery(const std::string sql);

					template<typename... Args>
					std::vector<std::tuple<Args...>> getResultSet(const std::string& sql) {
						std::vector<std::tuple<Args...>> resultSet;
						sqlite3_stmt* stmt = nullptr;
						int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
						if (result == SQLITE_OK) {
							int numColumns = sqlite3_column_count(stmt);
							while (sqlite3_step(stmt) == SQLITE_ROW) {
								resultSet.push_back(getResultRow<Args...>(stmt));
							}
							sqlite3_finalize(stmt);
						}
						return resultSet;
					}

					void close();
				private:
					sqlite3* db;
			};

		}
	}
}