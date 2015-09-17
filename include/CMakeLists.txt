file(GLOB headers "tthread/*.h")

install(FILES ${headers}
  DESTINATION include/tthread
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
