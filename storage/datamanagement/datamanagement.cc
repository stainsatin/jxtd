#include "datamanagement.h"
#include <fcntl.h>
#include "storage/fs/local/Fileoperations.h" // 目前只有local的，需要请自行添加 // 这里使<fcntl.h>添加进来了，主要是为了使用O_等flags
#include <unistd.h>
#include <unordered_map>

namespace jxtd
{
    namespace storage
    {
        namespace datamanagement
        {
            // 服务名（文件系统名）和id的一个对应关系
            static std::unordered_map<std::string, int> service_name_id;
            // 注册为service，id为1，scope为"local",并且使能，默认使用local文件系统
            Datamanager::Datamanager()
            {
                this->is_init = false;
                scheduler = new Scheduler;
                scheduler->register_service(1, ::jxtd::storage::fs::local::Get_local_fileoperations(), "local");
                scheduler->enable_service({1});
                service_name_id.emplace("local", 1);
                scheduler->strategy_choose(Policy::Single, 1);
            }

            Datamanager::~Datamanager()
            {
                service_name_id.clear();
                delete this->scheduler;
            }

            // path为根目录的绝对路径，new_为是否为新的
            int Datamanager::init(const std::string &path, bool is_new_root)
            {
                this->task_engine.clear();
                this->task_extfiles.clear();
                int judge = -1;
                if(is_new_root)
                {
                    judge = this->scheduler->mkdir(path.c_str(), 0770);
                    if(judge == -1)
                        return -1;
                    judge = this->scheduler->opendir(path.c_str(), O_DIRECTORY, 0770);
                    if(judge == -1)
                        return -1;
                    this->scheduler->mkdir((path + "/runtime").c_str(), 0770);
                    this->scheduler->mkdir((path + "/engine").c_str(), 0770);
                    this->scheduler->mkdir((path + "/binary").c_str(), 0770); // 没有检查是否创建成功
                }
                else
                {
                    judge = this->scheduler->opendir(path.c_str(), O_DIRECTORY, 0770);
                    if(judge == -1)
                        return -1;
                }
                this->root_path = path;
                this->scheduler->closedir(judge);
                this->is_init = true;
                return 0;
            }

            void Datamanager::deinit()
            {
                if(!this->is_init)
                    return;
                this->task_extfiles.clear();
                this->task_engine.clear();
                this->scheduler->rmdir(this->root_path.c_str());
                this->is_init = false;
            }

            // 选择文件系统，目前只提供了single类型，如果需要可进行重载，经过backend_choose后还需要再次init
            int Datamanager::backend_choose(const std::string &filesystem)
            {
                auto it = service_name_id.find(filesystem);
                if(it == service_name_id.end())
                    return -1;
                this->scheduler->strategy_choose(Policy::Single, it->second);
                if(this->scheduler->get_current_service() == nullptr) // 一般不会，除非未经过使能
                {
                    // 切换回本地文件系统
                    this->scheduler->strategy_choose(Policy::Single, 1);
                    return -1;
                }
                this->root_path.clear();
                this->is_init = false;
                return 0;
            }

            #define PULL_DATA \
                if(fd == -1) \
                    return ""; \
                int size = this->scheduler->fsize(fd); \
                if(size == -1) \
                { \
                    this->scheduler->close(fd); \
                    return ""; \
                } \
                std::string data(size, '\0'); \
                if(this->scheduler->read(fd, &data[0], size) == -1) \
                { \
                    this->scheduler->close(fd); \
                    return ""; \
                } \
                this->scheduler->close(fd); \
                return std::move(data)

            #define PUSH_DATA(data_) \
                if(fd == -1) \
                    return -1; \
                if(this->scheduler->write(fd, data_.data(), data_.size()) == -1) \
                { \
                    this->scheduler->close(fd); \
                    return -1; \
                } \
                this->scheduler->close(fd); \
                return 0

            std::string Datamanager::runtime_pull(uint64_t taskid)
            {
                int fd = this->scheduler->open((this->root_path + "/runtime/Task_" + std::to_string(taskid)).c_str(), O_RDONLY, 0660);
                PULL_DATA;
            }

            int Datamanager::runtime_push(uint64_t taskid, const std::string &runtime)
            {
                int fd = this->scheduler->open((this->root_path + "/runtime/Task_" + std::to_string(taskid)).c_str(), O_CREAT | O_WRONLY | O_APPEND, 0660);
                PUSH_DATA(runtime);
            }

