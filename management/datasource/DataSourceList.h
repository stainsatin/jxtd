#pragma once
#include<map>
#include<tuple>
#include<string>
#include<cstdint>
namespace jxtd {
	namespace management {
		namespace datasource {
			class DataSourceList {
				public:
					DataSourceList() = default;
					~DataSourceList() = default;
					//proto, url, port
					std::tuple<uint32_t, std::string, uint32_t> get_datasource(const std::string& name) const;
					bool has_datasource(const std::string& name) const;
					std::map<std::string, std::tuple<uint32_t, std::string, uint32_t>> get_alldatasource()const;
					void set_datasource(const std::map<std::string, std::tuple<uint32_t, std::string, uint32_t>>& datasources);
					void add_datasource(const std::string& name, const std::tuple<uint32_t, std::string, uint32_t>& endpoint);
			private:
					std::map<std::string, std::tuple<uint32_t, std::string, uint32_t>> member;
			};
		}
	}
}
