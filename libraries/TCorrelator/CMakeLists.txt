
#this will compile the current library.
add_library(TCORRELATOR SHARED TCorrelator.cxx)

target_include_directories(TCORRELATOR PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)  


target_link_libraries(TCORRELATOR PUBLIC ${ROOT_LIBRARIES} HISTOGRAMER TFDSI UNPACKER)

#link the compiled libraries above to the final executable.
#target_link_libraries(BETAINT PRIVATE TCORRELATOR)        

