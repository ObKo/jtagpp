#include "jtagpp/Device.hpp"
#include "jtagpp/private/Device_p.hpp"

#include "jtagpp/DeviceDB.hpp"
#include "jtagpp/Log.hpp"

namespace jtagpp
{
Device::Device(const IDCode& id):
    _d(spimpl::make_unique_impl<DevicePrivate>())
{
    JTAGPP_D(Device);
    d->id = id;
}

Device::Device(const IDCode& id, spimpl::unique_impl_ptr<DevicePrivate>&& p):
    _d(std::forward<spimpl::unique_impl_ptr<DevicePrivate>>(p))
{
    JTAGPP_D(Device);
    d->id = id;
}

Device::~Device()
{
}

Device::DevicePrivate::~DevicePrivate()
{
}

DevicePtr Device::pointer()
{
    return shared_from_this();
}

DevicePtr Device::create(const IDCode& id)
{
    return std::shared_ptr<Device>(new Device(id));
}

int Device::irLength() const
{
    return 6;
}


Device::IDCode::IDCode()
{
    manufacturer = 0;
    partNumber = 0;
    version = 0;
}

Device::IDCode::IDCode(uint32_t jtag)
{
    manufacturer = (jtag >> 1) & 0x7FF;
    partNumber = (jtag >> 12) & 0xFFFF;
    version = (jtag >> 12) & 0xF;
}

uint32_t Device::IDCode::jtag() const
{
    return ((version & 0xF) << 28) | ((partNumber & 0xFFFF) << 12) |
           ((manufacturer & 0x7FF) << 1) | 1;
}

Log::Logger& operator <<(Log::Logger& l, const Device::IDCode& code)
{
    std::string vendor = DeviceDB::vendorName(code.manufacturer);

    if (vendor.empty())
        vendor = "Unknown";

    l << vendor << " 0x" << std::hex << code.partNumber << " 0x" << (int)code.version << std::dec;
    return l;
}
}
