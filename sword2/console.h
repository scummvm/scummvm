/* Copyright (C) 1994-2003 Revolution Software Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	C_ONSOLE_H
#define	C_ONSOLE_H

#include "driver/driver96.h"
#include "memory.h"

namespace Sword2 {

#ifdef _SWORD2_DEBUG

void Init_console(void);
uint32 One_console(void);
void StartConsole(void);
void EndConsole(void);

void Con_fatal_error(const char *format, ...);
void Print_to_console(const char *format, ...);
void Temp_print_to_console(const char *format, ...);
void Scroll_console(void);
void Clear_console_line(void);

extern mem *console_sprite;
extern uint32 con_y;
extern uint32 con_depth;
extern uint32 con_width;

extern uint8 grabbingSequences;
extern uint8 wantSfxDebug;	// sfx debug file enabled/disabled from console

#else

void Init_console(void);
uint32 One_console(void);	
void StartConsole(void);
void EndConsole(void);

#define Con_fatal_error		error

void Print_to_console(const char *format, ...);
void Temp_print_to_console(const char *format, ...);
void Clear_console_line(void);
void Scroll_console(void);

#endif

extern	uint32	console_status;

} // End of namespace Sword2

#endif
