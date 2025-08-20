
#this will compile the current library.
add_library(UNPACKER SHARED Unpacker.cxx)
#target_link_libraries(UNPACKER ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(UNPACKER TFDSI)        
target_link_libraries(UNPACKER TANALYZER)        

target_link_libraries(BETAINT UNPACKER)        
