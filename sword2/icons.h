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

#ifndef	_ICONS
#define	_ICONS

#include "object.h"

#define MENU_MASTER_OBJECT	44
#define TOTAL_subjects		(375 - 256 + 1)	// the speech subject bar
#define TOTAL_engine_pockets	(15 + 10)	// +10 for overflow

// define these in a script and then register them with the system

typedef	struct {
	int32 icon_resource;	// icon graphic graphic
	int32 luggage_resource;	// luggage icon resource (for attaching to
				// mouse pointer)
} menu_object;

extern menu_object master_menu_list[TOTAL_engine_pockets];

void Build_top_menu(void);
void Build_system_menu(void);

#endif
