
#this will compile the current library.
add_library(PIPELINE SHARED Pipeline.cxx)

target_include_directories(PIPELINE PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)

target_link_libraries(PIPELINE PUBLIC ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(BETAINT PRIVATE PIPELINE)        

