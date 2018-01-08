#ifndef JTAGPP_CHAIN_H
#define JTAGPP_CHAIN_H

#include <jtagpp/jtagpp.hpp>
#include <jtagpp/Device.hpp>

#include <memory>
#include <vector>

namespace jtagpp
{
class JTAGInterface;
class TAPController;
class Chain;

typedef std::shared_ptr<Chain> ChainPtr;

class Chain: public std::enable_shared_from_this<Chain>
{
    JTAGPP_BASE_CLASS_NOCOPY(Chain)

public:
    virtual ~Chain();

    static ChainPtr create(std::shared_ptr<JTAGInterface> iface);

    ChainPtr pointer();

    std::shared_ptr<TAPController> tap() const;

    std::vector<Device::IDCode> scan();

protected:
    Chain(std::shared_ptr<JTAGInterface> iface);
    Chain(std::shared_ptr<JTAGInterface> iface, spimpl::unique_impl_ptr<ChainPrivate>&& p);
};
}

#endif // JTAGPP_CHAIN_H