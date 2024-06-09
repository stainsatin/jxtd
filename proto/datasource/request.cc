#include "message.h"
#include <cerrno>
#include <cstring>
#include <map>
#include "misc/protocol/protocol.h"

namespace jxtd::proto::datasource
{
    Request::Request():buf_(nullptr), size_(0){}

    Request::~Request(){
        if (this->buf_)
            delete[] this->buf_;
        this->buf_ = nullptr;
    }

    Request::Request(const Request &other){
        this->req_.CopyFrom(other.req_);
        if(this->buf_ != nullptr)
            delete[] this->buf_;
        this->buf_ = new char[other.size_];
        memcpy(this->buf_, other.buf_, other.size_);
        this->size_ = other.size_;
    }

    Request::Request(Request &&other) noexcept{
        this->req_.Swap(&(other.req_));
        this->buf_ = other.buf_;
        other.buf_ = nullptr;
        this->size_ = other.size_;
        other.size_ = 0;
    }

    Request &Request::operator=(const Request &other){
        if(&other == this)
            return *this;
        this->req_.CopyFrom(other.req_);
        if(this->buf_ != nullptr)
            delete[] this->buf_;
        this->buf_ = new char[other.size_];
        memcpy(this->buf_, other.buf_, other.size_);
        this->size_ = other.size_;
        return *this;
   }

    Request &Request::operator=(Request &&other) noexcept{
        if(&other == this)
            return *this;
        this->req_.Swap(&(other.req_));
        this->buf_ = other.buf_;
        other.buf_ = nullptr;
        this->size_ = other.size_;
        other.size_ = 0;
        return *this;
    }

    int Request::encode(struct iovec vectors[], int max){
        this->req_.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATASOURCE>());
        this->size_ = this->req_.ByteSizeLong();
        this->buf_ = new char[this->size_];
        this->req_.SerializeToArray(this->buf_, this->size_);
        vectors[0].iov_base = this->buf_;
        vectors[0].iov_len = this->size_;
        return 1;
    }

    int Request::append(const void *buf, size_t *size){
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

        if(!this->req_.ParseFromArray(this->buf_, this->size_))
            return 0;
        delete[] this->buf_;
        this->buf_ = nullptr;
        this->size_ = 0;
        if(this->req_.magic() != ::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATASOURCE>()){
            errno = MAGIC_MISMATCH;
            this->req_.Clear();
            return -1;
        }
        return 1;
    }

    void Request::set_token(const std::string &token){
        this->req_.set_token(token);
    }

    std::string Request::get_token() const{
        return this->req_.token();
    }

    void Request::set_name(const std::string &name){
         this->req_.set_name(name);
    }

    std::string Request::get_name() const{
        return this->req_.name();
    }

    void Request::set_action(uint32_t action){
        this->req_.set_action(action);
    }

    uint32_t Request::get_action() const{
        return this->req_.action();
    }

    void Request::set_proto(uint32_t proto){
        this->req_.set_proto(proto);
    }

    uint32_t Request::get_proto() const{
        return this->req_.proto();
    }

    void Request::set_url(const std::string &url){
        this->req_.set_url(url);
    }

    std::string Request::get_url() const{
        return this->req_.url();
    }

    void Request::set_port(uint32_t port){
        this->req_.set_port(port);
    }

    uint32_t Request::get_port() const{
        return this->req_.port();
    }

    std::map<std::string, std::string> Request::get_allmetadata() const{
        return this->myMetadata;
    }

    std::string Request::get_metadata(const std::string &key) const{
        auto it = this->myMetadata.find(key);
        if(it != this->myMetadata.end()){
            return it->second;
        }
            return "";
    }

    void Request::set_metadata(const std::map<std::string, std::string> &metadata){
        for (const auto& entry : metadata) {
            metaentry* meta_entry = this->req_.add_metadata();
            meta_entry->set_key(entry.first);
            meta_entry->set_value(entry.second);
            this->myMetadata[entry.first] = entry.second;
        }
    }

    void Request::add_metadata(const std::string &key,const std::string &value){
            metaentry* meta_entry = this->req_.add_metadata();
            meta_entry->set_key(key);
            meta_entry->set_value(value);
            this->myMetadata[key] = value;
    }
}
