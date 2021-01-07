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

#include "nancy/console.h"
#include "nancy/detection.h"

#include "engines/engine.h"
#include "common/file.h"

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
class SceneManager;
class Logic;
class GraphicsManager;
class InputManager;

class NancyEngine : public Engine {
public:
	friend class Logic;
	friend class LogoSequence;
	friend class SceneManager;
	friend class NancyConsole;

	NancyEngine(OSystem *syst, const NancyGameDescription *gd);
	~NancyEngine();

	OSystem *_system;

	GUI::Debugger *getDebugger();

	Common::RandomSource *_rnd;


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

	bool launchConsole;
	
	// Chunks found in BOOT get extracted and cached at startup, this function lets other classes access them
	Common::SeekableReadStream *getBootChunkStream(const Common::String &name);

	// Managers
	ResourceManager *_res;
	Logic *logic;
	SceneManager *sceneManager;
	GraphicsManager *graphics;
	InputManager *input;

protected:
	// Engine APIs
	Common::Error run();

	void bootGameEngine();

	bool addBootChunk(const Common::String &name, Common::SeekableReadStream *stream);
	void clearBootChunks();

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
		kPartnerLogo, // v2 only
		kLogo,
		kCredits,
		kMap, // v0, v1 only
		kMainMenu,
		kLoadSave,
		kSetup,
		// unknown/invalid
		kHelp,
		kScene,
		// CD change
		kCheat,
		kQuit,
		// regain focus
		kIdle
	};

	struct GameFlow {
		GameState minGameState;
		GameState previousGameState;
	};

	typedef Common::Array<Image> ImageList;

	ImageList _logos;
	ImageList _frames;
	ImageList _objects;
	uint16 _firstSceneID;
	int32 _fontSize;
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
	Common::HashMap<Common::String, Common::SeekableReadStream *> _bootChunks;

	void initialize();
};

} // End of namespace Nancy

#endif // NANCY_H