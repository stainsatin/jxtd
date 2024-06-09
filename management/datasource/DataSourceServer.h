#pragma once
#include <string>
#include "DataSourceList.h"
#include <misc/adapter/noncopy.h>
//struct jxtd::management::datasource:: DataSourceServerSetupContext

namespace jxtd {
	namespace management {
		namespace datasource {
			struct DataSourceServerSetupContext {
				int family;
				std::string addr;
				int port;
				bool ssl;
				std::string ca_path;
				std::string key_path;
				std::string cert_path;
			};
			struct DataSourceServerContext;
			//class jxtd::management::datasource::DataSourceServer
			class DataSourceServer:private ::jxtd::misc::adapter::NonCopy {
			public:
				DataSourceServer();
				~DataSourceServer();
				explicit DataSourceServer(const DataSourceServerSetupContext& ctx);
				void setup_context(const DataSourceServerSetupContext& ctx);
				int start();
				int stop();
				const DataSourceList* get_datasource() const;
			private:
				DataSourceServerContext* ctx;
			};
		}
	}
}
