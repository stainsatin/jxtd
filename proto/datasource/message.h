#pragma once
#include <workflow/ProtocolMessage.h>
#include "ProtocolMessage.pb.h"
#include <string>
#include <vector>

namespace jxtd{
    namespace proto{
        namespace datasource{
            using metaentry = ::jxtd::proto::datasource::metaentry;
            using request = ::jxtd::proto::datasource::request;
            using response = ::jxtd::proto::datasource::response;


            class Request : public ::protocol::ProtocolMessage{
            public:
                enum error_req_msg{
                    MAGIC_MISMATCH = 257
                };
                enum action{
                    Register = 0,
                    Unregister,
                    Offline,
                    Online
                };
                enum proto{
                    Http = 0,
                    Https,
                    ssh,
                    jxtd,
                    tcp
                };
                Request();
                ~Request();
                Request(const Request &other);
                Request(Request &&other) noexcept;
                Request &operator=(const Request &other);
                Request &operator=(Request &&other) noexcept;
                int encode(struct iovec vectors[], int max) override;
                int append(const void *buf, size_t *size) override;

                void set_token(const std::string &token);
                std::string get_token() const;
                void set_name(const std::string &name);
                std::string get_name() const;
                void set_action(uint32_t action);
                uint32_t get_action() const;
                void set_proto(uint32_t proto);
                uint32_t get_proto() const;
                void set_url(const std::string &url);
                std::string get_url() const;
                void set_port(uint32_t port);
                uint32_t get_port() const;
                std::map<std::string, std::string> get_allmetadata() const;
                std::string get_metadata(const std::string &key) const;
                void set_metadata(const std::map<std::string, std::string> &metadata);
                void add_metadata(const std::string &key,const std::string &value);

            private:
                char *buf_;
                int size_;
                request req_;
                std::map<std::string, std::string> myMetadata;
            };

            class Response : public ::protocol::ProtocolMessage{
            public:
                enum error_resp_msg{
                    MAGIC_MISMATCH = 257
                };
                enum status_msg{
                    Status_ok = 0,
                    Status_err = 1
                };
                Response();
                ~Response();
                Response(const Response &other);
                Response(Response &&other) noexcept;
                Response& operator=(const Response &other);
                Response& operator=(Response &&other) noexcept;
                int encode(struct iovec vectors[] , int max) override;
                int append(const void *buf , size_t *size) override;

                void set_token(const std::string &token);
                std::string get_token() const;
                void set_name(const std::string &name);
                std::string get_name() const;
                void set_status(uint32_t status);
                uint32_t get_status() const;

            private:
                char *buf_;
                int size_;
                response resp_;
            };
        }
    }
}
