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
/*
 Description:   
 
    Scene management module public header file

 Notes: 
*/

#ifndef SAGA_SCENE_MOD_H__
#define SAGA_SCENE_MOD_H__

#include "text_mod.h"

namespace Saga {

/*
 * r_scene.c                                                
\*--------------------------------------------------------------------------*/
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

	uchar *bg_buf;
	size_t bg_buflen;

};

struct R_SCENE_INFO {

	SCENE_ZINFO z_info;
	SCENE_BGINFO bg_info;

	R_TEXTLIST *text_list;

};

typedef int (R_SCENE_PROC) (int, R_SCENE_INFO *);

int SCENE_Register(void);
int SCENE_Init(void);
int SCENE_Shutdown(void);

int SCENE_Start(void);
int SCENE_Next(void);
int SCENE_Skip(void);
int SCENE_End(void);

int SCENE_Draw(R_SURFACE *);

int SCENE_GetMode(void);
int SCENE_GetBGMaskInfo(int *w, int *h, uchar ** buf, size_t * buf_len);

int SCENE_IsBGMaskPresent(void);
int SCENE_GetBGInfo(SCENE_BGINFO * bginfo);
int SCENE_GetZInfo(SCENE_ZINFO * zinfo);
int SCENE_GetBGPal(PALENTRY ** pal);

int SCENE_GetInfo(R_SCENE_INFO * si);

} // End of namespace Saga

#endif				/* SAGA_SCENE_MOD_H__ */
