#include "jtagpp/Chain.hpp"
#include "jtagpp/private/Chain_p.hpp"

#include "jtagpp/Log.hpp"
#include "jtagpp/TAPController.hpp"
#include "jtagpp/Device.hpp"
#include "jtagpp/Interface.hpp"

#include <algorithm>
#include <cstring>

namespace jtagpp
{
namespace {
    static const int MAX_JTAG_DEVICES = 64;
    static const int DEFAULT_IR_WIDTH = 6;
}

Chain::Chain(std::shared_ptr<Interface> iface):
    _d(spimpl::make_unique_impl<ChainPrivate>())
{
    JTAGPP_D(Chain);
    d->q = this;
    d->interface = iface;
    d->tap = TAPController::create(iface);
    d->currentDevice = -1;
}

Chain::Chain(std::shared_ptr<Interface> iface, spimpl::unique_impl_ptr<ChainPrivate>&& p):
    _d(std::forward<spimpl::unique_impl_ptr<ChainPrivate>>(p))
{
    JTAGPP_D(Chain);
    d->q = this;
    d->interface = iface;
    d->tap = TAPController::create(iface);
    d->currentDevice = -1;
}

Chain::~Chain()
{
}

Chain::ChainPrivate::~ChainPrivate()
{
}

ChainPtr Chain::pointer()
{
    return shared_from_this();
}

ChainPtr Chain::create(std::shared_ptr<Interface> iface)
{
    return std::shared_ptr<Chain>(new Chain(iface));
}

std::shared_ptr<TAPController> Chain::tap() const
{
    JTAGPP_D(const Chain);
    return d->tap;
}

std::vector<Device::IDCode> Chain::scan()
{
    std::vector<Device::IDCode> result;

    std::shared_ptr<TAPController> t = tap();

    t->moveToState(TAPController::S_TEST_LOGIC_RESET);

    int chainIRWidth = MAX_JTAG_DEVICES * DEFAULT_IR_WIDTH / 8 + 1;
    uint8_t ones[chainIRWidth];
    int chainTestWidth = MAX_JTAG_DEVICES / 8 + 1;
    uint8_t zeroes[chainTestWidth];
    uint8_t onesOut[chainTestWidth];

    for (int i = 0; i < chainIRWidth; i++)
        ones[i] = 0xFF;

    for (int i = 0; i < chainTestWidth; i++)
        zeroes[i] = 0x00;

    // Shift BYPASS everywhere
    t->shiftIR(ones, nullptr, chainIRWidth * 8, true);
    t->shiftDR(zeroes, nullptr, chainTestWidth * 8, false);
    t->shiftDR(ones, onesOut, chainTestWidth * 8, true);

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
    uint32_t idCodes[length];
    // Fetch ID codes, every TAP should load IDCODE on Test-Logic-Reset
    t->moveToState(TAPController::S_TEST_LOGIC_RESET);
    t->shiftDR(nullptr, (uint8_t*)idCodes, length * 32, true);

    Log::info("Chain") << "Found " << length << " devices in chain: ";

    for (int i = 0; i < length; i++)
        Log::info("Chain") << "\t" << i << ": " << Device::IDCode(idCodes[i]);

    for (int i = length - 1; i >= 0; i--)
        result.push_back(Device::IDCode(idCodes[i]));

    return result;
}

std::vector<DevicePtr> Chain::devices() const
{
    JTAGPP_D(const Chain);
    return d->devices;
}

void Chain::addDevice(DevicePtr device)
{
    JTAGPP_D(Chain);

    if (!device)
        return;

    d->devices.push_back(device);
    device->setChain(pointer());
}

void Chain::removeDevice(DevicePtr device)
{
    JTAGPP_D(Chain);

    auto it = std::find(d->devices.begin(), d->devices.end(), device);

    if (it == d->devices.end())
    {
        Log::warning("Chain") << "removeDevice(): device isn't in chain";
        return;
    }

    (*it)->setChain(nullptr);

    DevicePtr cur = currentDevice();
    d->devices.erase(it);
    setCurrentDevice(cur);
}

void Chain::insertDevice(int i, DevicePtr device)
{
    JTAGPP_D(Chain);

    if (!device)
        return;

    if (i > d->devices.size())
        i = d->devices.size();

    if (i < 0)
        i = 0;

    d->devices.insert(d->devices.begin() + i, device);
    device->setChain(pointer());
}

DevicePtr Chain::currentDevice() const
{
    JTAGPP_D(const Chain);

    if ((d->currentDevice < 0) || (d->currentDevice >= d->devices.size()))
        return DevicePtr();

    return d->devices.at(d->currentDevice);
}

void Chain::setCurrentDevice(DevicePtr device)
{
    JTAGPP_D(Chain);

    if (!device)
    {
        d->currentDevice = -1;
        d->initChain();
        return;
    }

    if (currentDevice() == device)
        return;

    auto it = std::find(d->devices.begin(), d->devices.end(), device);

    if (it == d->devices.end())
    {
        Log::warning("Chain") << "setCurrentDevice(): device isn't in chain";
        return;
    }

    d->currentDevice = it - d->devices.begin();
    d->initChain();
}

void Chain::shiftIR(const uint8_t *in, uint8_t *out)
{
    JTAGPP_D(Chain);

    int totalIRLength = 0, totalIRLengthByte = 0;

    for (auto& dev : d->devices)
        totalIRLength += dev->irLength();

    totalIRLengthByte = (totalIRLength - 1) / 8 + 1;

    uint8_t irOut[totalIRLengthByte];
    uint8_t irIn[totalIRLengthByte];

    // Write BYPASS for all devices
    memset(irIn, 0xFF, totalIRLengthByte);

    // ... except current
    int bitp = 0;

    if (in)
    {
        for (int i = d->devices.size() - 1; i >= 0 ; i--)
        {
            if (i == d->currentDevice)
            {
                for (int j = 0; j < d->devices[i]->irLength(); j++)
                {
                    int outp = bitp + j;

                    irIn[outp / 8] &= ~(1 << (outp % 8));
                    if (in[j / 8] & (1 << (j % 8)))
                        irIn[outp / 8] |= (1 << (outp % 8));
                }
            }
            bitp += d->devices[i]->irLength();
        }
    }

    d->tap->shiftIR(irIn, irOut, totalIRLength, true);

    if (out)
    {
        bitp = 0;
        for (int i = d->devices.size() - 1; i >= 0 ; i--)
        {
            if (i == d->currentDevice)
            {
                for (int j = 0; j < d->devices[i]->irLength(); j++)
                {
                    int inp = bitp + j;

                    out[j / 8] &= ~(1 << (j % 8));
                    if (irOut[inp / 8] & (1 << (inp % 8)))
                        out[j / 8] |= (1 << (j % 8));
                }
            }
            bitp += d->devices[i]->irLength();
        }
    }
}

void Chain::shiftDR(const uint8_t *in, uint8_t *out, int bitlength, bool first, bool last)
{
    JTAGPP_D(Chain);

    int preBypassLength = 0, postBypassLength = 0;

    if (d->currentDevice >= 0)
    {
        preBypassLength = first ? (d->devices.size() - 1 - d->currentDevice) : 0;
        postBypassLength = last ? d->currentDevice : 0;
    }

    uint8_t pre[(preBypassLength - 1) / 8 + 1];
    uint8_t post[(postBypassLength - 1) / 8 + 1];

    memset(pre, 0x00, (preBypassLength - 1) / 8 + 1);
    memset(post, 0x00, (postBypassLength - 1) / 8 + 1);

    if (preBypassLength)
        d->tap->shiftDR(pre, nullptr, preBypassLength, false);

    d->tap->shiftDR(in, out, bitlength, postBypassLength ? false : last);

    if (postBypassLength)
        d->tap->shiftDR(pre, nullptr, postBypassLength, last);
}

void Chain::reset()
{
    JTAGPP_D(Chain);
    d->initChain();
}

void Chain::ChainPrivate::initChain()
{
    int irLength = 6;
    DevicePtr cd = q->currentDevice();

    if (cd)
        irLength = cd->irLength();

    uint8_t bp[(irLength - 1) / 8 + 1];

    memset(bp, 0xFF, (irLength - 1) / 8 + 1);

    tap->moveToState(TAPController::S_TEST_LOGIC_RESET);
    q->shiftIR(bp, nullptr);
}

void Chain::cycleMsec(int msec)
{
    JTAGPP_D(Chain);

    long bitcount = ((long)msec * (long)d->interface->frequency() - 1) / 1000L + 1;
    cycle(bitcount);
}

void Chain::cycleUsec(int usec)
{
    JTAGPP_D(Chain);

    long bitcount = ((long)usec * (long)d->interface->frequency() - 1) / 1000000L + 1;
    cycle(bitcount);
}

void Chain::cycle(int bitlength)
{
    JTAGPP_D(Chain);
    d->tap->cycle(bitlength);
}
}
