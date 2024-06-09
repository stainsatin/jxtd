#pragma once
#include <workflow/ProtocolMessage.h>
#include <string>
#include <vector>

namespace jxtd
{
    namespace proto
    {
        namespace datatransfer
        {
            // taskid, type---0x0 - engine, 0x1 - endpoint/datasource, 0x2 - binary, engineattr---first - mid, second -name, endpointattr---first - proto, second - url, binary 
            struct Adtsrc
            {
                uint32_t taskid = 0x0;
                uint32_t type;
                std::pair<std::string, std::string> engineattr;
                std::pair<std::string, std::string> endpointattr;
                std::string binary;
            };

            struct Dtsrc
            {
                Adtsrc master;
                std::vector<std::pair<Adtsrc, std::string>> deps;
            };
            
            class Request : public ::protocol::ProtocolMessage
            {
            public:
                Request();
                ~Request();
                explicit Request(const Request &req);
                explicit Request(Request &&req) noexcept;
                Request &operator=(const Request &req);
                Request &operator=(Request &&req) noexcept;
                int encode(struct iovec vectors[], int max) override;
                int append(const void *buf, size_t *size) override;

                // magic在创建时自动设置，response同理
                void set_src(const std::string &src_);
                const std::string &get_src() const;
                int set_direction(uint32_t direction_); // direction只能为 0x0---pull 和 0x1---push
                uint32_t get_direction() const;
                void set_token(const std::string &token_);
                const std::string &get_token() const;
                void set_dest(const std::string &dest_);
                const std::string &get_dest() const;

                // 一个request中有多个adtsrc
                void add_adtsrc(const Adtsrc &adtsrc_);
                const std::vector<Adtsrc> &get_adtsrcs() const;
                const Adtsrc *get_adtsrc(int pos) const;

            private:
                // response同理
                char *buf_;   // req_ SerializeToArray后存放的地方
                int size_;    // buf size

                std::string token;
                std::string src;
                std::string dest;
                uint32_t direction;
                std::vector<Adtsrc> adtsrcs;
            };

            class Response : public ::protocol::ProtocolMessage
            {
            public:
                Response();
                ~Response();
                explicit Response(const Response &resp);
                explicit Response(Response &&resp) noexcept;
                Response &operator=(const Response &resp);
                Response &operator=(Response &&resp) noexcept;
                int encode(struct iovec vectors[], int max) override;
                int append(const void *buf, size_t *size) override;

                void set_src(const std::string &src_);
                const std::string &get_src() const;
                void set_token(const std::string &token_);
                const std::string &get_token() const;
                void set_dest(const std::string &dest_);
                const std::string &get_dest() const;
                // 一个response中有多个dtsrc，每一个dtsrc中只有一个master adtsrc，且该adtsrc对应着多个dep adtsrc和对应dep adtsrc的相对路径
                void add_dtsrc(const Dtsrc &dtsrc_);
                const std::vector<Dtsrc> &get_dtsrcs() const;
                const Dtsrc *get_dtsrc(int pos) const;

            private:
                char *buf_;
                int size_;
                std::string src;
                std::string dest;
                std::string token;
                std::vector<Dtsrc> dtsrcs;
            };
        }
    }
}
