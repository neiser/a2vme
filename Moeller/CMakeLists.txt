include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )

add_executable(MoellerVMEReadout src/MoellerVMEReadout.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
add_executable(MoellerVMEReadoutScaler src/MoellerVMEReadoutScaler.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
