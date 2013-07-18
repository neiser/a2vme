# very simple executables
set(DEPS vmebus.h vmebus.c)
add_executable(vmestd16  vmestd16.c ${DEPS})
add_executable(vmestd32  vmestd32.c ${DEPS})
add_executable(vmeext    vmeext.c ${DEPS})
add_executable(eltec_ram eltec_ram.c ${DEPS})
add_executable(mem       mem.c ${DEPS})
