include_directories(src ${CMAKE_SOURCE_DIR}/vmebus/src )
add_executable(sadc_test src/sadc_test.cpp src/gesica_lib.cpp ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
add_executable(program_sadc src/program_sadc.cpp src/gesica_lib.cpp ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
add_executable(program_gesica src/program_gesica.cpp  src/gesica_lib.cpp ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
