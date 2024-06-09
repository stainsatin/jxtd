#include "message.h"
#include <cerrno>
#include <cstring>
#include "ProtocolMessage.pb.h"
#include "misc/protocol/protocol.h"

namespace jxtd
{
    namespace proto
    {
        namespace datatransfer
        {
            using request = ::jxtd::proto::datatransfer::request;
            using response = ::jxtd::proto::datatransfer::response;
            
            enum class Message_Errno : int
            {
                MAGIC_MISMATCH = 257,
            };

            void assignadtsrc(const Adtsrc &src, adtsrc *dest)
            {
                if(src.taskid != 0x0)
                    dest->set_taskid(src.taskid);
                dest->set_type(src.type);
                if(!src.endpointattr.first.empty() && !src.endpointattr.second.empty())
                {
                    auto endpoint = dest->mutable_endpoint();
                    endpoint->set_proto(src.endpointattr.first);
                    endpoint->set_url(src.endpointattr.second);
                }
                if(!src.engineattr.first.empty() && !src.engineattr.second.empty())
                {
                    auto engine = dest->mutable_engine();
                    engine->set_mid(src.engineattr.first);
                    engine->set_name(src.engineattr.second);
                }
                if(!src.binary.empty())
                    dest->set_binary(src.binary);
            }

            void assignAdtsrc(const adtsrc &src, Adtsrc *dest)
            {
                dest->taskid = src.taskid();
                dest->type = src.type();
                auto &endpointattr = dest->endpointattr;
                endpointattr.first = src.endpoint().proto();
                endpointattr.second = src.endpoint().url();
                auto &engineattr = dest->engineattr;
                engineattr.first = src.engine().mid();
                engineattr.second = src.engine().name();
                dest->binary = src.binary();
            }

            Request::Request()
            {
                this->buf_ = nullptr;
                this->size_ = 0;
            }

            Request::~Request()
            {
                if (this->buf_)
                    delete[] this->buf_;
                this->buf_ = nullptr;
            }

            Request::Request(const Request &req):
                src(req.src),
                dest(req.dest),
                direction(req.direction),
                token(req.token),
                adtsrcs(req.adtsrcs)
            {
                if (this->buf_ != nullptr)
                    delete[] this->buf_;
                this->buf_ = new char[req.size_];
                memcpy(this->buf_, req.buf_, req.size_);
                this->size_ = req.size_;
            }

            Request::Request(Request &&req) noexcept:
                src(std::move(req.src)),
                dest(std::move(req.dest)),
                direction(req.direction),
                token(std::move(req.token)),
                adtsrcs(std::move(req.adtsrcs))
            {
                this->buf_ = req.buf_;
                req.buf_ = nullptr;
                this->size_ = req.size_;
                req.size_ = 0;
            }

            Request &Request::operator=(const Request &req)
            {
                if (&req == this)
                    return *this;
                this->src = req.src;
                this->dest = req.dest;
                this->direction = req.direction;
                this->token = req.token;
                this->adtsrcs = req.adtsrcs;
                if (this->buf_ != nullptr)
                    delete[] this->buf_;
                this->buf_ = new char[req.size_];
                memcpy(this->buf_, req.buf_, req.size_);
                this->size_ = req.size_;
                return *this;
            }

            Request &Request::operator=(Request &&req) noexcept
            {
                if (&req == this)
                    return *this;
                this->src = std::move(req.src);
                this->dest = std::move(req.dest);
                this->direction = req.direction;
                this->token = std::move(req.token);
                this->adtsrcs = std::move(req.adtsrcs);
                this->buf_ = req.buf_;
                req.buf_ = nullptr;
                this->size_ = req.size_;
                req.size_ = 0;
                return *this;
            }

            int Request::encode(struct iovec vectors[], int max)
            {
                request req_;
                req_.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATATRANSFER>());
                req_.set_src(this->src);
                req_.set_dest(this->dest);
                req_.set_direction(this->direction);
                req_.set_token(this->token);
                for(const auto &adtsrc_ : adtsrcs)
                {
                    auto temp = req_.add_master();
                    assignadtsrc(adtsrc_, temp);
                }
                this->size_ = req_.ByteSizeLong();
                if(this->buf_ != nullptr)
                    delete this->buf_;
                this->buf_ = new char[this->size_];
                req_.SerializeToArray(this->buf_, this->size_);
                vectors[0].iov_base = this->buf_;
                vectors[0].iov_len = this->size_;
                return 1;
            }

