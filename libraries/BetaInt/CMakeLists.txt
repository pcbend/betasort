
#this will compile the current library.
add_library(BETAINT SHARED BetaInt.cxx)

target_include_directories(BETAINT PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)


root_generate_dictionary(G__BETAINT 
  ${CMAKE_SOURCE_DIR}/include/BetaInt.h 
  MODULE BETAINT 
  LINKDEF Linkdef.h
)

target_link_libraries(BETAINT PUBLIC 
  ${ROOT_LIBRARIES} 
  PIPELINE
)
#  ROOTOUT TFDSI)

#link the compiled libraries above to the final executable.
target_link_libraries(betasort PRIVATE BETAINT)        

