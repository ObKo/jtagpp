#include <iostream>

#include <jtagpp/FTDIInterface.hpp>
#include <jtagpp/TAPController.hpp>
#include <jtagpp/Log.hpp>

using namespace jtagpp;

int main()
{
    Log::instance().addHandler([=](const LogEntry& e) {
        auto& stream = (e.level == LogEntry::ERROR) ? std::cerr : std::cout;
        stream << "[" << e.module << "] " << e.text << std::endl;
    });

    JTAGInterfacePtr iface = FTDIInterface::create(std::string("0x0403:0x6014:Digilent USB Device:0:0xe8:0xeb:0x00:0x60"));

    iface->setFrequency(3000000);
    iface->open();

    TAPControllerPtr tap = TAPController::create(iface);

    uint8_t ir = 0x09;
    uint32_t idCode = 0;

    tap->shiftIR(&ir, nullptr, 6, true);
    tap->shiftDR(nullptr, (uint8_t*)&idCode, 32, true);

    return 0;
}
