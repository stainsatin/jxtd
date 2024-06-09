#include "Fileoperations.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

namespace jxtd
{
    namespace storage
    {
        namespace fs
        {
            namespace local
            {
                static int open(const char *filename, int flags, mode_t mode)
                {
                    return ::open(filename, flags, mode);
                }

                static int close(int fd)
                {
                    struct stat st;
                    if(fstat(fd, &st) == -1)
                        return -1;
                    if (S_IFDIR & st.st_mode)
                        return -1;
                    return ::close(fd);
                }

                static ssize_t write(int fd, const void *buf, size_t count)
                {
                    return ::write(fd, buf, count);
                }

                static ssize_t read(int fd, void *buf, size_t count)
                {
                    return ::read(fd, buf, count);
                }

                static off_t seek(int fd, off_t offset, int whence)
                {
                    return ::lseek(fd, offset, whence);
                }

                static int remove(const char *filepath)
                {
                    return ::unlink(filepath);
                }

                static int opendir(const char *filename, int flags, mode_t mode)
                {
                    return ::open(filename, O_DIRECTORY | flags, mode);
                }

                static int mkdir(const char *path, mode_t mode)
                {
                    return ::mkdir(path, mode);
                }

                static int closedir(int fd)
                {
                    struct stat st;
                    if(fstat(fd, &st) == -1)
                        return -1;
                    if (S_IFDIR & st.st_mode)
                        return ::close(fd);
                    return -1;
                }

                static int readdir(const char *filepath)
                {
                    DIR* dir = ::opendir(filepath);
                    if (dir == nullptr)
                        return -1;
                    // 遍历目录
                    struct dirent* entry;
                    while ((entry = ::readdir(dir)) != nullptr) {
                        std::cout << entry->d_name << std::endl;
                    }
                    ::closedir(dir);
                    return 0;
                }

                static int rmdir(const char *filepath)
                {
                    auto dir = ::opendir(filepath);
                    if(!dir)
                        return -1;
                    auto entry = ::readdir(dir);
                    int flag = 0;
                    std::string next_path = filepath;
                    while(entry){
                        std::string tmp_path = next_path;
                        tmp_path += "/";
                        tmp_path += entry -> d_name;
                        if(!strcmp(entry -> d_name, ".")){
                            entry = ::readdir(dir);
                            continue;
                        }
                        if(!strcmp(entry -> d_name, "..")){
                            entry = ::readdir(dir);
                            continue;
                        }
                        if(entry -> d_type == DT_DIR)
                            flag &= rmdir(tmp_path.c_str());
                        else
                            flag &= ::unlink(tmp_path.c_str());
                        entry = ::readdir(dir);
                    }
                    flag &= ::rmdir(filepath);
                    ::closedir(dir);
                    return flag;
                }

                static off_t fsize(int fd)
                {
                    struct stat st;
                    if(fstat(fd, &st) == -1)
                        return -1;
                    return st.st_size;
                }

                static int ftrun(int fd, off_t length)
                {
                    return ::ftruncate(fd, length);
                }

                static int fmod(int fd)
                {
                    struct stat st;
                    if(fstat(fd, &st) == -1)
                        return -1;
                    return st.st_mode;
                }

                static int fchmod(int fd, mode_t mode)
                {
                    return ::fchmod(fd, mode);
                }
                
                jxtd::storage::fs::scheduler::Operations Get_local_fileoperations()
                {
                    return {
                        nullptr, open, close, write, read, seek,
                        remove, opendir, mkdir, closedir, readdir, rmdir,
                        fsize, ftrun, fmod, fchmod, nullptr};
                }
            }
        }
    }
}
