#include <iostream>

#include <jtagpp/FTDIInterface.hpp>
#include <jtagpp/TAPController.hpp>
#include <jtagpp/Log.hpp>
#include <jtagpp/DeviceDB.hpp>

#include <string>
#include <iomanip>

using namespace jtagpp;

static const int MAX_JTAG_DEVICES = 64;
static const int IR_WIDTH = 6;

int main()
{
    Log::instance().addHandler([=](const LogEntry& e) {
        auto& stream = (e.level == LogEntry::ERROR) ? std::cerr : std::cout;
        stream << "[" << e.module << "] " << e.text << std::endl;
    });

    JTAGInterfacePtr iface = FTDIInterface::create(std::string("0x0403:0x6010:Digilent USB Device:0:0xe8:0xeb:0x00:0x60"));

    iface->setFrequency(3000000);
    iface->open();

    TAPControllerPtr tap = TAPController::create(iface);

    int chainIRWidth = MAX_JTAG_DEVICES * IR_WIDTH / 8 + 1;
    uint8_t *ones = new uint8_t[chainIRWidth];
    int chainTestWidth = MAX_JTAG_DEVICES / 8 + 1;
    uint8_t *zeroes = new uint8_t[chainTestWidth];
    uint8_t *onesOut = new uint8_t[chainTestWidth];

    for (int i = 0; i < chainIRWidth; i++)
        ones[i] = 0xFF;

    for (int i = 0; i < chainTestWidth; i++)
        zeroes[i] = 0x00;

    // Shift BYPASS everywhere
    tap->shiftIR(ones, nullptr, chainIRWidth * 8, true);
    tap->shiftDR(zeroes, nullptr, chainTestWidth * 8, false);
    tap->shiftDR(ones, onesOut, chainTestWidth * 8, true);

    auto compareBits = [] (const uint8_t *a, uint8_t *b, int length, int loffset) -> bool
    {
        for (int i = 0; (i + loffset) < length; i++)
        {
            if (((a[i / 8] >> (i % 8)) & 0x1) != ((b[(i + loffset) / 8] >> ((i + loffset) % 8)) & 0x1))
                return false;
        }
        return true;
    };

    int length = 0;
    for (int i = 0; i < chainTestWidth * 8; i++)
    {
        if (compareBits(ones, onesOut, chainTestWidth * 8, i))
        {
            length = i;
            break;
        }
    }
    Log::info() << "Found " << length << " devices in chain: ";

    uint32_t idCodes[length];
    // Fetch ID codes, every TAP should load IDCODE on Test-Logic-Reset
    tap->moveToState(TAPController::S_TEST_LOGIC_RESET);
    tap->shiftDR(nullptr, (uint8_t*)idCodes, length * 32, true);

    for (int i = 0; i < length; i++)
    {
        std::string vendor = DeviceDB::vendorName((idCodes[i] >> 1) & 0x1FF);
        uint16_t device = (idCodes[i] >> 12) & 0xFFFF;
        uint16_t version = (idCodes[i] >> 28) & 0xF;

        if (vendor.empty())
            vendor = "Unknown";

        Log::info() << "\t" << i << ": " << vendor << " 0x" << std::hex << device << " 0x" << version;

    }

    return 0;
}
