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

#ifndef ULTIMA8_WORLD_BOBOBOOMERPROCESS_H
#define ULTIMA8_WORLD_BOBOBOOMERPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

class Item;

/**
 * A process to make a bunch of explosions. bo-bo-boom!
 * Only used in No Regret.
 */
class BoboBoomerProcess : public Process {
public:
	BoboBoomerProcess();
	BoboBoomerProcess(const Item *item);

	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

private:
	int32 _counter;
	int32 _x;
	int32 _y;
	int32 _z;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
