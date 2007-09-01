/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef _OSCALLS_H_
#define _OSCALLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PALMOS_ARM
#	define __68K(a)	__68k_##a
#else
#	define __68K(a)	a
#endif

Err __68k_StatShow();
Err __68k_StatHide();
Err __68k_PINSetInputAreaState(UInt16 state);
Err __68k_SysSetOrientation(UInt16 orientation);
UInt16 __68k_SysGetOrientation(void);
Err __68k_SysSetOrientationTriggerState(UInt16 triggerState);
UInt16 __68k_SysGetOrientationTriggerState(void);

#ifdef __cplusplus
}
#endif

#endif
