#ifndef JTAGPP_FTDIINTERFACE_H
#define JTAGPP_FTDIINTERFACE_H

#include <jtagpp/JTAGInterface.hpp>

namespace jtagpp
{
class FTDIInterface: public JTAGInterface
{
    JTAGPP_CLASS_NOCOPY(FTDIInterface)

public:
    virtual ~FTDIInterface();

    static JTAGInterfacePtr create(const std::string& config);

    virtual bool open();
    virtual bool isOpen() const;
    virtual void close();

    virtual int shift(const uint8_t *tdi, uint8_t *tdo, int bitlength, bool last = false);
    virtual int shiftTMS(const uint8_t *tms, int bitlength);
    virtual int throttle(int bitlength);
    virtual void flush();

protected:
    FTDIInterface(const std::string& config);
    FTDIInterface(const std::string& config, spimpl::unique_impl_ptr<JTAGInterfacePrivate>&& p);
};
}

#endif // JTAGPP_FTDIINTERFACE_H
