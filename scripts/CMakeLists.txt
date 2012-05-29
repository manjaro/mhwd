INSTALL(PROGRAMS mhwd
  DESTINATION /var/lib/mhwd/scripts)

INSTALL(PROGRAMS mhwd-gpu
  DESTINATION /usr/bin)

FILE(GLOB INCLUDEFILES "include/*")

INSTALL(PROGRAMS ${INCLUDEFILES}
  DESTINATION /var/lib/mhwd/scripts/include)
