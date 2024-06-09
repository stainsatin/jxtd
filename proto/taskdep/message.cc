#include "message.h"
#include <cerrno>
#include "string.h"
#include "taskdep_message.pb.h"
#include "misc/protocol/protocol.h"

namespace jxtd
{
    namespace proto
    {
        namespace taskdep
        {
            using ENDPOINT = ::jxtd::proto::taskdep::ENDPOINT;
            using REQUEST = ::jxtd::proto::taskdep::REQUEST;
            using RESPONSE = ::jxtd::proto::taskdep::RESPONSE;

            static void assignEndpoint(const Endpoint &src, ENDPOINT *dest)
            {
                dest->set_proto(src.get_proto());
                dest->set_url(src.get_url());
                dest->set_port(src.get_port());
                dest->set_path(src.get_path());
                auto dest_attr = dest->mutable_attr();
                dest_attr->set_authtype(src.get_authtype());
                auto authstring = src.get_authstring();
                for (const auto &authstring_ : authstring)
                {
                    auto temp = dest_attr->add_authstring();
                    temp->set_token(authstring_.token);
                    temp->mutable_uppair()->set_name(authstring_.uppair.first);
                    temp->mutable_uppair()->set_key(authstring_.uppair.second);
                    temp->mutable_pki()->set_prvkey(authstring_.pki.first);
                    temp->mutable_pki()->set_pubkey(authstring_.pki.second);
                }
            }

            static void assignENDPOINT(const ENDPOINT &src, Endpoint *dest)
            {
                dest->set_proto(src.proto());
                dest->set_url(src.url());
                dest->set_port(src.port());
                dest->set_path(src.path());
                dest->set_authtype(src.attr().authtype());
                const auto &attr = src.attr();
                // 是否需要添加一个clear authstring的过程
                for (int i = 0; i < attr.authstring().size(); i++)
                {
                    const auto &authstring = attr.authstring(i);
                    dest->add_authstring(authstring.token(), authstring.uppair().name(), authstring.uppair().key(), authstring.pki().prvkey(), authstring.pki().pubkey());
                }
            }

            static void assignEngine(const Request::Engine &src, REQUEST::ENGINE *dest)
            {
                if (!src.binary.empty())
                    dest->set_binary(src.binary);
                if (!src.id.empty())
                    dest->set_id(src.id);
                if (!src.name.empty())
                    dest->set_name(src.name);
                for (const auto &extfile_ : src.extfiles)
                {
                    auto temp = dest->add_extfiles();
                    temp->set_path(extfile_.path);
                    assignEndpoint(extfile_.storage, temp->mutable_storage());
                }
                for (const auto &engine_ : src.deps)
                {
                    assignEngine(engine_, dest->add_deps());
                }
            }

            static void assignENGINE(const REQUEST::ENGINE &src, Request::Engine *dest)
            {
                dest->binary = src.binary();
                dest->id = src.id();
                dest->name = src.name();
                dest->deps.clear();
                dest->extfiles.clear(); // 是否需要clear
                for (int i = 0; i < src.extfiles().size(); i++)
                {
                    dest->extfiles.push_back({Endpoint(), src.extfiles(i).path()});
                    assignENDPOINT(src.extfiles(i).storage(), &dest->extfiles.at(i).storage);
                }
                for (int i = 0; i < src.deps().size(); i++)
                {
                    dest->deps.push_back(Request::Engine());
                    assignENGINE(src.deps(i), &dest->deps.at(i));
                }
            }

            Endpoint::Endpoint()
            {
                this->port = 0; // 如果要检查port，是否这么做，或者就不检查port，或者直接 = INT_MAX
                this->authtype = 0x0;
            }

            bool Endpoint::check() const
            {
                return !(this->proto.empty() || this->url.empty() || this->port == 0);
            }

            int Endpoint::set_proto(const std::string &proto)
            {
                if (proto == "file" || proto == "tcp" || proto == "tls" || proto == "http" || proto == "https" || proto == "ssh" || proto == "jxtd")
                {
                    this->proto = proto;
                    return 0;
                }
                return -1;
            }

            const std::string &Endpoint::get_proto() const
            {
                return this->proto;
            }

            void Endpoint::set_url(const std::string &url)
            {
                this->url = url;
            }

            const std::string &Endpoint::get_url() const
            {
                return this->url;
            }

            void Endpoint::set_port(uint32_t port)
            {
                this->port = port;
            }

            uint32_t Endpoint::get_port() const
            {
                return this->port;
            }

            void Endpoint::set_path(const std::string &path)
            {
                this->path = path;
            }

            const std::string &Endpoint::get_path() const
            {
                return this->path;
            }

            int Endpoint::set_authtype(uint32_t authtype)
            {
                if (authtype >= 0x5)
                    return -1;
                this->authtype = authtype;
                return 0;
            }

            uint32_t Endpoint::get_authtype() const
            {
                return this->authtype;
            }

