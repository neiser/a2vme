include_directories(src)

# very simple executables for VUPROM flashing
# they differ by some preprocessor variable RANGE_NUMBER
add_executable(vuprom  src/vuprom.c)
SET_TARGET_PROPERTIES(vuprom PROPERTIES COMPILE_DEFINITIONS "RANGENUMBER=0")

add_executable(vuprom2 src/vuprom.c)
SET_TARGET_PROPERTIES(vuprom2 PROPERTIES COMPILE_DEFINITIONS "RANGENUMBER=2")

add_executable(vuprom3 src/vuprom.c)
SET_TARGET_PROPERTIES(vuprom3 PROPERTIES COMPILE_DEFINITIONS "RANGENUMBER=4")

add_executable(vuprom4 src/vuprom.c)
SET_TARGET_PROPERTIES(vuprom4 PROPERTIES COMPILE_DEFINITIONS "RANGENUMBER=6")

# compile xsvf flasher, enable int main() via compile definition
add_executable(playxsvf src/playxsvf.c src/lenval.c src/ports.c)
SET_TARGET_PROPERTIES(playxsvf PROPERTIES COMPILE_DEFINITIONS "XSVF_MAIN")

# install the scripts

file(GLOB SCRIPTS "scripts/*")
foreach(f ${SCRIPTS})
  get_filename_component(f_name ${f} NAME)
  configure_file(${f} "${EXECUTABLE_OUTPUT_PATH}/${f_name}" COPY_ONLY)
endforeach()

