/*
    test for scheduler
*/
#include <gtest/gtest.h>
#include "string.h"
#include "storage/fs/scheduler/Scheduler.h"
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <functional>

struct File {
    std::string name;
    bool isDirectory;
    std::vector<char> data;

    File() = default;
    File(const std::string& name_, bool isDirectory_) : name(name_), isDirectory(isDirectory_) {}
};

class FileSystem {
private:
    std::unordered_map<std::string, File> files;

public:
    int init(void* ctx) {
        // 初始化文件系统
        files.clear();
        return 0;
    }

    int open(const char* filename, int flags, mode_t mode) {
        // 打开文件
        std::string name(filename);
        if (files.find(name) != files.end()) {
            // 文件已存在
            return -1;
        }

        File file(name, false);
        files[name] = file;
        return 0;
    }

    int close(int fd) {
        // 关闭文件
        // 这里不需要实现具体的逻辑，因为文件系统是在内存中的
        return 0;
    }

    ssize_t write(int fd, const void* buf, size_t count) {
        // 写入文件
        std::string name = getFilename(fd);
        if (name.empty()) {
            return -1;
        }

        if (files[name].isDirectory) {
            // 不能写入目录
            return -1;
        }

        const char* data = static_cast<const char*>(buf);
        files[name].data.insert(files[name].data.end(), data, data + count);
        return count;
    }

    ssize_t read(int fd, void* buf, size_t count) {
        // 读取文件
        std::string name = getFilename(fd);
        if (name.empty()) {
            return -1;
        }

        if (files[name].isDirectory) {
            // 不能读取目录
            return -1;
        }

        char* data = static_cast<char*>(buf);
        size_t size = std::min(count, files[name].data.size());
        std::copy(files[name].data.begin(), files[name].data.begin() + size, data);
        return size;
    }

    off_t seek(int fd, off_t offset, int whence) {
        // 设置文件指针位置
        // 这里不需要实现具体的逻辑，因为文件系统是在内存中的
        return 0;
    }

    int remove(const char* filepath) {
        // 删除文件或目录
        std::string path(filepath);
        if (files.find(path) == files.end()) {
            // 文件或目录不存在
            return -1;
        }

        if (files[path].isDirectory) {
            // 删除目录
            for (auto it = files.begin(); it != files.end(); ) {
                if (it->first.find(path + "/") == 0) {
                    // 递归删除子文件或子目录
                    it = files.erase(it);
                } else {
                    ++it;
                }
            }
        }

        files.erase(path);
        return 0;
    }

    int opendir(const char* filename, int flags, mode_t mode) {
        // 打开目录
        std::string name(filename);
        if (files.find(name) != files.end()) {
            // 目录已存在
            return false;
        }

        File file(name, true);
        files[name] = file;
        return true;
    }

    int closedir(int fd) {
        // 关闭目录
        // 这里不需要实现具体的逻辑，因为文件系统是在内存中的
        return 0;
    }

    int readdir(const char* filepath) {
        // 读取目录
        std::string path(filepath);
        if (files.find(path) == files.end() || !files[path].isDirectory) {
            // 目录不存在或不是目录
            return -1;
        }

        for (const auto& entry : files) {
            if (entry.first.find(path + "/") == 0) {
                // 输出子文件或子目录的名称
                std::cout << entry.first << std::endl;
            }
        }

        return 0;
    }

    int rmdir(const char* filepath) {
        // 删除目录
        std::string path(filepath);
        if (files.find(path) == files.end() || !files[path].isDirectory) {
            // 目录不存在或不是目录
            return -1;
        }

        for (auto it = files.begin(); it != files.end(); ) {
            if (it->first.find(path + "/") == 0) {
                // 递归删除子文件或子目录
                it = files.erase(it);
            } else {
                ++it;
            }
        }

        files.erase(path);
        return 0;
    }

    off_t fsize(int fd) {
        // 获取文件大小
        std::string name = getFilename(fd);
        if (name.empty()) {
            return -1;
        }

        if (files[name].isDirectory) {
            // 目录没有大小
            return -1;
        }

        return files[name].data.size();
    }

    int ftrun(int fd, off_t offset) {
        // 截断文件
        std::string name = getFilename(fd);
        if (name.empty()) {
            return -1;
        }

        if (files[name].isDirectory) {
            // 目录不能截断
            return -1;
        }

        files[name].data.resize(offset);
        return 0;
    }

    int fmod(int fd) {
        // 修改文件
        std::string name = getFilename(fd);
        if (name.empty()) {
            return -1;
        }

        if (files[name].isDirectory) {
            // 目录不能修改
            return -1;
        }

        // 这里可以实现具体的修改文件的逻辑
        return 0;
    }

    int fchmod(int fd, mode_t mode) {
        // 修改文件权限
        std::string name = getFilename(fd);
        if (name.empty()) {
            return -1;
        }

        if (files[name].isDirectory) {
            // 目录权限不能修改
            return -1;
        }

        // 这里可以实现具体的修改文件权限的逻辑
        return 0;
    }

    void destroy() {
        // 销毁文件系统
        files.clear();
    }

private:
    std::string getFilename(int fd) {
        // 根据文件描述符获取文件名
        // 这里简单地将文件描述符转换为字符串作为文件名
        return std::to_string(fd);
    }
};

