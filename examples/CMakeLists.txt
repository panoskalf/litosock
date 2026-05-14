add_executable(showip showip.cpp)
add_executable(server server.cpp)


target_link_libraries(showip PRIVATE LitoSock)
target_link_libraries(server PRIVATE LitoSock)