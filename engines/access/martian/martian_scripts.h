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

#ifndef ACCESS_MARTIAN_SCRIPTS_H
#define ACCESS_MARTIAN_SCRIPTS_H

#include "common/scummsys.h"
#include "access/scripts.h"

namespace Access {

namespace Martian {

class MartianEngine;

class MartianScripts : public Scripts {
private:
	MartianEngine *_game;

	void cmdSpecial0();
	void cmdSpecial1(int param1);
	void cmdSpecial3();
	void doIntro(int param1);
	void cmdSpecial6();
	void cmdSpecial7();

protected:
	void executeSpecial(int commandIndex, int param1, int param2) override;
	void executeCommand(int commandIndex) override;

public:
	MartianScripts(AccessEngine *vm);
};

} // End of namespace Martian

} // End of namespace Access

#endif /* ACCESS_MARTIAN_SCRIPTS_H */
