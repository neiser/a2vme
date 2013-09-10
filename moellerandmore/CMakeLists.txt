include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )

add_executable(CheckRAM src/CheckRAM.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
add_executable(ConfigureRAM src/ConfigureRAM.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
add_executable(EmulateVMECPU src/EmulateVMECPU.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
