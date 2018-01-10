#ifndef JTAGPP_FTDIINTERFACE_H
#define JTAGPP_FTDIINTERFACE_H

#include <jtagpp/Interface.hpp>

namespace jtagpp
{
class FTDIInterface: public Interface
{
    JTAGPP_CLASS_NOCOPY(FTDIInterface)

public:
    virtual ~FTDIInterface();

    static InterfacePtr create(const std::string& config);

    virtual bool open();
    virtual bool isOpen() const;
    virtual void close();

    virtual int shift(const uint8_t *tdi, uint8_t *tdo, int bitlength, bool last = false);
    virtual int shiftTMS(const uint8_t *tms, int bitlength);
    virtual int cycle(int bitlength);
    virtual void flush();

protected:
    FTDIInterface(const std::string& config);
    FTDIInterface(const std::string& config, spimpl::unique_impl_ptr<InterfacePrivate>&& p);
};
}

#endif // JTAGPP_FTDIINTERFACE_H
