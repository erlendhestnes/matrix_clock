// File containing User defined definitions such as configuring the si114x_init file for an indoors or general mode
#ifndef INDOORS
#ifndef GENERAL

// If either INDOORS or GENERAL is defined, it must have been defined via a command-line switch. In this case,
// this code is never reached and the command line switch takes precedence.
//
// However, if neither INDOORS nor GENERAL is defined, then we need to define it here. 
//
// Use #define GENERAL to configure slider for general use mode
// Use #define INDOORS to configure slider for indoors and long range use
//
// Do not set both GENERAL and INDDORS at the same time.
//
#define INDOORS

#endif
#endif