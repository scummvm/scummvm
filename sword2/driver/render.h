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

#ifndef RENDER_H
#define RENDER_H

#include "sword2/driver/menu.h"

namespace Sword2 {

#define RENDERWIDE		640
#define ALIGNRENDERDEEP		480
#define RENDERDEEP		(ALIGNRENDERDEEP - (MENUDEEP * 2))

typedef struct {
	uint16 packets;
	uint16 offset;
} _parallaxLine;

} // End of namespace Sword2

#endif
