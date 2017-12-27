FIND_PACKAGE(LibFTDI1 NO_MODULE REQUIRED)

INCLUDE(${LIBFTDI_USE_FILE})

SET(JTAGPP_HEADERS
    ../include/jtagpp/FTDIInterface.hpp
    ../include/jtagpp/JTAGInterface.hpp
    ../include/jtagpp/jtagpp.hpp
    ../include/jtagpp/Log.hpp
    ../include/jtagpp/spimpl.hpp
)

SET(JTAGPP_PRIVATE_HEADERS
    ../include/jtagpp/private/FTDIInterface_p.hpp
    ../include/jtagpp/private/JTAGInterface_p.hpp
)

SET(JTAGPP_SOURCES 
    FTDIInterface.cpp
    JTAGInterface.cpp
    Log.cpp
)

ADD_LIBRARY(jtagpp ${JTAGPP_SOURCES} ${JTAGPP_HEADERS} ${JTAGPP_PRIVATE_HEADERS})
TARGET_LINK_LIBRARIES(jtagpp ${LIBFTDI_LIBRARIES})
SET_PROPERTY(TARGET jtagpp PROPERTY CXX_STANDARD 11)
