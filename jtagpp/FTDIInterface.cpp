#include "jtagpp/FTDIInterface.hpp"
#include "jtagpp/private/FTDIInterface_p.hpp"

#include "jtagpp/Log.hpp"

#include <chrono>

/*
 * Config string for FTDI:
 *   VID:PID:PRODDESC:INTERFACE:DBUS_DATA:DBUS_EN:CBUS_DATA:CBUS_EN
 */

namespace jtagpp {
namespace {
    static const uint16_t FTDI_DEFAULT_VID = 0x0403;
    static const uint16_t FTDI_DEFAULT_PID = 0x6010;
    static const std::string FTDI_DEFAULT_DESCRIPTION = std::string();
    static const uint8_t FTDI_DEFAULT_INTERFACE = 0;
    static const uint8_t FTDI_DEFAULT_DBUS = 0x00;
    static const uint8_t FTDI_DEFAULT_DBUS_EN = 0x0b;
    static const uint8_t FTDI_DEFAULT_CBUS = 0x00;
    static const uint8_t FTDI_DEFAULT_CBUS_EN = 0x00;
    static const int FTDI_DEFAULT_SPEED = 1500000;

    static const int FTDI_TIMEOUT_MS = 1000;
}

FTDIInterface::FTDIInterface(const std::string& config)
    : Interface(config,
        spimpl::unique_impl_ptr<InterfacePrivate>(
            new FTDIInterfacePrivate(), &spimpl::details::default_delete<InterfacePrivate>))
{
}

FTDIInterface::FTDIInterface(
    const std::string& config, spimpl::unique_impl_ptr<InterfacePrivate>&& p)
    : Interface(config, std::forward<spimpl::unique_impl_ptr<InterfacePrivate>>(p))
{
}

FTDIInterface::~FTDIInterface() { }

InterfacePtr FTDIInterface::create(const std::string& config)
{
    return InterfacePtr(new FTDIInterface(config));
}

bool FTDIInterface::open()
{
    JTAGPP_D(FTDIInterface);
    return d->open();
}

bool FTDIInterface::isOpen() const
{
    JTAGPP_D(const FTDIInterface);
    return d->ftdi != nullptr;
}

void FTDIInterface::close()
{
    JTAGPP_D(FTDIInterface);
    return d->close();
}

FTDIInterface::FTDIInterfacePrivate::FTDIInterfacePrivate()
    : InterfacePrivate()
    , mpssePos(0)
{
}

FTDIInterface::FTDIInterfacePrivate::~FTDIInterfacePrivate() { close(); }

static int str2int(const std::string& str, int def)
{
    int res = def;
    try {
        res = std::stoi(str, nullptr, 0);
    } catch (std::exception& e) {
        res = def;
    }
    return res;
}

bool FTDIInterface::FTDIInterfacePrivate::open()
{
    ftdi = ftdi_new();

    if (!ftdi)
        return false;

    auto fail = [this](const std::string& what) {
        if (ftdi)
            Log::error("FTDIInterface") << what << " failed: " << ftdi_get_error_string(ftdi);
        else
            Log::error("FTDIInterface") << what << " failed";

        if (ftdi)
            ftdi_free(ftdi);
        ftdi = nullptr;
    };

    uint16_t vid = FTDI_DEFAULT_VID;
    uint16_t pid = FTDI_DEFAULT_PID;
    std::string desc = FTDI_DEFAULT_DESCRIPTION;
    uint8_t iface = FTDI_DEFAULT_INTERFACE;
    uint8_t dbus = FTDI_DEFAULT_DBUS;
    uint8_t dbusEn = FTDI_DEFAULT_DBUS_EN;
    uint8_t cbus = FTDI_DEFAULT_CBUS;
    uint8_t cbusEn = FTDI_DEFAULT_CBUS_EN;
    int speedHz = frequency ? frequency : FTDI_DEFAULT_SPEED;

    std::vector<std::string> cfg = parseConfig();

    if ((cfg.size() > 0) && !cfg[0].empty())
        vid = str2int(cfg[0], vid);

    if ((cfg.size() > 1) && !cfg[1].empty())
        pid = str2int(cfg[1], pid);

    if ((cfg.size() > 2) && !cfg[2].empty())
        desc = cfg[2];

    if ((cfg.size() > 3) && !cfg[3].empty())
        iface = str2int(cfg[3], iface);

    if ((cfg.size() > 4) && !cfg[4].empty())
        dbus = str2int(cfg[4], dbus);

    if ((cfg.size() > 5) && !cfg[5].empty())
        dbusEn = str2int(cfg[5], dbusEn);

    if ((cfg.size() > 6) && !cfg[6].empty())
        cbus = str2int(cfg[6], cbus);

    if ((cfg.size() > 7) && !cfg[7].empty())
        cbusEn = str2int(cfg[7], cbusEn);

    if (ftdi_usb_open_desc_index(ftdi, vid, pid, desc.empty() ? nullptr : desc.c_str(), nullptr, 0) < 0) {
        fail("FDTI open");
        return false;
    }

    uint8_t tmp[5];

    if (ftdi_set_interface(ftdi, (enum ftdi_interface)iface)) {
        fail("ftdi_set_interface()");
        return false;
    }
    if (ftdi_set_bitmode(ftdi, 0x00, BITMODE_RESET)) {
        fail("ftdi_set_bitmode()");
        return false;
    }
    if (ftdi_usb_purge_buffers(ftdi)) {
        fail("ftdi_usb_purge_buffers()");
        return false;
    }
    if (ftdi_set_latency_timer(ftdi, 1)) {
        fail("ftdi_set_latency_timer()");
        return false;
    }
    if (ftdi_set_bitmode(ftdi, 0xfb, BITMODE_MPSSE)) {
        fail("ftdi_set_bitmode()");
        return false;
    }
    if (ftdi_write_data_set_chunksize(ftdi, TX_CHUNK_SIZE)) {
        fail("ftdi_write_data_set_chunksize()");
        return false;
    }
    ftdi_read_data(ftdi, tmp, 5);

    uint16_t div = 0;
    int baseClockHz;

    switch (ftdi->type) {
    case TYPE_2232H:
    case TYPE_4232H:
    case TYPE_232H:
        baseClockHz = 60000000;
        break;
    default:
        baseClockHz = 12000000;
        break;
    }

    if ((speedHz == 0) || (speedHz >= baseClockHz / 2))
        div = 0;
    else
        div = baseClockHz / 2 / speedHz - 1;

    frequency = baseClockHz / 2 / (1 + div);

    Log::info("FTDIInterface") << "Real TCK frequency: " << frequency / 1000 << " kHz";

    uint8_t setupCMDs[9] = { SET_BITS_LOW, dbus, dbusEn, TCK_DIVISOR, (uint8_t)(div & 0xFF),
        (uint8_t)((div >> 8) & 0xFF), SET_BITS_HIGH, cbus, cbusEn };

    mpsseCmd(setupCMDs, 9);
    mpsseFlush();

    return true;
}

void FTDIInterface::FTDIInterfacePrivate::close()
{
    if (ftdi) {
        ftdi_usb_close(ftdi);
        ftdi_free(ftdi);
        ftdi = nullptr;
    }
}

bool FTDIInterface::FTDIInterfacePrivate::mpsseCmd(const uint8_t* cmd, int size)
{
    bool good = true;
    int written = 0;

    if ((mpssePos + size) > TX_CHUNK_SIZE)
        good = mpsseFlush();

    while (good && (written < size)) {
        int chunkSize = std::min(size, TX_CHUNK_SIZE - mpssePos);
        std::copy(cmd, cmd + chunkSize, mpsseBuffer + mpssePos);
        mpssePos += chunkSize;
        written += chunkSize;

        if (mpssePos >= TX_CHUNK_SIZE)
            good = mpsseFlush();
    }

    return good;
}

bool FTDIInterface::FTDIInterfacePrivate::mpsseFlush()
{
    if (!mpssePos)
        return true;

    int result = ftdiWrite(mpsseBuffer, mpssePos);

    if (result != mpssePos) {
        Log::error("FTDIInterface")
            << "Error writing MPSSE buffer: " << ftdi_get_error_string(ftdi);
        return false;
    }

    mpssePos = 0;
    return true;
}

bool FTDIInterface::FTDIInterfacePrivate::mpsseResponse(uint8_t* data, int size)
{
    uint8_t buf = SEND_IMMEDIATE;
    mpsseCmd(&buf, 1);
    mpsseFlush();
    int result = ftdiRead(data, size);

    if (size != result)
        return false;
    else
        return true;
}

int FTDIInterface::FTDIInterfacePrivate::ftdiRead(uint8_t* buf, int size)
{
    if (!ftdi)
        return -1;

    auto start = std::chrono::high_resolution_clock::now();
    int written = 0;

    while (written < size) {
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
            > FTDI_TIMEOUT_MS) {
            Log::error("FTDIInterface") << "Read timeout";
            return written;
        }

        int res = ftdi_read_data(ftdi, buf + written, size - written);

        if (res < 0) {
            Log::error("FTDIInterface") << "Read error";
            return written ? written : res;
        }

        written += res;
    }
    return written;
}

