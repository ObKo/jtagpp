#ifndef JTAGPP_TAPCONTROLLER_H
#define JTAGPP_TAPCONTROLLER_H

#include <jtagpp/jtagpp.hpp>

#include <memory>
#include <string>

namespace jtagpp
{
class Interface;
class TAPController;

typedef std::shared_ptr<TAPController> TAPControllerPtr;

class TAPController: public std::enable_shared_from_this<TAPController>
{
    JTAGPP_BASE_CLASS_NOCOPY(TAPController)

public:
    enum State {S_UNKNOWN, S_TEST_LOGIC_RESET, S_IDLE, S_SELECT_DR_SCAN,
                S_CAPTURE_DR, S_SHIFT_DR, S_EXIT1_DR, S_PAUSE_DR, S_EXIT2_DR, 
                S_UPDATE_DR, S_SELECT_IR_SCAN, S_CAPTURE_IR, S_SHIFT_IR, 
                S_EXIT1_IR, S_PAUSE_IR, S_EXIT2_IR, S_UPDATE_IR};

    virtual ~TAPController();

    static TAPControllerPtr create(std::shared_ptr<Interface> interface);

    TAPControllerPtr pointer();
    
    State state() const;
    bool moveToState(State state);

    void shiftIR(const uint8_t *in, uint8_t *out, int bitlength, bool last);
    void shiftDR(const uint8_t *in, uint8_t *out, int bitlength, bool last);

protected:
    TAPController(std::shared_ptr<Interface> interface);
    TAPController(std::shared_ptr<Interface> interface, spimpl::unique_impl_ptr<TAPControllerPrivate>&& p);
};
}

#endif // JTAGPP_TAPCONTROLLER_H
