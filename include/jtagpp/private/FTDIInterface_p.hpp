#ifndef JTAGPP_FTDIINTERFACE_P_H
#define JTAGPP_FTDIINTERFACE_P_H

#include "JTAGInterface_p.hpp"

#include <jtagpp/FTDIInterface.hpp>

extern "C" {
#include <ftdi.h>
}

namespace jtagpp
{
namespace {
    static const int TX_CHUNK_SIZE = 512;
}

class FTDIInterface::FTDIInterfacePrivate: public JTAGInterface::JTAGInterfacePrivate
{
public:
    FTDIInterfacePrivate();
    virtual ~FTDIInterfacePrivate();

    bool open();
    void close();

    bool mpsseCmd(const uint8_t *cmd, int size);
    bool mpsseFlush();
    bool mpsseResponse(uint8_t *data, int size);

    int ftdiRead(uint8_t *buf, int size);
    int ftdiWrite(uint8_t *buf, int size);

    int shift(const uint8_t *tdi, uint8_t *tdo, int bitlength, bool last);
    int shiftTMS(const uint8_t *tms, int bitlength);
    int throttle(int bitlength);

    struct ftdi_context *ftdi;

    uint8_t mpsseBuffer[TX_CHUNK_SIZE];
    int mpssePos;
};
}

#endif // JTAGPP_FTDIINTERFACE_P_H