            int Request::append(const void *buf, size_t *size)
            {
                if (!this->buf_)
                {
                    this->buf_ = new char[*size];
                    memmove(this->buf_, buf, *size);
                }
                else
                {
                    auto temp = this->buf_;
                    this->buf_ = new char[this->size_ + *size];
                    memmove(this->buf_, temp, this->size_);
                    memmove(this->buf_ + this->size_, buf, *size);
                    delete[] temp;
                }
                this->size_ += *size;

                request req_;
                if (!req_.ParseFromArray(this->buf_, this->size_))
                    return 0;
                delete[] this->buf_;
                this->buf_ = nullptr;
                this->size_ = 0;
                if (req_.magic() != ::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATATRANSFER>())
                {
                    errno = static_cast<int>(Message_Errno::MAGIC_MISMATCH);
                    return -1;
                }
                this->src = req_.src();
                this->dest = req_.dest();
                this->direction = req_.direction();
                this->token = req_.token();
                this->adtsrcs.clear();
                for(int i = 0; i < req_.master().size(); i++)
                {
                    this->adtsrcs.push_back(Adtsrc());
                    assignAdtsrc(req_.master(i) ,&this->adtsrcs.back());
                }
                return 1;
            }

            void Request::set_src(const std::string &src_)
            {
                this->src = src_;
            }

            const std::string &Request::get_src() const
            {
                return this->src;
            }

            int Request::set_direction(uint32_t direction_)
            {
                if(direction_ == 0x0 || direction_ == 0x1)
                {
                    this->direction = direction_;
                    return 0;
                }
                else
                    return -1;
            }

            uint32_t Request::get_direction() const
            {
                return this->direction;
            }

            void Request::set_token(const std::string &token_)
            {
                this->token = token_;
            }

            const std::string &Request::get_token() const
            {
                return this->token;
            }

            void Request::set_dest(const std::string &dest_)
            {
                this->dest = dest_;
            }

            const std::string &Request::get_dest() const
            {
                return this->dest;
            }

            void Request::add_adtsrc(const Adtsrc &adtsrc_)
            {
                this->adtsrcs.emplace_back(adtsrc_);
            }

            const std::vector<Adtsrc> &Request::get_adtsrcs() const
            {
                return this->adtsrcs;
            }

            const Adtsrc *Request::get_adtsrc(int pos) const
            {
                if (pos < 0 || pos >= this->adtsrcs.size())
                    return nullptr;
                return &this->adtsrcs.at(pos);
            }

            Response::Response()
            {
                this->buf_ = nullptr;
                this->size_ = 0;
            }

            Response::~Response()
            {
                if (this->buf_)
                    delete[] this->buf_;
                this->buf_ = nullptr;
            }

            Response::Response(const Response &resp):
                src(resp.src),
                dest(resp.dest),
                token(resp.token),
                dtsrcs(resp.dtsrcs)
            {
                if (this->buf_ != nullptr)
                    delete[] this->buf_;
                this->buf_ = new char[resp.size_];
                memcpy(this->buf_, resp.buf_, resp.size_);
                this->size_ = resp.size_;
            }

            Response::Response(Response &&resp) noexcept:
                src(std::move(resp.src)),
                dest(std::move(resp.dest)),
                token(std::move(resp.token)),
                dtsrcs(std::move(resp.dtsrcs))
            {
                this->buf_ = resp.buf_;
                resp.buf_ = nullptr;
                this->size_ = resp.size_;
                resp.size_ = 0;
            }

            Response &Response::operator=(const Response &resp)
            {
                if (&resp == this)
                    return *this;
                this->src = resp.src;
                this->dest = resp.dest;
                this->token = resp.token;
                this->dtsrcs = resp.dtsrcs;
                if (this->buf_ != nullptr)
                    delete[] this->buf_;
                this->buf_ = new char[resp.size_];
                memcpy(this->buf_, resp.buf_, resp.size_);
                this->size_ = resp.size_;
                return *this;
            }

