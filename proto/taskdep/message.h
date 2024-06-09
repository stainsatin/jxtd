#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <workflow/ProtocolMessage.h>

namespace jxtd
{
    namespace proto
    {
        namespace taskdep
        {
            enum class Message_Errno : int
            {
                MAGIC_MISMATCH = 257,
            };

            class Endpoint
            {
            public:
                enum class Attr_authtype : uint32_t
                {
                    NONE = 0x0,
                    TOKEN = 0x1,
                    USERPASSWD = 0x2,
                    PKI = 0x4,
                    OAUTH2 = 0x5
                };

                struct Authstring
                {
                    std::string token;                          // optional
                    std::pair<std::string, std::string> uppair; // first---name, second---key // optional
                    std::pair<std::string, std::string> pki;    // first---prvkey, second---pubkey // optional
                    // reserved // 待修改，包括assignENDPOINT, assignEndpoint, 以及Endpoint的API
                };

                Endpoint();
                ~Endpoint() = default;
                Endpoint(const Endpoint &other) = default;
                Endpoint(Endpoint &&other) noexcept = default;
                Endpoint &operator=(const Endpoint &other) = default;
                Endpoint &operator=(Endpoint &&other) noexcept = default;

                int set_proto(const std::string &proto);
                const std::string &get_proto() const;
                void set_url(const std::string &url);
                const std::string &get_url() const;
                void set_port(uint32_t port);
                uint32_t get_port() const;
                void set_path(const std::string &path);
                const std::string &get_path() const;
                int set_authtype(uint32_t authtype);
                uint32_t get_authtype() const;
                void add_authstring(const std::string &token, const std::string &name, const std::string &key, const std::string &prvkey, const std::string &pubkey);
                void add_authstring_token(const std::string &token);
                void add_authstring_uppair(const std::string &name, const std::string &key);
                void add_authstring_pki(const std::string &prvkey, const std::string &pubkey);
                std::string get_authstring_token(int pos) const;
                std::pair<const std::string &, const std::string &> get_authstring_uppair(int pos) const;
                std::pair<const std::string &, const std::string &> get_authstring_pki(int pos) const;
                const std::vector<Endpoint::Authstring> &get_authstring() const;
                bool check() const; // 没有检查port，且要满足proto，url，path非空，且authtype对应的authstring至少有一个，且非空

            private:
                std::string proto;
                std::string url;
                uint32_t port;
                std::string path;
                uint32_t authtype;
                std::vector<Authstring> authstring; // optional // authtype + authstring = attr
            };

            enum class ADDR_TYPE : uint32_t
            {
                storage = 0x0,
                cluster = 0x1,
                management = 0x2
            };

            enum class MSG_TYPE : uint32_t
            {
                loopback = 0x0,
                neogiate = 0x1,
                deploy = 0x2,
                terminate = 0x3,
                trigger = 0x4,
                done = 0x5
            };

            class Request : public ::protocol::ProtocolMessage
            {
            public:
                enum class Datasource_dest_directorypolicy : uint32_t
                {
                    override = 0x0,
                    append = 0x1
                };

                enum class Datasource_dest_emptypolicy : uint32_t
                {
                    create = 0x0,
                    fail = 0x1
                };

                enum class Attr_tigger : uint32_t
                {
                    manual = 0x0,
                    create = 0x1,
                    deps = 0x2
                };

                enum class Attr_period_resultpolicy : uint32_t
                {
                    all = 0x0,
                    packed = 0x1,
                    separate = 0x2
                };

                enum class Attr_period_triggerpolicy : uint32_t
                {
                    single = 0x0,
                    continuous = 0x1,
                    desinated = 0x2
                };

                struct Datasource
                {
                    struct Src
                    {
                        std::string input;
                        std::string type;
                    };

                    struct Dest
                    {
                        std::string output;
                        uint32_t directorypolicy;
                        uint32_t emptypolicy;
                    };

                    Src src;
                    Dest dest;
                };

                struct Engine
                {
                    struct Extfile
                    {
                        Endpoint storage;
                        std::string path;
                    };

                    std::string binary;
                    std::string id;
                    std::string name;
                    std::vector<Engine> deps;
                    std::vector<Extfile> extfiles;
                };

                struct Attr
                {
                    struct Period
                    {
                        uint32_t interval;
                        uint32_t resultpolicy;
                        uint32_t triggerpolicy;
                        std::vector<uint32_t> backupid;
                    };

                    uint32_t trigger;
                    Period *period = nullptr;
                    std::vector<uint64_t> deps;
                    bool virtstorage;
                };

                struct Object
                {
                    Endpoint *management = nullptr;
                    Endpoint *cluster = nullptr;
                    Endpoint *storage = nullptr;
                };

                Request();
                ~Request();
                Request(const Request &other);
                Request(Request &&other) noexcept;
                Request &operator=(const Request &other);
                Request &operator=(Request &&other) noexcept;

