
# added by ../CMakeLists.txt

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)   # chrono literals

# TODO other builds for sanitizers; maybe just cmd line for circle?
if(MSVC)
    set(CMAKE_CXX_FLAGS "/W4 /WX")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=thread")
else()
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread") 
endif()

add_executable(jack_example_channel ex_channel.cpp)
add_executable(jack_example_broadcast_event ex_broadcast_event.cpp)
target_link_libraries(jack_example_channel PRIVATE asynchronize)
target_link_libraries(jack_example_broadcast_event PRIVATE asynchronize)