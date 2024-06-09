#pragma once
#include "storage/fs/scheduler/Scheduler.h"
#include "proto/taskdep/message.h"
#include <string>
#include <cstdint>

namespace jxtd
{
    namespace storage
    {
        namespace datamanagement
        {
            using Scheduler = ::jxtd::storage::fs::scheduler::Scheduler;
            using Operations = ::jxtd::storage::fs::scheduler::Operations;
            using Policy = ::jxtd::storage::fs::scheduler::SchedulingPolicy;
            using Range = ::jxtd::storage::fs::scheduler::ServiceRange;
            using Engine = ::jxtd::proto::taskdep::Request::Engine;
            using Extfile = Engine::Extfile;

            class Datamanager
            {
            public:
                // 将local的operations注册并使能，如果需要则添加，且默认为local，如果需要使用其他文件系统，则使用backend_choose
                Datamanager();
                ~Datamanager();

                // scheduler操作，详情见scheduler.cc（暂未实现）
                
                // 初始化根目录，如果new_为true，已存在则打开失败，如果new_为false，不存在则打开失败, path为根目录的绝对路径
                int init(const std::string &path, bool is_new_root);
                // 清除根目录
                void deinit();
                // 选择文件系统后端，指定，Policy为Single（目前），如果需要可以继续添加，此时不能直接使用，而要继续init
                int backend_choose(const std::string &filesystem);
                // runtime数据的操作
                std::string runtime_pull(uint64_t taskid);
                int runtime_push(uint64_t taskid, const std::string &runtime);
                void runtime_remove(uint64_t taskid);
                // engine操作
                std::string engine_pull(const std::string &mid, const std::string &name);
                int engine_push(const std::string &mid, const std::string &name, const std::string &engine);
                void engine_remove(const std::string &mid, const std::string &name);
                // binary普通数据操作，该path为路径且最后为文件名
                std::string binary_pull(const std::string &path);
                int binary_push(const std::string &path, const std::string &binary);
                void binary_remove(const std::string &path);

                void task_bind(uint64_t taskid, std::pair<std::string, std::string> engine, const std::vector<Engine> &deps);
                void task_unbind(uint64_t taskid);
                bool task_exist(uint64_t taskid);
                std::pair<std::pair<std::string, std::string>, std::vector<Engine>> &get_task_engine(uint64_t taskid);

                void add_extfiles(uint64_t taskid, const std::vector<Extfile> &extfiles);
                void remove_extfiles(uint64_t taskid);
                bool extfiles_exist(uint64_t taskid);
                std::vector<Extfile> &get_task_extfiles(uint64_t taskid);

                bool get_is_init() const;
                const std::string &get_root_path() const;
                
            private:
                Scheduler *scheduler;
                std::string root_path;
                bool is_init;
                std::unordered_map<uint64_t, std::vector<Extfile>> task_extfiles; // uint64_t---taskid，std::vector<Extfile>---extfiles(endpoint and path)
                std::unordered_map<uint64_t, std::pair<std::pair<std::string, std::string>, std::vector<Engine>>> task_engine; // uint64_t---taskid，std::pair<std::string, std::string>---engines
            };
        }
    }
}