                int encode(struct iovec vectors[], int max) override;
                int append(const void *buf, size_t *size) override;

                void set_type(uint32_t src, uint32_t dest, uint32_t msg);
                uint32_t get_type_src() const;
                uint32_t get_type_dest() const;
                uint32_t get_type_msg() const;
                void set_token(const std::string &token);
                const std::string &get_token() const;
                void set_taskid(uint64_t taskid);
                uint64_t get_taskid() const;
                int set_datasource(const Datasource::Src &src, const Datasource::Dest &dest);
                const Datasource::Src *get_datasource_src() const;
                const Datasource::Dest *get_datasource_dest() const;
                void set_engine_binary(const std::string &binary);
                void set_engine_id_name(const std::string &id, const std::string &name);
                void add_engine_dep(const Engine &engine);
                void add_engine_extfile(const Engine::Extfile &extfile);
                const Engine *get_engine_dep(int pos) const;
                const Engine::Extfile *get_engine_extfile(int pos) const;
                const Engine *get_engine() const;
                int set_attr_master(uint32_t trigger, bool virtstorage);
                int set_attr_period(uint32_t interval, uint32_t resultpolicy, uint32_t triggerpolicy, const std::vector<uint32_t> &backupid);
                void add_attr_dep(uint64_t dep);
                uint64_t get_attr_dep(int pos) const;
                const Attr::Period *get_attr_period() const;
                const Attr *get_attr() const;
                void set_object_management(const Endpoint &management);
                void set_object_storage(const Endpoint &storage);
                void set_object_cluster(const Endpoint &cluster);
                const Endpoint *get_object_management() const;
                const Endpoint *get_object_storage() const;
                const Endpoint *get_object_cluster() const;

            private:
                char *buf;
                size_t size;

                uint32_t src;
                uint32_t dest;
                uint32_t msg;
                std::string token;
                uint64_t taskid;
                Datasource *datasource;
                Engine *engine;
                Attr *attr;
                Object object;
            };

            class Response : public ::protocol::ProtocolMessage
            {
            public:
                enum class Neogiate_status : uint32_t
                {
                    succeed = 0x0,
                    err = 0x1,
                    conflict = 0x2,
                    illegal = 0x3
                };

                enum class Deploy_status : uint32_t
                {
                    succeed = 0x0,
                    err = 0x1,
                    busy = 0x2,
                    depsbroken = 0x3,
                    enginebroken = 0x4,
                    illegal = 0x5
                };

                enum class Deploy_taskstatus : uint32_t
                {
                    deployed = 0x0,
                    triggered = 0x1,
                    prepared = 0x2,
                    fetched = 0x3,
                    running = 0x4,
                    writeback = 0x5,
                    terminating = 0x6,
                    partial = 0x7,
                    waiting = 0x8,
                    exiting = 0x9,
                    detached = 0xa
                };

                struct Neogiate
                {
                    uint32_t status;
                    std::string errstr;
                    std::vector<uint64_t> alternative;
                };

                struct Deploy
                {
                    uint32_t status;
                    std::string errstr;
                    uint32_t taskstatus;
                };

                struct Object
                {
                    Endpoint *management = nullptr;
                    Endpoint *cluster = nullptr;
                    Endpoint *storage = nullptr;
                };

                Response();
                ~Response();
                Response(const Response &other);
                Response(Response &&other) noexcept;
                Response &operator=(const Response &other);
                Response &operator=(Response &&other) noexcept;

                int encode(struct iovec vectors[], int max) override;
                int append(const void *buf, size_t *size) override;

                void set_type(uint32_t src, uint32_t dest, uint32_t msg);
                uint32_t get_type_src() const;
                uint32_t get_type_dest() const;
                uint32_t get_type_msg() const;
                void set_token(const std::string &token);
                const std::string &get_token() const;
                void set_taskid(uint64_t taskid);
                uint64_t get_taskid() const;
                int set_neogiate_status(uint32_t status);
                void set_neogiate_errstr(const std::string &errstr);
                void add_neogiate_alternative(uint64_t alternative);
                uint64_t get_neogiate_alternative(int pos) const;
                const Neogiate *get_neogiate() const;
                int set_deploy_status(uint32_t status);
                void set_deploy_errstr(const std::string &errstr);
                int set_deploy_taskstatus(uint32_t taskstatus);
                const Deploy *get_deploy() const;
                void set_object_management(const Endpoint &management);
                void set_object_storage(const Endpoint &storage);
                void set_object_cluster(const Endpoint &cluster);
                const Endpoint *get_object_management() const;
                const Endpoint *get_object_storage() const;
                const Endpoint *get_object_cluster() const;

            private:
                char *buf;
                size_t size;

                uint32_t src;
                uint32_t dest;
                uint32_t msg;
                std::string token;
                uint64_t taskid;
                Neogiate *neogiate;
                Deploy *deploy;
                Object object;
            };
        }
    }
}
