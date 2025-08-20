
#this will compile the current library.
add_library(ROOTOUT SHARED TTreeOut.cxx)
#root_generate_dictionary(G__Utilities Utilities.h MODULE UTILITIES LINKDEF Linkdef.h)
#target_link_libraries(UTILITIES ROOT::Core)
target_link_libraries(ROOTOUT ${ROOT_LIBRARIES} TCORRELATOR)

#link the compiled libraries above to the final executable.
target_link_libraries(betasort ROOTOUT)        

