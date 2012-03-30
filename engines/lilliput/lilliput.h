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

#ifndef LILLIPUT_LILLIPUT_H
#define LILLIPUT_LILLIPUT_H

#include "engines/engine.h"
#include "common/file.h"
#include "lilliput/console.h"

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the Robin engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Adventures of Robin Hood
 * - Rome: Pathway to Rome
 */
namespace Lilliput {

static const int kSavegameVersion = 1;

enum GameType {
	kGameTypeNone  = 0,
	kGameTypeRobin,
	kGameTypeRome
};

enum LilliputDebugChannels {
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

struct LilliputGameDescription;

class LilliputEngine : public Engine {
public:
	LilliputEngine(OSystem *syst, const LilliputGameDescription *gd);
	~LilliputEngine();

	OSystem *_system;

	GUI::Debugger *getDebugger();

	Common::RandomSource *_rnd;

	const LilliputGameDescription *_gameDescription;
	uint32 getFeatures() const;
	const char *getGameId() const;

	void initGame(const LilliputGameDescription *gd);

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
	static LilliputEngine *s_Engine;

	LilliputConsole *_console;
	GameType _gameType;
	Common::Platform _platform;

	void initialize();
};

} // End of namespace Lilliput

#endif
