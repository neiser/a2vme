include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )

add_executable(SCVMEReadoutScaler_taps  src/SCVMEReadoutScaler_taps.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
