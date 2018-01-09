#ifndef JTAGPP_INTERFACE_P_H
#define JTAGPP_INTERFACE_P_H

#include <jtagpp/Interface.hpp>
#include <vector>

namespace jtagpp
{
class Interface::InterfacePrivate
{
public:
    virtual ~InterfacePrivate();

    std::vector<std::string> parseConfig() const;

    std::string config;
    int frequency;
};
}

#endif // JTAGPP_INTERFACE_P_H