            void Endpoint::add_authstring(const std::string &token, const std::string &name, const std::string &key, const std::string &prvkey, const std::string &pubkey)
            {
                this->authstring.push_back(Endpoint::Authstring());
                this->authstring.back().token = token;
                this->authstring.back().uppair = std::make_pair(name, key);
                this->authstring.back().pki = std::make_pair(prvkey, pubkey);
            }

            void Endpoint::add_authstring_token(const std::string &token)
            {
                this->authstring.push_back(Endpoint::Authstring());
                this->authstring.back().token = token;
            }

            void Endpoint::add_authstring_uppair(const std::string &name, const std::string &key)
            {
                this->authstring.push_back(Endpoint::Authstring());
                this->authstring.back().uppair = std::make_pair(name, key);
            }

            void Endpoint::add_authstring_pki(const std::string &prvkey, const std::string &pubkey)
            {
                this->authstring.push_back(Endpoint::Authstring());
                this->authstring.back().pki = std::make_pair(prvkey, pubkey);
            }

            std::string Endpoint::get_authstring_token(int pos) const
            {
                if (pos < 0 || pos >= this->authstring.size())
                    return "";
                return this->authstring.at(pos).token;
            }

            std::pair<const std::string &, const std::string &> Endpoint::get_authstring_uppair(int pos) const
            {
                if (pos < 0 || pos >= this->authstring.size())
                    return std::make_pair("", "");
                return this->authstring.at(pos).uppair;
            }

            std::pair<const std::string &, const std::string &> Endpoint::get_authstring_pki(int pos) const
            {
                if (pos < 0 || pos >= this->authstring.size())
                    return std::make_pair("", "");
                return this->authstring.at(pos).pki;
            }

            const std::vector<Endpoint::Authstring> &Endpoint::get_authstring() const
            {
                return this->authstring;
            }

            Request::Request()
            {
                this->datasource = nullptr;
                this->attr = nullptr;
                this->engine = nullptr;
                this->buf = nullptr;
                this->size = 0;
            }

            Request::~Request()
            {
                if (this->datasource != nullptr)
                    delete this->datasource;
                if (this->attr != nullptr)
                {
                    if(this->attr->period != nullptr)
                        delete this->attr->period;
                    delete this->attr;
                }
                if (this->engine != nullptr)
                    delete this->engine;
                if (this->object.management != nullptr)
                    delete this->object.management;
                if (this->object.cluster != nullptr)
                    delete this->object.cluster;
                if (this->object.storage != nullptr)
                    delete this->object.storage;
                if (this->buf != nullptr)
                    delete[] this->buf;
            }

            Request::Request(const Request &other): 
                size(other.size),
                src(other.src),
                dest(other.dest),
                msg(other.msg),
                token(other.token),
                taskid(other.taskid)
            {
                if (other.datasource != nullptr)
                {
                    this->datasource = new Request::Datasource;
                    *this->datasource = *other.datasource; // 直接复制
                }
                else
                {
                    this->datasource = nullptr;
                }

                if (other.engine != nullptr)
                {
                    this->engine = new Request::Engine;
                    *this->engine = *other.engine; // 直接复制
                }
                else
                {
                    this->engine = nullptr;
                }

                if (other.attr != nullptr)
                {
                    this->attr = new Request::Attr;
                    this->attr->trigger = other.attr->trigger;
                    this->attr->deps = other.attr->deps;
                    this->attr->virtstorage = other.attr->virtstorage;
                    if(other.attr->period != nullptr)
                    {
                        this->attr->period = new Request::Attr::Period;
                        *this->attr->period = *other.attr->period;
                    }
                    else
                    {
                        this->attr->period = nullptr;
                    }
                }
                else
                {
                    this->attr = nullptr;
                }

                if (other.object.management != nullptr)
                {
                    this->object.management = new Endpoint;
                    *this->object.management = *other.object.management; // 直接复制
                }
                else
                {
                    this->object.management = nullptr;
                }

                if (other.object.storage != nullptr)
                {
                    this->object.storage = new Endpoint;
                    *this->object.storage = *other.object.storage; // 直接复制
                }
                else
                {
                    this->object.storage = nullptr;
                }

                if (other.object.cluster != nullptr)
                {
                    this->object.cluster = new Endpoint;
                    *this->object.cluster = *other.object.cluster; // 直接复制
                }
                else
                {
                    this->object.cluster = nullptr;
                }

                if(other.buf != nullptr)
                {
                    this->buf = new char[other.size];
                    memcpy(this->buf, other.buf, other.size);
                }
                else
                {
                    this->buf = nullptr;
                }
            }

            Request::Request(Request &&other) noexcept: 
                size(other.size),
                buf(other.buf),
                src(other.src),
                dest(other.dest),
                msg(other.msg),
                token(std::move(other.token)),
                taskid(other.taskid),
                datasource(other.datasource),
                engine(other.engine),
                attr(other.attr),
                object(other.object)
            {
                other.datasource = nullptr;
                other.engine = nullptr;
                other.attr = nullptr;
                other.buf = nullptr;
                other.object.management = nullptr;
                other.object.cluster = nullptr;
                other.object.storage = nullptr;
                other.size = 0;
            }

