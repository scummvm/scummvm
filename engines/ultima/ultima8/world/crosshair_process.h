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

#ifndef ULTIMA8_WORLD_CROSSHAIRPROCESS_H
#define ULTIMA8_WORLD_CROSSHAIRPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/misc/classtype.h"

namespace Ultima {
namespace Ultima8 {

/**
 * A process to update the location of the small red crosshairs in Crusader.
 * The crosshairs are just based on avatar angle and don't move with the
 * environment, which makes them simpler than the dynamic target reticle.
 */
class CrosshairProcess : public Process {
public:
	CrosshairProcess();
	~CrosshairProcess();

	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

	static CrosshairProcess *get_instance() {
		return _instance;
	}

private:
	static CrosshairProcess *_instance;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
