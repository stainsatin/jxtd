#include "message.h"
#include "jxtd.proto.onvif.pb.h"
#include <cstdint>
#include <misc/protocol/protocol.h>
namespace jxtd{
    namespace proto{
        namespace onvif{
            Message::Message()
                :buf(nullptr), buf_size(0),
                transaction(0), src(0), dest(0),
                token(), actions(), ext(){}
            Message::Message(const Message& msg)
                :buf(new char[msg.buf_size]),
                buf_size(msg.buf_size),
                transaction(msg.transaction),
                src(msg.src), dest(msg.dest),
                token(msg.token), actions(msg.actions),
                ext(msg.ext)
            {
                    memcpy(this-> buf, msg.buf, msg.buf_size);
            }
            Message::Message(Message&& msg) noexcept
                :buf(msg.buf),
                buf_size(msg.buf_size),
                transaction(msg.transaction),
                src(msg.src), dest(msg.dest),
                token(msg.token), actions(msg.actions),
                ext(msg.ext)
            {
                msg.buf = nullptr;
                msg.buf_size = 0;
            }
            Message& Message::operator=(const Message& msg){
                if(&msg == this)
                    return *this;
                this -> buf = new char[msg.buf_size];
                this -> buf_size = msg.buf_size;
                this -> transaction = msg.transaction;
                this -> src = msg.src;
                this -> dest = msg.dest;
                this -> token = msg.token;
                this -> actions = msg.actions;
                this -> ext = msg.ext;
                memcpy(this -> buf, msg.buf, msg.buf_size);
                return *this;
            }
            Message& Message::operator=(Message&& msg) noexcept{
                if(&msg == this)
                    return *this;
                this -> buf = msg.buf;
                this -> buf_size = msg.buf_size;
                this -> transaction = msg.transaction;
                this -> src = msg.src;
                this -> dest = msg.dest;
                this -> token = msg.token;
                this -> actions = msg.actions;
                this -> ext = msg.ext;
                msg.buf = nullptr;
                msg.buf_size = 0;
                return *this;
            }
            Message::~Message(){
                if(this -> buf)
                    delete buf;
            }
            int Message::encode(struct iovec vectors[], int max){
                ::jxtd::proto::onvif::OnvifMessage msg;
                msg.set_magic(::jxtd::misc::protocol::get_magic<::jxtd::misc::protocol::ONVIFFORWARD>());
                msg.set_transactionid(this -> transaction);
                msg.set_srcuid(this -> src);
                msg.set_destuid(this -> dest);
                msg.set_token(this -> token);
                msg.set_extension(this -> ext);
                for(const auto& action:this -> actions){
                    auto proto_action = msg.add_actions();
                    proto_action -> set_ipc(action.target);
                    proto_action -> set_result(action.result);
                    proto_action -> set_ack(action.ack);
                    proto_action -> set_scan(action.scan);
                    proto_action -> set_streaming(action.streaming);
                    proto_action -> set_snap(action.snap);
                    proto_action -> set_move(action.moving);
                    proto_action -> set_absolute(action.absmov);
                    proto_action -> set_pan(action.pan * 1e5);
                    proto_action -> set_tilt(action.tilt * 1e5);
                    proto_action -> set_zoom(action.zoom * 1e5);
                    proto_action -> set_timeout(action.timeout);
                }
                this -> buf_size = msg.ByteSizeLong();
                this -> buf = new char[this -> buf_size];
                msg.SerializeToArray(this -> buf, this -> buf_size);
                vectors[0].iov_base = this -> buf;
                vectors[0].iov_len = this -> buf_size;
                return 1;
            }
            int Message::append(const void* buf, size_t* size){
                if(!this -> buf){
                    this -> buf = new char[*size];
                    memmove(this -> buf, buf, *size);
                }else{
                    auto tmp = this -> buf;
                    this -> buf = new char[this -> buf_size + *size];
                    memmove(this -> buf, tmp, this -> buf_size);
                    memmove(this -> buf + this -> buf_size, buf, *size);
                    delete[] tmp;
                }
                this -> buf_size += *size;
                ::jxtd::proto::onvif::OnvifMessage msg;
                if(!msg.ParseFromArray(this -> buf, this -> buf_size))
                    return 0;
                delete[] this -> buf;
                this -> buf = nullptr;
                this -> buf_size = 0;

                this -> transaction = msg.transactionid();
                this -> src = msg.srcuid();
                this -> dest = msg.destuid();
                this -> token = msg.token();
                this -> ext = msg.extension();
                for(auto i = 0;i < msg.actions_size();i++){
                    const auto& proto_action = msg.actions(i);
                    this -> actions.push_back({
                        proto_action.ipc(),
                        proto_action.result(),
                        proto_action.ack(),
                        proto_action.scan(),
                        proto_action.streaming(),
                        proto_action.snap(),
                        proto_action.move(),
                        proto_action.absolute(),
                        proto_action.pan() / 1e5,
                        proto_action.tilt() / 1e5,
                        proto_action.zoom() / 1e5,
                        proto_action.timeout()
                    });
                }
                return 1;
            }
            uint32_t Message::get_transaction() const{
                return this -> transaction;
            }
            void Message::set_transaction(uint32_t transaction){
                this -> transaction = transaction;
            }
            uint32_t Message::get_src() const{
                return this -> src;
            }
            void Message::set_src(uint32_t src){
                this -> src = src;
            }
            uint32_t Message::get_dest() const{
                return this -> dest;
            }
            void Message::set_dest(uint32_t dest){
                this -> dest = dest;
            }
            std::string Message::get_token() const{
                return this -> token;
            }
            void Message::set_token(const std::string& token){
                this -> token = token;
            }
            std::vector<Message::Action> Message::get_actions() const{
                return this -> actions;
            }
            Message::Action Message::get_action(int idx) const{
                return this -> actions[idx];
            }
            void Message::set_actions(const std::vector<Message::Action>& actions){
                this -> actions = actions;
            }
            void Message::add_action(const Message::Action& action){
                this -> actions.push_back(action);
            }
            std::string Message::get_ext() const{
                return this -> ext;
            }
            void Message::set_ext(const std::string& ext){
                this -> ext = ext;
            }
        }
    }
}