            Request &Request::operator=(const Request &other)
            {
                if (&other == this)
                    return *this;
                if (other.datasource != nullptr)
                {
                    if(this->datasource == nullptr)
                        this->datasource = new Request::Datasource;
                    *this->datasource = *other.datasource; // 直接复制
                }
                else
                {
                    if(this->datasource != nullptr)
                        delete this->datasource;
                    this->datasource = nullptr;
                }

                if (other.engine != nullptr)
                {
                    if(this->engine == nullptr)
                        this->engine = new Request::Engine;
                    *this->engine = *other.engine; // 直接复制
                }
                else
                {
                    if(this->engine != nullptr)
                        delete this->engine;
                    this->engine = nullptr;
                }

                if (other.attr != nullptr)
                {
                    if(this->attr == nullptr)
                        this->attr = new Request::Attr;
                    this->attr->trigger = other.attr->trigger;
                    this->attr->deps = other.attr->deps;
                    this->attr->virtstorage = other.attr->virtstorage;
                    if(other.attr->period != nullptr)
                    {
                        if(this->attr->period == nullptr)
                            this->attr->period = new Request::Attr::Period;
                        *this->attr->period = *other.attr->period;
                    }
                    else
                    {
                        if(this->attr->period != nullptr)
                            delete this->attr->period;
                        this->attr->period = nullptr;
                    }
                }
                else
                {
                    if(this->attr != nullptr)
                    {
                        if(this->attr->period != nullptr)
                            delete this->attr->period;
                        delete this->attr;
                    }
                    this->attr = nullptr;
                }

                if (other.object.management != nullptr)
                {
                    if(this->object.management == nullptr)
                        this->object.management = new Endpoint;
                    *this->object.management = *other.object.management; // 直接复制
                }
                else
                {
                    if(this->object.management != nullptr)
                        delete this->object.management;
                    this->object.management = nullptr;
                }

                if (other.object.storage != nullptr)
                {
                    if(this->object.storage == nullptr)
                        this->object.storage = new Endpoint;
                    *this->object.storage = *other.object.storage; // 直接复制
                }
                else
                {
                    if(this->object.storage != nullptr)
                        delete this->object.storage;
                    this->object.storage = nullptr;
                }

                if (other.object.cluster != nullptr)
                {
                    if(this->object.cluster == nullptr)
                        this->object.cluster = new Endpoint;
                    *this->object.cluster = *other.object.cluster; // 直接复制
                }
                else
                {
                    if(this->object.cluster != nullptr)
                        delete this->object.cluster;
                    this->object.cluster = nullptr;
                }

                if(other.buf != nullptr)
                {
                    if(this->buf != nullptr)
                        delete[] this->buf;
                    this->buf = new char[other.size];
                    memcpy(this->buf, other.buf, other.size);
                }
                else
                {
                    if(this->buf != nullptr)
                        delete[] this->buf;
                    this->buf = nullptr;
                }
                this->size = other.size;
                this->src = other.src;
                this->dest = other.dest;
                this->msg = other.msg;
                this->token = other.token;
                this->taskid = other.taskid;
                return *this;
            }

            Request &Request::operator=(Request &&other) noexcept
            {
                if (&other == this)
                    return *this;
                this->buf = other.buf;
                other.buf = nullptr;
                this->size = other.size;
                other.size = 0;
                this->src = other.src;
                this->dest = other.dest;
                this->msg = other.msg;
                this->token = std::move(other.token);
                this->taskid = other.taskid;
                this->datasource = other.datasource;
                other.datasource = nullptr;
                this->engine = other.engine;
                other.engine = nullptr;
                this->attr = other.attr;
                other.attr = nullptr;
                this->object = other.object;
                other.object.management = nullptr;
                other.object.storage = nullptr;
                other.object.cluster = nullptr;
                return *this;
            }

