//////////////////////////////////////////////////////////////////////////////
// GP32.h                                                                   //
//////////////////////////////////////////////////////////////////////////////
#ifndef	_GP32_H
#define	_GP32_H

/*
	GP32 stuff
	just a bunch of includes
*/

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
/*
#include "24x.h"
#include "gpdef.h"
#include "gpos_def.h"
#include "gpmem.h"
#include "gpstdio.h"
#include "gpstdlib.h"
#include "gpmm.h"
#include "gpgraphic.h"
#include "gpfont.h"
#include "gpsockdef.h"
#include "gpcomm.h"
#include "gpnetlib.h"
*/
//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
// C++ bools
//typedef int		bool;
#define false	0
#define true	1

typedef unsigned char	u8;
typedef volatile u8		vu8;
typedef signed char		s8;
typedef volatile s8		vs8;
typedef unsigned short	u16;
typedef volatile u16	vu16;
typedef signed short	s16;
typedef volatile s16	vs16;
typedef unsigned int	u32;
typedef volatile u32	vu32;
typedef signed int		s32;
typedef volatile s32	vs32;

// memory/register typecasting
#define MEM1(addr)				( (u8 *)(addr))
#define MEM2(addr)				( (u16*)(addr))
#define MEM4(addr)				( (u32*)(addr))
#define REG1(addr)				(*(vu8 *)(addr))
#define REG2(addr)				(*(vu16*)(addr))
#define REG4(addr)				(*(vu32*)(addr))
#define RPTR(addr)				(*(void **)(addr))

// array length
#define lengthof(id)			(sizeof(id) / sizeof((id)[0]))

//////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                 //
//////////////////////////////////////////////////////////////////////////////




#endif	// _GP32_H
