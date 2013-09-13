include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )
add_executable(vitec_test src/vitec_test.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
