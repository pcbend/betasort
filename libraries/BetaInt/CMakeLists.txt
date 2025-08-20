
#this will compile the current library.
add_library(BETAINT SHARED BetaInt.cxx)
root_generate_dictionary(G__BETAINT BetaInt.h MODULE BETAINT LINKDEF Linkdef.h)
target_link_libraries(BETAINT ${ROOT_LIBRARIES} ROOTOUT TFDSI)

#target_link_libraries(BETAINT UNPACKER)        

#link the compiled libraries above to the final executable.
target_link_libraries(betasort BETAINT)        

