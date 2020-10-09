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

#ifndef ULTIMA8_GRAPHICS_CYCLEPROCESS_H
#define ULTIMA8_GRAPHICS_CYCLEPROCESS_H

#include "ultima/ultima8/kernel/process.h"
#include "ultima/ultima8/graphics/palette_manager.h"
#include "ultima/ultima8/usecode/intrinsics.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

/** The process to cycle some palette colors in Crusader */
class CycleProcess : public Process {
	uint8 	_running;
	uint8 	_cycleColData[7][3];
public:
	static CycleProcess *_instance;

	// p_dynamic_class stuff
	ENABLE_RUNTIME_CLASSTYPE()
	CycleProcess();
	~CycleProcess(void) override;

	void run() override;

	static CycleProcess *get_instance();

	void pauseCycle() {
		_running = 0;
	}

	void resumeCycle() {
		_running = 1;
	}

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
