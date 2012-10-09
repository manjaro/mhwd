INSTALL(PROGRAMS mhwd
  DESTINATION /var/lib/mhwd/scripts)

INSTALL(PROGRAMS mhwd-gpu
  DESTINATION /usr/bin)

INSTALL(PROGRAMS mhwd-kernel
  DESTINATION /usr/bin)

INSTALL(PROGRAMS mhwd-fb
  DESTINATION /usr/bin)

FILE(GLOB INCLUDEFILES "include/*")

INSTALL(PROGRAMS ${INCLUDEFILES}
  DESTINATION /var/lib/mhwd/scripts/include)

INSTALL(FILES "initcpio/hooks/mhwd-fb"
  DESTINATION /usr/lib/initcpio/hooks)

INSTALL(FILES "initcpio/install/mhwd-fb"
  DESTINATION /usr/lib/initcpio/install)
