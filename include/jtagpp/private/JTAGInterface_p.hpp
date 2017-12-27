#ifndef JTAGPP_JTAGINTERFACE_P_H
#define JTAGPP_JTAGINTERFACE_P_H

#include <jtagpp/JTAGInterface.hpp>
#include <vector>

namespace jtagpp
{
class JTAGInterface::JTAGInterfacePrivate
{
public:
    virtual ~JTAGInterfacePrivate();

    std::vector<std::string> parseConfig() const;

    std::string config;
    int frequency;
};
}

#endif // JTAGPP_JTAGINTERFACE_P_H
