/****************************************************************************\
 ____      ___       _               _ _
|  _ \ ___|_ _|_ __ | |__   ___ _ __(_) |_
| |_) / _ \| || '_ \| '_ \ / _ \ '__| | __|
|  _ <  __/| || | | | | | |  __/ |  | | |_
|_| \_\___|___|_| |_|_| |_|\___|_|  |_|\__|

 ----------------------------------------------------------------------------
 Project:		ReInherit Engine
 Version:		.1a
 
 The ReInherit Engine is (C)2000-2001 by Daniel Balsom
 This code is subject to the terms and conditions of the General Public
 License (GPL).
 
 ----------------------------------------------------------------------------
 
 File:			x86-32.h
 Revision:		$Revision$

 Description:	
 
	Architecture-specific typedefs for Intel(tm) compatible 32 bit processors

 Notes: 

\****************************************************************************/
#ifndef R_ARCH_H
#define R_ARCH_H

namespace Saga {

#define R_LITTLE_ENDIAN

typedef unsigned int R_UINT32;
typedef signed int R_SINT32;

typedef unsigned short R_UINT16;
typedef signed short R_SINT16;

typedef unsigned char R_UINT8;
typedef signed char R_SINT8;

} // End of namespace Saga

#endif				/* R_ARCH_H */
