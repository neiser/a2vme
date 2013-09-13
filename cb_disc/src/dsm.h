// dsm.h: debug-support-macros, Dirk Krambrich, Okt. 2002.
//
//          This header-file is aimed to contain macros for debugging
//          purposes.
//
//        05. Mai 03 -dk- replaced 0 -> /* */ to suppress warnings
//        03. Jun 03 -dk- introduced "DEBUG_BREAK"
//
// You can preprocess your programms in different debuging levels: 
//     (-DDEBUG_LEVEL = x)
//
// BASIC = 1
// EXTRA = 2
// SUPER = 4
//
// or combine levels, eg.
//
// BASIC and SUPER == 0001 and 0100 == 5
//
// This will only paste debugging functions defined for the specified level
// in your code. 

// Examples:
//  int i = 10;
//  DEBUG_PRINT("in Line " << __LINE__ << ": i 0x" << hex << i << dec << " :" 
//       	<< i);
// will print something like
// DEBUG: dsl_tst.cc:  in Line 21: i 0xa :10
// to cerr.
//  if (DEBUG_LEVEL & BASIC_DEBUG) GIVE_COMPILE_INFO;
// will print compile-infos if BASIC_DEBUG is enabled.
//
// Macros defined in this header:
// DSM_h:                Identifier
// DEBUG_LEVEL:          Switch to determine what to put into the code
// BASIC_DEBUG  
// EXTRA_DEBUG:          The levels
// SUPER_DEBUG
// DEBUG_PRINT(TEXT):    Print info if LEVEL > 0
// DEBUG_PRINT_B(TEXT):  Print info in BASIC LEVEL
// DEBUG_PRINT_E(TEXT):  Print info in EXTRA LEVEL
// DEBUG_PRINT_S(TEXT):  Print info in SUPER LEVEL
// DEBUG_BREAK(TEXT):    Print info if LEVEL > 0   and wait for "RETURN"
// DEBUG_BREAK_B(TEXT):  Print info in BASIC LEVEL and wait for "RETURN"
// DEBUG_BREAK_E(TEXT):  Print info in EXTRA LEVEL and wait for "RETURN"
// DEBUG_BREAK_S(TEXT):  Print info in SUPER LEVEL and wait for "RETURN"
//
// GIVE_COMPILE_INFO:    Tell something about the currently executing code.
//

#ifndef DSM_h
#define DSM_h
#include <iostream>
#include <stdio.h>

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL  0
#endif

#define BASIC_DEBUG  1
#define EXTRA_DEBUG  2
#define SUPER_DEBUG  4

// Basic definitions:

#if (DEBUG_LEVEL > 0)
  #define DEBUG_PRINT(TEXT) (cerr << "DEBUG: " __FILE__ ":\t" << TEXT << endl)
#else
  #define DEBUG_PRINT(TEXT) /* */
#endif

#if (DEBUG_LEVEL > 0)
  #define DEBUG_BREAK(TEXT) ({cerr << "DEBUG: " __FILE__ ":\t" << TEXT << endl; \
			      cerr << "DEBUG: " __FILE__ ":\t--> press RETURN to continue <--" \
				   << endl; getchar();})
#else
  #define DEBUG_BREAK(TEXT) /* */
#endif

// Extended definitions:

#if (DEBUG_LEVEL & BASIC_DEBUG)
  #define DEBUG_PRINT_B(TEXT) DEBUG_PRINT(TEXT)
#else
  #define DEBUG_PRINT_B(TEXT) /* */
#endif

#if (DEBUG_LEVEL & EXTRA_DEBUG)
  #define DEBUG_PRINT_E(TEXT) DEBUG_PRINT(TEXT)
#else
  #define DEBUG_PRINT_E(TEXT) /* */
#endif

#if (DEBUG_LEVEL & SUPER_DEBUG)
  #define DEBUG_PRINT_S(TEXT) DEBUG_PRINT(TEXT)
#else
  #define DEBUG_PRINT_S(TEXT) /* */
#endif

// --

#if (DEBUG_LEVEL & BASIC_DEBUG)
  #define DEBUG_BREAK_B(TEXT) DEBUG_BREAK(TEXT)
#else
  #define DEBUG_BREAK_B(TEXT) /* */
#endif

#if (DEBUG_LEVEL & EXTRA_DEBUG)
  #define DEBUG_BREAK_E(TEXT) DEBUG_BREAK(TEXT)
#else
  #define DEBUG_BREAK_E(TEXT) /* */
#endif

#if (DEBUG_LEVEL & SUPER_DEBUG)
  #define DEBUG_BREAK_S(TEXT) DEBUG_BREAK(TEXT)
#else
  #define DEBUG_BREAK_S(TEXT) /* */
#endif

// Additional macros:

#define GIVE_COMPILE_INFO \
if (DEBUG_LEVEL) \
  (cerr << "Compile-Info: "  \
   << endl << "\tFile: " __FILE__  ", compiled: " __DATE__ " at " __TIME__ "." << endl \
   << "\tDebug Level: " << DEBUG_LEVEL << "." << endl)

#endif
