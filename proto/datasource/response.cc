#include "message.h"
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <misc/protocol/protocol.h>

namespace jxtd::proto::datasource{

    Response::Response(): buf_(nullptr),size_(0){}

    Response::~Response(){
        if(this->buf_ != nullptr)
            delete[] this->buf_;
        this->buf_ = nullptr;
    }

    Response::Response(const Response &other){
        this->resp_.CopyFrom(other.resp_);
        if(this->buf_ != nullptr)
            delete[] this->buf_;
        this->buf_ = new char[other.size_];
        memcpy(this->buf_, other.buf_, other.size_);
        this->size_ = other.size_;
    }

    Response::Response(Response &&other) noexcept{
        this->resp_.Swap(&(other.resp_));
        this->buf_ = other.buf_;
        other.buf_ = nullptr;
        this->size_ = other.size_;
        other.size_ = 0;
    }

    Response &Response::operator=(const Response &other){
        if(&other == this)
            return *this;
        this->resp_.CopyFrom(other.resp_);
        if(this->buf_ != nullptr)
            delete[] this->buf_;
        this->buf_ = new char[other.size_];
        memcpy(this->buf_, other.buf_, other.size_);
        this->size_ = other.size_;
        return *this;
    }

    Response &Response::operator=(Response &&other) noexcept{
         if(&other == this)
            return *this;
        this->resp_.Swap(&(other.resp_));
        this->buf_ = other.buf_;
        other.buf_ = nullptr;
        this->size_ = other.size_;
        other.size_ = 0;
        return *this;
    }

    int Response::encode(struct iovec vectors[] , int max){
        this->resp_.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATASOURCE>());
        this->size_ = this->resp_.ByteSizeLong();
        this->buf_ = new char[this->size_];
        this->resp_.SerializeToArray(this->buf_, this->size_);
        vectors[0].iov_base = this->buf_;
        vectors[0].iov_len = this->size_;
        return 1;
    }

    int Response::append(const void *buf , size_t *size){
        if(!this->buf_){
            this->buf_ = new char[*size];
            memmove(this->buf_, buf, *size);
        }
        else{
            auto temp = this->buf_;
            this->buf_ = new char[this->size_ + *size];
            memmove(this->buf_, temp, this->size_);
            memmove(this->buf_ + this->size_, buf, *size);
            delete[] temp;
        }

        this->size_ += *size;

        if(!this->resp_.ParseFromArray(this->buf_, this->size_))
            return 0;
        delete[] this->buf_;
        this->buf_ = nullptr;
        this->size_ = 0;
        if (this ->resp_.magic() != ::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATASOURCE>())
        {
            errno = MAGIC_MISMATCH;
            this->resp_.Clear();
            return -1;
        }
        return 1;
    }

    void Response::set_token(const std::string &token){
        this->resp_.set_token(token);
    }

    std::string Response::get_token() const{
        return this->resp_.token();
    }

    void Response::set_name(const std::string &name){
        this->resp_.set_name(name);
    }

    std::string Response::get_name() const{
        return this->resp_.name();
    }

    void Response::set_status(uint32_t status){
        this->resp_.set_status(status);
    }
    uint32_t Response::get_status() const{
        return this->resp_.status();
    }
}
