cmake_minimum_required( VERSION 2.8 )

project( proxy_cpp )
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++17 -g" )

add_executable( proxy src/proxy.cpp )
target_link_libraries( proxy boost_system pthread )
