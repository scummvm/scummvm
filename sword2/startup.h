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

#ifndef	_STARTUP
#define	_STARTUP

namespace Sword2 {

#define	MAX_starts	100
#define	MAX_description	100

typedef	struct {
	char description[MAX_description];

	// id of screen manager object
	uint32 start_res_id;

	//tell the manager which startup you want (if there are more than 1)
	// (i.e more than 1 entrance to a screen and/or seperate game boots)
	uint32 key;
} _startup;

extern _startup start_list[MAX_starts];

uint32 Init_start_menu(void);
uint32 Con_print_start_menu(void);
uint32 Con_start(uint8 *input);

} // End of namespace Sword2

#endif