using Scheduler = ::jxtd::storage::fs::scheduler::Scheduler;
using Operations = ::jxtd::storage::fs::scheduler::Operations;
using Policy = ::jxtd::storage::fs::scheduler::SchedulingPolicy;
using Range = ::jxtd::storage::fs::scheduler::ServiceRange;
TEST(testfs, testscheduler)
{
    FileSystem fs;

    // 设置文件系统的API
    std::function<int(void*)> init = [&](void* ctx) { return fs.init(ctx); };
    std::function<int(const char*, int, mode_t)> open = [&](const char* filename, int flags, mode_t mode) { return fs.open(filename, flags, mode); };
    std::function<int(int)> close = [&](int fd) { return fs.close(fd); };
    std::function<ssize_t(int, const void*, size_t)> write = [&](int fd, const void* buf, size_t count) { return fs.write(fd, buf, count); };
    std::function<ssize_t(int, void*, size_t)> read = [&](int fd, void* buf, size_t count) { return fs.read(fd, buf, count); };
    std::function<off_t(int, off_t, int)> seek = [&](int fd, off_t offset, int whence) { return fs.seek(fd, offset, whence); };
    std::function<int(const char*)> remove = [&](const char* filepath) { return fs.remove(filepath); };
    std::function<bool(const char*, int, mode_t)> opendir = [&](const char* filename, int flags, mode_t mode) { return fs.opendir(filename, flags, mode); };
    std::function<int(int)> closedir = [&](int fd) { return fs.closedir(fd); };
    std::function<int(const char*)> readdir = [&](const char* filepath) { return fs.readdir(filepath); };
    std::function<int(const char*)> rmdir = [&](const char* filepath) { return fs.rmdir(filepath); };
    std::function<off_t(int)> fsize = [&](int fd) { return fs.fsize(fd); };
    std::function<int(int, off_t)> ftrun = [&](int fd, off_t offset) { return fs.ftrun(fd, offset); };
    std::function<int(int)> fmod = [&](int fd) { return fs.fmod(fd); };
    std::function<int(int, mode_t)> fchmod = [&](int fd, mode_t mode) { return fs.fchmod(fd, mode); };
    std::function<void()> destroy = [&]() { fs.destroy(); };

    Scheduler scheduler;
    Operations ops = {init, open, close, write, read, seek, remove, opendir, nullptr, closedir, readdir, rmdir, fsize, ftrun, fmod, fchmod, destroy}; 
    EXPECT_EQ(scheduler.register_service(1, ops, "a"), 0);
    EXPECT_EQ(scheduler.register_service(1, ops, "a"), -1);
    EXPECT_EQ(scheduler.register_service(2, ops, "a"), 0);
    ops.seek = nullptr;
    EXPECT_EQ(scheduler.register_service(3, ops, "b"), 0);
    scheduler.enable_service({2, 3});
    EXPECT_EQ(scheduler.find_service(2)->get_status(), true);
    EXPECT_EQ(scheduler.find_service(1)->get_status(), false);
    EXPECT_EQ(scheduler.find_service(3)->get_status(), true);
    scheduler.strategy_choose(Policy::Single, 5);
    EXPECT_EQ(scheduler.get_current_service(), nullptr);
    scheduler.strategy_choose(Policy::LoadBalance, -1, Range::Specified, "a");
    EXPECT_EQ(scheduler.get_current_service()->get_scope(), "a");
    EXPECT_EQ(scheduler.open("1", 0, 0), 0);
    char buf[100] = {"12345678"};
    EXPECT_EQ(scheduler.write(1, buf, strlen(buf) + 1), strlen(buf) + 1);
    char buf_[100];
    EXPECT_EQ(scheduler.read(1, buf_, strlen(buf) + 1), strlen(buf) + 1);
    EXPECT_EQ(scheduler.close(1), 0);
    EXPECT_EQ(strcmp(buf_, buf), 0);
    EXPECT_EQ(scheduler.seek(1, 1, 1), 0);
    EXPECT_EQ(scheduler.opendir("2", 1 ,1), 1);
    EXPECT_EQ(scheduler.closedir(2), 0);
    EXPECT_EQ(scheduler.readdir("2"), 0);
    EXPECT_EQ(scheduler.fsize(2), -1);
    EXPECT_EQ(scheduler.fsize(1), 9);
    EXPECT_EQ(scheduler.ftrun(1, 6), 0);
    EXPECT_EQ(scheduler.fsize(1), 6);
    EXPECT_EQ(scheduler.fmod(1), 0);
    EXPECT_EQ(scheduler.fchmod(1, 2), 0);
    EXPECT_EQ(scheduler.rmdir("1"), -1);
    EXPECT_EQ(scheduler.rmdir("2"), 0);
    EXPECT_EQ(scheduler.remove("1"), 0);
    EXPECT_EQ(scheduler.remove("3"), -1);
    scheduler.disable_service({2, 3});
    scheduler.enable_service({3});
    scheduler.strategy_choose(Policy::Single, 4);
    EXPECT_EQ(scheduler.get_current_service(), nullptr);
    scheduler.strategy_choose(Policy::LoadBalance, -1, Range::All);
    EXPECT_EQ(scheduler.seek(1, 1, 1), -1);
    EXPECT_EQ(scheduler.get_current_service()->get_scope(), "b");
    scheduler.unload_service({2, 3});
}