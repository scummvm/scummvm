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

#ifndef SLUDGE_SLUDGE_H
#define SLUDGE_SLUDGE_H

#include "common/hash-str.h"

#include "engines/engine.h"

namespace Common {
class RandomSource;
}

namespace Graphics {
struct PixelFormat;
}

namespace Sludge {

class CursorManager;
class EventManager;
class FatalMsgManager;
class FloorManager;
class GraphicsManager;
class LanguageManager;
class ObjectManager;
class PeopleManager;
class ResourceManager;
class RegionManager;
class SoundManager;
class SpeechManager;
class StatusBarManager;
class TextManager;
class Timer;

struct SludgeGameDescription;

#define IN_THE_CENTRE 65535

// debug channels
enum {
	kSludgeDebugFatal = 1,
	kSludgeDebugDataLoad,
	kSludgeDebugStackMachine,
	kSludgeDebugBuiltin,
	kSludgeDebugGraphics,
	kSludgeDebugZBuffer,
	kSludgeDebugSound,
};

class SludgeEngine: public Engine {
protected:
	// Engine APIs
	Common::Error run() override;

public:
	// global String variables
	Common::String launchNext;
	Common::String loadNow;
	Common::String gamePath;

	// timer
	Timer *_timer;

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
	StatusBarManager *_statusBar;

	bool _dumpScripts;

	SludgeEngine(OSystem *syst, const SludgeGameDescription *gameDesc);
	~SludgeEngine() override;

	uint getLanguageID() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::String getTargetName() const { return _targetName; }
	Common::Language getLanguage() const;
	Graphics::PixelFormat *getScreenPixelFormat() const;
	Graphics::PixelFormat *getOrigPixelFormat() const;
	Common::RandomSource *getRandomSource() const { return _rnd; };

	const char *getGameFile() const;

	bool hasFeature(EngineFeature f) const override;

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) override;
	Common::Error loadGameState(int slot) override;

	const SludgeGameDescription *_gameDescription;

	Common::HashMap<Common::String, int> _saveNameToSlot;

private:
	Common::RandomSource *_rnd;
	Graphics::PixelFormat *_pixelFormat;
	Graphics::PixelFormat *_origFormat;
};

extern SludgeEngine *g_sludge;

} // End of namespace Sludge

#endif
