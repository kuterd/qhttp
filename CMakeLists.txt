set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required (VERSION 3.10)
project (qHTTP)

find_package (Threads)

add_executable(qhttp main.c list.c eventloop.c http.c chunk.c chunk_builder.c debug.c)
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
target_link_libraries (qhttp ${CMAKE_THREAD_LIBS_INIT})

		      
