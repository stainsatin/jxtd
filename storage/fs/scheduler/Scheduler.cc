#include "Scheduler.h"
#include <algorithm>
#include <ctime>

namespace jxtd
{
    namespace storage
    {
        namespace fs
        {
            namespace scheduler
            {
                Scheduler::Service::Service(): ctx(nullptr), status(false) {}

                void Scheduler::Service::set_operations(const Operations &operations)
                {
                    this->operations = operations;
                }

                Operations &Scheduler::Service::get_operations()
                {
                    return this->operations;
                }

                void Scheduler::Service::set_ctx(void *ctx)
                {
                    this->ctx = ctx;
                }

                void *Scheduler::Service::get_ctx()
                {
                    return this->ctx;
                }

                bool Scheduler::Service::get_status() const
                {
                    return this->status;
                }

                const std::string &Scheduler::Service::get_scope() const
                {
                    return this->scope;
                }

                Scheduler::Scheduler() : policy(SchedulingPolicy::LoadBalance), range(ServiceRange::All), current_service(nullptr), select_id(-1) {}

                Scheduler::~Scheduler()
                {
                    service_scope.clear();
                    for (auto it = services.begin(); it != services.end(); it++)
                    {
                        if(it->second->status && it->second->operations.destroy != nullptr)
                            it->second->operations.destroy();
                        delete it->second;
                        it->second = nullptr;
                    }
                    services.clear();
                }

                Scheduler::Service *Scheduler::find_service(int id)
                {
                    auto it = this->services.find(id);
                    if(it == this->services.end())
                        return nullptr;
                    return it->second;
                }

                // 注册服务，不支持数组
                int Scheduler::register_service(int id, const Operations &operations, const std::string &scope)
                {
                    if(id <= 0 || this->find_service(id) != nullptr)
                        return -1;
                    auto service_ptr = new Service;
                    service_ptr->operations = operations;
                    service_ptr->scope = scope;
                    services.emplace(id, service_ptr);
                    service_scope.emplace(scope, service_ptr);
                    return 0;
                }

                // 使能服务
                void Scheduler::enable_service(const std::vector<int> &ids)
                {
                    for (const auto &id : ids)
                    {
                        auto it = services.find(id);
                        if (it != services.end())
                        {
                            Service *service = it->second;
                            if (!service->status)
                            {
                                service->status = true;
                                // 初始化服务
                                if (service->operations.init != nullptr && service->operations.init(service->ctx) == -1)
                                {
                                    if (service->operations.destroy != nullptr)
                                        service->operations.destroy();
                                    service->status = false;
                                }
                            }
                        }
                    }
                }

                // 服务下线
                void Scheduler::disable_service(const std::vector<int> &ids)
                {
                    for (const auto &id : ids)
                    {
                        auto it = services.find(id);
                        if (it != services.end())
                        {
                            Service *service = it->second;
                            if (service->status)
                            {
                                if (service->operations.destroy != nullptr)
                                    service->operations.destroy();
                                service->status = false;
                            }
                        }
                    }
                }

                // 卸载服务
                void Scheduler::unload_service(const std::vector<int> &ids)
                {
                    disable_service(ids);
                    for (const auto &id : ids)
                    {
                        auto it = services.find(id);
                        if (it != services.end())
                        {
                            auto range = service_scope.equal_range(it->second->scope);
                            for(auto iter = range.first; iter != range.second; iter++)
                            {
                                if(iter->second == it->second)
                                {
                                    service_scope.erase(iter);
                                    break;
                                }
                            }
                            delete it->second;
                            it->second = nullptr;
                            services.erase(it);
                        }
                    }
                }

                // 策略选择
                void Scheduler::strategy_choose(SchedulingPolicy policy, int select_id, ServiceRange range, const std::string &scope)
                {
                    this->policy = policy;
                    this->select_id = select_id;
                    if(policy == SchedulingPolicy::LoadBalance)
                        this->select_id = -1;
                    this->range = range;
                    this->select_scope = scope;
                    if(range == ServiceRange::All)
                        this->select_scope = "";
                    this->current_service = service_choose();
                }

