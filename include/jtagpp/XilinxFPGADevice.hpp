#ifndef JTAGPP_XILINXFPGADEVICE_H
#define JTAGPP_XILINXFPGADEVICE_H

#include <jtagpp/Device.hpp>

namespace jtagpp {
class XilinxFPGADevice : public Device {
    JTAGPP_CLASS_NOCOPY(XilinxFPGADevice)

public:
    enum Family { FAMILY_UNKNOWN, FAMILY_7SERIES, FAMILY_ULTRASCALE };

    virtual ~XilinxFPGADevice();

    static DevicePtr create(const IDCode& id);

    __uint128_t readDNA();

    Family family() const;

    void startProgram();
    void programData(uint8_t* bitdata, int bytelength, bool last);
    void endProgram();

protected:
    XilinxFPGADevice(const IDCode& id);
    XilinxFPGADevice(const IDCode& id, spimpl::unique_impl_ptr<DevicePrivate>&& p);
};
}

#endif // JTAGPP_XILINXFPGADEVICE_H
