include_directories( ${CMAKE_SOURCE_DIR}/vmebus/src )
add_executable(vitek_test  src/vitek_test.c ${CMAKE_SOURCE_DIR}/vmebus/src/vmebus.c)
