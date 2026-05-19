
#this will compile the current library.
add_library(UNPACKER SHARED Unpacker.cxx)

target_include_directories(UNPACKER PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${ROOT_INCLUDE_DIRS}
)                    
          
#target_link_libraries(UNPACKER ${ROOT_LIBRARIES})

#link the compiled libraries above to the final executable.
target_link_libraries(UNPACKER PUBLIC TFDSI TANALYZER)        

target_link_libraries(BETAINT PRIVATE UNPACKER)        
