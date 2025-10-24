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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/framelimiter.h"
#include "graphics/thumbnail.h"
#include "graphics/managed_surface.h"
#include "image/png.h"
#include "video/avi_decoder.h"
#include "video/mpegps_decoder.h"

#include "alcachofa/alcachofa.h"
#include "alcachofa/metaengine.h"
#include "alcachofa/console.h"
#include "alcachofa/detection.h"
#include "alcachofa/player.h"
#include "alcachofa/rooms.h"
#include "alcachofa/script.h"
#include "alcachofa/global-ui.h"
#include "alcachofa/menu.h"
#include "alcachofa/debug.h"
#include "alcachofa/game.h"

using namespace Math;
using namespace Graphics;

namespace Alcachofa {

constexpr uint kDefaultFramerate = 100; // the original target framerate, not critical

AlcachofaEngine *g_engine;

AlcachofaEngine::AlcachofaEngine(OSystem *syst, const AlcachofaGameDescription *gameDesc)
	: Engine(syst)
	, _gameDescription(gameDesc)
	, _eventLoopSemaphore("engine") {
	assert(gameDesc != nullptr);
	g_engine = this;
}

AlcachofaEngine::~AlcachofaEngine() {
	// do not delete, this is done by `Engine::~Engine` with `delete _debugger;`
	_console = nullptr; //-V773
}

uint32 AlcachofaEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String AlcachofaEngine::getGameId() const {
	return _gameDescription->desc.gameId;
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
	setMillis(0);
	game().onLoadedGameFiles();

	if (!tryLoadFromLauncher()) {
		_script->createProcess(MainCharacterKind::None, "CREDITOS_INICIALES");
		_scheduler.run();
		// we run once to set the initial room, otherwise we could run into currentRoom == nullptr
	}

	Common::Event e;
	Graphics::FrameLimiter limiter(g_system, kDefaultFramerate, false);
	while (!shouldQuit()) {
		_input.nextFrame();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (_input.handleEvent(e))
				continue;
			if (e.type == EVENT_CUSTOM_ENGINE_ACTION_START &&
				e.customType == (CustomEventType)EventAction::LoadFromMenu)
				menu().triggerLoad();
		}

		_sounds.update();
		_renderer->begin();
		_drawQueue->clear();
		_camera.shake() = Vector2d();
		_player->preUpdate();
		if (_player->currentRoom() != nullptr)
			_player->currentRoom()->update();
		if (_player->currentRoom() != nullptr) // update() might clear currentRoom
			_player->currentRoom()->draw();
		_player->postUpdate();
		if (_debugHandler != nullptr)
			_debugHandler->update();
		_renderer->end();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		if (!_renderer->hasOutput()) {
			limiter.delayBeforeSwap();
			g_system->updateScreen();
		}
		// else we just rendered to some surface and will use it in the next frame
		// no need to update the screen or wait 
		limiter.startFrame();
	}

	return Common::kNoError;
}

void AlcachofaEngine::playVideo(int32 videoId) {
	if (game().isKnownBadVideo(videoId)) {
		warning("Skipping known bad video %d", videoId);
		return;
	}

	// Video files are either MPEG PS or AVI
	FakeLock lock("playVideo", _eventLoopSemaphore);
	File *file = new File();
	if (!file->open(Path(Common::String::format("Data/DATA%02d.BIN", videoId + 1)))) {
		delete file;
		game().invalidVideo(videoId, "open file");
		return;
	}
	char magic[4];
	if (file->read(magic, sizeof(magic)) != sizeof(magic) || !file->seek(0)) {
		delete file;
		game().invalidVideo(videoId, "read magic");
		return;
	}
	ScopedPtr<Video::VideoDecoder> decoder;
	if (memcmp(magic, "RIFF", sizeof(magic)) == 0)
		decoder.reset(new Video::AVIDecoder());
	else
		decoder.reset(new Video::MPEGPSDecoder());
	if (!decoder->loadStream(file)) {
		game().invalidVideo(videoId, "decode video");
		return;
	}
	decoder->setOutputPixelFormat(g_engine->renderer().getPixelFormat());

	Vector2d texMax(1.0f, 1.0f);
	int16 texWidth = decoder->getWidth(), texHeight = decoder->getHeight();
	ManagedSurface tmpSurface;
	if (_renderer->requiresPoTTextures() &&
		(!isPowerOfTwo(texWidth) || !isPowerOfTwo(texHeight))) {
		texWidth = nextHigher2(texWidth);
		texHeight = nextHigher2(texHeight);
		texMax = {
			decoder->getWidth() / (float)texWidth,
			decoder->getHeight() / (float)texHeight,
		};
		tmpSurface.create(texWidth, texHeight, _renderer->getPixelFormat());
	}
	auto texture = _renderer->createTexture(texWidth, texHeight, false);

	Common::Event e;
	_sounds.stopAll();
	decoder->start();
	while (!decoder->endOfVideo() && !shouldQuit()) {
		if (decoder->needsUpdate()) {
			auto surface = decoder->decodeNextFrame();
			if (surface) {
				if (tmpSurface.empty())
					texture->update(*surface);
				else {
					tmpSurface.blitFrom(*surface);
					texture->update(tmpSurface);
				}
			}
			_renderer->begin();
			_renderer->setBlendMode(BlendMode::Alpha);
			_renderer->setLodBias(0.0f);
			_renderer->setTexture(texture.get());
			_renderer->quad(
				{},
				{ (float)g_system->getWidth(), (float)g_system->getHeight() },
				kWhite,
				{},
				{},
				texMax);
			_renderer->end();
			g_system->updateScreen();
		}

		_input.nextFrame();
		while (g_system->getEventManager()->pollEvent(e)) {
			if (_input.handleEvent(e))
				continue;
		}
		if (_input.wasAnyMouseReleased() || _input.wasMenuKeyPressed())
			break;

		g_system->delayMillis(decoder->getTimeToNextFrame());
	}
	decoder->stop();
}

