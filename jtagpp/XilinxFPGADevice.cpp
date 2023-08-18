#include "jtagpp/XilinxFPGADevice.hpp"
#include "jtagpp/private/XilinxFPGADevice_p.hpp"

namespace jtagpp {
namespace {
    static const uint16_t XILINX_ID = 0x049;
    static const uint16_t XILINX_7SERIES_ID = 0x1B;
    static const uint16_t XILINX_ULTRASCALE_ID = 0x1C;
    static const uint16_t XILINX_ULTRASCALE_P_ID = 0x25;

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
    static const int XILINX_7_JPROGRAM_WAIT_MS = 10;
    static const int XILINX_US_CFG_IN_WAIT_MS = 20;
}

XilinxFPGADevice::XilinxFPGADevice(const IDCode& id)
    : Device(id,
        spimpl::unique_impl_ptr<DevicePrivate>(
            new XilinxFPGADevicePrivate(), &spimpl::details::default_delete<DevicePrivate>))
{
    JTAGPP_D(XilinxFPGADevice);
    d->init();
}

XilinxFPGADevice::XilinxFPGADevice(const IDCode& id, spimpl::unique_impl_ptr<DevicePrivate>&& p)
    : Device(id, std::forward<spimpl::unique_impl_ptr<DevicePrivate>>(p))
{
    JTAGPP_D(XilinxFPGADevice);
    d->init();
}

XilinxFPGADevice::~XilinxFPGADevice() { }

XilinxFPGADevice::XilinxFPGADevicePrivate::~XilinxFPGADevicePrivate() { }

DevicePtr XilinxFPGADevice::create(const IDCode& id)
{
    return std::shared_ptr<Device>(new XilinxFPGADevice(id));
}

void XilinxFPGADevice::XilinxFPGADevicePrivate::init()
{
    family = FAMILY_UNKNOWN;
    supported = false;
    if (id.manufacturer != XILINX_ID) {
        Log::error("XilinxFPGADevice") << "Invalid manufacturer id: 0x" << std::hex
                                       << id.manufacturer << ", seems not a Xilinx device";
    } else if (((id.partNumber >> 9) & 0x7F) == XILINX_7SERIES_ID) {
        family = FAMILY_7SERIES;
        supported = true;
    } else if (((id.partNumber >> 9) & 0x7F) == XILINX_ULTRASCALE_ID) {
        family = FAMILY_ULTRASCALE;
        supported = true;
    } else if (((id.partNumber >> 9) & 0x7F) == XILINX_ULTRASCALE_P_ID) {
        family = FAMILY_ULTRASCALE_P;
        supported = true;
    } else {
        Log::error("XilinxFPGADevice")
            << "Invalid Xilinx part 0x" << std::hex << ((id.partNumber >> 9) & 0x7F)
            << ", only some 7 series and UltraScale(+) FPGA supported";
    }
}

XilinxFPGADevice::DNA XilinxFPGADevice::readDNA()
{
    DNA dna = { 0, 0 };

    // shiftIR(&XILINX_OP_ISC_ENABLE, nullptr);
    // cycle(XILINX_TCK_WAIT_CYCLES);

    shiftIR(&XILINX_OP_FUSE_DNA, nullptr);
    if (family() == FAMILY_7SERIES)
        shiftDR(nullptr, (uint8_t*)&dna, 64, true, true);
    else
        shiftDR(nullptr, (uint8_t*)&dna, 96, true, true);

    // shiftIR(&XILINX_OP_ISC_DISABLE, nullptr);
    // cycle(XILINX_TCK_WAIT_CYCLES);

    return dna;
}

void XilinxFPGADevice::startProgram()
{
    JTAGPP_D(XilinxFPGADevice);

    reset();

    shiftIR(&XILINX_OP_JPROGRAM);
    if (d->family == FAMILY_7SERIES) {
        shiftIR(&XILINX_OP_JPROGRAM);
        shiftIR(&XILINX_OP_BYPASS);
        cycleMsec(XILINX_7_JPROGRAM_WAIT_MS);
    }

    shiftIR(&XILINX_OP_CFG_IN);
    if ((d->family == FAMILY_ULTRASCALE) || (d->family == FAMILY_ULTRASCALE_P))
        cycleMsec(XILINX_US_CFG_IN_WAIT_MS);

    d->firstProgram = true;
}

XilinxFPGADevice::Family XilinxFPGADevice::family() const
{
    JTAGPP_D(const XilinxFPGADevice);
    return d->family;
}

void XilinxFPGADevice::programData(uint8_t* bitdata, int bytelength, bool last)
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
