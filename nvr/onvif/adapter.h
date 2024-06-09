#pragma once
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>
namespace jxtd{
    namespace nvr{
        namespace onvif{
            //adapter for actual onvif driver
            class OnvifAdapter{
                public:
                    struct OnvifDevice{
                        std::string xaddr;
                        int port;
                        std::string snapurl;
                        std::string streamurl;
                        std::tuple<float, float, float> geo;
                    };
                    OnvifAdapter() =default;
                    virtual ~OnvifAdapter() =default;
                    std::vector<OnvifDevice> get_devices();
                    int32_t ptz_device(const std::string& xaddr, bool absolute, const std::tuple<float, float, float>& geo, uint16_t timeout);
                private:
                    virtual std::vector<OnvifDevice> devices_list() =0;
                    virtual int32_t absolute_move(const std::string& xaddr, const std::tuple<float, float, float>& geo) =0;
                    virtual int32_t continous_move(const std::string& xaddr, const std::tuple<float, float, float>& geo_velocity, uint16_t timeout) =0;
            };
        }
    }
}
