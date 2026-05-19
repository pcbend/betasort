
#this will compile the current library.
add_library(DDASHIT SHARED ddasHit.cxx)

target_include_directories(DDASHIT PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)  

root_generate_dictionary(G__DDASHIT 
  ddasHit.h 
  MODULE DDASHIT 
  LINKDEF Linkdef.h
)

target_link_libraries(DDASHIT PUBLIC ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(TANALYZER PRIVATE DDASHIT)        

