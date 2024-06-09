#pragma once
#include <string>
#include <cstdint>
#include "computation/infer/Tensor.h"

namespace jxtd
{
    namespace computation
    {
        namespace engine
        {
            using Tensor = ::jxtd::computation::infer::Tensor;
            
            class jxtd_Engine
            {
            public:
                enum
                {
                    ENGINE_INFER = 0,
                    ENGINE_COMPUTE,
                    ENGINE_BOOTSTRAP,
                    ENGINE_DEFAULT,
                    ENGINE_OTHER
                };

                jxtd_Engine();
                virtual ~jxtd_Engine() = default;
                int set_id(const std::string &mid);
                std::string get_id() const;
                int set_name(const std::string &name);
                std::string get_name() const;
                int set_type(uint32_t type);
                uint32_t get_type() const;
                int set_parent(const std::string &mid, const std::string &name);
                std::pair<std::string, std::string> get_parent();
                void set_path(const std::string &dlpath);
                const std::string &get_path() const;

                int init_env();
                int compute(const Tensor *input, Tensor *output);
                int post_env();

                int set_model(const std::string &path);
                std::string get_model() const;
                int set_ctx(void *ctx);
                void *get_ctx();
                bool check() const;

            private:
                virtual int init();
                virtual int infer(const Tensor *input, Tensor *output);
                virtual int post();

                std::string mid;
                std::string name;
                std::string path;
                std::string parent_mid;
                std::string parent_name;
                std::string dlpath;
                uint32_t type;
                void *ctx;
                void *handle;
                jxtd_Engine *parent;
            };

            #define GENERATOR_BINDPOINT(Engine_type, init_func, mid, name) \
            extern "C"                                                 \
            {                                                          \
                jxtd_Engine *generate_##mid##_##name()                 \
                {                                                      \
                    jxtd_Engine *engine_new = new Engine_type;         \
                    engine_new->set_id(#mid);                          \
                    engine_new->set_name(#name);                       \
                    init_func(engine_new);                             \
                    if(!engine_new->check())                           \
                    {                                                  \
                        delete engine_new;                             \
                        engine_new = nullptr;                          \
                        return nullptr;                                \
                    }                                                  \
                    return engine_new;                                 \
                }                                                      \
            }
        }
    }
}
