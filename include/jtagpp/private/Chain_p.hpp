#ifndef JTAGPP_CHAIN_P_H
#define JTAGPP_CHAIN_P_H

#include <jtagpp/Chain.hpp>

namespace jtagpp
{
class Chain::ChainPrivate
{
public:
    virtual ~ChainPrivate();

    std::shared_ptr<TAPController> tap;
    std::shared_ptr<JTAGInterface> interface;
};
}

#endif // JTAGPP_CHAIN_P_H
