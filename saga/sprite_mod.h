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

// Sprite management module public header file

#ifndef SAGA_SPRITE_MOD_H__
#define SAGA_SPRITE_MOD_H__

namespace Saga {

typedef struct R_SPRITELIST_tag R_SPRITELIST;

int SPRITE_Init();
int SPRITE_Shutdown();
int SPRITE_LoadList(int resource_num, R_SPRITELIST **sprite_list_p);
int SPRITE_AppendList(int resource_num, R_SPRITELIST *spritelist);
int SPRITE_GetListLen(R_SPRITELIST *spritelist);
int SPRITE_Free(R_SPRITELIST *spritelist);
int SPRITE_Draw(R_SURFACE *ds, R_SPRITELIST *sprite_list, int sprite_num, int spr_x, int spr_y);
int SPRITE_DrawOccluded(R_SURFACE *ds, R_SPRITELIST *sprite_list, int sprite_num, int spr_x, int spr_y);

} // End of namespace Saga

#endif
