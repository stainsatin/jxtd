#pragma once
#include <string>
#include <cstdint>
#include <vector>
#include <workflow/Workflow.h>
#include <misc/adapter/noncopy.h>
#include <computation/drccp/server.h>
#include "SubmitData.pb.h"
#include <map>
#include <unordered_map>
#include "proto/taskdep/message.h"
#include <mutex>

namespace jxtd
{
	namespace computation
	{
		namespace infer
		{
			class SubmitRequest
			{
			public:
				SubmitRequest() = default;
				~SubmitRequest() = default;
				SubmitRequest(const SubmitRequest &other);
				SubmitRequest(SubmitRequest &&other) noexcept;
				SubmitRequest &operator=(const SubmitRequest &other);
				SubmitRequest &operator=(SubmitRequest &&other) noexcept;
				int set_taskid(uint32_t taskid);
				uint32_t get_taskid() const;
				int set_input(const std::string &url, const std::string &data);
				std::string get_input_path() const;
				std::string get_input_data() const;
				int set_output(const std::string &url, uint32_t directory_policy, uint32_t empty_policy);
				std::string get_output_path() const;
                uint32_t get_directory_policy() const;
                uint32_t get_empty_policy() const;
				// times:需要执行次数
				int set_period(uint32_t times);
				uint32_t get_period() const;
				std::string dump() const;
				int parse(const std::string &binary);

			private:
				RequestData SubmitRequestData;
			};

			class SubmitResponse
			{
			public:
				SubmitResponse() = default;
				~SubmitResponse() = default;
				SubmitResponse(const SubmitResponse &other);
				SubmitResponse(SubmitResponse &&other) noexcept;
				SubmitResponse &operator=(const SubmitResponse &other);
				SubmitResponse &operator=(SubmitResponse &&other) noexcept;
				int set_taskid(uint32_t taskid);
				uint32_t get_taskid() const;
				// times:需要执行次数
				int set_times(uint32_t times);
				uint32_t get_times() const;
				int set_result(const std::string &result);
				std::string get_result() const;
				std::string dump() const;
                int set_output(const std::string &output_path, uint32_t directory_policy, uint32_t empty_policy);
                std::string get_output_path() const;
                uint32_t get_directory_policy() const;
                uint32_t get_empty_policy() const;
				int parse(const std::string &binary);

			private:
				ResponseData SubmitResponseData;
			};

			class Taskmanager;
			class DrccpServerManager;

			class DrccpServerManager : private jxtd::misc::adapter::NonCopy
			{
			public:
				using Server = struct
				{
					jxtd::computation::drccp::DrccpServer server;
					jxtd::computation::drccp::DrccpServer::DrccpSetupContext SetupContext;
					jxtd::computation::drccp::DrccpServer::DrccpProxyOriginContext OriginContext;
				};

				DrccpServerManager() = default;
				~DrccpServerManager();
				// 加载一个DrccpServerManager并设置name
				void load_server_config(const std::string &token, const std::string &url, int port, bool ssl, const std::string &name, const jxtd::computation::drccp::DrccpServer::DrccpSetupContext &setup, const jxtd::computation::drccp::DrccpServer::DrccpProxyOriginContext &proxy);
				int start(const std::string &name);
				int stop(const std::string &name);
				// 通用形式，指定任意入口节点
				int signin(const std::string &name, const std::string &cluster_tag, const std::string &boot_addr, int port);
				// 指定已加载的节点
				int signin(const std::string &name, const std::string &tag, const std::string &peer);
				int logout(const std::string &name);
				SubTask *submit(const std::string &name, const SubmitRequest &req, Taskmanager &manager);

			private:
				std::map<std::string, Server *> servers;
			};

			using Endpoint = ::jxtd::proto::taskdep::Endpoint;

            class Taskmanager
            {
            public:
				struct Taskinfo
                {
					uint64_t dep_pos;
                    std::string input_path;
					std::string input_data;
                    std::string output_path;
					uint32_t directory_policy;
					uint32_t empty_policy;
                    uint32_t period;
					std::vector<uint32_t> backupid;
					uint32_t result_policy;
					uint32_t trigger_policy;
                };

				Taskmanager();
				~Taskmanager();
                int deploy_task(uint64_t taskid, const std::string &input_path, const std::string &input_data, const std::string &output_path, uint32_t directorypolicy, uint32_t emptypolicy, uint32_t period, uint32_t resultpolicy, uint32_t triggerpolicy, std::vector<uint32_t> backupid);
                int add_deps(uint64_t taskid, std::vector<uint64_t> deps);
                SubTask *submit(DrccpServerManager &manager, const std::string &name, uint64_t taskid);
				const Taskinfo *get_task(uint64_t taskid);
                void remove_task(uint64_t taskid, bool send);
				int set_src(const Endpoint &endpoint, bool ssl, const std::string &token);
				int set_dest(const Endpoint &storage);
                
			private:
				struct ClusterClient
				{
					Endpoint endpoint;
                	bool ssl;
					std::string token;
				};

                std::vector<std::vector<bool>> edges;
                std::unordered_map<uint64_t, Taskinfo> tasks;
				ClusterClient *client;
				Endpoint *server;
				std::mutex task_mutex;
            };
			
			void set_storage_datatransfer(const std::string &url, int port, bool ssl);
		}
	}
}
