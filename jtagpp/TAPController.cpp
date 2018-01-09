#include "jtagpp/TAPController.hpp"
#include "jtagpp/private/TAPController_p.hpp"

#include "jtagpp/Interface.hpp"
#include "jtagpp/Log.hpp"

#include <cstring>

namespace jtagpp
{
TAPController::TAPController(std::shared_ptr<Interface> interface):
    _d(spimpl::make_unique_impl<TAPControllerPrivate>())
{
    JTAGPP_D(TAPController);
    d->state = S_UNKNOWN;
    d->interface = interface;
}

TAPController::TAPController(std::shared_ptr<Interface> interface, spimpl::unique_impl_ptr<TAPControllerPrivate>&& p):
    _d(std::forward<spimpl::unique_impl_ptr<TAPControllerPrivate>>(p))
{
    JTAGPP_D(TAPController);
    d->state = S_UNKNOWN;
    d->interface = interface;
}

TAPController::~TAPController()
{
}

TAPController::TAPControllerPrivate::~TAPControllerPrivate()
{
}

TAPControllerPtr TAPController::create(std::shared_ptr<Interface> interface)
{
    return TAPControllerPtr(new TAPController(interface));
}

TAPControllerPtr TAPController::pointer()
{
    return shared_from_this();
}

TAPController::State TAPController::state() const
{
    JTAGPP_D(const TAPController);
    return d->state;
}

static void addBits(const char *bits, uint32_t &tms, int& tmsLength)
{
    int count = strlen(bits);
    for (int i = 0; i < count; i++)
    {
        if (bits[i] == '1')
            tms |= (1 << (tmsLength + i));
    }
    tmsLength += count;
}

void TAPController::TAPControllerPrivate::walkToState(
        TAPController::State& current,
        const TAPController::State& target,
        uint32_t& tms, int& tmsLength)
{
    if ((target == S_UNKNOWN) || (current == target))
        return;

    if (target == S_TEST_LOGIC_RESET)
    {
        addBits("11111", tms, tmsLength);
        current = S_TEST_LOGIC_RESET;
        return;
    }

    if (current == S_UNKNOWN)
        walkToState(current, S_TEST_LOGIC_RESET, tms, tmsLength);

    if (target == S_IDLE)
    {
        switch (current)
        {
        case S_TEST_LOGIC_RESET:
        case S_UPDATE_DR:
        case S_UPDATE_IR:
            addBits("0", tms, tmsLength);
            break;

        case S_SELECT_IR_SCAN:
        case S_EXIT1_DR:
        case S_EXIT1_IR:
        case S_EXIT2_DR:
        case S_EXIT2_IR:
            addBits("10", tms, tmsLength);
            break;

        case S_SELECT_DR_SCAN:
        case S_CAPTURE_DR:
        case S_CAPTURE_IR:
        case S_SHIFT_DR:
        case S_SHIFT_IR:
        case S_PAUSE_DR:
        case S_PAUSE_IR:
            addBits("110", tms, tmsLength);
            break;
        }
        current = S_IDLE;
    }
    else if (target == S_SELECT_DR_SCAN)
    {
        if ((current != S_UPDATE_DR) && (current != S_IDLE))
            walkToState(current, S_IDLE, tms, tmsLength);

        addBits("1", tms, tmsLength);
        current = target;
    }
    else if (target == S_SELECT_IR_SCAN)
    {
        if (current == S_UPDATE_IR)
            addBits("1", tms, tmsLength);
        else
        {
            walkToState(current, S_IDLE, tms, tmsLength);
            addBits("11", tms, tmsLength);
        }
        current = target;
    }
    else if ((target == S_CAPTURE_DR) || (target == S_CAPTURE_IR))
    {
        walkToState(current, (target == S_CAPTURE_DR) ? S_SELECT_DR_SCAN : S_SELECT_IR_SCAN, tms, tmsLength);
        addBits("0", tms, tmsLength);
        current = target;
    }
    else if ((target == S_SHIFT_DR) || (target == S_SHIFT_IR))
    {
        if (current != ((target == S_SHIFT_DR) ? S_EXIT2_DR : S_EXIT2_IR))
            walkToState(current, (target == S_SHIFT_DR) ? S_CAPTURE_DR : S_CAPTURE_IR, tms, tmsLength);
        addBits("0", tms, tmsLength);
        current = target;
    }
    else if ((target == S_EXIT1_DR) || (target == S_EXIT1_IR))
    {
        if (current != ((target == S_EXIT1_DR) ? S_SHIFT_DR : S_SHIFT_IR))
            walkToState(current, (target == S_EXIT1_DR) ? S_CAPTURE_DR : S_CAPTURE_IR, tms, tmsLength);
        addBits("1", tms, tmsLength);
        current = target;
    }
    else if ((target == S_PAUSE_DR) || (target == S_PAUSE_IR))
    {
        walkToState(current, (target == S_PAUSE_DR) ? S_EXIT1_DR : S_EXIT1_IR, tms, tmsLength);
        addBits("0", tms, tmsLength);
        current = target;
    }
    else if ((target == S_EXIT2_DR) || (target == S_EXIT2_IR))
    {
        walkToState(current, (target == S_EXIT2_DR) ? S_PAUSE_DR : S_PAUSE_IR, tms, tmsLength);
        addBits("1", tms, tmsLength);
        current = target;
    }
    else if ((target == S_UPDATE_DR) || (target == S_UPDATE_IR))
    {
        if (current != ((target == S_UPDATE_DR) ? S_EXIT2_DR : S_EXIT2_IR))
        {
            if (current == ((target == S_UPDATE_DR) ? S_PAUSE_DR : S_PAUSE_IR))
                walkToState(current, (target == S_UPDATE_DR) ? S_EXIT2_DR : S_EXIT2_IR, tms, tmsLength);
            else
                walkToState(current, (target == S_UPDATE_DR) ? S_EXIT1_DR : S_EXIT1_IR, tms, tmsLength);
        }
        addBits("1", tms, tmsLength);
        current = target;
    }
}

bool TAPController::moveToState(TAPController::State newState)
{
    JTAGPP_D(TAPController);

    TAPController::State current = d->state;
    uint32_t tms = 0;
    int len = 0;

    d->walkToState(current, newState, tms, len);

    std::string dbgString(len, '0');
    for (int i = 0; i < len; i++)
    {
        if (tms & (1 << i))
            dbgString[i] = '1';
    }
    Log::debug("TAPController") << "Move from " << d->state << " to " << newState << ": " << dbgString;

    if (current != newState)
    {
        Log::error("TAPController") << "Cannot find path in TAP FSM from " << d->state << " to " << newState;
        return false;
    }

    uint8_t tmsVector[4];
    tmsVector[0] = tms & 0xFF;
    tmsVector[1] = (tms >>  8) & 0xFF;
    tmsVector[2] = (tms >> 16) & 0xFF;
    tmsVector[3] = (tms >> 24) & 0xFF;

    if (d->interface->shiftTMS(tmsVector, len) != len)
    {
        Log::error("TAPController") << "TMS move failed";
        d->state = S_UNKNOWN;
        return false;
    }

    d->state = newState;
    return true;
}

void TAPController::shiftIR(const uint8_t *in, uint8_t *out, int bitlength, bool last)
{
    JTAGPP_D(TAPController);

    moveToState(S_SHIFT_IR);
    d->interface->shift(in, out, bitlength, last);

    if (last)
        d->state = S_EXIT1_IR;

    if (in)
    {
        std::string dbgString(bitlength, '0');
        for (int i = bitlength - 1; i >= 0; i--)
        {
            if (in[i / 8] & (1 << (i % 8)))
                dbgString[i] = '1';
        }
        Log::debug("TAPController") << "IR ->: " << dbgString;
    }
    if (out)
    {
        std::string dbgString(bitlength, '0');
        for (int i = bitlength - 1; i >= 0; i--)
        {
            if (out[i / 8] & (1 << (i % 8)))
                dbgString[i] = '1';
        }
        Log::debug("TAPController") << "IR <-: " << dbgString;
    }
}

void TAPController::shiftDR(const uint8_t *in, uint8_t *out, int bitlength, bool last)
{
    JTAGPP_D(TAPController);

    moveToState(S_SHIFT_DR);
    d->interface->shift(in, out, bitlength, last);

    if (last)
        d->state = S_EXIT1_DR;

    std::string dbgString(bitlength, '0');

    if (in)
    {
        for (int i = bitlength - 1; i >= 0; i--)
        {
            if (in[i / 8] & (1 << (i % 8)))
                dbgString[i] = '1';
        }
        Log::debug("TAPController") << "DR ->: " << dbgString;
    }
    if (out)
    {
        for (int i = bitlength - 1; i >= 0; i--)
        {
            if (out[i / 8] & (1 << (i % 8)))
                dbgString[i] = '1';
        }
        Log::debug("TAPController") << "DR <-: " << dbgString;
    }
}
}