            int Request::encode(struct iovec vectors[], int max)
            {
                REQUEST request;
                request.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::TASKDEP>());
                auto type = request.mutable_type();
                type->set_src(this->src);
                type->set_dest(this->dest);
                type->set_msg(this->msg);
                request.set_token(this->token);
                request.set_taskid(this->taskid);
                if (this->datasource != nullptr) // 这里是任选一个数据进行判断是否未设置还是所有的都要判断
                {
                    {
                        auto datasource_src = request.mutable_datasource()->mutable_src();
                        datasource_src->set_type(this->datasource->src.type);
                        datasource_src->set_input(this->datasource->src.input);
                    }

                    {
                        auto datasource_dest = request.mutable_datasource()->mutable_dest();
                        datasource_dest->set_directorypolicy(this->datasource->dest.directorypolicy);
                        datasource_dest->set_emptypolicy(this->datasource->dest.emptypolicy);
                        datasource_dest->set_output(this->datasource->dest.output);
                    }
                }
                auto engine = request.mutable_engine();
                if(this->engine != nullptr)
                    assignEngine(*this->engine, engine);
                auto attr = request.mutable_attr();
                if(this->attr != nullptr)
                {
                    attr->set_trigger(this->attr->trigger);
                    attr->set_virtstorage(this->attr->virtstorage);
                    if (this->attr->period != nullptr)
                    {
                        const auto &period_ = *(this->attr->period);
                        auto period = attr->mutable_period();
                        period->set_interval(period_.interval);
                        period->set_resultpolicy(period_.resultpolicy);
                        period->set_triggerpolicy(period_.triggerpolicy);
                        for (const auto &backupid : period_.backupid)
                        {
                            period->add_backupid(backupid);
                        }
                    }
                    for (const auto &dep : this->attr->deps)
                    {
                        attr->add_deps(dep);
                    }
                }
                auto object = request.mutable_object();
                if(this->object.management != nullptr)
                    assignEndpoint(*this->object.management, object->mutable_management());
                if(this->object.cluster != nullptr)
                    assignEndpoint(*this->object.cluster, object->mutable_cluster());
                if(this->object.storage != nullptr)
                    assignEndpoint(*this->object.storage, object->mutable_storage());

                this->size = request.ByteSizeLong();
                if (this->buf != nullptr)
                    delete[] this->buf; // 这一步是否有必要
                this->buf = new char[this->size];
                request.SerializeToArray(this->buf, this->size);
                vectors[0].iov_base = this->buf;
                vectors[0].iov_len = this->size;
                return 1;
            }

            int Request::append(const void *buf, size_t *size)
            {
                if (!this->buf)
                {
                    this->buf = new char[*size];
                    memmove(this->buf, buf, *size);
                }
                else
                {
                    auto temp = this->buf;
                    this->buf = new char[this->size + *size];
                    memmove(this->buf, temp, this->size);
                    memmove(this->buf + this->size, buf, *size);
                    delete[] temp;
                }
                this->size += *size;
                REQUEST request;
                if (!request.ParseFromArray(this->buf, this->size))
                    return 0;
                this->buf = nullptr;
                this->size = 0;
                if (request.magic() != ::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::TASKDEP>())
                {
                    errno = static_cast<int>(Message_Errno::MAGIC_MISMATCH);
                    return -1;
                }
                const auto &type = request.type();
                this->src = type.src();
                this->dest = type.dest();
                this->msg = type.msg();
                this->token = request.token();
                this->taskid = request.taskid();
                if(!request.has_datasource())
                {
                    if(this->datasource != nullptr)
                        delete this->datasource;
                    this->datasource = nullptr;
                }
                else
                {
                    const auto &datasource = request.datasource();
                    if(this->datasource == nullptr)
                        this->datasource = new Request::Datasource;
                    this->datasource->src.type = datasource.src().type();
                    this->datasource->src.input = datasource.src().input();
                    this->datasource->dest.output = datasource.dest().output();
                    this->datasource->dest.directorypolicy = datasource.dest().directorypolicy();
                    this->datasource->dest.emptypolicy = datasource.dest().emptypolicy();
                }
                if(!request.has_engine())
                {
                    if(this->engine != nullptr)
                        delete this->engine;
                    this->engine = nullptr;
                }
                else
                {
                    const auto &engine = request.engine();
                    if(this->engine == nullptr)
                        this->engine = new Request::Engine;
                    assignENGINE(engine, this->engine);
                }
                if(!request.has_attr())
                {
                    if(this->attr != nullptr)
                    {
                        if(this->attr->period != nullptr)
                            delete this->attr->period;
                        this->attr->period = nullptr;
                        delete this->attr;
                        this->attr = nullptr;
                    }
                }
                else
                {
                    const auto &attr = request.attr();
                    if(this->attr == nullptr)
                        this->attr = new Request::Attr;
                    this->attr->trigger = attr.trigger();
                    this->attr->deps.clear(); // 是否需要
                    for (int i = 0; i < attr.deps().size(); i++)
                    {
                        this->attr->deps.emplace_back(attr.deps(i));
                    }
                    this->attr->virtstorage = attr.virtstorage();

                    if(!attr.has_period())
                    {
                        if(this->attr->period != nullptr)
                            delete this->attr->period;
                        this->attr->period = nullptr;
                    }
                    else
                    {
                        if(this->attr->period == nullptr)
                            this->attr->period = new Request::Attr::Period;
                        this->attr->period->interval = attr.period().interval();
                        this->attr->period->resultpolicy = attr.period().resultpolicy();
                        this->attr->period->triggerpolicy = attr.period().triggerpolicy();
                        this->attr->period->backupid.clear();
                        for (int i = 0; i < attr.period().backupid().size(); i++)
                        {
                            this->attr->period->backupid.emplace_back(attr.period().backupid(i));
                        }  
                    }
                }
                const auto &object = request.object();
                if(object.has_management())
                {
                    if(this->object.management == nullptr)
                        this->object.management = new Endpoint;
                    assignENDPOINT(object.management(), this->object.management);
                }
                if(object.has_cluster())
                {
                    if(this->object.cluster == nullptr)
                        this->object.cluster = new Endpoint;
                    assignENDPOINT(object.cluster(), this->object.cluster);
                }
                if(object.has_storage())
                {
                    if(this->object.storage == nullptr)
                        this->object.storage = new Endpoint;
                    assignENDPOINT(object.storage(), this->object.storage);
                }
                return 1;
            }

