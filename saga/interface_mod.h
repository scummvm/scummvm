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
 
	Game interface module public header file

 Notes: 
*/

#ifndef SAGA_INTERFACE_MOD_H
#define SAGA_INTERFACE_MOD_H

namespace Saga {

typedef enum INTERFACE_UPDATE_FLAGS_tag {

	UPDATE_MOUSEMOVE = 1,
	UPDATE_MOUSECLICK

} INTERFACE_UPDATE_FLAGS;

int INTERFACE_RegisterLang(void);

int INTERFACE_Init(void);
int INTERFACE_Shutdown(void);

int INTERFACE_Activate(void);
int INTERFACE_Deactivate(void);

int INTERFACE_SetStatusText(const char *new_txt);
int INTERFACE_Draw(void);
int INTERFACE_Update(R_POINT * imouse_pt, int update_flag);

} // End of namespace Saga

#endif				/* SAGA_INTERFACE_MOD_H */
