include_directories(. vita)

set( HEADERS
    mhwd.h
    mhwd_p.h
    const.h
    vita/string.hpp
)

set( SOURCES
    mhwd.cpp
    vita/string.cpp
)

set( LIBS hd)


add_library (mhwd SHARED ${SOURCES} ${HEADERS})
target_link_libraries(mhwd ${LIBS})


add_subdirectory(test)
