#include"DataSourceList.h"
namespace jxtd {
	namespace management {
		namespace datasource {
			std::tuple<uint32_t, std::string, uint32_t> DataSourceList::get_datasource(const std::string& name) const {
				const auto iter = member.find(name);
				if (iter == member.end())
					return { -1, "", -1 };
				return iter->second;
			}
			bool DataSourceList::has_datasource(const std::string& name) const {
				return (member.find(name)!=member.end());
			}
			std::map<std::string, std::tuple<uint32_t, std::string, uint32_t>> DataSourceList::get_alldatasource()const {
				return member;
			}
			void DataSourceList::set_datasource(const std::map<std::string, std::tuple<uint32_t, std::string, uint32_t>>& datasources) {
				member = datasources;
			}
			void  DataSourceList::add_datasource(const std::string& name, const std::tuple<uint32_t, std::string, uint32_t>& endpoint) {
				member.insert({ name , endpoint });
			}
		}
	}
}
