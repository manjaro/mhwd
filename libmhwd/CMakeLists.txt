include_directories(. vita ${mhwd_SOURCE_DIR}/src)

###
### mhwd library
###

set( LIB_HEADERS
    const.h
    vita/string.hpp
)

set( LIB_SOURCES
    vita/string.cpp
)

set( LIB_LIBS hd)


add_library (mhwd SHARED ${LIB_SOURCES} ${LIB_HEADERS})
target_link_libraries(mhwd ${LIB_LIBS})


INSTALL(TARGETS mhwd
  LIBRARY DESTINATION lib
)
