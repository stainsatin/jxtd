#include "computation/engine/Engine.h"
#include <fstream>
#include <functional>
#include <dlfcn.h>

namespace jxtd
{
    namespace computation
    {
        namespace engine
        {
            jxtd_Engine::jxtd_Engine() : type(5), ctx(nullptr), handle(nullptr), parent(nullptr), dlpath("./") { }

            int jxtd_Engine::set_id(const std::string &mid)
            {
                if (mid.empty())
                    return -1;
                this->mid = mid;
                return 0;
            }

            std::string jxtd_Engine::get_id() const
            {
                return this->mid;
            }

            int jxtd_Engine::set_name(const std::string &name)
            {
                if (name.empty())
                    return -1;
                this->name = name;
                return 0;
            }

            std::string jxtd_Engine::get_name() const
            {
                return this->name;
            }

            int jxtd_Engine::set_type(uint32_t type)
            {
                if (type > 4)
                    return -1;
                this->type = type;
                return 0;
            }

            uint32_t jxtd_Engine::get_type() const
            {
                return this->type;
            }

            int jxtd_Engine::set_parent(const std::string &mid, const std::string &name)
            {
                if (mid.empty() || name.empty())
                    return -1;
                this->parent_mid = mid;
                this->parent_name = name;
                return 0;
            }

            std::pair<std::string, std::string> jxtd_Engine::get_parent()
            {
                return std::make_pair(this->parent_mid, this->parent_name);
            }

            void jxtd_Engine::set_path(const std::string &dlpath)
            {
                this->dlpath = dlpath;
            }

            const std::string &jxtd_Engine::get_path() const
            {
                return this->dlpath;
            }

            int jxtd_Engine::init_env()
            {
                if (this->parent_mid.empty() || this->parent_name.empty())
                    return this->init();
                this->handle = dlopen((this->dlpath + "JXTDENGINE_" + parent_mid + "_" + parent_name + ".so").c_str(), RTLD_LAZY);
                if (this->handle == nullptr)
                {
                    return -1;
                }
                typedef jxtd_Engine *(*generate)();
                generate generate_parent = reinterpret_cast<generate>(dlsym(handle, ("generate_" + parent_mid + "_" + parent_name).c_str()));
                if (generate_parent == nullptr)
                {
                    dlclose(this->handle);
                    this->handle = nullptr;
                    return -1;
                }
                this->parent = generate_parent();
                if (this->parent == nullptr || this->parent->init_env() == -1)
                {
                    delete this->parent;
                    this->parent = nullptr;
                    dlclose(this->handle);
                    this->handle = nullptr;
                    return -1;
                }
                return this->init();
            }

            int jxtd_Engine::compute(const Tensor *input, Tensor *output)
            {
                if (this->handle == nullptr || this->parent == nullptr)
                    return this->infer(input, output);
                if (this->parent->compute(input, output) == -1)
                {
                    delete this->parent;
                    this->parent = nullptr;
                    dlclose(handle);
                    this->handle = nullptr;
                    return -1;
                }
                return this->infer(input, output);
            }

            int jxtd_Engine::post_env()
            {
                if (this->handle == nullptr || this->parent == nullptr)
                    return this->post();
                if (this->parent->post_env() == -1)
                {
                    delete this->parent;
                    this->parent = nullptr;
                    dlclose(handle);
                    this->handle = nullptr;
                    return -1;
                }
                delete this->parent;
                this->parent = nullptr;
                if (dlclose(this->handle) == -1)
                {
                    this->handle = nullptr;
                    return -1;
                }
                this->handle = nullptr;
                return this->post();
            }

            int jxtd_Engine::set_model(const std::string &path)
            {
                if (path.empty())
                    return -1;
                this->path = path;
                return 0;
            }

            std::string jxtd_Engine::get_model() const
            {
                if (path.empty())
                    return "";
                std::ifstream filein;
                filein.open("./" + path, std::ios::binary | std::ios::in);
                if (!filein.is_open())
                    return "";
                filein.seekg(0, std::ios::end);
                auto fileSize = filein.tellg();
                filein.seekg(0, std::ios::beg);
                std::string content(fileSize, '\0');
                filein.read(&content[0], fileSize);
                filein.close();
                return content;
            }

            int jxtd_Engine::set_ctx(void *ctx)
            {
                if (ctx == nullptr)
                    return -1;
                this->ctx = ctx;
                return 0;
            }

            void *jxtd_Engine::get_ctx()
            {
                return this->ctx;
            }

            bool jxtd_Engine::check() const
            {
                return !(this->mid.empty() || this->name.empty() || this->path.empty() || this->type > 4);
            }

            int jxtd_Engine::init()
            {
                return 0;
            }

            int jxtd_Engine::infer(const Tensor *input, Tensor *output)
            {
                return 0;
            }

            int jxtd_Engine::post()
            {
                return 0;
            }
        }
    }
}