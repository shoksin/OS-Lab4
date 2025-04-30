add_library(common STATIC common.cpp common.h)

add_executable(receiver receiver.cpp)
target_link_libraries(receiver common)

add_executable(sender sender.cpp)
target_link_libraries(sender common)