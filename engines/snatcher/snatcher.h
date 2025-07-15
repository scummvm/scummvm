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

#ifndef SNATCHER_H
#define SNATCHER_H

#include "common/rect.h"
#include "engines/engine.h"
#include "snatcher/detection.h"
#include "snatcher/info.h"

class SnatcherMetaEngine;

namespace Snatcher {

class ActionSequenceHandler;
class CmdQueue;
class GraphicsEngine;
class FIO;
class MemAccessHandler;
class ResourcePointer;
class SaveLoadManager;
class ScriptEngine;
class SoundEngine;
class SceneModule;
class UI;

struct GameState;
struct Script;

class SnatcherEngine : public Engine {
friend class CmdQueue;
friend class SaveLoadManager;
public:
	SnatcherEngine(OSystem *system, GameDescription &dsc);
	~SnatcherEngine() override;

	SoundEngine *sound() const { return _snd; }
	GraphicsEngine *gfx() const { return _gfx; }

private:
	// Startup
	Common::Error run() override;
	bool initResource();
	bool initGfx(Common::Platform platform, bool use8BitColorMode);
	bool initSaveLoad();
	bool initSound(Audio::Mixer *mixer, Common::Platform platform, int soundOptions);
	bool initScriptEngine();
	void reset();
	void playBootLogoAnimation(const GameState &state);

	// Main loop
	bool start();
	void delayUntil(uint32 end);
	void checkEvents(const GameState &state);

	void updateMainState(GameState &state);
	void updateModuleState(GameState &state);

	const uint32 _frameLen;
	bool _reset;

	// ConfigManager sync
	void syncSoundSettings() override;

	// GMM, Save, Load
	void pauseEngineIntern(bool pause) override;
	bool canLoadGameStateCurrently(Common::U32String*) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	SaveLoadManager *_saveMan;

	// MetaEngine
	bool hasFeature(EngineFeature f) const override;
	GameDescription _game;

	// Resource
	FIO *_fio;
	SceneModule *_module;
	ResourcePointer *_scd;

	// Graphics
	GraphicsEngine *_gfx;
	VMInfo _gfxInfo;

	// Interface
	ActionSequenceHandler *_aseq;
	UI *_ui;

	// Sound
	SoundEngine *_snd;

	// Script
	CmdQueue *_cmdQueue;
	MemAccessHandler *_memHandler;
	ScriptEngine *_scriptEngine;

public:
	// Input
	struct Input {
		Input() : singleFrameControllerFlags(0), sustainedControllerFlags(0), singleFrameControllerFlagsRemapped(0), sustainedControllerFlagsRemapped(0), lightGunPos() {}
		uint16 singleFrameControllerFlags;
		uint16 sustainedControllerFlags;
		uint16 singleFrameControllerFlagsRemapped;
		uint16 sustainedControllerFlagsRemapped;
		Common::Point lightGunPos;
	};

	const Input &input() const { return _input; }
	void calibrateLightGun(GameState &state);
	void toggleKeyRepeat(bool enableRepeat) { _keyRepeat = enableRepeat; }
	void allowLightGunInput(bool enable);

private:
	uint16 _lastKeys;
	uint16 _releaseKeys;
	Common::Point _realLightGunPos;
	bool _keyRepeat;
	bool _enableLightGun;

	Input _input;
};

} // End of namespace Snatcher

#endif // SNATCHER_H
