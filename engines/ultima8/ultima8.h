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

#ifndef ULTIMA8_ULTIMA8
#define ULTIMA8_ULTIMA8

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"

namespace Ultima8 {

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

enum Ultima8DebugChannels {
	kDebugPath     = 1 << 0,
	kDebugGraphics = 1 << 1
};

struct Ultima8GameDescription;

class Ultima8Engine : public Engine {
private:
	const Ultima8GameDescription *_gameDescription;
	Common::RandomSource _randomSource;
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	Ultima8Engine(OSystem *syst, const Ultima8GameDescription *gameDesc);
	virtual ~Ultima8Engine();
	void GUIError(const Common::String &msg);

	uint32 getFeatures() const;

	/**
	 * Returns a file system node for the game directory
	 */
	Common::FSNode getGameDirectory() const;

	/**
	 * Returns a random number
	 */
	uint getRandomNumber(uint maxVal) {
		return _randomSource.getRandomNumber(maxVal);
	}
};

extern Ultima8Engine *g_vm;

} // End of namespace Ultima8

#endif
