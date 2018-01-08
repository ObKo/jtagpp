#include <iostream>

#include <jtagpp/FTDIInterface.hpp>
#include <jtagpp/TAPController.hpp>
#include <jtagpp/Log.hpp>
#include <jtagpp/DeviceDB.hpp>
#include <jtagpp/Device.hpp>
#include <jtagpp/Chain.hpp>

#include <string>
#include <iomanip>

using namespace jtagpp;


int main()
{
    Log::instance().addHandler([=](const LogEntry& e) {
        auto& stream = (e.level == LogEntry::ERROR) ? std::cerr : std::cout;
        stream << "[" << e.module << "] " << e.text << std::endl;
    });

    JTAGInterfacePtr iface = FTDIInterface::create(std::string("0x0403:0x6010:Digilent USB Device:0:0xe8:0xeb:0x00:0x60"));

    iface->setFrequency(3000000);
    iface->open();

    ChainPtr chain = Chain::create(iface);
    chain->scan();

    return 0;
}