int FTDIInterface::FTDIInterfacePrivate::ftdiWrite(uint8_t* buf, int size)
{
    if (!ftdi)
        return -1;

    return ftdi_write_data(ftdi, buf, size);
}

int FTDIInterface::FTDIInterfacePrivate::shift(
    const uint8_t* tdi, uint8_t* tdo, int bitlength, bool last)
{
    bool lastbit = false;

    if (tdi)
        lastbit = ((tdi[(bitlength - 1) / 8] >> ((bitlength - 1) % 8)) & 0x1) ? true : false;

    if (last)
        bitlength--;

    int bytelength = bitlength / 8;
    int rem = bitlength % 8;
    int chunkSize = TX_CHUNK_SIZE - 4;
    int chunkCount = bytelength ? ((bytelength - 1) / chunkSize + 1) : 0;
    const uint8_t* tdip = tdi;
    uint8_t* tdop = tdo;

    auto fail = [&](const std::string what) -> int {
        Log::error("FTDIInterface") << what << " failed at bit " << (tdip - tdi) * 8;
        return (tdip - tdi) * 8;
    };

    // Process complete bytes
    for (int i = 0; i < chunkCount; i++) {
        uint8_t cmd[3];

        int curSize = (i == chunkCount - 1) ? bytelength % chunkSize : chunkSize;

        cmd[0] = ((tdo) ? (MPSSE_DO_READ | MPSSE_READ_NEG) : 0) | ((tdi) ? MPSSE_DO_WRITE : 0)
            | MPSSE_LSB | MPSSE_WRITE_NEG;
        cmd[1] = (curSize - 1) & 0xff;
        cmd[2] = ((curSize - 1) >> 8) & 0xff;

        if (!mpsseCmd(cmd, 3))
            return fail("MPSSE write");

        if (tdi) {
            if (!mpsseCmd(tdip, curSize))
                return fail("MPSSE write");
        }

        if (tdo) {
            if (!mpsseResponse(tdop, curSize))
                return fail("MPSSE read");
        }

        if (tdi)
            tdip += curSize;
        if (tdo)
            tdop += curSize;
    }

    // Process last byte
    if (rem) {
        uint8_t cmd[2];

        cmd[0] = ((tdo) ? (MPSSE_DO_READ | MPSSE_READ_NEG) : 0) | ((tdi) ? MPSSE_DO_WRITE : 0)
            | MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG;
        cmd[1] = rem - 1;

        if (!mpsseCmd(cmd, 2))
            return fail("MPSSE write");

        if (tdi) {
            if (!mpsseCmd(tdip, 1))
                return fail("MPSSE write");
            tdip++;
        }
    }

    // If last - send last bit and assert TMS
    if (last) {
        uint8_t cmd[3];

        cmd[0] = MPSSE_WRITE_TMS | ((tdo) ? (MPSSE_DO_READ | MPSSE_READ_NEG) : 0) | MPSSE_LSB
            | MPSSE_BITMODE | MPSSE_WRITE_NEG;
        cmd[1] = 0;
        cmd[2] = lastbit ? 0x81 : 1;
        mpsseCmd(cmd, 3);
    }

    // A bit tangled here because using single read for last two commands
    if (tdo) {
        if (last) {
            uint8_t tmp[2] = { 0, 0 };
            if (rem) {
                if (!mpsseResponse(tmp, 2))
                    fail("MPSSE read");
            } else {
                if (!mpsseResponse(tmp + 1, 1))
                    fail("MPSSE read");
            }
            *tdop = (tmp[0] >> (8 - rem)) | ((tmp[1] & 0x80) >> (7 - rem));
            tdop++;
        } else if (rem) {
            if (!mpsseResponse(tdop, 1))
                fail("MPSSE read");
            tdop++;
        }
    }
    return bitlength;
}

