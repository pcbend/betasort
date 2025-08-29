
#this will compile the current library.
add_library(TFDSI SHARED TFDSi.cxx TPSPMT.cxx TPID.cxx TPIN.cxx TClover.cxx TClock.cxx TOFCorrector.cxx TVandle.cxx)
root_generate_dictionary(G__TFDSI TFDSi.h TPSPMT.h TPID.h TPIN.h TClover.h TClock.h TOFCorrector.h TVandle.h MODULE TFDSI LINKDEF Linkdef.h)
target_link_libraries(TFDSI ${ROOT_LIBRARIES} ROOT::Spectrum HISTOGRAMER)

#link the compiled libraries above to the final executable.
target_link_libraries(BETAINT TANALYZER TFDSI)        