            Response &Response::operator=(Response &&resp) noexcept
            {
                if (&resp == this)
                    return *this;
                this->src = std::move(resp.src);
                this->dest = std::move(resp.dest);
                this->token = std::move(resp.token);
                this->dtsrcs = std::move(resp.dtsrcs);
                this->buf_ = resp.buf_;
                resp.buf_ = nullptr;
                this->size_ = resp.size_;
                resp.size_ = 0;
                return *this;
            }

            int Response::encode(struct iovec vectors[], int max)
            {
                response resp_;
                resp_.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATATRANSFER>());
                resp_.set_src(this->src);
                resp_.set_dest(this->dest);
                resp_.set_token(this->token);
                for(const auto &dtsrc_ : dtsrcs)
                {
                    auto temp = resp_.add_master();
                    assignadtsrc(dtsrc_.master, temp->mutable_master());
                    for(const auto &dep : dtsrc_.deps)
                    {
                        auto tempdep = temp->add_deps();
                        assignadtsrc(dep.first, tempdep->mutable_endpoint());
                        if(!dep.second.empty())
                            tempdep->set_relative(dep.second);
                    }
                }
                this->size_ = resp_.ByteSizeLong();
                if(this->buf_ != nullptr)
                    delete this->buf_;
                this->buf_ = new char[this->size_];
                resp_.SerializeToArray(this->buf_, this->size_);
                vectors[0].iov_base = this->buf_;
                vectors[0].iov_len = this->size_;
                return 1;
            }

            int Response::append(const void *buf, size_t *size)
            {
                if (!this->buf_)
                {
                    this->buf_ = new char[*size];
                    memmove(this->buf_, buf, *size);
                }
                else
                {
                    auto temp = this->buf_;
                    this->buf_ = new char[this->size_ + *size];
                    memmove(this->buf_, temp, this->size_);
                    memmove(this->buf_ + this->size_, buf, *size);
                    delete[] temp;
                }
                this->size_ += *size;

                response resp_;
                if (!resp_.ParseFromArray(this->buf_, this->size_))
                    return 0;
                delete[] this->buf_;
                this->buf_ = nullptr;
                this->size_ = 0;
                if (resp_.magic() != ::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DATATRANSFER>())
                {
                    errno = static_cast<int>(Message_Errno::MAGIC_MISMATCH);
                    return -1;
                }
                this->src = resp_.src();
                this->dest = resp_.dest();
                this->token = resp_.token();
                this->dtsrcs.clear();
                for(int i = 0; i < resp_.master().size(); i++)
                {
                    this->dtsrcs.push_back(Dtsrc());
                    assignAdtsrc(resp_.master(i).master() ,&this->dtsrcs.back().master);
                    for(int j = 0; j < resp_.master(i).deps().size(); j++)
                    {
                        this->dtsrcs.back().deps.push_back({Adtsrc(), ""});
                        assignAdtsrc(resp_.master(i).deps(j).endpoint(), &this->dtsrcs.back().deps.back().first);
                        this->dtsrcs.back().deps.back().second = resp_.master(i).deps(j).relative();
                    }
                }
                return 1;
            }

            void Response::set_src(const std::string &src_)
            {
                this->src = src_;
            }

            const std::string &Response::get_src() const
            {
                return this->src;
            }

            void Response::set_token(const std::string &token_)
            {
                this->token = token_;
            }

            const std::string &Response::get_token() const
            {
                return this->token;
            }

            void Response::set_dest(const std::string &dest_)
            {
                this->dest = dest_;
            }

            const std::string &Response::get_dest() const
            {
                return this->dest;
            }

            void Response::add_dtsrc(const Dtsrc &dtsrc_)
            {
                this->dtsrcs.emplace_back(dtsrc_);
            }

            const std::vector<Dtsrc> &Response::get_dtsrcs() const
            {
                return this->dtsrcs;
            }

            const Dtsrc *Response::get_dtsrc(int pos) const
            {
                if(pos < 0 || pos >= this->dtsrcs.size())
                    return nullptr;
                return &this->dtsrcs.at(pos);
            }
        }
    }
}