include_directories(. ../libmhwd ../libmhwd/vita)

###
### mhwd binary
###

set( HEADERS
	Config.hpp
	Data.hpp
	Device.hpp
	Enums.hpp
	Mhwd.hpp
	Printer.hpp
	Transaction.hpp
)

set( SOURCES
    Config.cpp
    Data.cpp
    Device.cpp
    main.cpp
    Mhwd.cpp
    Printer.cpp
    Transaction.cpp
)

set( LIBS mhwd)


add_executable(mhwd-bin ${SOURCES} ${HEADERS})
target_link_libraries(mhwd-bin ${LIBS})
set_target_properties(mhwd-bin PROPERTIES OUTPUT_NAME mhwd)


INSTALL(TARGETS mhwd-bin
  RUNTIME DESTINATION bin
)
