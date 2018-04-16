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

namespace Lab {

struct ViewData;

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

	kTextkLampOn,

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
	kTextTurnkLampOn,
	kTextTurnLampOff,
	kTextUseWhiskey,
	kTextUsePith,
	kTextUseHelmet
};

class Resource {
public:
	Resource(LabEngine *vm);
	~Resource() {}

	Common::File *openDataFile(const Common::String filename, uint32 fileHeader = 0);
	void readRoomData(const Common::String fileName);
	InventoryData *readInventory(const Common::String fileName);
	void readViews(uint16 roomNum);
	TextFont *getFont(const Common::String fileName);
	Common::String getText(const Common::String fileName);
	Common::String getStaticText(byte index) const { return _staticText[index]; }

private:
	LabEngine *_vm;
	Common::String readString(Common::File *file);
	Common::Array<int16> readConditions(Common::File *file);
	void readRule(Common::File *file, RuleList &rules);
	void readAction(Common::File *file, ActionList &action);
	void readCloseUps(uint16 depth, Common::File *file, CloseDataList &close);
	void readView(Common::File *file, ViewDataList &view);
	void readStaticText();
	Common::String translateFileName(const Common::String filename);

	Common::String _staticText[48];
};

} // End of namespace Lab

#endif // LAB_RESOURCE_H
