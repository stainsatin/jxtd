#pragma once
#include <cstdint>
#include <vector>
#include <functional>
#include <string>

namespace jxtd
{
    namespace management
    {
        namespace inferprovider
        {
            enum class Trigger_method : uint32_t
            {
                Dependency,
                Manual,
                Create
            };

            class Task
            {
            public:
                Task();
                ~Task() = default;

                void set_taskid(uint64_t taskid);
                uint64_t get_taskid() const;
                void add_dep(uint64_t depid);
                const std::vector<uint64_t> &get_deps() const;
                int switch_engine(std::string name);
                void add_engine(std::string name);
                std::string get_engine_current();
                void delete_engine(std::string name);
                void set_run_count(int run_count);
                int get_run_count() const;
                void set_trigger(Trigger_method method);
                Trigger_method get_trigger() const;
                void set_output(const std::string &output);
                std::string get_output() const;
                void start();
                void stop();
                bool get_status() const;

            private:
                uint64_t taskid;
                std::vector<std::string> engines;
                std::vector<uint64_t> deps;
                int run_count;
                bool status;
                Trigger_method method;
                std::string current_engine;
                std::string output;
            };

            class Task_Manager
            {
            public:
                Task_Manager() = default;
                ~Task_Manager();
                Task *create_task(uint64_t taskid);
                int add_task(Task *task);
                Task *get_task(uint64_t taskid);
                void delete_task(uint64_t taskid);
                const std::vector<Task *> &get_tasks() const;

            private:
                std::vector<Task *> tasks;
            };
        }
    }
}
