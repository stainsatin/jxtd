#pragma once
#include <workflow/ProtocolMessage.h>
#include <cstdint>
#include <string>
#include <vector>

namespace jxtd{
    namespace proto{
        namespace onvif{
            class Message:public ::protocol::ProtocolMessage{
                public:
                    struct Action{
                        std::string target;
                        std::string result;
                        bool ack;
                        bool scan;
                        bool streaming;
                        bool snap;
                        bool moving;
                        bool absmov;
                        float pan;
                        float tilt;
                        float zoom;
                        uint16_t timeout;
                    };
                    Message();
                    explicit Message(const Message& msg);
                    explicit Message(Message&& msg) noexcept;
                    Message& operator=(const Message& msg);
                    Message& operator=(Message&& msg) noexcept;
                    ~Message();
                    int encode(struct iovec vectors[], int max) override;
                    int append(const void* buf, size_t* size) override;
                    uint32_t get_transaction() const;
                    void set_transaction(uint32_t transaction);
                    uint32_t get_src() const;
                    void set_src(uint32_t src);
                    uint32_t get_dest() const;
                    void set_dest(uint32_t dest);
                    std::string get_token() const;
                    void set_token(const std::string& token);
                    std::vector<Action> get_actions() const;
                    Action get_action(int idx) const;
                    void set_actions(const std::vector<Action>& actions);
                    void add_action(const Action& action);
                    std::string get_ext() const;
                    void set_ext(const std::string& ext);
                private:
                    char* buf;
                    int buf_size;
                    uint32_t transaction;
                    uint32_t src;
                    uint32_t dest;
                    std::string token;
                    std::vector<Action> actions;
                    std::string ext;
            };
        }
    }
}
