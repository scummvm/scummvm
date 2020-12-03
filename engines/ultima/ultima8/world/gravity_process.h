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

#ifndef ULTIMA8_WORLD_GRAVITYPROCESS_H
#define ULTIMA8_WORLD_GRAVITYPROCESS_H

#include "ultima/ultima8/kernel/process.h"

namespace Ultima {
namespace Ultima8 {

class Item;
class Actor;

class GravityProcess : public Process {
public:
	GravityProcess();
	GravityProcess(Item *item, int gravity);

	// add (xs,ys,zs) to current speed vector
	void move(int xs, int ys, int zs);
	void setGravity(int gravity);

	void init();

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;
	void terminate() override;

	void dumpInfo() const override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	void fallStopped();

	void actorFallStoppedU8(Actor *actor, int height);
	void actorFallStoppedCru(Actor *actor, int height);

	int _gravity;
	int _xSpeed, _ySpeed, _zSpeed;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
