/* Header is not present in Windows CE SDK */

#include "common/util.h"

#define assert(e) ((e) ? 0 : (error("Assertion failed " #e " (%s, %d)", __FILE__, __LINE__)))
	
