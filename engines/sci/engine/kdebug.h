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

/* Kernel debug defines */

#ifndef SCI_ENGINE_KDEBUG_H
#define SCI_ENGINE_KDEBUG_H

namespace Sci {

struct EngineState;

#define SCIk_DEBUG_MODES 17

#define SCIkWARNING_NR -1
#define SCIkFUNCCHK_NR 5
#define SCIkSOUNDCHK_NR 7
#define SCIkGFXDRIVER_NR 8
#define SCIkBASESETTER_NR 9
#define SCIkPARSER_NR 10
#define SCIkAVOIDPATH_NR 17

#define SCIkNODES      s, __FILE__, __LINE__, 1
#define SCIkGRAPHICS   s, __FILE__, __LINE__, 2
#define SCIkSTRINGS    s, __FILE__, __LINE__, 3
#define SCIkMEM        s, __FILE__, __LINE__, 4
#define SCIkFUNCCHK    s, __FILE__, __LINE__, SCIkFUNCCHK_NR
#define SCIkBRESEN     s, __FILE__, __LINE__, 6
#define SCIkSOUND      s, __FILE__, __LINE__, SCIkSOUNDCHK_NR
#define SCIkGFXDRIVER  s, __FILE__, __LINE__, SCIkGFXDRIVER_NR
#define SCIkBASESETTER s, __FILE__, __LINE__, SCIkBASESETTER_NR
#define SCIkPARSER     s, __FILE__, __LINE__, SCIkPARSER_NR
#define SCIkMENU       s, __FILE__, __LINE__, 11
#define SCIkSAID       s, __FILE__, __LINE__, 12
#define SCIkFILE       s, __FILE__, __LINE__, 13
#define SCIkAVOIDPATH  s, __FILE__, __LINE__, SCIkAVOIDPATH_NR

#define SCI_KERNEL_DEBUG

#ifdef SCI_KERNEL_DEBUG
  #define SCIkdebug _SCIkdebug
#else
  #define SCIkdebug 1? (void)0 : _SCIkdebug
#endif

#define SCIkwarn _SCIkwarn

/* Internal functions */
void _SCIkwarn(EngineState *s, const char *file, int line, int area, const char *format, ...);
void _SCIkdebug(EngineState *s, const char *file, int line, int area, const char *format, ...);

/* If mode=1, enables debugging for specified areas. If mode=0, disables
** debugging for specified areas.
** Valid area characters: ulgcmfbad
*/
void set_debug_mode(EngineState *s, int mode, const char *areas);

extern int sci_debug_flags;

/* Debug flags */
#define _DEBUG_FLAG_LOGGING 1 /* Log each command executed */
#define _DEBUG_FLAG_BREAK_ON_WARNINGS 2 /* Break on warnings */

} // End of namespace Sci

#endif // SCI_ENGINE_KDEBUG_H
