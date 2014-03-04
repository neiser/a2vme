include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )
add_executable(sadc_test src/sadc_test.cpp ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
