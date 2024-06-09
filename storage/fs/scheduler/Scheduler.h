#pragma once
#include <unordered_map>
#include <string>
#include <functional>

namespace jxtd
{
    namespace storage
    {
        namespace fs
        {
            namespace scheduler
            {
                struct Operations
                {
                    std::function<int(void *ctx)> init;
                    std::function<int(const char *filename, int flags, mode_t mode)> open;
                    std::function<int(int fd)> close;
                    std::function<ssize_t(int fd, const void * buf, size_t count)> write;
                    std::function<ssize_t(int fd, void * buf, size_t count)> read;
                    std::function<off_t(int fd, off_t offset, int whence)> seek;
                    std::function<int(const char *filepath)> remove;
                    std::function<int(const char *filename, int flags, mode_t mode)> opendir;
                    std::function<int(const char *filepath, mode_t mode)> mkdir;
                    std::function<int(int fd)> closedir;
                    std::function<int(const char *filepath)> readdir;
                    std::function<int(const char *filepath)> rmdir;
                    std::function<off_t(int fd)> fsize;
                    std::function<int(int fd, off_t offset)> ftrun;
                    std::function<int(int fd)> fmod;
                    std::function<int(int fd, mode_t mode)> fchmod;
                    std::function<void()> destroy;
                };

                enum class SchedulingPolicy
                {
                    LoadBalance,
                    Single
                };

                enum class ServiceRange
                {
                    All,
                    Specified
                };

                // 提供了获取，注册，使能，策略选择，下线，卸载，以及一系列操作接口，操作接口
                // 支持重复new Service
                // 必须通过一次策略选择才能开始工作，默认为均衡调度全体服务，scope为空
                // 一次策略选择会选定当前的service，每次的操作都会通过该service进行
                // 全体服务决定服务范围为所有服务，指定服务决定服务范围为对应scope的服务（scope可以为空）
                // 均衡调度在服务的范围内任意选择服务，单一调度通过提供的id来决定使用哪个服务（不根据服务范围，而是直接通过id）
                // 策略选择时需要提供策略以及在指定服务的时候的需要指定的scope
                // 通过获取的service来设置ctx和operations
                // 如果某一个service没有提供对应的操作接口，无法找到拥有该接口的服务（也可以添加该API）
                // 每一次切换service都需要调用一次策略选择
                // 如果服务没有对应的操作接口，而使用了该操作接口，则直接返回失败
                class Scheduler
                {
                public:
                    // 只提供了对于ctx和operations的接口, 和获取status和scope的接口，服务id无法获取，需要用户自行管理，并且status，id和scope
                    class Service
                    {
                    public:
                        Service();

                        void set_operations(const Operations &operations);
                        Operations &get_operations();
                        void set_ctx(void *ctx);
                        void *get_ctx();
                        bool get_status() const;
                        const std::string &get_scope() const;

                    private:
                        friend class Scheduler;

                        bool status;
                        std::string scope;
                        void *ctx;
                        Operations operations;
                    };

                    Scheduler();
                    ~Scheduler();
                    Scheduler(const Scheduler &other) = delete;
                    Scheduler(Scheduler &&other) = delete;
                    Scheduler &operator=(const Scheduler &other) = delete;
                    Scheduler &operator=(Scheduler &&other) = delete;
                    // 获取服务
                    Service *find_service(int id);
                    // 注册服务
                    int register_service(int id, const Operations &operations, const std::string &scope);
                    // 使能服务
                    void enable_service(const std::vector<int> &ids);
                    // 服务下线
                    void disable_service(const std::vector<int> &ids);
                    // 卸载服务
                    void unload_service(const std::vector<int> &ids);
                    // 策略选择
                    void strategy_choose(SchedulingPolicy policy = SchedulingPolicy::LoadBalance, int select_id = -1, ServiceRange range = ServiceRange::All, const std::string &select_scope = "");
                    // 改变scope(查找失败也不会返回错误)
                    void change_scope(int id, const std::string &scope);
                    // 获取当前service
                    const Service *get_current_service() const;

                    int open(const char *filename, int flags, mode_t mode);
                    int close(int fd);
                    ssize_t write(int fd, const void *buf, size_t count);
                    ssize_t read(int fd, void *buf, size_t count);
                    off_t seek(int fd, off_t offset, int whence);
                    int remove(const char *filepath);
                    int opendir(const char *filename, int flags, mode_t mode);
                    int mkdir(const char *filepath, mode_t mode);
                    int closedir(int fd);
                    int readdir(const char *filepath);
                    int rmdir(const char *filepath);
                    off_t fsize(int fd);
                    int ftrun(int fd, off_t length);
                    int fmod(int fd);
                    int fchmod(int fd, mode_t mode);

                private:
                    std::unordered_multimap<std::string, Service *> service_scope;
                    std::unordered_map<int, Service *> services;
                    SchedulingPolicy policy;
                    ServiceRange range;
                    std::string select_scope;
                    int select_id;
                    Service *current_service;

                    Service *service_choose();
                };
            }
        }
    }
}