int FTDIInterface::FTDIInterfacePrivate::shiftTMS(const uint8_t* tms, int bitlength)
{
    unsigned char cmd[3] = { MPSSE_WRITE_TMS | MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG, 0, 0 };

    int len = bitlength;

    if (!len)
        return 0;

    int j = 0;

    while (len > 0) {
        /* Attention: Bug in FT2232L(D?, H not!).
           With 7 bits TMS shift, static TDO
           value gets set to TMS on last TCK edge*/

        uint8_t bl = (len > 6) ? 6 : len;

        cmd[1] = bl - 1;
        cmd[2] = 0x80;

        for (int i = 0; i < bl; i++) {
            cmd[2] |= (((tms[j / 8] & (1 << (j % 8))) ? 1 : 0) << i);
            j++;
        }

        if (!mpsseCmd(cmd, 3)) {
            Log::error("FTDIInterface") << "MPSSE TMS write failed at bit " << bitlength - len;
            return bitlength - len;
        }
        len -= bl;
    }

    return bitlength;
}

int FTDIInterface::FTDIInterfacePrivate::throttle(int bitlength)
{
    bool hasClockThrottle;

    switch (ftdi->type) {
    case TYPE_2232H:
    case TYPE_4232H:
    case TYPE_232H:
        hasClockThrottle = true;
        break;
    default:
        hasClockThrottle = false;
        break;
    }

    int bytelength = bitlength / 8;
    int rem = bitlength % 8;
    int chunkSize = hasClockThrottle ? 65536 : (TX_CHUNK_SIZE - 4);
    int chunkCount = bytelength ? ((bytelength - 1) / chunkSize + 1) : 0;

    if (hasClockThrottle) {
        unsigned char cmd[3] = { 0x8f };

        for (int i = 0; i < chunkCount; i++) {
            int curSize = ((i == (chunkCount - 1)) ? (bytelength % chunkSize) : chunkSize) - 1;
            cmd[1] = curSize & 0xff;
            cmd[2] = (curSize >> 8) & 0xff;
            mpsseCmd(cmd, 3);
        }
        cmd[0] = 0x8e;
        cmd[1] = rem ? rem - 1 : 0;
        mpsseCmd(cmd, 2);
    } else {
        unsigned char cmd[3];

        for (int i = 0; i < chunkCount; i++) {
            uint8_t dummy = 0;
            int curSize = (i == (chunkCount - 1)) ? (bytelength % chunkSize) : chunkSize;

            cmd[0] = MPSSE_DO_WRITE | MPSSE_LSB | MPSSE_WRITE_NEG;
            cmd[1] = curSize & 0xff;
            cmd[2] = (curSize >> 8) & 0xff;

            mpsseCmd(cmd, 3);
            for (int j = 0; j < curSize; j++)
                mpsseCmd(&dummy, 1);
        }
        cmd[0] = MPSSE_DO_WRITE | MPSSE_LSB | MPSSE_BITMODE | MPSSE_WRITE_NEG;
        cmd[1] = rem - 1;
        cmd[2] = 0;
        mpsseCmd(cmd, 3);
    }
    mpsseFlush();

    return bitlength;
}

int FTDIInterface::shift(const uint8_t* tdi, uint8_t* tdo, int bitlength, bool last)
{
    if (!isOpen())
        return 0;

    JTAGPP_D(FTDIInterface);
    return d->shift(tdi, tdo, bitlength, last);
}

int FTDIInterface::shiftTMS(const uint8_t* tms, int bitlength)
{
    if (!isOpen())
        return 0;

    JTAGPP_D(FTDIInterface);
    return d->shiftTMS(tms, bitlength);
}

int FTDIInterface::cycle(int bitlength)
{
    if (!isOpen())
        return 0;

    JTAGPP_D(FTDIInterface);
    return d->throttle(bitlength);
}

void FTDIInterface::flush()
{
    if (!isOpen())
        return;

    JTAGPP_D(FTDIInterface);
    d->mpsseFlush();
}
}