            void Request::set_type(uint32_t src, uint32_t dest, uint32_t msg)
            {
                this->src = src;
                this->dest = dest;
                this->msg = msg;
            }

            uint32_t Request::get_type_src() const
            {
                return this->src;
            }

            uint32_t Request::get_type_dest() const
            {
                return this->dest;
            }

            uint32_t Request::get_type_msg() const
            {
                return this->msg;
            }

            void Request::set_token(const std::string &token)
            {
                this->token = token;
            }

            const std::string &Request::get_token() const
            {
                return this->token;
            }

            void Request::set_taskid(uint64_t taskid)
            {
                this->taskid = taskid;
            }

            uint64_t Request::get_taskid() const
            {
                return this->taskid;
            }

            int Request::set_datasource(const Datasource::Src &src, const Datasource::Dest &dest)
            {
                if (dest.emptypolicy >= 0x2 || dest.directorypolicy >= 0x2)
                    return -1;
                if(this->datasource == nullptr)
                    this->datasource = new Request::Datasource;
                this->datasource->src = src;
                this->datasource->dest = dest;
                return 0;
            }

            const Request::Datasource::Src *Request::get_datasource_src() const
            {
                if(this->datasource == nullptr)
                    return nullptr;
                return &(this->datasource->src);
            }

            const Request::Datasource::Dest *Request::get_datasource_dest() const
            {
                if(this->datasource == nullptr)
                    return nullptr;
                return &(this->datasource->dest);
            }

            void Request::set_engine_binary(const std::string &binary)
            {
                if(this->engine == nullptr)
                    this->engine = new Request::Engine;
                this->engine->binary = binary;
            }

            void Request::set_engine_id_name(const std::string &id, const std::string &name)
            {
                if(this->engine == nullptr)
                    this->engine = new Request::Engine;
                this->engine->id = id;
                this->engine->name = name;
            }

            void Request::add_engine_dep(const Engine &engine)
            {
                if(this->engine == nullptr)
                    this->engine = new Request::Engine;
                this->engine->deps.emplace_back(engine);
            }

            void Request::add_engine_extfile(const Engine::Extfile &extfile)
            {
                if(this->engine == nullptr)
                    this->engine = new Request::Engine;
                this->engine->extfiles.emplace_back(extfile);
            }

            const Request::Engine *Request::get_engine() const
            {
                return this->engine;
            }

            const Request::Engine *Request::get_engine_dep(int pos) const
            {
                if(this->engine == nullptr)
                    return nullptr;
                if(pos < 0 || pos >= this->engine->deps.size())
                    return nullptr;
                return &(this->engine->deps.at(pos));
            }

            const Request::Engine::Extfile *Request::get_engine_extfile(int pos) const
            {
                if(this->engine == nullptr)
                    return nullptr;
                if (pos < 0 || pos >= this->engine->extfiles.size())
                    return nullptr;
                return &(this->engine->extfiles.at(pos));
            }

            int Request::set_attr_master(uint32_t trigger, bool virtstorage)
            {
                if(trigger >= 0x3)
                    return -1;
                if(this->attr == nullptr)
                    this->attr = new Request::Attr;
                this->attr->trigger = trigger;
                this->attr->virtstorage = virtstorage;
                return 0;
            }

            int Request::set_attr_period(uint32_t interval, uint32_t resultpolicy, uint32_t triggerpolicy, const std::vector<uint32_t> &backupid)
            {
                if (resultpolicy >= 0x3 || triggerpolicy >= 0x3)
                    return -1;
                if(this->attr == nullptr)
                    this->attr = new Request::Attr;
                if(this->attr->period == nullptr)
                    this->attr->period = new Request::Attr::Period;
                this->attr->period->interval = interval;
                this->attr->period->resultpolicy = resultpolicy;
                this->attr->period->triggerpolicy = triggerpolicy;
                this->attr->period->backupid = backupid;
                return 0;
            }

            const Request::Attr::Period *Request::get_attr_period() const
            {
                if(this->attr == nullptr)
                    return nullptr;
                return this->attr->period;
            }

            const Request::Attr *Request::get_attr() const
            {
                return this->attr;
            }