void AlcachofaEngine::fadeExit() {
	constexpr uint kFadeOutDuration = 1000;
	FakeLock lock("fadeExit", _eventLoopSemaphore);
	Event e;
	Graphics::FrameLimiter limiter(g_system, kDefaultFramerate, false);
	uint32 startTime = g_system->getMillis();

	Room *room = g_engine->player().currentRoom();
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
		_drawQueue->add<FadeDrawRequest>(FadeType::ToBlack, t, -kForegroundOrderCount);
		if (room != nullptr)
			room->draw(); // this executes the drawQueue as well
		_renderer->end();

		limiter.delayBeforeSwap();
		g_system->updateScreen();
		limiter.startFrame();
	}

	quitGame();
	player().changeRoom("SALIR", false); // this skips some update steps along the way
}

void AlcachofaEngine::setDebugMode(DebugMode mode, int32 param) {
	switch (mode) {
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
	default:
		_debugHandler.reset(nullptr);
		break;
	}
	_input.toggleDebugInput(isDebugModeActive());
}

uint32 AlcachofaEngine::getMillis() const {
	// Time is stored in savestate at various points e.g. to persist animation progress
	// We wrap the system-provided time to offset it to the expected game-time
	// This would also double as playtime
	return g_system->getMillis() - _timeNegOffset + _timePosOffset;
}

void AlcachofaEngine::setMillis(uint32 newMillis) {
	const uint32 sysMillis = g_system->getMillis();
	if (newMillis > sysMillis) {
		_timeNegOffset = 0;
		_timePosOffset = newMillis - sysMillis;
	} else {
		_timeNegOffset = sysMillis - newMillis;
		_timePosOffset = 0;
	}
}

void AlcachofaEngine::pauseEngineIntern(bool pause) {
	// Audio::Mixer also implements recursive pausing,
	// so ScummVM pausing and Menu pausing will not conflict
	_sounds.pauseAll(pause);

	if (pause)
		_timeBeforePause = getMillis();
	else
		setMillis(_timeBeforePause);
}

bool AlcachofaEngine::canLoadGameStateCurrently(U32String *msg) {
	if (!_eventLoopSemaphore.isReleased())
		return false;
	return
		(menu().isOpen() && menu().interactionSemaphore().isReleased()) ||
		player().isAllowedToOpenMenu();
}

Common::String AlcachofaEngine::getSaveStatePattern() {
	return getMetaEngine()->getSavegameFilePattern(_targetName.c_str());
}

Common::Error AlcachofaEngine::syncGame(MySerializer &s) {
	if (!s.syncVersion((Serializer::Version)kCurrentSaveVersion))
		return { kUnknownError, "Gamestate version is higher than expected" };

	Graphics::ManagedSurface *thumbnail = nullptr;
	if (s.isSaving()) {
		thumbnail = new Graphics::ManagedSurface();
		getSavegameThumbnail(*thumbnail->surfacePtr());
	}
	bool couldSyncThumbnail = syncThumbnail(s, thumbnail);
	if (thumbnail != nullptr)
		delete thumbnail;
	if (!couldSyncThumbnail)
		return { kUnknownError, "Could not read thumbnail" };

	uint32 millis = menu().isOpen()
		? menu().millisBeforeMenu()
		: getMillis();
	s.syncAsUint32LE(millis);
	if (s.isLoading())
		setMillis(millis);

	/* Some notes about the order:
	 * 1. The scheduler should prepare due to our FakeSemaphores
	 *    By destructing all previous processes we also release all locks and
	 *    can assert that the semaphores are released on loading.
	 * 2. The player should come late as it changes the room
	 * 3. With the room current, the tasks can now better find the referenced objects
	 * 4. Redundant: The world has to be synced before the tasks to reset the semaphores to 0
	 */

	scheduler().prepareSyncGame(s);
	world().syncGame(s);
	camera().syncGame(s);
	script().syncGame(s);
	globalUI().syncGame(s);
	player().syncGame(s);
	scheduler().syncGame(s);

	if (s.isLoading()) {
		menu().resetAfterLoad();
		sounds().stopAll();
		sounds().setMusicToRoom(player().currentRoom()->musicID());
	}

	return Common::kNoError;
}

