
#this will compile the current library.
add_library(TANALYZER SHARED TAnalyzer.cxx)

target_include_directories(TANALYZER PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)     

root_generate_dictionary(G__TANALYZER 
  TAnalyzer.h 
  MODULE TANALYZER 
  LINKDEF Linkdef.h
)

target_link_libraries(TANALYZER PUBLIC ${ROOT_LIBRARIES} HISTOGRAMER)

#link the compiled libraries above to the final executable.
target_link_libraries(BETAINT PRIVATE TANALYZER)        

