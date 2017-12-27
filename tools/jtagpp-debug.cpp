#include <iostream>

#include <jtagpp/FTDIInterface.hpp>
#include <jtagpp/Log.hpp>

using namespace jtagpp;

int main()
{
    Log::instance().addHandler([=](const LogEntry& e) {
        auto& stream = (e.level == LogEntry::ERROR) ? std::cerr : std::cout;
        stream << "[" << e.module << "] " << e.text << std::endl;
    });

    JTAGInterfacePtr iface = FTDIInterface::create(std::string("0x0403:0x6014:Digilent USB Device:0:0xe8:0xeb:0x00:0x60"));

    iface->setFrequency(30000000);
    iface->open();

    uint8_t tapJump[2] = {0xDF, 0x00};
    uint8_t tapExit[1] = {0x01};
    uint8_t tapDR[1] = {0x01};
    uint8_t ir = 0x09;
    uint32_t idCode = 0;

    iface->shiftTMS(tapJump, 10);
    iface->shift(&ir, nullptr, 6, true);
    iface->shiftTMS(tapExit, 2);
    iface->shiftTMS(tapDR, 3);
    iface->shift(nullptr, (uint8_t*)&idCode, 32, true);
    iface->shiftTMS(tapExit, 10);

    return 0;
}
