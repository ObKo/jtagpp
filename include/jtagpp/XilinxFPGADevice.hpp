#ifndef JTAGPP_XILINXFPGADEVICE_H
#define JTAGPP_XILINXFPGADEVICE_H

#include <jtagpp/Device.hpp>

namespace jtagpp {
class XilinxFPGADevice : public Device {
    JTAGPP_CLASS_NOCOPY(XilinxFPGADevice)

public:
    enum Family { FAMILY_UNKNOWN, FAMILY_7SERIES, FAMILY_ULTRASCALE };
    struct DNA {
        uint64_t low;
        uint32_t high;
    };

    virtual ~XilinxFPGADevice();

    static DevicePtr create(const IDCode& id);

    DNA readDNA();

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
