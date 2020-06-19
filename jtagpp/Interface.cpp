#include "jtagpp/Interface.hpp"
#include "jtagpp/private/Interface_p.hpp"

namespace jtagpp {
Interface::Interface(const std::string& config)
    : _d(spimpl::make_unique_impl<InterfacePrivate>())
{
    JTAGPP_D(Interface);
    d->config = config;
    d->frequency = 0;
}

Interface::Interface(const std::string& config, spimpl::unique_impl_ptr<InterfacePrivate>&& p)
    : _d(std::forward<spimpl::unique_impl_ptr<InterfacePrivate>>(p))
{
    JTAGPP_D(Interface);
    d->config = config;
    d->frequency = 0;
}

Interface::~Interface() { }

Interface::InterfacePrivate::~InterfacePrivate() { }

InterfacePtr Interface::pointer() { return shared_from_this(); }

std::vector<std::string> Interface::InterfacePrivate::parseConfig() const
{
    std::vector<std::string> result;

    int pos = 0;
    while (pos < (int)config.length()) {
        int nextPos = config.find(':', pos);

        if (nextPos == (int)std::string::npos)
            nextPos = config.length();

        result.push_back(config.substr(pos, nextPos - pos));

        pos = nextPos + 1;
    }
    return result;
}

void Interface::setFrequency(int hz)
{
    JTAGPP_D(Interface);
    d->frequency = hz;
}

int Interface::frequency() const
{
    JTAGPP_D(const Interface);
    return d->frequency;
}
}
