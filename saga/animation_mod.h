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

// Background animation management module public header

#ifndef SAGA_ANIMATION_MOD_H__
#define SAGA_ANIMATION_MOD_H__

namespace Saga {

enum ANIM_FLAGS {
	ANIM_LOOP = 0x01,
	ANIM_ENDSCENE = 0x80	// When animation ends, dispatch scene end event
};

int ANIM_Register(void);
int ANIM_Init(void);
int ANIM_Shutdown(void);
int ANIM_Load(const byte *anim_resdata, size_t anim_resdata_len, uint16 *anim_id_p);
int ANIM_Free(uint16 anim_id);
int ANIM_Play(uint16 anim_id, int vector_time);
int ANIM_Link(uint16 anim_id1, uint16 anim_id2);
int ANIM_SetFlag(uint16 anim_id, uint16 flag);
int ANIM_SetFrameTime(uint16 anim_id, int time);
int ANIM_Reset(void);

} // End of namespace Saga

#endif
