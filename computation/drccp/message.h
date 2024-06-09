#pragma once
#include <workflow/ProtocolMessage.h>
#include <tuple>
#include <string>
#include <vector>
namespace jxtd{
    namespace computation{
        namespace drccp{
            class Message:public ::protocol::ProtocolMessage{
                public:
                    Message();
                    ~Message();
                    explicit Message(const Message& msg);
                    explicit Message(Message&& msg) noexcept;
                    Message& operator=(const Message& msg);
                    Message& operator=(Message&& msg) noexcept;
                    int encode(struct iovec vectors[], int max) override;
                    int append(const void* buf, size_t* size) override;
                    std::tuple<std::string, int> getsrc() const;
                    void setsrc(const std::string& addr, int port);
                    std::tuple<std::string, int> getdest() const;
                    void setdest(const std::string& addr, int port);
                    std::tuple<std::string, std::string>
                        gettag() const;
                    void settag(const std::string& cluster,
                                const std::string& msg);
                    int gettype() const;
                    void settype(int type);
                    int getversion() const;
                    void setversion(int version);
                    const std::string& getraw() const;
                    void setraw(const std::string& raw);
                    const std::vector<std::tuple<std::tuple<std::string, int>, float>>&
                        getshares() const;
                    const std::tuple<std::tuple<std::string, int>, float>&
                        getshare(int i) const;
                    void addshare(const std::tuple<std::tuple<std::string, int>, float>& share);
                    void setshare(const std::vector<std::tuple<std::tuple<std::string, int>, float>>& shares);
                    enum DRCCP_ERR{
                        E_MAGIC_MISMATCH = 257,
                    };
                private:
                    std::string cluster;
                    std::string msg;
                    std::tuple<std::string, int> src;
                    std::tuple<std::string, int> dest;
                    int type;
                    int version;
                    std::string raw;
                    std::vector<std::tuple<std::tuple<std::string, int>, float>> shares;
                    char* buf;
                    int size;
            };
        }
    }
}
