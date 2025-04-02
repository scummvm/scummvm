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

#include "snatcher/detection.h"
#include "snatcher/info.h"
#include "engines/engine.h"

class SnatcherMetaEngine;

namespace Snatcher {

class GraphicsEngine;
class FIO;
class SoundEngine;
class SceneModule;
struct GameState;

class SnatcherEngine : public Engine {
public:
	SnatcherEngine(OSystem *system, GameDescription &dsc);
	~SnatcherEngine() override;

	SoundEngine *sound() const { return _snd; }
	GraphicsEngine *gfx() const { return _gfx; }

private:
	// Startup
	Common::Error run() override;
	bool initResource();
	bool initGfx(Common::Platform platform);
	bool initSound(Common::Platform platform, int soundOptions);

	// Main loop
	bool start();
	void delayUntil(uint32 end);
	void updateEvents();

	bool runInitSequence(GameState &state);
	void updateModuleState(GameState &state);

	// ConfigManager sync
	void registerDefaultSettings();
	void syncSoundSettings() override;

	// GMM
	void pauseEngineIntern(bool pause) override;

	// MetaEngine
	bool hasFeature(EngineFeature f) const override;
	GameDescription _game;

	// Resource
	FIO *_fio;
	SceneModule *_module;

	// Graphics
	GraphicsEngine *_gfx;
	VMInfo _gfxInfo;

	// Sound
	SoundEngine *_snd;

public:
	// Input
	byte _commandsFromMain;
};

} // End of namespace Snatcher

#endif // SNATCHER_H
