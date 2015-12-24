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

#ifndef LAB_LABSETS_H
#define LAB_LABSETS_H

namespace Lab {

//---------------------------
//----- From LabSets.c ------
//---------------------------

class LabEngine;

class LargeSet {
public:
	LargeSet(uint16 last, LabEngine *vm);
	~LargeSet();
	bool in(uint16 element);
	void inclElement(uint16 element);
	void exclElement(uint16 element);
	bool readInitialConditions(const Common::String fileName);

private:
	LabEngine *_vm;

public:
	uint16 _lastElement;
	uint16 *_array;
};

} // End of namespace Lab

#endif // LAB_LABSETS_H
