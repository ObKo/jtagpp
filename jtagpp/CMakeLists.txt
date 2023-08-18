set(JTAGPP_HEADERS
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

set(JTAGPP_PRIVATE_HEADERS
    ../include/jtagpp/private/FTDIInterface_p.hpp
    ../include/jtagpp/private/Interface_p.hpp
    ../include/jtagpp/private/TAPController_p.hpp
    ../include/jtagpp/private/Device_p.hpp
    ../include/jtagpp/private/Chain_p.hpp
    ../include/jtagpp/private/XilinxFPGADevice_p.hpp
)

set(JTAGPP_SOURCES 
    FTDIInterface.cpp
    Interface.cpp
    TAPController.cpp
    JEP106.cpp
    Log.cpp
    Device.cpp
    Chain.cpp
    XilinxFPGADevice.cpp
)

add_library(jtagpp SHARED ${JTAGPP_SOURCES} ${JTAGPP_HEADERS} ${JTAGPP_PRIVATE_HEADERS})
target_include_directories(jtagpp PUBLIC ${LIBFTDI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(jtagpp PRIVATE ${LIBFTDI_LIBRARIES})
target_link_directories(jtagpp PRIVATE ${LIBFTDI_LIBRARY_DIRS})
set_property(TARGET jtagpp PROPERTY CXX_STANDARD 11)

add_library(jtagpp_static STATIC ${JTAGPP_SOURCES} ${JTAGPP_HEADERS} ${JTAGPP_PRIVATE_HEADERS})
target_include_directories(jtagpp_static PUBLIC ${LIBFTDI_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_link_libraries(jtagpp_static PRIVATE ${LIBFTDI_LIBRARIES})
target_link_directories(jtagpp_static PRIVATE ${LIBFTDI_LIBRARY_DIRS})
set_property(TARGET jtagpp_static PROPERTY CXX_STANDARD 11)

if(WIN32)
    target_compile_definitions(jtagpp PRIVATE NOGDI ${LIBFTDI_DEFINITIONS})
    target_compile_definitions(jtagpp_static PRIVATE NOGDI ${LIBFTDI_DEFINITIONS})
else()
    target_compile_definitions(jtagpp PRIVATE ${LIBFTDI_DEFINITIONS})
    target_compile_definitions(jtagpp_static PRIVATE ${LIBFTDI_DEFINITIONS})
endif()
