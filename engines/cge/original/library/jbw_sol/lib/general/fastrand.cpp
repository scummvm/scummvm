#include <general.h>



static	unsigned	Seed = 1;



unsigned FastRand (void) { return Seed = 257 * Seed + 817; }



unsigned FastRand (unsigned s) { return Seed = 257 * s + 817; }
