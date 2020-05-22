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

#ifndef WORLD_ACTORS_QUICKAVATARMOVERPROCESS_H
#define WORLD_ACTORS_QUICKAVATARMOVERPROCESS_H

#include "ultima/ultima8/kernel/process.h"

namespace Ultima {
namespace Ultima8 {

class QuickAvatarMoverProcess : public Process {
public:
	QuickAvatarMoverProcess();

	QuickAvatarMoverProcess(int x, int y, int z, int _dir);
	~QuickAvatarMoverProcess() override;

	// p_dynamic_cast stuff
	ENABLE_RUNTIME_CLASSTYPE()

	void run() override;
	void terminate() override;

	static bool isQuarterSpeed() {
		return _quarter;
	}
	static bool isClipping() {
		return _clipping;
	}
	static void setQuarterSpeed(bool q) {
		_quarter = q;
	}
	static void toggleClipping() {
		_clipping = !_clipping;
	}

	static void terminateMover(int _dir);
	static void startMover(int x, int y, int z, int _dir);

	bool loadData(Common::ReadStream *rs, uint32 version);
	void saveData(Common::WriteStream *ws) override;

protected:
	int _dx, _dy, _dz, _dir;
	static ProcId _amp[6];
	static bool _clipping;
	static bool _quarter;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