            void Request::add_attr_dep(uint64_t dep)
            {
                if(this->attr == nullptr)
                    this->attr = new Request::Attr;
                this->attr->deps.emplace_back(dep);
            }

            uint64_t Request::get_attr_dep(int pos) const
            {
                if(this->attr == nullptr)
                    return 0x0;
                if (pos < 0 || pos >= this->attr->deps.size())
                    return 0x0;
                return this->attr->deps.at(pos);
            }

            void Request::set_object_management(const Endpoint &management)
            {
                if(this->object.management == nullptr)
                    this->object.management = new Endpoint;
                *this->object.management = management;
            }

            void Request::set_object_storage(const Endpoint &storage)
            {
                if(this->object.storage == nullptr)
                    this->object.storage = new Endpoint;
                *this->object.storage = storage;
            }

            void Request::set_object_cluster(const Endpoint &cluster)
            {
                if(this->object.cluster == nullptr)
                    this->object.cluster = new Endpoint;
                *this->object.cluster = cluster;
            }

            const Endpoint *Request::get_object_management() const
            {
                return this->object.management;
            }

            const Endpoint *Request::get_object_storage() const
            {
                return this->object.storage;
            }

            const Endpoint *Request::get_object_cluster() const
            {
                return this->object.cluster;
            }

            Response::Response()
            {
                this->deploy = nullptr;
                this->neogiate = nullptr;
                this->buf = nullptr;
                this->size = 0;
            }

            Response::~Response()
            {
                if(this->neogiate != nullptr)
                    delete this->neogiate;
                if(this->deploy != nullptr)
                    delete this->deploy;
                if (this->object.management != nullptr)
                    delete this->object.management;
                if (this->object.cluster != nullptr)
                    delete this->object.cluster;
                if (this->object.storage != nullptr)
                    delete this->object.storage;
                if(this->buf != nullptr)
                    delete[] this->buf;
            }

            Response::Response(const Response &other): 
                size(other.size),
                src(other.src),
                dest(other.dest),
                msg(other.msg),
                token(other.token),
                taskid(other.taskid)
            {
                if(other.neogiate != nullptr)
                {
                    this->neogiate = new Response::Neogiate;
                    *this->neogiate = *other.neogiate; // 直接复制
                }
                else
                {
                    this->neogiate = nullptr;
                }

                if(other.deploy != nullptr)
                {
                    this->deploy = new Response::Deploy;
                    *this->deploy = *other.deploy; // 直接复制
                }
                else
                {
                    this->deploy = nullptr;
                }

                if (other.object.management != nullptr)
                {
                    this->object.management = new Endpoint;
                    *this->object.management = *other.object.management; // 直接复制
                }
                else
                {
                    this->object.management = nullptr;
                }

                if (other.object.storage != nullptr)
                {
                    this->object.storage = new Endpoint;
                    *this->object.storage = *other.object.storage; // 直接复制
                }
                else
                {
                    this->object.storage = nullptr;
                }

                if (other.object.cluster != nullptr)
                {
                    this->object.cluster = new Endpoint;
                    *this->object.cluster = *other.object.cluster; // 直接复制
                }
                else
                {
                    this->object.cluster = nullptr;
                }

                if(other.buf != nullptr)
                {
                    this->buf = new char[other.size];
                    memcpy(this->buf, other.buf, other.size);
                }
                else
                {
                    this->buf = nullptr;
                }
            }

            Response::Response(Response &&other) noexcept: 
                buf(other.buf),                                                          
                size(other.size),
                src(other.src),
                dest(other.dest),
                msg(other.msg),
                token(std::move(other.token)),
                taskid(other.taskid),
                neogiate(other.neogiate),
                deploy(other.deploy),
                object(other.object)
            {
                other.neogiate = nullptr;
                other.deploy = nullptr;
                other.buf = nullptr;
                other.object.management = nullptr;
                other.object.cluster = nullptr;
                other.object.storage = nullptr;
                other.size = 0;
            }

