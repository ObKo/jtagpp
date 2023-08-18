#ifndef JTAGPP_DEVICE_P_H
#define JTAGPP_DEVICE_P_H

#include <jtagpp/Device.hpp>

namespace jtagpp {
class Device::DevicePrivate {
public:
    virtual ~DevicePrivate();

    Device::IDCode id;
    std::weak_ptr<Chain> chain;
    bool supported;
};
}

#endif // JTAGPP_DEVICE_P_H
