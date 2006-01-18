/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2005 Won Star - GP32 Backend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef __GP32STD_INPUT_H
#define __GP32STD_INPUT_H

#include "stdafx.h"
#include "common/scummsys.h"

#define BUTTON_DOWN 1
#define BUTTON_UP 2

struct GP32BtnEvent {
	uint16 type;
	uint16 button;
};

extern int gp_trapKey();

extern bool gp_getButtonEvent(uint32 *nKeyUD, uint16 *nKeyP);

#define gpd_getEventDown(a) (((a) >> 12) & 0x0fff)
#define gpd_getEventUp(a) ((a) & 0x0fff)
#define gpd_getEventPressed(a) ((a) & 0x0fff)
#define gpd_getButtonDown(a,button) ((a) & (button) << 12)
#define gpd_getButtonUp(a,button)   ((a) & (button))
#define gpd_getButtonPressed(a,button)   ((a) & (button))

extern bool gp_pollButtonEvent(GP32BtnEvent *ev);
extern bool gp_getButtonPressed(uint16 button);

#endif
