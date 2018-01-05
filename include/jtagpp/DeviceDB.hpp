#ifndef JTAGPP_DEVICEDB_H
#define JTAGPP_DEVICEDB_H

#include <jtagpp/jtagpp.hpp>

namespace jtagpp
{
class DeviceDB
{
public:
    static std::string vendorName(uint16_t jtagID);
};
}

#endif // JTAGPP_DEVICEDB_H
