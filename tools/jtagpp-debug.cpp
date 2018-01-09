#include <iostream>

#include <jtagpp/FTDIInterface.hpp>
#include <jtagpp/TAPController.hpp>
#include <jtagpp/Log.hpp>
#include <jtagpp/Device.hpp>
#include <jtagpp/DeviceDB.hpp>
#include <jtagpp/Chain.hpp>

#include <string>
#include <iomanip>

using namespace jtagpp;


int main(int argc, char **argv)
{
    Log::instance().addHandler([=](const LogEntry& e) {
        auto& stream = (e.level == LogEntry::ERROR) ? std::cerr : std::cout;
        stream << "[" << e.module << "] " << e.text << std::endl;
    });

    std::string config;

    if (argc > 1)
        config = argv[1];

    JTAGInterfacePtr iface = FTDIInterface::create(config);

    iface->setFrequency(3000000);
    iface->open();

    ChainPtr chain = Chain::create(iface);
    std::vector<Device::IDCode> codes = chain->scan();

    for (const Device::IDCode& code : codes)
        chain->addDevice(Device::create(code));

    DevicePtr dev = chain->devices().at(0);

    uint8_t ir = 0x09;
    dev->shiftIR(&ir, nullptr);

    uint32_t idcode;
    dev->shiftDR(nullptr, (uint8_t*)&idcode, 32, true, true);
    Log::debug() << Device::IDCode(idcode);

    return 0;
}
