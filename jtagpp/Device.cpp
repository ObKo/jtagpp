#include "jtagpp/Device.hpp"
#include "jtagpp/private/Device_p.hpp"

#include "jtagpp/Chain.hpp"
#include "jtagpp/DeviceDB.hpp"
#include "jtagpp/Log.hpp"

namespace jtagpp {
Device::Device(const IDCode& id)
    : _d(spimpl::make_unique_impl<DevicePrivate>())
{
    JTAGPP_D(Device);
    d->id = id;
}

Device::Device(const IDCode& id, spimpl::unique_impl_ptr<DevicePrivate>&& p)
    : _d(std::forward<spimpl::unique_impl_ptr<DevicePrivate>>(p))
{
    JTAGPP_D(Device);
    d->id = id;
}

Device::~Device() { }

Device::DevicePrivate::~DevicePrivate() { }

DevicePtr Device::pointer() { return shared_from_this(); }

DevicePtr Device::create(const IDCode& id) { return std::shared_ptr<Device>(new Device(id)); }

int Device::irLength() const { return 6; }

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
    return ((version & 0xF) << 28) | ((partNumber & 0xFFFF) << 12) | ((manufacturer & 0x7FF) << 1)
        | 1;
}

std::ostream& operator<<(std::ostream& s, const Device::IDCode& code)
{
    std::string vendor = DeviceDB::vendorName(code.manufacturer);

    if (vendor.empty())
        vendor = "Unknown";

    s << vendor << " 0x" << std::hex << code.partNumber << " 0x" << (int)code.version << std::dec;
    return s;
}

std::shared_ptr<Chain> Device::chain() const
{
    JTAGPP_D(const Device);
    return d->chain.lock();
}

void Device::setChain(std::shared_ptr<Chain> chain)
{
    JTAGPP_D(Device);
    d->chain = chain;
}

void Device::shiftIR(const uint8_t* in, uint8_t* out)
{
    JTAGPP_D(Device);

    std::shared_ptr<Chain> chain = d->chain.lock();

    if (!chain) {
        Log::warning("Device") << "Cannot shift IR because device isn't assigned to any chain";
        return;
    }
    chain->setCurrentDevice(pointer());
    chain->shiftIR(in, out);
}

void Device::shiftDR(const uint8_t* in, uint8_t* out, int bitlength, bool first, bool last)
{
    JTAGPP_D(Device);

    std::shared_ptr<Chain> chain = d->chain.lock();

    if (!chain) {
        Log::warning("Device") << "Cannot shift DR because device isn't assigned to any chain";
        return;
    }
    chain->setCurrentDevice(pointer());
    chain->shiftDR(in, out, bitlength, first, last);
}

void Device::cycleMsec(int msec)
{
    JTAGPP_D(Device);

    std::shared_ptr<Chain> chain = d->chain.lock();

    if (!chain) {
        Log::warning("Device") << "Cannot cycle TMS because device isn't assigned to any chain";
        return;
    }
    chain->setCurrentDevice(pointer());
    chain->cycleMsec(msec);
}

void Device::cycleUsec(int usec)
{

    JTAGPP_D(Device);

    std::shared_ptr<Chain> chain = d->chain.lock();

    if (!chain) {
        Log::warning("Device") << "Cannot cycle TMS because device isn't assigned to any chain";
        return;
    }
    chain->setCurrentDevice(pointer());
    chain->cycleUsec(usec);
}

void Device::cycle(int bitlength)
{
    JTAGPP_D(Device);

    std::shared_ptr<Chain> chain = d->chain.lock();

    if (!chain) {
        Log::warning("Device") << "Cannot cycle TMS because device isn't assigned to any chain";
        return;
    }
    chain->setCurrentDevice(pointer());
    chain->cycle(bitlength);
}

void Device::reset()
{
    JTAGPP_D(Device);

    std::shared_ptr<Chain> chain = d->chain.lock();

    if (!chain) {
        Log::warning("Device") << "Cannot reset device because device isn't assigned to any chain";
        return;
    }
    chain->reset();
}
}