                static Scheduler::Service *Balance_Select_Service(const std::unordered_map<std::string, Scheduler::Service *>::iterator &begin, const std::unordered_map<std::string, Scheduler::Service *>::iterator &end)
                {
                    srand((unsigned)time(nullptr));
                    if(begin == end)
                        return nullptr;
                    std::vector<Scheduler::Service *> services;
                    for(auto it = begin; it != end; it++)
                    {
                        if(it->second->get_status())
                            services.emplace_back(it->second);
                    }
                    if(services.empty())
                        return nullptr;
                    return services.at((rand() % services.size()));
                }

                Scheduler::Service *Scheduler::service_choose()
                {
                    if (this->policy == SchedulingPolicy::Single)
                    {
                        auto service = this->find_service(select_id);
                        if(service == nullptr || !service->status)
                            return nullptr;
                        return service;
                    }
                    else
                    {
                        if(this->range == ServiceRange::Specified)
                        {
                            auto range = service_scope.equal_range(this->select_scope);
                            return Balance_Select_Service(range.first, range.second); // 将end++以便于能够选择到end对应的service
                        }
                        else
                        {
                            return Balance_Select_Service(this->service_scope.begin(), this->service_scope.end());
                        }
                    }
                }

                void Scheduler::change_scope(int id, const std::string &scope)
                {
                    auto service_ptr = this->find_service(id);
                    if(service_ptr == nullptr)
                        return;
                    auto range = this->service_scope.equal_range(service_ptr->scope);
                    for(auto it = range.first; it != range.second; it++)
                    {
                        if(it->second == service_ptr)
                        {
                            service_scope.erase(it);
                            service_ptr->scope = scope;
                            service_scope.emplace(scope, service_ptr);
                            return;
                        }
                    }
                    return;
                }

                const Scheduler::Service *Scheduler::get_current_service() const
                {
                    return this->current_service;
                }

                #define operate(operator) \
                if (this->current_service == nullptr || this->current_service->operations.operator == nullptr) \
                    return -1; \
                return this->current_service->operations.operator

                int Scheduler::open(const char *filename, int flags, mode_t mode)
                {
                    operate(open)(filename, flags, mode);
                }

                int Scheduler::close(int fd)
                {
                    operate(close)(fd);
                }

                ssize_t Scheduler::write(int fd, const void *buf, size_t count)
                {
                    operate(write)(fd, buf, count);
                }

                ssize_t Scheduler::read(int fd, void *buf, size_t count)
                {
                    operate(read)(fd, buf, count);
                }

                off_t Scheduler::seek(int fd, off_t offset, int whence)
                {
                    operate(seek)(fd, offset, whence);
                }

                int Scheduler::remove(const char *filepath)
                {
                    operate(remove)(filepath);
                }

                int Scheduler::opendir(const char *filepath, int flags, mode_t mode)
                {
                    operate(opendir)(filepath, flags, mode);
                }

                int Scheduler::mkdir(const char *filepath, mode_t mode)
                {
                    operate(mkdir)(filepath, mode);
                }

                int Scheduler::closedir(int fd)
                {
                    operate(closedir)(fd);
                }

                int Scheduler::readdir(const char *filepath)
                {
                    operate(readdir)(filepath);
                }

                int Scheduler::rmdir(const char *filepath)
                {
                    operate(rmdir)(filepath);
                }

                off_t Scheduler::fsize(int fd)
                {
                    operate(fsize)(fd);
                }

                int Scheduler::ftrun(int fd, off_t length)
                {
                    operate(ftrun)(fd, length);
                }

                int Scheduler::fmod(int fd)
                {
                    operate(fmod)(fd);
                }

                int Scheduler::fchmod(int fd, mode_t mode)
                {
                    operate(fchmod)(fd, mode);
                }
            }
        }
    }
}