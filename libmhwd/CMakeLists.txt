include_directories(. vita)

###
### mhwd library
###

set( LIB_HEADERS
    mhwd.h
    mhwd_p.h
    const.h
    vita/string.hpp
)

set( LIB_SOURCES
    mhwd.cpp
    vita/string.cpp
)

set( LIB_LIBS hd)


add_library (mhwd SHARED ${LIB_SOURCES} ${LIB_HEADERS})
target_link_libraries(mhwd ${LIB_LIBS})


INSTALL(TARGETS mhwd
  LIBRARY DESTINATION lib
)

INSTALL(FILES mhwd.h
  DESTINATION /usr/include)
