SET(JTAGPP_HEADERS
    ../include/jtagpp/FTDIInterface.hpp
    ../include/jtagpp/Interface.hpp
    ../include/jtagpp/TAPController.hpp
    ../include/jtagpp/DeviceDB.hpp
    ../include/jtagpp/Device.hpp
    ../include/jtagpp/Chain.hpp
    ../include/jtagpp/XilinxFPGADevice.hpp
    ../include/jtagpp/jtagpp.hpp
    ../include/jtagpp/Log.hpp
    ../include/jtagpp/spimpl.hpp
)

SET(JTAGPP_PRIVATE_HEADERS
    ../include/jtagpp/private/FTDIInterface_p.hpp
    ../include/jtagpp/private/Interface_p.hpp
    ../include/jtagpp/private/TAPController_p.hpp
    ../include/jtagpp/private/Device_p.hpp
    ../include/jtagpp/private/Chain_p.hpp
    ../include/jtagpp/private/XilinxFPGADevice_p.hpp
)

SET(JTAGPP_SOURCES 
    FTDIInterface.cpp
    Interface.cpp
    TAPController.cpp
    JEP106.cpp
    Log.cpp
    Device.cpp
    Chain.cpp
    XilinxFPGADevice.cpp
)

ADD_LIBRARY(jtagpp ${JTAGPP_SOURCES} ${JTAGPP_HEADERS} ${JTAGPP_PRIVATE_HEADERS})
TARGET_LINK_LIBRARIES(jtagpp ${LIBFTDI_LIBRARIES})
SET_PROPERTY(TARGET jtagpp PROPERTY CXX_STANDARD 11)

IF(WIN32)
    TARGET_COMPILE_DEFINITIONS(jtagpp PRIVATE NOGDI)
ENDIF()
