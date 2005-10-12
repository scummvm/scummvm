#ifndef PALMVERSION_H
#define PALMVERSION_H

#if defined(PALMOS_68K)
#	include <PalmOS.h>
#	include <VFSMgr.h>

#elif defined(COMPILE_ZODIAC) && defined(PALMOS_ARM)
#	include <tapwave.h>
#endif

#endif
