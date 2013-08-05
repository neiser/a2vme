include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )

add_executable(TAPSScalers src/TAPSScalers.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)

add_executable(TAPSVMEReadoutScaler src/TAPSVMEReadoutScaler.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)

add_executable(TAPSUserScaler src/TAPSUserScaler.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