static constexpr uint32 kNoThumbnailMagicValue = 0xBADBAD;

bool AlcachofaEngine::syncThumbnail(MySerializer &s, Graphics::ManagedSurface *thumbnail) {
	if (s.isLoading()) {
		auto prevPosition = s.readStream().pos();
		Image::PNGDecoder pngDecoder;
		if (pngDecoder.loadStream(s.readStream()) && pngDecoder.getSurface() != nullptr) {
			if (thumbnail != nullptr) {
				thumbnail->free();
				thumbnail->copyFrom(*pngDecoder.getSurface());
			}
		} else {
			// If we do not get a thumbnail, maybe we get at least the marker that there is no thumbnail
			s.readStream().seek(prevPosition, SEEK_SET);
			uint32 magicValue = s.readStream().readUint32LE();
			if (magicValue != kNoThumbnailMagicValue)
				return false; // the savegame is not valid
			else // this is not an error, just a pity
				warning("No thumbnail stored in in-game savestate");
		}
	} else {
		if (thumbnail == nullptr ||
			thumbnail->getPixels() == nullptr ||
			!Image::writePNG(s.writeStream(), *thumbnail)) {
			// We were not able to get a thumbnail, save a value that denotes that situation
			warning("Could not save in-game thumbnail");
			s.writeStream().writeUint32LE(kNoThumbnailMagicValue);
		}
	}
	return true;
}

void AlcachofaEngine::getSavegameThumbnail(Graphics::Surface &thumbnail) {
	thumbnail.free();

	auto *bigThumbnail = g_engine->menu().getBigThumbnail();
	if (bigThumbnail != nullptr) {
		// we still have a one from the in-game menu opening, reuse that
		thumbnail.copyFrom(*bigThumbnail);
		return;
	}

	// otherwise we have to rerender
	thumbnail.create(kBigThumbnailWidth, kBigThumbnailHeight, g_engine->renderer().getPixelFormat());
	if (g_engine->player().currentRoom() == nullptr)
		return; // but without a room we would render only black anyway

	g_engine->drawQueue().clear();
	g_engine->renderer().begin();
	g_engine->renderer().setOutput(thumbnail);
	g_engine->player().currentRoom()->draw(); // drawQueue is drawn here as well
	g_engine->renderer().end();

	// we should be within the event loop. as such it is quite safe to mess with the drawQueue or renderer
}

bool AlcachofaEngine::tryLoadFromLauncher() {
	int saveSlot = ConfMan.getInt("save_slot");
	if (!ConfMan.hasKey("save_slot") || saveSlot < 0)
		return false;
	auto *saveFile = g_system->getSavefileManager()->openForLoading(getSaveStateName(saveSlot));
	if (saveFile == nullptr)
		return false;
	bool result = loadGameStream(saveFile).getCode() == kNoError;
	delete saveFile;
	return result;
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
	ConfMan.flushToDisk();
	// ^ a bit unfortunate, that means if you change in-game it overrides.
	// if you set it in ScummVMs dialog it sticks
}

DelayTask::DelayTask(Process &process, uint32 millis)
	: Task(process)
	, _endTime(millis) {}

DelayTask::DelayTask(Process &process, Serializer &s)
	: Task(process) {
	DelayTask::syncGame(s);
}

TaskReturn DelayTask::run() {
	TASK_BEGIN;
	_endTime += g_engine->getMillis();
	while (g_engine->getMillis() < _endTime)
		TASK_YIELD(1);
	TASK_END;
}

void DelayTask::debugPrint() {
	uint32 remaining = g_engine->getMillis() <= _endTime ? _endTime - g_engine->getMillis() : 0;
	g_engine->getDebugger()->debugPrintf("Delay for further %ums\n", remaining);
}

void DelayTask::syncGame(Serializer &s) {
	Task::syncGame(s);
	s.syncAsUint32LE(_endTime);
}

DECLARE_TASK(DelayTask)

} // End of namespace Alcachofa
