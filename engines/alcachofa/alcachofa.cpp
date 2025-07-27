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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/framelimiter.h"
#include "video/mpegps_decoder.h"

#include "alcachofa.h"
#include "console.h"
#include "detection.h"
#include "player.h"
#include "rooms.h"
#include "script.h"
#include "global-ui.h"
#include "menu.h"
#include "debug.h"
#include "game.h"

using namespace Math;

namespace Alcachofa {

constexpr uint kDefaultFramerate = 100; // the original target framerate, not critical

AlcachofaEngine *g_engine;

AlcachofaEngine::AlcachofaEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst)
	, _gameDescription(gameDesc)
	, _randomSource("Alcachofa") {
	g_engine = this;
}

AlcachofaEngine::~AlcachofaEngine() {
}

uint32 AlcachofaEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String AlcachofaEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error AlcachofaEngine::run() {
	g_system->showMouse(false);
	setDebugger(_console);
	_game.reset(Game::createForMovieAdventure());
	_renderer.reset(IRenderer::createOpenGLRenderer(Common::Point(1024, 768)));
	_drawQueue.reset(new DrawQueue(_renderer.get()));
	_world.reset(new World());
	_script.reset(new Script());
	_player.reset(new Player());
	_globalUI.reset(new GlobalUI());
	_menu.reset(new Menu());

	_script->createProcess(MainCharacterKind::None, "CREDITOS_INICIALES");
	_scheduler.run();

	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, kDefaultFramerate, false);
	while (!shouldQuit()) {
		_input.nextFrame();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (_input.handleEvent(e))
				continue;
		}

		_sounds.update();
		_renderer->begin();
		_drawQueue->clear();
		_camera.shake() = Vector2d();
		_player->preUpdate();
		_player->currentRoom()->update();
		if (_player->currentRoom() != nullptr)
			_player->currentRoom()->draw();
		_player->postUpdate();
		if (_debugHandler != nullptr)
			_debugHandler->update();
		_renderer->end();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	return Common::kNoError;
}

void AlcachofaEngine::playVideo(int32 videoId) {
	Video::MPEGPSDecoder decoder;
	if (!decoder.loadFile(Common::Path(Common::String::format("Data/DATA%02d.BIN", videoId + 1))))
		error("Could not find video %d", videoId);
	_sounds.stopAll();
	auto texture = _renderer->createTexture(decoder.getWidth(), decoder.getHeight(), false);
	decoder.start();

	Common::Event e;
	while (!decoder.endOfVideo() && !shouldQuit()) {
		if (decoder.needsUpdate())
		{
			auto surface = decoder.decodeNextFrame();
			if (surface)
				texture->update(*surface);
			_renderer->begin();
			_renderer->setBlendMode(BlendMode::Alpha);
			_renderer->setLodBias(0.0f);
			_renderer->setTexture(texture.get());
			_renderer->quad({}, { (float)g_system->getWidth(), (float)g_system->getHeight() });
			_renderer->end();
		}

		_input.nextFrame();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (_input.handleEvent(e))
				continue;
		}
		if (_input.wasAnyMouseReleased() || _input.wasMenuKeyPressed())
			break;

		g_system->updateScreen();
		g_system->delayMillis(decoder.getTimeToNextFrame() / 2);
	}
	decoder.stop();
}

void AlcachofaEngine::fadeExit() {
	constexpr uint kFadeOutDuration = 1000;
	Event e;
	Graphics::FrameLimiter limiter(g_system, kDefaultFramerate, false);
	uint32 startTime = g_system->getMillis();

	_renderer->end(); // we were in a frame, let's exit
	while (g_system->getMillis() - startTime < kFadeOutDuration && !shouldQuit()) {
		_input.nextFrame();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (_input.handleEvent(e))
				continue;
		}

		_renderer->begin();
		_drawQueue->clear();
		float t = ((float)(g_system->getMillis() - startTime)) / kFadeOutDuration;
		// TODO: Implement cross-fade and add to fadeExit
		_drawQueue->add<FadeDrawRequest>(FadeType::ToBlack, t, -kForegroundOrderCount);
		_drawQueue->draw();
		_renderer->end();

		limiter.delayBeforeSwap();
		limiter.startFrame();
	}

	quitGame();
	player().changeRoom("SALIR", false); // this skips some update steps along the way
}

void AlcachofaEngine::setDebugMode(DebugMode mode, int32 param) {
	switch (mode)
	{
	case DebugMode::ClosestFloorPoint:
		_debugHandler.reset(new ClosestFloorPointDebugHandler(param));
		break;
	case DebugMode::FloorIntersections:
		_debugHandler.reset(new FloorIntersectionsDebugHandler(param));
		break;
	case DebugMode::TeleportCharacter:
		_debugHandler.reset(new TeleportCharacterDebugHandler(param));
		break;
	case DebugMode::FloorAlpha:
		_debugHandler.reset(FloorColorDebugHandler::create(param, false));
		break;
	case DebugMode::FloorColor:
		_debugHandler.reset(FloorColorDebugHandler::create(param, true));
		break;
	default: _debugHandler.reset(nullptr);
	}
	_input.toggleDebugInput(isDebugModeActive());
}

Common::Error AlcachofaEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

Config::Config() {
	loadFromScummVM();
}

void Config::loadFromScummVM() {
	_musicVolume = (uint8)CLIP(ConfMan.getInt("music_volume"), 0, 255);
	_speechVolume = (uint8)CLIP(ConfMan.getInt("speech_volume"), 0, 255);
	_subtitles = ConfMan.getBool("subtitles");
	_highQuality = ConfMan.getBool("high_quality");
	_bits32 = ConfMan.getBool("32_bits");
}

void Config::saveToScummVM() {
	ConfMan.setBool("subtitles", _subtitles);
	ConfMan.setBool("high_quality", _highQuality);
	ConfMan.setBool("32_bits", _bits32);
	ConfMan.setInt("music_volume", _musicVolume);
	ConfMan.setInt("speech_volume", _speechVolume);
	ConfMan.setInt("sfx_volume", _speechVolume);
	// ^ a bit unfortunate, that means if you change in-game it overrides.
	// if you set it in ScummVMs dialog it sticks
}

} // End of namespace Alcachofa
