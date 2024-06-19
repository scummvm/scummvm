/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL$
 * $Id$
 *
 */

#ifndef PRISONER_MENUMANAGER_H
#define PRISONER_MENUMANAGER_H

namespace Prisoner {

enum MENU_PANELS {
	SAVE_PANEL,
	OPTIONS_PANEL,
	MAIN_PANEL,
	VOLOPT_PANEL,
	DIVOPT_PANEL,
	PAUSE_PANEL,
	SNDCFG_PANEL,
	LOAD_PANEL,
	ASKER1_PANEL,
	ASKER2_PANEL,
	ASKER3_PANEL,
	OPTGFX_PANEL,
	FORLOOKINV_PANEL,
	FORIDM_PANEL,
	PANEL_SIZE_END
};

struct _PakMenuDirectoryEntry {
	MENU_PANELS menuPanel;
	const char *pakName;
	uint32 pakSlot;
	uint8 outlineColor;
	uint8 inkColor;
};

struct _MenuOffset {
	int16 x1;
	int16 y1;
	int16 x2;
	int16 y2;
};

} // namespace Prisoner

#endif
