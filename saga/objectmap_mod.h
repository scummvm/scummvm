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

// Object map module public header file

#ifndef SAGA_OBJECTMAP_MOD_H__
#define SAGA_OBJECTMAP_MOD_H__

namespace Saga {

enum R_OBJECT_FLAGS {
	R_OBJECT_NORMAL = 0x02
};

int OBJECTMAP_Register();
int OBJECTMAP_Init();
int OBJECTMAP_Shutdown();
int OBJECTMAP_Load(const byte *om_res, size_t om_res_len);
int OBJECTMAP_Free();
int OBJECTMAP_LoadNames(const byte *onl_res, size_t onl_res_len);
int OBJECTMAP_FreeNames();
int OBJECTMAP_GetName(int object, const char **name);
int OBJECTMAP_GetFlags(int object, uint16 *flags);
int OBJECTMAP_GetEPNum(int object, int *ep_num);
int OBJECTMAP_Draw(R_SURFACE *draw_surface, R_POINT *imouse_pt, int color, int color2);
int OBJECTMAP_HitTest(R_POINT *imouse_pt, int *object_num);

} // End of namespace Saga

#endif
