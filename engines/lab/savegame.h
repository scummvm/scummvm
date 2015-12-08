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

#ifndef LAB_SAVEGAME_H
#define LAB_SAVEGAME_H

namespace Lab {

class LabEngine;

struct SaveGameHeader {
	byte _version;
	SaveStateDescriptor _descr;
	uint16 _roomNumber;
	uint16 _direction;
};

bool saveGame(uint16 Direction, uint16 Quarters, int slot, Common::String desc);
bool loadGame(uint16 *Direction, uint16 *Quarters, int slot);
bool readSaveGameHeader(Common::InSaveFile *in, SaveGameHeader &header);

} // End of namespace Lab

#endif // LAB_SAVEGAME_H
