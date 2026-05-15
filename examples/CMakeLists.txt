add_executable(showip showip.cpp)
add_executable(server server.cpp)
add_executable(client client.cpp)
add_executable(listener listener.cpp)
add_executable(talker talker.cpp)


target_link_libraries(showip PRIVATE LitoSock)
target_link_libraries(server PRIVATE LitoSock)
target_link_libraries(client PRIVATE LitoSock)
target_link_libraries(listener PRIVATE LitoSock)
target_link_libraries(talker PRIVATE LitoSock)