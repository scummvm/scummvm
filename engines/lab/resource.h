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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#ifndef LAB_RESOURCE_H
#define LAB_RESOURCE_H

#include "lab/labfun.h"
#include "lab/text.h"

namespace Lab {

enum StaticText {
	kTextLowerFloor,
	kTextMiddleFloor,
	kTextUpperFloor,
	kTextMedMazeFloor,
	kTextHedgeMazeFloor,
	kTextSurMazeFloor,
	kTextCarnivalFloor,

	kTextSurmazeMessage,

	kTextFacingNorth,
	kTextFacingEast,
	kTextFacingSouth,
	kTextFacingWest,

	kTextLampOn,

	kTextTurnLeft,
	kTextTurnRight,
	kTextGoForward,
	kTextNoPath,
	kTextTakeItem,
	kTextSave,
	kTextLoad,
	kTextBookmark,
	kTextPersonal,
	kTextDisk,
	kTextSaveBook,
	kTextRestoreBook,
	kTextSaveFlash,
	kTextRestoreFlash,
	kTextSaveDisk,
	kTextRestoreDisk,
	kTextNoDiskInDrive,
	kTextWriteProtected,
	kTextSelectDisk,
	kTextFormatFloppy,
	kTextFormatting,

	kTextNothing,
	kTextUseOnWhat,
	kTextTakeWhat,
	kTextMoveWhat,
	kTextOpenWhat,
	kTextCloseWhat,
	kTextLookWhat,

	kTextUseMap,
	kTextUseJournal,
	kTextTurnLampOn,
	kTextTurnLampOff,
	kTextUseWhiskey,
	kTextUsePith,
	kTextUseHelmet
};

class Resource {
public:
	Resource(LabEngine *vm);
	~Resource() {}

	Common::File *openDataFile(const char * fileName, uint32 fileHeader);
	bool readRoomData(const char *fileName);
	bool readInventory(const char *fileName);
	bool readViews(uint16 roomNum);
	TextFont *getFont(const char *fileName);
	char *getText(const char *fileName);
	Common::String getStaticText(byte index) const { return _staticText[index]; }

private:
	LabEngine *_vm;
	char *readString(Common::File *file);
	int16 *readConditions(Common::File *file);
	RuleList *readRule(Common::File *file);
	Action *readAction(Common::File *file);
	CloseData *readCloseUps(uint16 depth, Common::File *file);
	ViewData *readView(Common::File *file);
	void readStaticText();

	Common::String _staticText[48];
};

} // End of namespace Lab

#endif /* LAB_RESOURCE_H */
