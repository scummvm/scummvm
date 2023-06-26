
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_GUI_GUI_VMNG_H
#define M4_GUI_GUI_VMNG_H

#include "m4/m4_types.h"
#include "m4/gui/gui_univ.h"
#include "m4/gui/gui_vmng_core.h"
#include "m4/gui/gui_vmng_rectangles.h"
#include "m4/gui/gui_vmng_screen.h"

namespace M4 {

enum {
	SCRN_DLG = 0, SCRN_BUF, SCRN_TEXT, SCRN_TRANS
};
enum {
	SCRN_ANY = 0, SCRN_ACTIVE, SCRN_INACTIVE, SCRN_UNDEFN
};

#define SF_LAYER       0x000f
#define SF_BACKGRND    0x0000
#define SF_DRIFTER     0x0001
#define SF_FLOATER     0x0002
#define SF_SURFACE     0x0003
#define SF_MOUSE       0x000e

#define SF_GET_NONE    0x0000
#define SF_GET_KEY     0x0010
#define SF_GET_MOUSE   0x0020
#define SF_GET_ALL     0x0030

#define SF_BLOCK_NONE  0x0000
#define SF_BLOCK_KEY   0x0040
#define SF_BLOCK_MOUSE 0x0080
#define SF_BLOCK_ALL   0x00c0

#define SF_IMMOVABLE    0x0100		// if set, it ain't draggable
#define SF_OFFSCRN		0x0200		// can be dragged off screen, if it's draggable
#define SF_TRANSPARENT	0x0400		// if the screen is transparent

#define SF_DEFAULT	SF_DRIFTER | SF_GET_ALL | SF_BLOCK_NONE
#define SF_ALERT	SF_FLOATER | SF_GET_ALL | SF_BLOCK_ALL

} // End of namespace M4

#endif
