
#this will compile the current library.
add_library(TCORRELATOR SHARED TCorrelator.cxx)

target_link_libraries(TCORRELATOR ${ROOT_LIBRARIES} HISTOGRAMER)
target_link_libraries(TCORRELATOR TFDSI UNPACKER)

#link the compiled libraries above to the final executable.
target_link_libraries(BETAINT TCORRELATOR)        

