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

// Scene management module public header file

#ifndef SAGA_SCENE_MOD_H__
#define SAGA_SCENE_MOD_H__

#include "saga/text.h"

namespace Saga {

enum R_SCENE_MODES {
	R_SCENE_MODE_INVALID,
	R_SCENE_MODE_NORMAL,
	R_SCENE_MODE_ISO
};

struct SCENE_ZINFO {
	int begin_slope;
	int end_slope;

};

struct SCENE_BGINFO {
	int bg_x;
	int bg_y;
	int bg_w;
	int bg_h;
	int bg_p;
	byte *bg_buf;
	size_t bg_buflen;
};

struct R_SCENE_INFO {
	SCENE_ZINFO z_info;
	SCENE_BGINFO bg_info;
	R_TEXTLIST *text_list;
};

typedef int (R_SCENE_PROC) (int, R_SCENE_INFO *);

int SCENE_Register();
int SCENE_Init();
int SCENE_Shutdown();
int SCENE_Start();
int SCENE_Next();
int SCENE_Skip();
int SCENE_End();
int SCENE_Draw(R_SURFACE *);
int SCENE_GetMode();
int SCENE_GetBGMaskInfo(int *w, int *h, byte **buf, size_t *buf_len);
int SCENE_IsBGMaskPresent(void);
int SCENE_GetBGInfo(SCENE_BGINFO *bginfo);
int SCENE_GetZInfo(SCENE_ZINFO *zinfo);
int SCENE_GetBGPal(PALENTRY **pal);
int SCENE_GetInfo(R_SCENE_INFO *si);

} // End of namespace Saga

#endif
