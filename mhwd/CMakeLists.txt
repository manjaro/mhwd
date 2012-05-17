include_directories(. ../libmhwd ../libmhwd/vita)

###
### mhwd binary
###

set( HEADERS
)

set( SOURCES
    mhwd.cpp
)

set( LIBS mhwd)


add_executable(mhwd-bin ${SOURCES} ${HEADERS})
target_link_libraries(mhwd-bin ${LIBS})
set_target_properties(mhwd-bin PROPERTIES OUTPUT_NAME mhwd)


INSTALL(TARGETS mhwd-bin
  RUNTIME DESTINATION bin
)
