INSTALL(PROGRAMS mhwd
  DESTINATION /var/lib/mhwd/scripts)

FILE(GLOB INCLUDEFILES "include/*")

INSTALL(PROGRAMS ${INCLUDEFILES}
  DESTINATION /var/lib/mhwd/scripts/include)
