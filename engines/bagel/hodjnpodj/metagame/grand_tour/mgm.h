// mgm.h -- header file for meta-game
// Written by John J. Xenakis, 1994, for Boffo Games

#ifndef __mgm_H__
#define __mgm_H__

#include <time.h>
#include "bagel/hodjnpodj/globals.h"
#include "resource.h"
#include "dibdoc.h"
#include "sprite.h"

#include "bagel/hodjnpodj/metagame/bgen/bgen.h"
#include "bagel/hodjnpodj/metagame/bgen/bgenut.h"
#include "bagel/hodjnpodj/metagame/bgen/btimeut.h"
// #include "mgmopt.h"

// the following are redefined at end of this header file
#undef PUBLIC
#undef PRIVATE
#undef PROTECTED
#undef PDFT
#define PUBLIC public:
#define PRIVATE private:
#define PROTECTED protected:
#define PDFT(value) = value

// DATADIR is directory for *.BMP and other data files
// #define DATADIR "..\\"
#define DATADIR ""








// the following are redefined from beginning of header file
#undef PUBLIC
#undef PRIVATE
#undef PROTECTED
#undef PDFT
#define PUBLIC
#define PRIVATE
#define PROTECTED
#define PDFT(value)

#endif // __mgm_H__

