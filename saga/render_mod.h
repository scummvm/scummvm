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

// Main rendering loop - public header

#ifndef SAGA_RENDER_MOD_H__
#define SAGA_RENDER_MOD_H__

namespace Saga {

enum RENDER_FLAGS {
	RF_SHOW_FPS = 0x01,
	RF_PALETTE_TEST = 0x02,
	RF_TEXT_TEST = 0x04,
	RF_OBJECTMAP_TEST = 0x08,
	RF_RENDERPAUSE = 0x10,
	RF_GAMEPAUSE = 0x20
};

struct R_BUFFER_INFO {
	byte *r_bg_buf;
	int r_bg_buf_w;
	int r_bg_buf_h;
	byte *r_tmp_buf;
	int r_tmp_buf_w;
	int r_tmp_buf_h;
};

int RENDER_Register();
int RENDER_Init(OSystem *system);
int RENDER_DrawScene();
unsigned int RENDER_GetFlags();
void RENDER_SetFlag(unsigned int);
void RENDER_ToggleFlag(unsigned int);
unsigned int RENDER_GetFrameCount(void);
unsigned int RENDER_ResetFrameCount(void);
int RENDER_GetBufferInfo(R_BUFFER_INFO *);

} // End of namespace Saga

#endif
