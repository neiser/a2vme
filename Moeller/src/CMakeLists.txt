include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )

add_executable(MoellerVMEReadout MoellerVMEReadout.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
add_executable(MoellerVMEReadoutScaler MoellerVMEReadoutScaler.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
