cmake_minimum_required (VERSION 2.6)
project (ERPF)
add_definitions(-std=c++0x)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
find_package( OpenCV REQUIRED )
#FIND_PACKAGE( Boost COMPONENTS program_options REQUIRED )
FIND_PACKAGE( Boost COMPONENTS filesystem system REQUIRED )
#boost_filesystem-mt
file(GLOB SOURCES src/*.cpp src/*.h)
add_executable(ERPF ${SOURCES})

TARGET_LINK_LIBRARIES(ERPF ${Boost_LIBRARIES}  ${OpenCV_LIBS})
