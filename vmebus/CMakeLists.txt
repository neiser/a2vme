# very simple executables
set(DEPS src/vmebus.h src/vmebus.c)

add_executable(vmestd16  src/vmestd16.c ${DEPS})
add_executable(vmestd32  src/vmestd32.c ${DEPS})
add_executable(vmeext    src/vmeext.c ${DEPS})
add_executable(eltec_ram src/eltec_ram.c ${DEPS})
add_executable(mem       src/mem.c ${DEPS})
add_executable(vmesio    src/vmesio.c ${DEPS})

#install the scripts

file(GLOB SCRIPTS "scripts/*")
foreach(f ${SCRIPTS})
  get_filename_component(f_name ${f} NAME)
  configure_file(${f} "${EXECUTABLE_OUTPUT_PATH}/${f_name}" COPY_ONLY)
endforeach()

