#ifndef JTAGPP_DEVICE_H
#define JTAGPP_DEVICE_H

#include <jtagpp/jtagpp.hpp>
#include <jtagpp/Log.hpp>

#include <memory>

namespace jtagpp
{
class Device;

typedef std::shared_ptr<Device> DevicePtr;

class Device: public std::enable_shared_from_this<Device>
{
    JTAGPP_BASE_CLASS_NOCOPY(Device)

public:
    struct IDCode
    {
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
    
protected:
    Device(const IDCode& id);
    Device(const IDCode& id, spimpl::unique_impl_ptr<DevicePrivate>&& p);
};

Log::Logger& operator <<(Log::Logger& l, const Device::IDCode& code);
}


#endif // JTAGPP_DEVICE_H
