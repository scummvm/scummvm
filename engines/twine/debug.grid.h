/** @file debug.grid.h
	@brief
	This file contains grid debug routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef GRIDDEBUG_H
#define GRIDDEBUG_H

#include "sys.h"

extern int32 useFreeCamera;
extern int32 canChangeScenes;

/** Change scenario camera positions */
void changeGridCamera(int16 pKey);
/** Change grid index */
void changeGrid(int16 pKey);
/** Apply and change disappear celling grid */
void applyCellingGrid(int16 pKey);

#endif
