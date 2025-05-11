// hhead.h -- header file for header files
// Written by John J. Xenakis for Boffo Games Inc., 1994

// This file is included at the beginning of each header file.
// It makes defines that are redefined in "htail.h" at the
// end of the header file.

// the following are redefined at end of the header file
#undef PUBLIC
#undef PRIVATE
#undef PROTECTED
#undef EXTERNC
#undef VIRTUAL
#undef STATIC
#undef PDFT
#define PUBLIC public:
#define PRIVATE private:
#define PROTECTED protected:
#define VIRTUAL virtual
#define STATIC static
#define PDFT(value) = value

