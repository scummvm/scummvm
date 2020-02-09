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

#ifndef SLUDGE_SLUDGE_H
#define SLUDGE_SLUDGE_H

#include "common/random.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "gui/debugger.h"

#include "sludge/console.h"
#include "sludge/fileset.h"
#include "sludge/language.h"
#include "sludge/objtypes.h"
#include "sludge/timing.h"

namespace Sludge {

extern SludgeEngine *g_sludge;

class CursorManager;
class EventManager;
class FatalMsgManager;
class FloorManager;
class GraphicsManager;
class PeopleManager;
class RegionManager;
class SoundManager;
class SpeechManager;
class TextManager;

class SludgeConsole;

struct SludgeGameDescription;

// debug channels
enum {
	kSludgeDebugFatal = 1 << 0,
	kSludgeDebugDataLoad = 1 << 1,
	kSludgeDebugStackMachine = 1 << 2,
	kSludgeDebugBuiltin = 1 << 3,
	kSludgeDebugGraphics = 1 << 4,
	kSludgeDebugZBuffer = 1 << 5,
	kSludgeDebugSound = 1 << 6
};

class SludgeEngine: public Engine {
protected:
	// Engine APIs
	Common::Error run() override;

public:
	// global String variables
	Common::String launchMe;
	Common::String launchNext;
	Common::String loadNow;
	Common::String gamePath;

	// timer
	Timer _timer;

	// managers
	ResourceManager *_resMan;
	LanguageManager *_languageMan;
	ObjectManager *_objMan;
	GraphicsManager *_gfxMan;
	EventManager *_evtMan;
	SoundManager *_soundMan;
	TextManager *_txtMan;
	CursorManager *_cursorMan;
	SpeechManager *_speechMan;
	RegionManager *_regionMan;
	PeopleManager *_peopleMan;
	FloorManager *_floorMan;
	FatalMsgManager *_fatalMan;

	SludgeEngine(OSystem *syst, const SludgeGameDescription *gameDesc);
	~SludgeEngine() override;

	uint getLanguageID() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Graphics::PixelFormat *getScreenPixelFormat() const;
	Graphics::PixelFormat *getOrigPixelFormat() const;
	Common::RandomSource *getRandomSource() const { return _rnd; };

	const char *getGameFile() const;

	const SludgeGameDescription *_gameDescription;

private:
	Common::RandomSource *_rnd;
	Graphics::PixelFormat *_pixelFormat;
	Graphics::PixelFormat *_origFormat;
};

} // End of namespace Sludge

#endif
