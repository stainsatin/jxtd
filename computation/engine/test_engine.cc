#include "Engine.h"
#include <gtest/gtest.h>

using jxtd_Engine = ::jxtd::computation::engine::jxtd_Engine;
using Tensor = ::jxtd::computation::engine::Tensor;
class A: public jxtd_Engine
{
public:
    ~A() = default;

private:
    virtual int init()
    {
        return 1;
    }

    virtual int infer(const Tensor *, Tensor *)
    {
        return 2;
    }

    virtual int post()
    {
        return 3;
    }
};

static void init(jxtd_Engine *engine)
{
    engine->set_ctx(nullptr);
    engine->set_model("hollowknight");
    engine->set_type(jxtd_Engine::ENGINE_COMPUTE);
    engine->set_parent("aaa", "bbb");
}

GENERATOR_BINDPOINT(A, init, hollow, knight);
TEST(testengine, test_engine)
{
    auto a = generate_hollow_knight();
    EXPECT_EQ(a->get_ctx(), nullptr);
    EXPECT_EQ(a->get_id(), "hollow");
    EXPECT_EQ(a->get_model(), "");
    EXPECT_EQ(a->get_name(), "knight");
    EXPECT_EQ(a->get_type(), 1);
    EXPECT_EQ(a->get_parent().first, "aaa");
    EXPECT_EQ(a->get_parent().second, "bbb");
    EXPECT_EQ(a->check(), true);
    // EXPECT_EQ(a->init_env(), 1);
    // EXPECT_EQ(a->compute(nullptr, nullptr), 2);
    // EXPECT_EQ(a->post_env(), 3);
    delete a;
}