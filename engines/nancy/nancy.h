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

#ifndef NANCY_H
#define NANCY_H

#include "engines/engine.h"
#include "common/file.h"
#include "nancy/console.h"

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the Nancy engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Nancy Drew 1
 * - ...
 */
namespace Nancy {

static const int kSavegameVersion = 1;

enum GameType {
	kGameTypeNone  = 0,
	kGameTypeNancy1,
	kGameTypeNancy2
};

enum NancyDebugChannels {
	kDebugSchedule  = 1 <<  0,
	kDebugEngine    = 1 <<  1,
	kDebugDisplay   = 1 <<  2,
	kDebugMouse     = 1 <<  3,
	kDebugParser    = 1 <<  4,
	kDebugFile      = 1 <<  5,
	kDebugRoute     = 1 <<  6,
	kDebugInventory = 1 <<  7,
	kDebugObject    = 1 <<  8,
	kDebugMusic     = 1 <<  9
};

struct NancyGameDescription;

class ResourceManager;
class IFF;

class NancyEngine : public Engine {
public:
	NancyEngine(OSystem *syst, const NancyGameDescription *gd);
	~NancyEngine();

	OSystem *_system;

	GUI::Debugger *getDebugger();

	Common::RandomSource *_rnd;

	ResourceManager *_res;

	const NancyGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void initGame(const NancyGameDescription *gd);

	GameType getGameType() const;
	Common::Platform getPlatform() const;

	bool hasFeature(EngineFeature f) const;
	const char *getCopyrightString() const;

	Common::String getSavegameFilename(int slot);
	void syncSoundSettings();

protected:

	// Engine APIs
	Common::Error run();

private:
	static NancyEngine *s_Engine;

	NancyConsole *_console;
	GameType _gameType;
	Common::Platform _platform;

	void initialize();
	void preloadCals(const IFF &boot);
};

} // End of namespace Nancy

#endif // Nancy_H
