#pragma once
#include "misc/parser/at/ATParser.h"
#include "management/inferprovider/task.h"
#include "management/taskdep/taskdep.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <router/lib/flag/flag.h>
#include <mutex>
#include <condition_variable>
#include <workflow/Workflow.h>
#include <workflow/WFHttpServer.h>

namespace jxtd
{
    namespace router
    {
        using ATMessage = ::jxtd::misc::parser::at::ATMessage;
        using Taskmanager = ::jxtd::management::inferprovider::Task_Manager;
        using Taskdepmanagement = ::jxtd::management::taskdep::ManagementClient;
        using Flag = ::jxtd::router::flag::Flag;

        class Taskresult
        {
        public:
            Taskresult();
            ~Taskresult();
            int start(); // start 和 top 可以放在taskmanager里面, 然后start绑定一个task模型进去
            int stop();
            void set(uint64_t taskid);
            std::string get_data();
            std::string &get_data_set();
            uint64_t get_taskid();
            void set_status(bool status);
            bool get_status();
            void remove();
            void wait();
            void notify();
            
        private:
            std::string data;
            std::mutex mutex_;
            std::condition_variable condition;
            WFHttpServer *server;
            bool status;
            uint64_t taskid;
        };

        extern Taskresult task;
        extern Taskmanager manager;
        extern Taskdepmanagement management;
        extern Flag flag_;

        std::string REG(ATMessage &message);
        std::string RTUNREG(ATMessage &message);
        std::string RTBUSY(ATMessage &message);

        std::string MDDECL(ATMessage &message);
        std::string MDRM(ATMessage &message);
        std::string MDPUSH(ATMessage &message);

        std::string CPSTART(ATMessage &message);
        std::string CPMOD(ATMessage &message);
        std::string CPRDY(ATMessage &message);
        std::string CPVAL(ATMessage &message);

        std::string TRANS(ATMessage &message);
        std::string STRANS(ATMessage &message);
    }
}