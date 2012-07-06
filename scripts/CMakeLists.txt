INSTALL(PROGRAMS mhwd
  DESTINATION /var/lib/mhwd/scripts)

INSTALL(PROGRAMS mhwd-gpu
  DESTINATION /usr/bin)

INSTALL(PROGRAMS mhwd-kernel
  DESTINATION /usr/sbin)

FILE(GLOB INCLUDEFILES "include/*")

INSTALL(PROGRAMS ${INCLUDEFILES}
  DESTINATION /var/lib/mhwd/scripts/include)
