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
 
 File:			sys_interface.h
 Revision:		$Revision$

 Description:	
 
	(Linux) System-specific interface data

 Notes: 
 
\****************************************************************************/
#ifndef REINHERIT_SYSINTERFACE_H
#define REINHERIT_SYSINTERFACE_H

namespace Saga {

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int

#define R_CFGFILE_NAME ".reinheritrc"

#define R_DIRECTORY_SEP '/'

#define R_STDOUT stdout
#define R_STDERR stderr

#define R_printf fprintf
#define R_vprintf vfprintf

} // End of namespace Saga

#endif				/* REINHERIT_SYSTYPES_H */
