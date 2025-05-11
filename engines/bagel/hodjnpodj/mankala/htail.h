// htail.h -- tail file for header files
// Written by John J. Xenakis for Boffo Games Inc., 1994

// This file is included at the end of each header file.
// It redefines defines that are made in "hhead.h" at the
// beginning of the header file.

// the following are redefined from beginning of header file
#undef PUBLIC
#undef PRIVATE
#undef PROTECTED
#undef EXTERNC
#undef VIRTUAL
#undef STATIC
#undef PDFT
#define PUBLIC
#define PRIVATE
#define PROTECTED
#define VIRTUAL
#define STATIC
#define PDFT(value)

