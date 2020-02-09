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

#ifndef TITANIC_BEDHEAD_H
#define TITANIC_BEDHEAD_H

#include "common/array.h"
#include "titanic/game/sgt/sgt_state_room.h"

namespace Titanic {

struct BedheadEntry {
	CString _name1;
	CString _name2;
	CString _name3;
	CString _name4;
	int _startFrame;
	int _endFrame;

	void load(Common::SeekableReadStream *s);
};
class BedheadEntries : public Common::Array<BedheadEntry> {
public:
	void load(Common::SeekableReadStream *s, int count);
};

struct TurnOnEntries {
	BedheadEntries _closed;
	BedheadEntries _restingTV;
	BedheadEntries _restingUV;
	BedheadEntries _closedWrong;

	void load(Common::SeekableReadStream *s);
};

struct TurnOffEntries {
	BedheadEntries _open;
	BedheadEntries _restingUTV;
	BedheadEntries _restingV;
	BedheadEntries _restingG;
	BedheadEntries _openWrong;
	BedheadEntries _restingDWrong;

	void load(Common::SeekableReadStream *s);
};

class CBedhead : public CSGTStateRoom {
	DECLARE_MESSAGE_MAP;
	bool TurnOn(CTurnOn *msg);
	bool TurnOff(CTurnOff *msg);
private:
	TurnOnEntries _on;
	TurnOffEntries _off;
public:
	CLASSDEF;
	CBedhead();

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

} // End of namespace Titanic

#endif /* TITANIC_BEDHEAD_H */