            void Datamanager::runtime_remove(uint64_t taskid)
            {
                this->scheduler->remove((this->root_path + "/runtime/Task_" + std::to_string(taskid)).c_str());
            }

            std::string Datamanager::engine_pull(const std::string &mid, const std::string &name)
            {
                int fd = this->scheduler->open((this->root_path + "/engine/JXTDENGINE_" + mid + "_" + name + ".so").c_str(), O_RDONLY, 0660);
                PULL_DATA;
            }

            int Datamanager::engine_push(const std::string &mid, const std::string &name, const std::string &engine)
            {
                int fd = this->scheduler->open((this->root_path + "/engine/JXTDENGINE_" + mid + "_" + name + ".so").c_str(), O_WRONLY | O_CREAT | O_EXCL, 0660);
                PUSH_DATA(engine);
            }

            void Datamanager::engine_remove(const std::string &mid, const std::string &name)
            {
                this->scheduler->remove((this->root_path + "/engine/JXTDENGINE_" + mid + "_" + name + ".so").c_str());
            }

            std::string Datamanager::binary_pull(const std::string &path)
            {
                int fd = this->scheduler->open((this->root_path + "/binary/" + path).c_str(), O_RDONLY, 0660);
                PULL_DATA;
            }

            int Datamanager::binary_push(const std::string &path, const std::string &binary)
            {
                // 该段代码为找到path的最后到之前的第一个未创建的目录，并从该处开始创建目录
                int fd;
                size_t pos = 0;
                if((pos = path.find_last_of("/")) != std::string::npos)
                {
                    bool create = false;
                    std::string path_ = path;
                    path_.erase(pos);
                    while ((fd = this->scheduler->opendir((this->root_path + "/binary/" + path_).c_str(), O_DIRECTORY, 0770)) == -1)
                    {
                        if((pos = path_.find_last_of("/")) == std::string::npos)
                        {
                            create = true;
                            break;
                        }
                        path_.erase(pos);
                    }
                    pos = path_.length() + 1;
                    std::string substr = path.substr(pos);
                    if(create)
                        this->scheduler->mkdir((this->root_path + "/binary/" + path_).c_str(), 0770);
                    path_ += "/";
                    while ((pos = substr.find_first_of("/")) != std::string::npos) 
                    {
                        path_ += substr.substr(0, pos + 1);
                        substr.erase(0, pos + 1);
                        this->scheduler->mkdir((this->root_path + "/binary/" + path_).c_str(), 0770);
                    }
                }
                fd = this->scheduler->open((this->root_path + "/binary/" + path).c_str(), O_WRONLY | O_CREAT | O_EXCL, 0660);
                PUSH_DATA(binary);
            }
            
            void Datamanager::binary_remove(const std::string &path)
            {
                this->scheduler->remove((this->root_path + "/binary/" + path).c_str());
            }

            bool Datamanager::get_is_init() const
            {
                return this->is_init;
            }

            const std::string &Datamanager::get_root_path() const
            {
                return this->root_path;
            }

            void Datamanager::task_bind(uint64_t taskid, std::pair<std::string, std::string> engine, const std::vector<Engine> &deps)
            {
                this->task_engine.emplace(taskid, std::make_pair(engine, deps));
            }

            void Datamanager::task_unbind(uint64_t taskid)
            {
                this->task_engine.erase(taskid);
            }

            bool Datamanager::task_exist(uint64_t taskid)
            {
                return this->task_engine.find(taskid) != this->task_engine.end();
            }

            std::pair<std::pair<std::string, std::string>, std::vector<Engine>> &Datamanager::get_task_engine(uint64_t taskid)
            {
                return this->task_engine.find(taskid)->second;
            }

            void Datamanager::add_extfiles(uint64_t taskid, const std::vector<Extfile> &extfiles)
            {
                this->task_extfiles.emplace(taskid, extfiles);
            }

            void Datamanager::remove_extfiles(uint64_t taskid)
            {
                this->task_extfiles.erase(taskid);
            }

            bool Datamanager::extfiles_exist(uint64_t taskid)
            {
                return this->task_extfiles.find(taskid) != this->task_extfiles.end();
            }

            std::vector<Extfile> &Datamanager::get_task_extfiles(uint64_t taskid)
            {
                return this->task_extfiles.find(taskid)->second;
            }
        }
    }
}
