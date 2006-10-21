#ifndef PALMVERSION_H
#define PALMVERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(COMPILE_ZODIAC)
#	include <tapwave.h>
#else
#	include <PalmOS.h>
#	include <VFSMgr.h>
#endif

#ifdef __cplusplus
}
#endif

#endif
