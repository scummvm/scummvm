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

#ifndef	_CONTROL_S
#define	_CONTROL_S

#include "common/scummsys.h"
//#include "src\driver96.h"

uint32	Restore_control(void);	//Tony20Mar97
void	Save_control(void);	//Tony1Apr97
void	Quit_control(void);	//Tony2Apr97
void	Restart_control(void);	//Tony4Apr97
void	Option_control(void);	//Pete5Jun97
int32   ReadOptionSettings(void);	//Pete10Jun97
void	UpdateGraphicsLevel(uint8 newLevel);	// (James13jun97)

extern uint8 subtitles;		// text selected
extern uint8 speechSelected;
extern uint8 current_graphics_level;

#endif
