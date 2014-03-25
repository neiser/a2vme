
# since this gesica lib is only used here,
# we make the macro a little bit different
add_library(gesica src/gesica.cpp)
include_directories(src)
macro(add_executable_vme_gesica NAME SRCS)
  add_executable_vme(${NAME} "${SRCS}")
  target_link_libraries(${NAME} gesica)
endmacro()


add_executable_vme_gesica(sadc_test src/sadc_test.cpp)
add_executable_vme_gesica(program_sadc src/program_sadc.cpp )
add_executable_vme_gesica(program_gesica src/program_gesica.cpp )
