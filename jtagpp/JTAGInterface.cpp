#include "jtagpp/JTAGInterface.hpp"
#include "jtagpp/private/JTAGInterface_p.hpp"

namespace jtagpp
{
JTAGInterface::JTAGInterface(const std::string& config):
    _d(spimpl::make_unique_impl<JTAGInterfacePrivate>())
{
    JTAGPP_D(JTAGInterface);
    d->config = config;
    d->frequency = 0;
}

JTAGInterface::JTAGInterface(const std::string& config, spimpl::unique_impl_ptr<JTAGInterfacePrivate>&& p):
    _d(std::forward<spimpl::unique_impl_ptr<JTAGInterfacePrivate>>(p))
{
    JTAGPP_D(JTAGInterface);
    d->config = config;
    d->frequency = 0;
}

JTAGInterface::~JTAGInterface()
{
}

JTAGInterface::JTAGInterfacePrivate::~JTAGInterfacePrivate()
{
}

JTAGInterfacePtr JTAGInterface::pointer()
{
    return shared_from_this();
}

std::vector<std::string> JTAGInterface::JTAGInterfacePrivate::parseConfig() const
{
    std::vector<std::string> result;

    int pos = 0;
    while (pos < (int)config.length())
    {
        int nextPos = config.find(':', pos);

        if (nextPos == (int)std::string::npos)
            nextPos = config.length();

        result.push_back(config.substr(pos, nextPos - pos));

        pos = nextPos + 1;
    }
    return result;
}


void JTAGInterface::setFrequency(int hz)
{
    JTAGPP_D(JTAGInterface);
    d->frequency = hz;
}

int JTAGInterface::frequency() const
{
    JTAGPP_D(const JTAGInterface);
    return d->frequency;
}

void JTAGInterface::throttleMsec(int msec)
{
    if (!msec)
        return;

    long bitcount = ((long)msec * (long)frequency() - 1) / 1000L + 1;
    throttle(bitcount);
}

void JTAGInterface::throttleUsec(int usec)
{
    if (!usec)
        return;

    long bitcount = ((long)usec * (long)frequency() - 1) / 1000000L + 1;
    throttle(bitcount);
}
}
