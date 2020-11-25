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

#ifndef AGS_AGS_H
#define AGS_AGS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/surface.h"

#include "ags/shared/gfx/bitmap.h"
#include "ags/lib/allegro/system.h"

namespace AGS {

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

enum AGSDebugChannels {
	kDebugPath     = 1 << 0,
	kDebugGraphics = 1 << 1
};

struct AGSGameDescription;

class AGSEngine : public Engine {
private:
	const AGSGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
public:
	::AGS3::BITMAP *_screen;
	::AGS3::GFX_DRIVER *_gfxDriver;
protected:
	// Engine APIs
	virtual Common::Error run();
public:
	AGSEngine(OSystem *syst, const AGSGameDescription *gameDesc);
	virtual ~AGSEngine();
	void GUIError(const Common::String &msg);

	void set_window_title(const char *str) {
		// No implementation
	}

	uint32 getFeatures() const;

	/**
	 * Returns the current list of savegames
	 */
	SaveStateList listSaves() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	/**
	 * Sets the random number seed
	 */
	void setRandomNumberSeed(uint32 seed) {
		_randomSource.setSeed(seed);
	}
};

extern AGSEngine *g_vm;
#define screen ::AGS::g_vm->_screen
#define gfx_driver ::AGS::g_vm->_gfxDriver

} // namespace AGS

#endif
