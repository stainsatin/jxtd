#pragma once
#include <cstdint>
namespace jxtd{
    namespace misc{
        namespace protocol{
            constexpr static int DRCCP = 0;
            constexpr static int ONVIFFORWARD = 1;
            constexpr static int DATATRANSFER = 2;
            constexpr static int TASKDEP = 3;
            constexpr static int DATASOURCE = 4;

            template <int _proto_idx>
            struct protocol_magic{
                constexpr static uint32_t magic = -1;
            };

#define register_protocol(idx, mag) \
            template <> \
            struct protocol_magic<idx>{ \
                constexpr static uint32_t magic = mag; \
            };

        register_protocol(DRCCP, 0x44524350);
        register_protocol(ONVIFFORWARD, 0x4f564653);
        register_protocol(DATATRANSFER, 0x44545452);
        register_protocol(TASKDEP, 0x544b4450);
        register_protocol(DATASOURCE, 0x64747372);
#undef register_protocol

            template <int _protocol>
            constexpr uint32_t get_magic(){
                return protocol_magic<_protocol>::magic;
            }
        }
    }
}
