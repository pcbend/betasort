
#this will compile the current library.
add_library(DDASHIT SHARED ddasHit.cxx)
root_generate_dictionary(G__DDASHIT ddasHit.h MODULE DDASHIT LINKDEF Linkdef.h)
target_link_libraries(DDASHIT ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(TANALYZER DDASHIT)        

