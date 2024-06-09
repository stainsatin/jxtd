#include "task.h"

namespace jxtd
{
    namespace management
    {
        namespace inferprovider
        {
            Task::Task():taskid(0), run_count(0), status(false), method(Trigger_method::Create) {}

            void Task::set_taskid(uint64_t taskid)
            {
                this->taskid = taskid;
            }

            uint64_t Task::get_taskid() const
            {
                return this->taskid;
            }

            void Task::add_dep(uint64_t depid)
            {
                this->deps.emplace_back(depid);
            }

            const std::vector<uint64_t> &Task::get_deps() const
            {
                return this->deps;
            }

            int Task::switch_engine(std::string name)
            {
                for(const auto &engine : this->engines)
                {
                    if(engine == name)
                    {
                        this->current_engine = name;
                        return 0;
                    }
                }
                return -1;
            }

            void Task::add_engine(std::string name)
            {
                for(const auto &engine : this->engines)
                    if(engine == name)
                        return;
                this->engines.emplace_back(name);
            }

            std::string Task::get_engine_current()
            {
                return this->current_engine;
            }

            void Task::delete_engine(std::string name)
            {
                for(auto it = this->engines.begin(); it != this->engines.end(); it++)
                {
                    if(*it == name)
                    {
                        if(name == this->current_engine)
                            this->current_engine.clear();
                        this->engines.erase(it);
                        break;
                    }
                }
            }

            void Task::set_run_count(int run_count)
            {
                this->run_count = run_count;
            }

            int Task::get_run_count() const
            {
                return this->run_count;
            }

            void Task::set_trigger(Trigger_method method)
            {
                this->method = method;
            }

            Trigger_method Task::get_trigger() const
            {
                return this->method;
            }

            void Task::set_output(const std::string &output)
            {
                this->output = output;
            }

            std::string Task::get_output() const
            {
                return this->output;
            }

            void Task::start()
            {
                this->status = true;
            }

            void Task::stop()
            {
                this->status = false;
            }

            bool Task::get_status() const
            {
                return this->status;
            }

            Task *Task_Manager::create_task(uint64_t taskid)
            {
                Task *task_new = new Task;
                task_new->set_taskid(taskid);
                return task_new;
            }

            int Task_Manager::add_task(Task *task)
            {
                this->tasks.emplace_back(task);
                return 0;
            }

            Task *Task_Manager::get_task(uint64_t taskid)
            {
                for(const auto task : this->tasks)
                {
                    if(task->get_taskid() == taskid)
                        return task;
                }
                return nullptr;
            }

            void Task_Manager::delete_task(uint64_t taskid) // 删除不存在的返回了0
            {
                for(auto iter = this->tasks.begin(); iter != this->tasks.end(); iter++)
                {
                    if((*iter)->get_taskid() == taskid)
                    {
                        delete (*iter);
                        *iter = nullptr;
                        this->tasks.erase(iter);
                        break;
                    }
                }
            }

            const std::vector<Task *> &Task_Manager::get_tasks() const
            {
                return this->tasks;
            }

            Task_Manager::~Task_Manager()
            {
                for(const auto &task : this->tasks)
                {
                    delete task;
                }
            }
        }
    }
}
