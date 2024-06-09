#include "DataSourceList.h"
#include <proto/datasource/message.h>
#include "DataSourceServer.h"
#include <workflow/WFServer.h>
#include <workflow/WFFacilities.h>
#include <string>
#include <functional>
namespace jxtd{
	namespace management {
		namespace datasource{
			using DataSourceRequest = ::jxtd::proto::datasource::Request;
			using DataSourceResponse = ::jxtd::proto::datasource::Response;
			using DataSourceWFServer = WFServer<DataSourceRequest, DataSourceResponse>;
			using DataSourceTask = WFNetworkTask<DataSourceRequest, DataSourceResponse>;
			static void process(DataSourceTask* task, DataSourceServerContext* ctx);
			struct DataSourceServerContext {
				int family;
				std::string addr;
				int port;
				bool ssl;
				std::string ca_path;
				std::string key_path;
				std::string cert_path;
				DataSourceWFServer* server = nullptr;
				DataSourceList* list = nullptr;
			};
			DataSourceServer::DataSourceServer() :ctx(new DataSourceServerContext) {
				ctx->list = new DataSourceList();
			}
			DataSourceServer::~DataSourceServer() {
				if (this->ctx->server)
					this->stop();
				delete this->ctx->list;
				delete this->ctx;
			}
			DataSourceServer::DataSourceServer(const DataSourceServerSetupContext& ctx): DataSourceServer() {
				setup_context(ctx);
			}
			void DataSourceServer::setup_context(const DataSourceServerSetupContext& ctx) {
				this->ctx->family = ctx.family;
				this->ctx->addr = ctx.addr;
				this->ctx->port = ctx.port;
				this->ctx->ssl = ctx.ssl;
				this->ctx->ca_path = ctx.ca_path;
				this->ctx->key_path = ctx.key_path;
				this->ctx->cert_path = ctx.cert_path;
			}
			int DataSourceServer::start() {
				ctx->server = new DataSourceWFServer(
					std::bind(process, std::placeholders::_1, this->ctx)
				);
				if (ctx->ssl)
					return this->ctx->server->start(
						ctx->family,
						ctx->addr.c_str(),
						ctx->port,
						ctx->cert_path.c_str(),
						ctx->key_path.c_str()
					);
				return this->ctx->server->start(
					ctx->family,
					ctx->addr.c_str(),
					ctx->port
				);
			}
			int DataSourceServer::stop() {
				if (!ctx->server)
					return 0;
				this->ctx->server->stop();
				delete this->ctx->server;
				this->ctx->server = nullptr;
				return 0;
			}
			const DataSourceList* DataSourceServer::get_datasource() const {
				return ctx->list;
			}

			static void process(DataSourceTask* task, DataSourceServerContext* ctx) {
				auto req = task->get_req();
				auto resp = task->get_resp();
				resp->set_token("");
				resp->set_name(req->get_name());
				if (ctx->list->has_datasource(req->get_name())) {
					resp->set_status(DataSourceResponse::Status_err);
					return;
				}
				ctx->list->add_datasource(req->get_name(), { req->get_proto(), req->get_url(), req->get_port() });
				resp->set_status(DataSourceResponse::Status_ok);
			}
		}
	}
}
