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

// Isometric level module - public module header

#ifndef SAGA_ISOMAP_MOD_H_
#define SAGA_ISOMAP_MOD_H_

namespace Saga {

int ISOMAP_Init(void);
int ISOMAP_LoadTileset(const byte *, size_t);
int ISOMAP_LoadMetaTileset(const byte *, size_t);
int ISOMAP_LoadMetamap(const byte *mm_res_p, size_t mm_res_len);
int ISOMAP_Draw(R_SURFACE *dst_s);

} // End of namespace Saga

#endif
