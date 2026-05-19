cmake_minimum_required(VERSION 3.13)

#set(CMAKE_VERBOSE_MAKEFILE ON)

#project information
project(groot 
        VERSION 0.2.0 
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES CXX)

#check for various ROOT libraries
find_package(ROOT REQUIRED COMPONENTS RIO Tree) 

#set the compiler to whatever root was compiled against
set(CMAKE_CXX_STANDARD ${ROOT_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


#create the executable, we do it 'first' so the exectuable variable is avail in subdirs
add_executable(betasort src/betasort.cxx)


#setting include paths: TODO, copy headers from library directories(?)
target_include_directories(betasort PRIVATE ${CMAKE_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS}) 

target_link_libraries(betasort PRIVATE ${ROOT_LIBRARIES})


if(APPLE) 
  message("Building for Darwin")
  add_compile_definitions(__DARWIN__=TRUE)
elseif(LINUX) 
  message("Building for Linux")
  add_compile_definitions(__LINUX__=TRUE)
endif()

#set(CMAKE_BUILD_TYPE Debug)

#setting output paths in the build directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


#This tells cmake to continue into these directories.
add_subdirectory(libraries)


