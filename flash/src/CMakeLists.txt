# very simple executables
# they differ by some preprocessor variable RANGE_NUMBER
add_executable(vuprom  vuprom.c)
SET_TARGET_PROPERTIES(vuprom PROPERTIES COMPILE_DEFINITIONS "RANGENUMBER=0")

add_executable(vuprom2 vuprom.c)
SET_TARGET_PROPERTIES(vuprom2 PROPERTIES COMPILE_DEFINITIONS "RANGENUMBER=2")

add_executable(vuprom3 vuprom.c)
SET_TARGET_PROPERTIES(vuprom3 PROPERTIES COMPILE_DEFINITIONS "RANGENUMBER=4")
