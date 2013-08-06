include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )

add_executable(VMEReadoutScaler_PairSpec src/VMEReadoutScaler_PairSpec.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
