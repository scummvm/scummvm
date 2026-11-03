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

#ifndef PRISONER_MENUMANAGER_H
#define PRISONER_MENUMANAGER_H

namespace Prisoner {

enum MenuPanel {
	kSavePanel,
	kOptionsPanel,
	kMainPanel,
	kVolOptPanel,
	kDivOptPanel,
	kPausePanel,
	kSndCfgPanel,
	kLoadPanel,
	kAsker1Panel,
	kAsker2Panel,
	kAsker3Panel,
	kOptGfxPanel,
	kForLookInvPanel,
	kForIdmPanel,
	kMenuPanelCount
};

struct PakMenuDirectoryEntry {
	MenuPanel menuPanel;
	const char *pakName;
	uint32 pakSlot;
	uint8 outlineColor;
	uint8 inkColor;
};

struct MenuOffset {
	int16 x1;
	int16 y1;
	int16 x2;
	int16 y2;
};

enum DialogType {
	kDialogNone,
	kDialogOk,
	kDialogCancel,
	kDialogIgnore,
};


} // namespace Prisoner

#endif
