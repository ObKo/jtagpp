#ifndef JTAGPP_CHAIN_P_H
#define JTAGPP_CHAIN_P_H

#include <jtagpp/Chain.hpp>

namespace jtagpp
{
class Chain::ChainPrivate
{
public:
    virtual ~ChainPrivate();

    Chain *q;

    std::shared_ptr<TAPController> tap;
    std::shared_ptr<JTAGInterface> interface;

    std::vector<std::shared_ptr<Device>> devices;
    int currentDevice;

    void initChain();
};
}

#endif // JTAGPP_CHAIN_P_H
