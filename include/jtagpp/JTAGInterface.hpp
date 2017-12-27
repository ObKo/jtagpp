#ifndef JTAGPP_JTAGINTERFACE_H
#define JTAGPP_JTAGINTERFACE_H

#include <jtagpp/jtagpp.hpp>

#include <memory>
#include <string>

namespace jtagpp
{
class JTAGInterface;

typedef std::shared_ptr<JTAGInterface> JTAGInterfacePtr;

class JTAGInterface: public std::enable_shared_from_this<JTAGInterface>
{
    JTAGPP_BASE_CLASS_NOCOPY(JTAGInterface)

public:
    virtual ~JTAGInterface();

    JTAGInterfacePtr pointer();

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
    JTAGInterface(const std::string& config);
    JTAGInterface(const std::string& config, spimpl::unique_impl_ptr<JTAGInterfacePrivate>&& p);
};
}

#endif // JTAGPP_JTAGINTERFACE_H
