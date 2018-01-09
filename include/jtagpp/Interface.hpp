#ifndef JTAGPP_INTERFACE_H
#define JTAGPP_INTERFACE_H

#include <jtagpp/jtagpp.hpp>

#include <memory>
#include <string>

namespace jtagpp
{
class Interface;

typedef std::shared_ptr<Interface> InterfacePtr;

class Interface: public std::enable_shared_from_this<Interface>
{
    JTAGPP_BASE_CLASS_NOCOPY(Interface)

public:
    virtual ~Interface();

    InterfacePtr pointer();

    void setFrequency(int hz);

    int frequency() const;

    void throttleMsec(int msec);
    void throttleUsec(int usec);

    virtual bool open() = 0;
    virtual bool isOpen() const = 0;
    virtual void close() = 0;

    virtual int shift(const uint8_t *tdi, uint8_t *tdo, int bitlength, bool last = false) = 0;
    virtual int shiftTMS(const uint8_t *tms, int bitlength) = 0;
    virtual int throttle(int bitlength) = 0;
    virtual void flush() = 0;

protected:
    Interface(const std::string& config);
    Interface(const std::string& config, spimpl::unique_impl_ptr<InterfacePrivate>&& p);
};
}

#endif // JTAGPP_INTERFACE_H
