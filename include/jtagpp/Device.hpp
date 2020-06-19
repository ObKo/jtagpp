#ifndef JTAGPP_DEVICE_H
#define JTAGPP_DEVICE_H

#include <jtagpp/Log.hpp>
#include <jtagpp/jtagpp.hpp>

#include <memory>

namespace jtagpp {
class Device;
class Chain;

typedef std::shared_ptr<Device> DevicePtr;

class Device : public std::enable_shared_from_this<Device> {
    JTAGPP_BASE_CLASS_NOCOPY(Device)

public:
    struct IDCode {
        IDCode();
        IDCode(uint32_t jtag);

        uint32_t jtag() const;

        uint16_t manufacturer;
        uint16_t partNumber;
        uint8_t version;
    };

    virtual ~Device();

    static DevicePtr create(const IDCode& id);

    DevicePtr pointer();

    virtual int irLength() const;

    std::shared_ptr<Chain> chain() const;
    void setChain(std::shared_ptr<Chain> chain);

    void reset();
    void shiftIR(const uint8_t* in, uint8_t* out = nullptr);
    void shiftDR(const uint8_t* in, uint8_t* out, int bitlength, bool first, bool last);
    void cycleMsec(int msec);
    void cycleUsec(int usec);
    void cycle(int bitlength);

protected:
    Device(const IDCode& id);
    Device(const IDCode& id, spimpl::unique_impl_ptr<DevicePrivate>&& p);
};

std::ostream& operator<<(std::ostream& s, const Device::IDCode& code);
}

#endif // JTAGPP_DEVICE_H
