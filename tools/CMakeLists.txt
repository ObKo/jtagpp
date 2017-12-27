ADD_EXECUTABLE(jtagpp-debug jtagpp-debug.cpp)
TARGET_LINK_LIBRARIES(jtagpp-debug jtagpp)
SET_PROPERTY(TARGET jtagpp PROPERTY CXX_STANDARD 11)
