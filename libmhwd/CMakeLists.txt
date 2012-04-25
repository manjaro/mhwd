include_directories (.)

set( HEADERS
    mhwd.h
    device.h
)

set( SOURCES
    mhwd.cpp
    device.cpp  
)

set( LIBS hd)


add_library (mhwd SHARED ${SOURCES} ${HEADERS})
target_link_libraries(mhwd ${LIBS})


add_subdirectory(test)