            Response &Response::operator=(const Response &other)
            {
                if (&other == this)
                    return *this;
                if(other.neogiate != nullptr)
                {
                    if(this->neogiate == nullptr)
                        this->neogiate = new Response::Neogiate;
                    *this->neogiate = *other.neogiate; // 直接复制
                }
                else
                {
                    if(this->neogiate != nullptr)
                        delete this->neogiate;
                    this->neogiate = nullptr;
                }

                if(other.deploy != nullptr)
                {
                    if(this->deploy == nullptr)
                        this->deploy = new Response::Deploy;
                    *this->deploy = *other.deploy; // 直接复制
                }
                else
                {
                    if(this->deploy != nullptr)
                        delete this->deploy;
                    this->deploy = nullptr;
                }

                if (other.object.management != nullptr)
                {
                    if(this->object.management == nullptr)
                        this->object.management = new Endpoint;
                    *this->object.management = *other.object.management; // 直接复制
                }
                else
                {
                    if(this->object.management != nullptr)
                        delete this->object.management;
                    this->object.management = nullptr;
                }

                if (other.object.storage != nullptr)
                {
                    if(this->object.storage == nullptr)
                        this->object.storage = new Endpoint;
                    *this->object.storage = *other.object.storage; // 直接复制
                }
                else
                {
                    if(this->object.storage != nullptr)
                        delete this->object.storage;
                    this->object.storage = nullptr;
                }

                if (other.object.cluster != nullptr)
                {
                    if(this->object.cluster == nullptr)
                        this->object.cluster = new Endpoint;
                    *this->object.cluster = *other.object.cluster; // 直接复制
                }
                else
                {
                    if(this->object.cluster != nullptr)
                        delete this->object.cluster;
                    this->object.cluster = nullptr;
                }

                if(other.buf != nullptr)
                {
                    if(this->buf != nullptr)
                        delete[] this->buf;
                    this->buf = new char[other.size];
                    memcpy(this->buf, other.buf, other.size);
                }
                else
                {
                    if(this->buf != nullptr)
                        delete[] this->buf;
                    this->buf = nullptr;
                }
                this->size = other.size;
                this->src = other.src;
                this->dest = other.dest;
                this->msg = other.msg;
                this->token = other.token;
                this->taskid = other.taskid;
                return *this;
            }

            Response &Response::operator=(Response &&other) noexcept
            {
                if (&other == this)
                    return *this;
                this->buf = other.buf;
                other.buf = nullptr;
                this->size = other.size;
                other.size = 0;
                this->src = other.src;
                this->dest = other.dest;
                this->msg = other.msg;
                this->token = std::move(other.token);
                this->taskid = other.taskid;
                this->neogiate = other.neogiate;
                other.neogiate = nullptr;
                this->deploy = other.deploy;
                other.deploy = nullptr;
                this->object = other.object;
                other.object.management = nullptr;
                other.object.storage = nullptr;
                other.object.cluster = nullptr;
                return *this;
            }

            int Response::encode(struct iovec vectors[], int max)
            {
                RESPONSE response;
                response.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::TASKDEP>());
                auto type = response.mutable_type();
                type->set_src(this->src);
                type->set_dest(this->dest);
                type->set_msg(this->msg);
                response.set_token(this->token);
                response.set_taskid(this->taskid);
                if (this->neogiate != nullptr)
                {
                    auto neogiate = response.mutable_neogiate();
                    neogiate->set_status(this->neogiate->status);
                    if (!this->neogiate->errstr.empty())
                        neogiate->set_errstr(this->neogiate->errstr);
                    for (const auto &alternative : this->neogiate->alternative)
                    {
                        neogiate->add_alternative(alternative);
                    }
                }
                if (this->deploy != nullptr)
                {
                    auto deploy = response.mutable_deploy();
                    deploy->set_status(this->deploy->status);
                    if (!this->deploy->errstr.empty())
                        deploy->set_errstr(this->deploy->errstr);
                    deploy->set_taskstatus(this->deploy->taskstatus);
                }
                auto object = response.mutable_object();
                if(this->object.management != nullptr)
                    assignEndpoint(*this->object.management, object->mutable_management());
                if(this->object.cluster != nullptr)
                    assignEndpoint(*this->object.cluster, object->mutable_cluster());
                if(this->object.storage != nullptr)
                    assignEndpoint(*this->object.storage, object->mutable_storage());

                this->size = response.ByteSizeLong();
                if (this->buf != nullptr)
                    delete[] this->buf; // 这一步是否有必要
                this->buf = new char[this->size];
                response.SerializeToArray(this->buf, this->size);
                vectors[0].iov_base = this->buf;
                vectors[0].iov_len = this->size;
                return 1;
            }

