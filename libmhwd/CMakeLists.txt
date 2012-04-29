include_directories(. vita)

set( HEADERS
    mhwd.h
    hwd.h
    const.h
    device.h
    config.h
    vita/config.hpp
    vita/string.hpp
)

set( SOURCES
    hwd.cpp
    device.cpp
    config.cpp
    vita/config.cpp
    vita/string.cpp
)

set( LIBS hd)


add_library (mhwd SHARED ${SOURCES} ${HEADERS})
target_link_libraries(mhwd ${LIBS})


add_subdirectory(test)
