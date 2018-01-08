#ifndef JTAGPP_DEVICE_P_H
#define JTAGPP_DEVICE_P_H

#include <jtagpp/Device.hpp>

namespace jtagpp
{
class Device::DevicePrivate
{
public:
    virtual ~DevicePrivate();

    Device::IDCode id;
};
}

#endif // JTAGPP_DEVICE_P_H
