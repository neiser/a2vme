include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )

add_executable(SCVMEReadoutScaler_moeller src/SCVMEReadoutScaler_moeller.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
