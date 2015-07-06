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
#include "nancy/logo.h"

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
	kGameTypeNone = 0,
	kGameTypeNancy1,
	kGameTypeNancy2,
	kGameTypeNancy3
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
class LogoSequence;

class NancyEngine : public Engine {
public:
	friend class LogoSequence;

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

	static NancyEngine *create(GameType type, OSystem *syst, const NancyGameDescription *gd);

protected:
	// Engine APIs
	Common::Error run();

	enum {
		kMaxFilenameLen = 32
	};

	struct Image {
		Common::String name;
		uint16 width;
		uint16 height;
	};

	struct Sound {
		Common::String name;
	};

	enum GameState {
		kBoot,
		kLogo,
		kIdle
	};

	struct GameFlow {
		GameState minGameState;
	};

	typedef Common::Array<Image> ImageList;

	Common::SeekableReadStream *_bsum;
	ImageList _logos, _frames;
	Sound _menuSound;
	GameFlow _gameFlow;

	void preloadCals(const IFF &boot);
	void readImageList(const IFF &boot, const Common::String &prefix, ImageList &list);
	void readSound(const IFF &boot, const Common::String &name, Sound &sound);
	Common::String readFilename(Common::ReadStream *stream) const;

	virtual uint getFilenameLen() const = 0;
	virtual void readBootSummary(const IFF &boot) = 0;

private:
	static NancyEngine *s_Engine;

	NancyConsole *_console;
	GameType _gameType;
	Common::Platform _platform;

	LogoSequence *_logoSequence;

	void initialize();
};

} // End of namespace Nancy

#endif // Nancy_H
