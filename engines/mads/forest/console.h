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

#ifndef MADS_FOREST_CONSOLE_H
#define MADS_FOREST_CONSOLE_H

#include "mads/console.h"

namespace MADS {
namespace Forest {

class Console : public MADS::Console {
private:
	bool cmdForestMusic(int argc, const char **argv);
	bool cmdForestSample(int argc, const char **argv);
	bool cmdForestSounds(int argc, const char **argv);

public:
	Console();
	~Console() override {}
};

} // namespace Forest
} // namespace MADS

#endif
