#ifndef JTAGPP_XILINXFPGADEVICE_P_H
#define JTAGPP_XILINXFPGADEVICE_P_H

#include "Device_p.hpp"

#include <jtagpp/XilinxFPGADevice.hpp>

namespace jtagpp {
class XilinxFPGADevice::XilinxFPGADevicePrivate : public Device::DevicePrivate {
public:
    virtual ~XilinxFPGADevicePrivate();

    void init();

    bool firstProgram;
    Family family;
};
}

#endif // JTAGPP_XILINXFPGADEVICE_P_H
