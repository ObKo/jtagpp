#ifndef JTAGPP_TAPCONTROLLER_P_H
#define JTAGPP_TAPCONTROLLER_P_H

#include <jtagpp/TAPController.hpp>
#include <vector>

namespace jtagpp {
class TAPController::TAPControllerPrivate {
public:
    virtual ~TAPControllerPrivate();

    void walkToState(TAPController::State& current, const TAPController::State& target,
        uint32_t& tms, int& tmsLength);

    State state;
    std::shared_ptr<Interface> interface;
};
}

#endif // JTAGPP_TAPCONTROLLER_P_H
