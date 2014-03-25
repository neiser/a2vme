
# handy command for subsequent binaries
# which access via those VMEbus routines..
add_library(vmebus src/vmebus.c)
macro(add_executable_vme NAME SRCS)
  include_directories(${CMAKE_SOURCE_DIR}/vmebus/src)
  add_executable(${NAME} ${SRCS})
  target_link_libraries(${NAME} vmebus)
endmacro()

# very simple executables
# all depend on the vmebus library...
add_executable_vme(vmestd16  src/vmestd16.c)
add_executable_vme(vmestd32  src/vmestd32.c)
add_executable_vme(vmeext    src/vmeext.c)
add_executable_vme(vmesio    src/vmesio.c)

# install the scripts
install_scripts_as_links()
