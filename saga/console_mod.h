/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:	
 
	Console module public header file

 Notes: 
*/

#ifndef SAGA_CONSOLE_MOD_H_
#define SAGA_CONSOLE_MOD_H_

namespace Saga {

int CON_Register(void);
int CON_Init(void);
int CON_Shutdown(void);

int CON_Activate(void);
int CON_Deactivate(void);
int CON_IsActive(void);

int CON_Type(int in_char);
int CON_Draw(R_SURFACE * ds);
int CON_Print(const char *fmt_str, ...);

int CON_CmdUp(void);
int CON_CmdDown(void);
int CON_PageUp(void);
int CON_PageDown(void);

int CON_DropConsole(double percent);
int CON_RaiseConsole(double percent);

} // End of namespace Saga

#endif				/* R_CONSOLE_MOD_H_ */
/* end "r_console_mod.h" */
