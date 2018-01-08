#include "jtagpp/Chain.hpp"
#include "jtagpp/private/Chain_p.hpp"

#include "jtagpp/Log.hpp"
#include "jtagpp/TAPController.hpp"
#include "jtagpp/Device.hpp"
#include "jtagpp/JTAGInterface.hpp"

namespace jtagpp
{
namespace {
    static const int MAX_JTAG_DEVICES = 64;
    static const int DEFAULT_IR_WIDTH = 6;
}

Chain::Chain(std::shared_ptr<JTAGInterface> iface):
    _d(spimpl::make_unique_impl<ChainPrivate>())
{
    JTAGPP_D(Chain);
    d->interface = iface;
    d->tap = TAPController::create(iface);
}

Chain::Chain(std::shared_ptr<JTAGInterface> iface, spimpl::unique_impl_ptr<ChainPrivate>&& p):
    _d(std::forward<spimpl::unique_impl_ptr<ChainPrivate>>(p))
{
    JTAGPP_D(Chain);
    d->interface = iface;
    d->tap = TAPController::create(iface);
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

ChainPtr Chain::create(std::shared_ptr<JTAGInterface> iface)
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
}
