add_executable(jtagpp-debug jtagpp-debug.cpp)
target_link_libraries(jtagpp-debug PRIVATE jtagpp)
set_property(TARGET jtagpp PROPERTY CXX_STANDARD 11)
