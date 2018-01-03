SET(JTAGPP_HEADERS
    ../include/jtagpp/FTDIInterface.hpp
    ../include/jtagpp/JTAGInterface.hpp
    ../include/jtagpp/TAPController.hpp
    ../include/jtagpp/jtagpp.hpp
    ../include/jtagpp/Log.hpp
    ../include/jtagpp/spimpl.hpp
)

SET(JTAGPP_PRIVATE_HEADERS
    ../include/jtagpp/private/FTDIInterface_p.hpp
    ../include/jtagpp/private/JTAGInterface_p.hpp
    ../include/jtagpp/private/TAPController_p.hpp
)

SET(JTAGPP_SOURCES 
    FTDIInterface.cpp
    JTAGInterface.cpp
    TAPController.cpp
    Log.cpp
)

ADD_LIBRARY(jtagpp ${JTAGPP_SOURCES} ${JTAGPP_HEADERS} ${JTAGPP_PRIVATE_HEADERS})
TARGET_LINK_LIBRARIES(jtagpp ${LIBFTDI_LIBRARIES})
SET_PROPERTY(TARGET jtagpp PROPERTY CXX_STANDARD 11)

IF(WIN32)
    TARGET_COMPILE_DEFINITIONS(jtagpp PRIVATE NOGDI)
ENDIF()
