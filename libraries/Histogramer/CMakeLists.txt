
#this will compile the current library.
add_library(HISTOGRAMER SHARED Histogramer.cxx)

target_include_directories(HISTOGRAMER PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)  

root_generate_dictionary(G__Histogramer 
  Histogramer.h 
  MODULE HISTOGRAMER 
  LINKDEF Linkdef.h
)

target_link_libraries(HISTOGRAMER PUBLIC ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(betasort PRIVATE HISTOGRAMER)        


