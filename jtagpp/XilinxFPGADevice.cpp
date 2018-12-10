#include "jtagpp/XilinxFPGADevice.hpp"
#include "jtagpp/private/XilinxFPGADevice_p.hpp"

namespace jtagpp
{
namespace {
    static const uint16_t XILINX_ID = 0x049;
    static const uint16_t XILINX_7SERIES_ID = 0x1B;

    static const uint8_t XILINX_OP_BYPASS = 0xFF;
    static const uint8_t XILINX_OP_ISC_ENABLE = 0x10;
    static const uint8_t XILINX_OP_ISC_DISABLE = 0x16;
    static const uint8_t XILINX_OP_XSC_DNA = 0x17;
    static const uint8_t XILINX_OP_FUSE_DNA = 0x32;
    static const uint8_t XILINX_OP_JPROGRAM = 0x0B;
    static const uint8_t XILINX_OP_JSHUTDOWN = 0x0D;
    static const uint8_t XILINX_OP_CFG_IN = 0x05;
    static const uint8_t XILINX_OP_JSTART = 0x0c;

    static const int XILINX_TCK_WAIT_CYCLES = 12;
    static const int XILINX_STARTUP_WAIT_CYCLES = 2000;
    static const int XILINX_JPROGRAM_WAIT_MS = 10;
}

XilinxFPGADevice::XilinxFPGADevice(const IDCode& id):
    Device(id, spimpl::unique_impl_ptr<DevicePrivate>(new XilinxFPGADevicePrivate(), &spimpl::details::default_delete<DevicePrivate>))
{
    JTAGPP_D(XilinxFPGADevice);
    d->init();
}

XilinxFPGADevice::XilinxFPGADevice(const IDCode& id, spimpl::unique_impl_ptr<DevicePrivate>&& p): 
    Device(id, std::forward<spimpl::unique_impl_ptr<DevicePrivate>>(p))
{
    JTAGPP_D(XilinxFPGADevice);
    d->init();
}

XilinxFPGADevice::~XilinxFPGADevice()
{
}

XilinxFPGADevice::XilinxFPGADevicePrivate::~XilinxFPGADevicePrivate()
{
}

DevicePtr XilinxFPGADevice::create(const IDCode& id)
{
    return std::shared_ptr<Device>(new XilinxFPGADevice(id));
}

void XilinxFPGADevice::XilinxFPGADevicePrivate::init()
{
    if (id.manufacturer != XILINX_ID)
    {
        Log::error("XilinxFPGADevice") << "Invalid manufacturer id: 0x" << std::hex << id.manufacturer << ", seems not a Xilinx device";
        supported = false;
    }
    else if (((id.partNumber >> 9) & 0x7F) != XILINX_7SERIES_ID)
    {
        Log::error("XilinxFPGADevice") << "Invalid Xilinx part 0x" << std::hex << ((id.partNumber >> 9) & 0x7F) << ", only 7 series FPGA supported";
        supported = false;
    }
    else
        supported = true;

}

uint64_t XilinxFPGADevice::readDNA()
{
    uint64_t dna = 0;

    shiftIR(&XILINX_OP_ISC_ENABLE, nullptr);
    cycle(XILINX_TCK_WAIT_CYCLES);

    shiftIR(&XILINX_OP_FUSE_DNA, nullptr);
    shiftDR(nullptr, (uint8_t*)&dna, 64, true, true);

    shiftIR(&XILINX_OP_ISC_DISABLE, nullptr);
    cycle(XILINX_TCK_WAIT_CYCLES);

    return dna;
}

void XilinxFPGADevice::startProgram()
{
    JTAGPP_D(XilinxFPGADevice);

    reset();

    shiftIR(&XILINX_OP_JPROGRAM);
    cycleMsec(XILINX_JPROGRAM_WAIT_MS);
    shiftIR(&XILINX_OP_JSHUTDOWN);
    cycleMsec(XILINX_TCK_WAIT_CYCLES);

    shiftIR(&XILINX_OP_CFG_IN);

    d->firstProgram = true;
}

void XilinxFPGADevice::programData(uint8_t *bitdata, int bytelength, bool last)
{
    JTAGPP_D(XilinxFPGADevice);
    shiftDR(bitdata, nullptr, bytelength * 8, d->firstProgram, last);
    d->firstProgram = false;
}

void XilinxFPGADevice::endProgram()
{
    shiftIR(&XILINX_OP_JSTART);
    reset();
    cycle(XILINX_STARTUP_WAIT_CYCLES);
}
}