            int Response::append(const void *buf, size_t *size)
            {
                if (!this->buf)
                {
                    this->buf = new char[*size];
                    memmove(this->buf, buf, *size);
                }
                else
                {
                    auto temp = this->buf;
                    this->buf = new char[this->size + *size];
                    memmove(this->buf, temp, this->size);
                    memmove(this->buf + this->size, buf, *size);
                    delete[] temp;
                }
                this->size += *size;
                RESPONSE response;
                if (!response.ParseFromArray(this->buf, this->size))
                    return 0;
                this->buf = nullptr;
                this->size = 0;
                if (response.magic() != ::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::TASKDEP>())
                {
                    errno = static_cast<int>(Message_Errno::MAGIC_MISMATCH);
                    return -1;
                }
                const auto &type = response.type();
                this->src = type.src();
                this->dest = type.dest();
                this->msg = type.msg();
                this->token = response.token();
                this->taskid = response.taskid();
                if(!response.has_neogiate())
                {
                    if(this->neogiate != nullptr)
                        delete this->neogiate;
                    this->neogiate = nullptr;
                }
                else
                {
                    if(this->neogiate == nullptr)
                        this->neogiate = new Response::Neogiate;
                    const auto &neogiate = response.neogiate();
                    this->neogiate->status = neogiate.status();
                    this->neogiate->errstr = neogiate.errstr();
                    this->neogiate->alternative.clear();
                    for (int i = 0; i < neogiate.alternative().size(); i++)
                    {
                        this->neogiate->alternative.emplace_back(neogiate.alternative(i));
                    }
                }
                if(!response.has_deploy())
                {
                    if(this->deploy != nullptr)
                        delete this->deploy;
                    this->deploy = nullptr;
                }
                else
                {
                    if(this->deploy == nullptr)
                        this->deploy = new Response::Deploy;
                    const auto &deploy = response.deploy();
                    this->deploy->status = deploy.status();
                    this->deploy->errstr = deploy.errstr();
                    this->deploy->taskstatus = deploy.taskstatus();
                }
                const auto &object = response.object();
                if(object.has_management())
                {
                    if(this->object.management == nullptr)
                        this->object.management = new Endpoint;
                    assignENDPOINT(object.management(), this->object.management);
                }
                if(object.has_cluster())
                {
                    if(this->object.cluster == nullptr)
                        this->object.cluster = new Endpoint;
                    assignENDPOINT(object.cluster(), this->object.cluster);
                }
                if(object.has_storage())
                {
                    if(this->object.storage == nullptr)
                        this->object.storage = new Endpoint;
                    assignENDPOINT(object.storage(), this->object.storage);
                }
                return 1;
            }

            void Response::set_type(uint32_t src, uint32_t dest, uint32_t msg)
            {
                this->src = src;
                this->dest = dest;
                this->msg = msg;
            }

            uint32_t Response::get_type_src() const
            {
                return this->src;
            }

            uint32_t Response::get_type_dest() const
            {
                return this->dest;
            }

            uint32_t Response::get_type_msg() const
            {
                return this->msg;
            }

            void Response::set_token(const std::string &token)
            {
                this->token = token;
            }

            const std::string &Response::get_token() const
            {
                return this->token;
            }

            void Response::set_taskid(uint64_t taskid)
            {
                this->taskid = taskid;
            }

            uint64_t Response::get_taskid() const
            {
                return this->taskid;
            }

            int Response::set_neogiate_status(uint32_t status)
            {
                if (status >= 0x4)
                    return -1;
                if(this->neogiate == nullptr)
                    this->neogiate = new Response::Neogiate;
                this->neogiate->status = status;
                return 0;
            }

            void Response::set_neogiate_errstr(const std::string &errstr)
            {
                if(this->neogiate == nullptr)
                    this->neogiate = new Response::Neogiate;
                this->neogiate->errstr = errstr;
            }

            void Response::add_neogiate_alternative(uint64_t alternative)
            {
                if(this->neogiate == nullptr)
                    this->neogiate = new Response::Neogiate;
                this->neogiate->alternative.emplace_back(alternative);
            }

            uint64_t Response::get_neogiate_alternative(int pos) const
            {
                if(this->neogiate == nullptr)
                    return 0x0;
                if (pos < 0 || pos >= this->neogiate->alternative.size())
                    return 0x0;
                return this->neogiate->alternative.at(pos);
            }

            const Response::Neogiate *Response::get_neogiate() const
            {
                return this->neogiate;
            }

            int Response::set_deploy_status(uint32_t status)
            {
                if (status >= 0x6)
                    return -1;
                if(this->deploy == nullptr)
                    this->deploy = new Response::Deploy;
                this->deploy->status = status;
                return 0;
            }

            void Response::set_deploy_errstr(const std::string &errstr)
            {
                if(this->deploy == nullptr)
                    this->deploy = new Response::Deploy;
                this->deploy->errstr = errstr;
            }

            int Response::set_deploy_taskstatus(uint32_t taskstatus)
            {
                if (taskstatus >= 0xb)
                    return -1;
                if(this->deploy == nullptr)
                    this->deploy = new Response::Deploy;
                this->deploy->taskstatus = taskstatus;
                return 0;
            }

            const Response::Deploy *Response::get_deploy() const
            {
                return this->deploy;
            }

            void Response::set_object_management(const Endpoint &management)
            {
                if(this->object.management == nullptr)
                    this->object.management = new Endpoint;
                *this->object.management = management;
            }

            void Response::set_object_storage(const Endpoint &storage)
            {
                if(this->object.storage == nullptr)
                    this->object.storage = new Endpoint;
                *this->object.storage = storage;
            }

            void Response::set_object_cluster(const Endpoint &cluster)
            {
                if(this->object.cluster == nullptr)
                    this->object.cluster = new Endpoint;
                *this->object.cluster = cluster;
            }

            const Endpoint *Response::get_object_management() const
            {
                return this->object.management;
            }

            const Endpoint *Response::get_object_storage() const
            {
                return this->object.storage;
            }

            const Endpoint *Response::get_object_cluster() const
            {
                return this->object.cluster;
            }
        }
    }
}
