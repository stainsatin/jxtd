#include "drccp13.pb.h"
#include "message.h"
#include <cerrno>
#include <misc/protocol/protocol.h>
#include <cstring>
namespace jxtd{
    namespace computation{
        namespace drccp{
            Message::Message():type(::jxtd::computation::drccp::Report::UNKNOWN),version(0),buf(nullptr),size(0){}
            Message::~Message(){
                if(buf)
                    delete[] buf;
                buf = nullptr;
            }
            Message::Message(const Message& other):
                cluster(other.cluster),
                msg(other.msg),
                src(other.src),
                dest(other.dest),
                type(other.type),
                version(other.version),
                raw(other.raw),
                shares(other.shares),
                size(other.size){
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
            Message::Message(Message&& other) noexcept:
                cluster(std::move(other.cluster)),
                msg(std::move(other.msg)),
                src(std::move(other.src)),
                dest(std::move(other.dest)),
                type(other.type),
                version(other.version),
                raw(std::move(other.raw)),
                shares(std::move(other.shares)),
                buf(other.buf),
                size(other.size){
                    other.buf = nullptr;
                    other.size = 0;
            }
            Message& Message::operator=(const Message& other){
                if(&other == this)
                    return *this;
                cluster = other.cluster;
                msg = other.msg;
                src = other.src;
                dest = other.dest;
                type = other.type;
                version = other.version;
                raw = other.raw;
                shares = other.shares;
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
                size = other.size;
                return *this;
            }
            Message& Message::operator=(Message&& other) noexcept{
                if(&other == this)
                    return *this;
                cluster = std::move(other.cluster);
                msg = std::move(other.msg);
                src = std::move(other.src);
                dest = std::move(other.dest);
                type = other.type;
                version = other.version;
                raw = std::move(other.raw);
                shares = std::move(other.shares);
                buf = other.buf;
                other.buf = nullptr;
                size = other.size;
                other.size = 0;
                return *this;
            }
            std::tuple<std::string, int> Message::getsrc() const{
                return src;
            }
            void Message::setsrc(const std::string& addr, int port){
                this -> src = {addr, port};
            }
            std::tuple<std::string, int> Message::getdest() const{
                return dest;
            }
            void Message::setdest(const std::string& addr, int port){
                this -> src = {addr, port};
            }
            std::tuple<std::string, std::string> Message::gettag() const{
                return {cluster, msg};
            }
            void Message::settag(const std::string& cluster, const std::string& tag){
                this -> cluster = cluster;
                this -> msg = tag;
            }
            int Message::gettype() const{
                return type;
            }
            void Message::settype(int type){
                this -> type = type;
            }
            int Message::getversion() const{
                return version;
            }
            void Message::setversion(int version){
                this -> version = version;
            }
            const std::string& Message::getraw() const{
                return raw;
            }
            void Message::setraw(const std::string& raw){
                this -> raw = raw;
            }
            const std::vector<std::tuple<std::tuple<std::string, int>, float>>&
            Message::getshares() const{
                return shares;
            }
            const std::tuple<std::tuple<std::string, int>, float>&
            Message::getshare(int i) const{
                return shares[i];
            }
            void Message::addshare(const std::tuple<std::tuple<std::string, int>, float>& share){
                shares.push_back(share);
            }
            void Message::setshare(const std::vector<std::tuple<std::tuple<std::string, int>, float>>& shares){
                this -> shares = shares;
            }
            int Message::encode(struct iovec vectors[], int max){
                ::jxtd::computation::drccp::Report report;
                report.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DRCCP>());
                report.set_version(3);
                report.set_type(static_cast<::jxtd::computation::drccp::Report_MessageType>(type));
                auto tag = report.mutable_tag();
                tag -> set_cluster(cluster);
                tag -> set_msg(msg);
                const auto& [srcaddr, srcport] = src;
                auto proto_src = report.mutable_src();
                proto_src -> set_addr(srcaddr);
                proto_src -> set_port(srcport);
                const auto& [destaddr, destport] = dest;
                auto proto_dest = report.mutable_dest();
                proto_dest -> set_addr(destaddr);
                proto_dest -> set_port(destport);
                auto payload = report.mutable_payload();
                payload -> set_raw(raw);
                for(const auto& [neigh, share]:shares){
                    const auto& [neighaddr, neighport] = neigh;
                    auto proto_share = payload -> add_shares();
                    auto proto_neigh = proto_share -> mutable_neigh();
                    proto_neigh -> set_addr(neighaddr);
                    proto_neigh -> set_port(neighport);
                    proto_share -> set_share(share);
                }
                size = report.ByteSizeLong();
                buf = new char[size];
                report.SerializeToArray(buf, size);
                vectors[0].iov_base = buf;
                vectors[0].iov_len = size;
                return 1;
            }
            int Message::append(const void* buf, size_t* size){
                if(!this->buf){
                    this->buf = new char[*size];
                    memmove(this -> buf, buf, *size);
                }
                else{
                    auto tmp = this -> buf;
                    this -> buf = new char[this -> size + *size];
                    memmove(this -> buf, tmp, this -> size);
                    memmove(this -> buf + this -> size, buf, *size);
                    delete[] tmp;
                }
                this -> size += *size;
                ::jxtd::computation::drccp::Report report;
                if(!report.ParseFromArray(this -> buf, this -> size))
                    return 0;
                delete[] this -> buf;
                this -> buf = nullptr;
                this -> size = 0;
                if(report.magic()!=::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::DRCCP>()){
                    errno = E_MAGIC_MISMATCH;
                    return -1;
                }
                const auto& proto_tag = report.tag();
                this -> cluster = proto_tag.cluster();
                this -> msg = proto_tag.msg();
                this -> version = report.version();
                this -> type = report.type();
                const auto& proto_src = report.src();
                this -> src = {proto_src.addr(), proto_src.port()};
                const auto& proto_dest = report.dest();
                this -> dest = {proto_dest.addr(), proto_dest.port()};
                const auto& payload = report.payload();
                this -> raw = payload.raw();
                for(int i = 0;i < payload.shares_size();i++){
                    const auto& share = payload.shares(i);
                    const auto& proto_neigh = share.neigh();
                    this -> shares.push_back({{proto_neigh.addr(), proto_neigh.port()}, share.share()});
                }
                return 1;
            }
        }
    }
}
