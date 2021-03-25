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

#ifndef TRECISION_LOGIC_H
#define TRECISION_LOGIC_H

#include "common/scummsys.h"

namespace Trecision {
class TrecisionEngine;

class LogicManager {
private:
	TrecisionEngine *_vm;

public:
	// panel puzzle 35
	uint16 Comb35[7];
	uint16 Count35 = 0;

	// sundial puzzle 49
	uint16 Comb49[4];

	// sundial puzzle 4CT
	uint16 Comb4CT[6];

	// keyboard puzzle 58
	uint16 Comb58[6];
	uint16 Count58;

	LogicManager(TrecisionEngine *vm);
	~LogicManager();

	void initScript();
	void initInventory();

	void setRoom(uint16 r, bool b);
	void useWithInventory(bool *updateInv, bool *printSentence);
	void useWithScreen(bool *updateInv, bool *printSentence);
	void roomOut(uint16 curObj, uint16 *action, uint16 *pos);
	bool mouseExamine(uint16 curObj);
	bool mouseOperate(uint16 curObj);
};
; // end of class

} // end of namespace
#endif

