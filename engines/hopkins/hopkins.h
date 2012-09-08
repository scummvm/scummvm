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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_H
#define HOPKINS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "hopkins/events.h"
#include "hopkins/globals.h"

/**
 * This is the namespace of the Hopkins engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Hopkins FBI
 */
namespace Hopkins {

enum {
	kHopkinsDebugAnimations = 1 << 0,
	kHopkinsDebugActions = 1 << 1,
	kHopkinsDebugSound = 1 << 2,
	kHopkinsDebugMusic = 1 << 3,
	kHopkinsDebugScripts = 1 << 4
};

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

struct HopkinsGameDescription;

class HopkinsEngine : public Engine {
private:
	const HopkinsGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Graphics::Surface VESA_SCREEN;
	Graphics::Surface VESA_BUFFER;
	Mouse _mouse;

	/**
	 * Processes the loaded list of ini file parameters
	 */
	void processIniParams(Common::StringMap &iniParams);

	void INIT_SYSTEM();
	void Init_Interrupt();
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;

public:
	Globals _globals;

public:
	HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc);
	virtual ~HopkinsEngine();
	void GUIError(const Common::String &msg);

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	uint16 getVersion() const;
	bool getIsDemo() const;

	int getRandomNumber(int maxNumber);
};

// Global reference to the HopkinsEngine object
extern HopkinsEngine *g_vm;

#define GLOBALS g_vm->_globals

} // End of namespace Hopkins

#endif /* HOPKINS_H */
