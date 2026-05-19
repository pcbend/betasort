
#this will compile the current library.
add_library(ROOTOUT SHARED TTreeOut.cxx)

target_include_directories(ROOTOUT PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)        

#root_generate_dictionary(G__Utilities Utilities.h MODULE UTILITIES LINKDEF Linkdef.h)
#target_link_libraries(UTILITIES ROOT::Core)

target_link_libraries(ROOTOUT PUBLIC ${ROOT_LIBRARIES} TCORRELATOR)

#link the compiled libraries above to the final executable.
target_link_libraries(betasort PRIVATE ROOTOUT)        

