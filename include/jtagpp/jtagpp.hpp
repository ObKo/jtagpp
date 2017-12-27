#ifndef JTAGPP_JTAGPP_H
#define JTAGPP_JTAGPP_H

#include <jtagpp/spimpl.hpp>

#define JTAGPP_CLASS(Class) \
    protected: \
        class Class##Private;

#define JTAGPP_CLASS_NOCOPY(Class) \
    protected: \
        class Class##Private;

#define JTAGPP_BASE_CLASS(Class) \
    protected: \
        class Class##Private;\
        spimpl::impl_ptr<Class##Private> _d;

#define JTAGPP_BASE_CLASS_NOCOPY(Class) \
    protected: \
        class Class##Private;\
        spimpl::unique_impl_ptr<Class##Private> _d;

#define JTAGPP_D(Class) \
    Class##Private *d = static_cast<Class##Private*>(_d.get())

#endif // JTAGPP_JTAGPP_H
