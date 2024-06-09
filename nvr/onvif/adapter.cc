#include "adapter.h"
#include <cstdint>
#include <tuple>
#include <vector>
namespace jxtd{
    namespace nvr{
        namespace onvif{
            std::vector<OnvifAdapter::OnvifDevice> OnvifAdapter::get_devices(){
                return this -> devices_list();
            }
            int32_t OnvifAdapter::ptz_device(
                const std::string& xaddr,
                bool absolute,
                const std::tuple<float, float, float>& geo,
                uint16_t timeout
            ){
                if(absolute)
                    return this -> absolute_move(xaddr, geo);
                return this -> continous_move(xaddr, geo, timeout);
            }
        }
    }
}
