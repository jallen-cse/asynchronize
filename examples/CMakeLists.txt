
# added by ../CMakeLists.txt

add_executable(jack_example_channel ex_channel.cpp)
add_executable(jack_example_broadcast_event ex_broadcast_event.cpp)
target_link_libraries(jack_example_channel PRIVATE asynchronize)
target_link_libraries(jack_example_broadcast_event PRIVATE asynchronize)