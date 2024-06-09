#include "task.h"
#include <gtest/gtest.h>

using Task = ::jxtd::management::inferprovider::Task;
using TaskManager = ::jxtd::management::inferprovider::Task_Manager;
using Method = ::jxtd::management::inferprovider::Trigger_method;

TEST(testtask, test_task)
{
    TaskManager manager;
    Task *task = manager.create_task(1);
    task->set_output("1");
    task->add_dep(2);
    task->add_engine("123");
    EXPECT_EQ(task->get_engine_current(), "");
    task->set_run_count(9);
    task->set_trigger(Method::Dependency);
    task->switch_engine("123");
    EXPECT_EQ(task->get_taskid(), 1);
    EXPECT_EQ(task->get_deps().front(), 2);
    EXPECT_EQ(task->get_run_count(), 9);
    EXPECT_EQ(task->get_output(), "1");
    EXPECT_EQ(task->get_trigger(), Method::Dependency);
    EXPECT_EQ(task->get_engine_current(), "123");
    EXPECT_EQ(task->get_status(), false);
    task->start();
    EXPECT_EQ(task->get_status(), true);
    task->delete_engine("123");
    EXPECT_EQ(task->get_engine_current(), "");
    manager.add_task(task);
    EXPECT_EQ(manager.get_task(1), task);
    manager.delete_task(1);
    EXPECT_EQ(manager.get_tasks().empty(), true);
}