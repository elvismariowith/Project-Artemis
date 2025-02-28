find_package(Boost REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_executable(client /webserver/client.cpp)
target_link_libraries(client ${Boost_LIBRARIES})
target_link_libraries(client ${OpenCV_LIBS})