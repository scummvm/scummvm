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

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/events.h"
#include "common/substream.h"
#include "common/timer.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "alg/graphics.h"
#include "alg/scene.h"

#include "alg/game.h"

namespace Alg {

Game::Game(AlgEngine *vm) {
	_vm = vm;
}

Game::~Game() {
	_libFile.close();
	_libFileEntries.clear();
	delete _rnd;
	delete[] _palette;
	delete _videoDecoder;
	delete _sceneInfo;
	if (_background) {
		_background->free();
		delete _background;
	}
	if (_screen) {
		_screen->free();
		delete _screen;
	}
	for (auto item : *_gun) {
		if (item) {
			item->free();
			delete item;
		}
	}
	for (auto item : *_numbers) {
		if (item) {
			item->free();
			delete item;
		}
	}
	delete _saveSound;
	delete _loadSound;
	delete _easySound;
	delete _avgSound;
	delete _hardSound;
	delete _skullSound;
	delete _shotSound;
	delete _emptySound;
}

void Game::init() {
	_inMenu = false;
	_palette = new uint8[257 * 3]();
	// blue for rect display
	_palette[5] = 0xFF;
	_paletteDirty = true;
	_screen = new Graphics::Surface();
	_rnd = new Common::RandomSource("alg");
	_screen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	_videoDecoder = new AlgVideoDecoder();
	_videoDecoder->setPalette(_palette);
	_sceneInfo = new SceneInfo();
}

Common::Error Game::run() {
	return Common::kNoError;
}

void Game::shutdown() {
	g_system->getMixer()->stopAll();
	_vm->quitGame();
}

bool Game::pollEvents() {
	Common::Event event;
	bool hasEvents = false;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_MOUSEMOVE) {
			_mousePos = event.mouse;
		} else if (event.type == Common::EVENT_LBUTTONDOWN) {
			_leftDown = true;
			_mousePos = event.mouse;
		} else if (event.type == Common::EVENT_RBUTTONDOWN) {
			_rightDown = true;
			_mousePos = event.mouse;
		} else if (event.type == Common::EVENT_LBUTTONUP) {
			_leftDown = false;
			_mousePos = event.mouse;
		} else if (event.type == Common::EVENT_RBUTTONUP) {
			_rightDown = false;
			_mousePos = event.mouse;
		}
		hasEvents = true;
	}
	return hasEvents;
}

void Game::loadLibArchive(const Common::Path &path) {
	debug("loading lib archive: %s", path.toString().c_str());
	if (!_libFile.open(path)) {
		error("Game::loadLibArchive(): Can't open library file '%s'", path.toString().c_str());
	}
	uint16 magicBytes = _libFile.readSint16LE();
	uint32 indexOffset = _libFile.readSint32LE();
	assert(magicBytes == 1020);
	_libFile.seek(indexOffset);
	uint16 indexSize = _libFile.readSint16LE();
	assert(indexSize > 0);
	while (true) {
		uint32 entryOffset = _libFile.readSint32LE();
		Common::String entryName = _libFile.readStream(13)->readString();
		if (entryName.empty()) {
			break;
		}
		entryName.toLowercase();
		_libFileEntries[entryName] = entryOffset;
	}
	_libFile.seek(0);
	_videoDecoder->setInputFile(&_libFile);
}

bool Game::loadScene(Scene *scene) {
	Common::String sceneFileName = Common::String::format("%s.mm", scene->_name.c_str());
	auto it = _libFileEntries.find(sceneFileName);
	if (it != _libFileEntries.end()) {
		debug("loaded scene %s", scene->_name.c_str());
		_videoDecoder->loadVideoFromStream(it->_value);
		return true;
	} else {
		return false;
	}
}

void Game::updateScreen() {
	if (!_inMenu) {
		Graphics::Surface *frame = _videoDecoder->getVideoFrame();
		_screen->copyRectToSurface(frame->getPixels(), frame->pitch, _videoPosX, _videoPosY, frame->w, frame->h);
	}
	debug_drawZoneRects();
	if (_paletteDirty || _videoDecoder->isPaletteDirty()) {
		g_system->getPaletteManager()->setPalette(_palette, 0, 256);
		_paletteDirty = false;
	}
	g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch, 0, 0, _screen->w, _screen->h);
	g_system->updateScreen();
}

uint32 Game::getMsTime() {
	return g_system->getMillis();
}

bool Game::fired(Common::Point *point) {
	_fired = false;
	pollEvents();
	if (_leftDown == true) {
		if (_buttonDown) {
			return false;
		}
		_fired = true;
		point->x = _mousePos.x;
		point->y = _mousePos.y;
		_buttonDown = true;
		return true;
	} else {
		_buttonDown = false;
		return false;
	}
}

Rect *Game::checkZone(Zone *zone, Common::Point *point) {
	for (auto &rect : zone->_rects) {
		if (point->x >= rect->left &&
			point->x <= rect->right &&
			point->y >= rect->top &&
			point->y <= rect->bottom) {
			return rect;
		}
	}
	return nullptr;
}

// This is used by earlier games
Zone *Game::checkZonesV1(Scene *scene, Rect *&hitRect, Common::Point *point) {
	for (auto &zone : scene->_zones) {
		uint32 startFrame = zone->_startFrame - _videoFrameSkip + 1;
		uint32 endFrame = zone->_endFrame + _videoFrameSkip - 1;
		if (_currentFrame >= startFrame && _currentFrame <= endFrame) {
			hitRect = checkZone(zone, point);
			if (hitRect != nullptr) {
				return zone;
			}
		}
	}
	return nullptr;
}

// This is used by later games
Zone *Game::checkZonesV2(Scene *scene, Rect *&hitRect, Common::Point *point) {
	for (auto &zone : scene->_zones) {
		uint32 startFrame = zone->_startFrame - (_videoFrameSkip + 1) + ((_difficulty - 1) * _videoFrameSkip);
		uint32 endFrame = zone->_endFrame + (_videoFrameSkip - 1) - ((_difficulty - 1) * _videoFrameSkip);
		if (_currentFrame >= startFrame && _currentFrame <= endFrame) {
			hitRect = checkZone(zone, point);
			if (hitRect != nullptr) {
				return zone;
			}
		}
	}
	return nullptr;
}

// only used by earlier games
void Game::adjustDifficulty(uint8 newDifficulty, uint8 oldDifficulty) {
	Common::Array<Scene *> *scenes = _sceneInfo->getScenes();
	for (const auto &scene : *scenes) {
		if (!(scene->_diff & 0x01)) {
			if (scene->_preop == "PAUSE" || scene->_preop == "PAUSFI" || scene->_preop == "PAUSPR") {
				scene->_dataParam1 = (scene->_dataParam1 * _pauseDiffScale[newDifficulty - 1]) / _pauseDiffScale[oldDifficulty - 1];
			}
		}
		for (const auto &zone : scene->_zones) {
			for (const auto &rect : zone->_rects) {
				if (!(scene->_diff & 0x02)) {
					int16 cx = (rect->left + rect->right) / 2;
					int16 cy = (rect->top + rect->bottom) / 2;
					int32 w = (rect->width() * _rectDiffScale[newDifficulty - 1]) / _rectDiffScale[oldDifficulty - 1];
					int32 h = (rect->height() * _rectDiffScale[newDifficulty - 1]) / _rectDiffScale[oldDifficulty - 1];
					rect->center(cx, cy, w, h);
				}
			}
		}
	}
}

uint32 Game::getFrame(Scene *scene) {
	if (_videoDecoder->getCurrentFrame() == 0) {
		return scene->_startFrame;
	}
	return scene->_startFrame + (_videoDecoder->getCurrentFrame() * _videoFrameSkip) - _videoFrameSkip;
}

int8 Game::skipToNewScene(Scene *scene) {
	if (!_gameInProgress || _sceneSkipped) {
		return 0;
	}
	if (scene->_dataParam2 == -1) {
		_sceneSkipped = true;
		return -1;
	} else if (scene->_dataParam2 > 0) {
		uint32 startFrame = scene->_dataParam3;
		if (startFrame == 0) {
			startFrame = scene->_startFrame + 15;
		}
		uint32 endFrame = scene->_dataParam4;
		if (_currentFrame < endFrame && _currentFrame > startFrame) {
			_sceneSkipped = true;
			return 1;
		}
	}
	return 0;
}

uint16 Game::randomUnusedInt(uint8 max, uint16 *mask, uint16 exclude) {
	if (max == 1) {
		return 0;
	}
	// reset mask if full
	uint16 fullMask = 0xFFFF >> (16 - max);
	if (*mask == fullMask) {
		*mask = 0;
	}
	uint16 randomNum = 0;
	// find an unused random number
	while (true) {
		randomNum = _rnd->getRandomNumber(max - 1);
		// check if bit is already used
		uint16 bit = 1 << randomNum;
		if (!((*mask & bit) || randomNum == exclude)) {
			// set the bit in mask
			*mask |= bit;
			break;
		}
	}
	return randomNum;
}

// Sound
Audio::SeekableAudioStream *Game::loadSoundFile(const Common::Path &path) {
	Common::File *file = new Common::File();
	if (!file->open(path)) {
		warning("Game::loadSoundFile(): Can't open sound file '%s'", path.toString().c_str());
		delete file;
		return nullptr;
	}
	return Audio::makeRawStream(file, 8000, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
}

void Game::playSound(Audio::SeekableAudioStream *stream) {
	if (stream != nullptr) {
		stream->rewind();
		g_system->getMixer()->stopHandle(_sfxAudioHandle);
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_sfxAudioHandle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}
}

void Game::doDiffSound(uint8 difficulty) {
	switch (difficulty) {
	case 1:
		return playSound(_easySound);
	case 2:
		return playSound(_avgSound);
	case 3:
		return playSound(_hardSound);
	}
}

void Game::doSaveSound() {
	playSound(_saveSound);
}

void Game::doLoadSound() {
	playSound(_loadSound);
}

void Game::doSkullSound() {
	playSound(_skullSound);
}

void Game::doShot() {
	playSound(_shotSound);
}

// Timer
static void cursorTimerCallback(void *refCon) {
	Game *game = static_cast<Game *>(refCon);
	game->runCursorTimer();
}

void Game::setupCursorTimer() {
	g_system->getTimerManager()->installTimerProc(&cursorTimerCallback, 1000000 / 50, (void *)this, "cursortimer");
}

void Game::removeCursorTimer() {
	g_system->getTimerManager()->removeTimerProc(&cursorTimerCallback);
}

void Game::runCursorTimer() {
	_thisGameTimer += 2;
	if (_whichGun == 9) {
		if (_emptyCount > 0) {
			_emptyCount--;
		} else {
			_whichGun = 0;
		}
	} else {
		if (_shotFired) {
			_whichGun++;
			if (_whichGun > 5) {
				_whichGun = 0;
				_shotFired = false;
			}
		} else {
			if (_inHolster > 0) {
				_inHolster--;
				if (_inHolster == 0 && _whichGun == 7) {
					_whichGun = 6;
				}
			}
		}
	}
}

// Script functions: Zone
void Game::zoneGlobalHit(Common::Point *point) {
	// do nothing
}

// Script functions: RectHit
void Game::rectHitDoNothing(Rect *rect) {
	// do nothing
}

void Game::rectNewScene(Rect *rect) {
	_score += rect->_score;
	if (!rect->_scene.empty()) {
		_curScene = rect->_scene;
	}
}

void Game::rectEasy(Rect *rect) {
	doDiffSound(1);
	_difficulty = 1;
}

void Game::rectAverage(Rect *rect) {
	doDiffSound(2);
	_difficulty = 2;
}

void Game::rectHard(Rect *rect) {
	doDiffSound(3);
	_difficulty = 3;
}

void Game::rectExit(Rect *rect) {
	shutdown();
}

// Script functions: Scene PreOps
void Game::scenePsoDrawRct(Scene *scene) {
}

void Game::scenePsoPause(Scene *scene) {
	_hadPause = false;
	_pauseTime = 0;
}

void Game::scenePsoDrawRctFadeIn(Scene *scene) {
	scenePsoDrawRct(scene);
	scenePsoFadeIn(scene);
}

void Game::scenePsoFadeIn(Scene *scene) {
	// do nothing
}

void Game::scenePsoPauseFadeIn(Scene *scene) {
	scenePsoPause(scene);
	scenePsoFadeIn(scene);
}

void Game::scenePsoPreRead(Scene *scene) {
	// do nothing
}

void Game::scenePsoPausePreRead(Scene *scene) {
	scenePsoPause(scene);
	scenePsoPreRead(scene);
}

// Script functions: Scene Scene InsOps
void Game::sceneIsoDoNothing(Scene *scene) {
	// do nothing
}

void Game::sceneIsoStartGame(Scene *scene) {
	_startScene = scene->_insopParam;
}

void Game::sceneIsoPause(Scene *scene) {
	bool checkPause = true;
	if (_hadPause) {
		checkPause = false;
	}
	if (_currentFrame > scene->_endFrame) {
		checkPause = false;
	}
	if (scene->_dataParam1 <= 0) {
		checkPause = false;
	}
	if (checkPause) {
		uint32 pauseStart = atoi(scene->_insopParam.c_str());
		uint32 pauseEnd = atoi(scene->_insopParam.c_str()) + _videoFrameSkip + 1;
		if (_currentFrame >= pauseStart && _currentFrame < pauseEnd && !_hadPause) {
			uint32 pauseDuration = scene->_dataParam1 * 0x90FF / 1000;
			_pauseTime = pauseDuration;
			_nextFrameTime += pauseDuration;
			_pauseTime += getMsTime();
			_hadPause = true;
		}
	}
	if (_pauseTime != 0) {
		if (getMsTime() > _pauseTime) {
			_pauseTime = 0;
		}
	}
}

// Script functions: Scene NxtScn
void Game::sceneNxtscnDoNothing(Scene *scene) {
	// do nothing
}

void Game::sceneDefaultNxtscn(Scene *scene) {
	_curScene = scene->_next;
}

// Script functions: ShowMsg
void Game::sceneSmDonothing(Scene *scene) {
	// do nothing
}

// Script functions: ScnScr
void Game::sceneDefaultScore(Scene *scene) {
	if (scene->_scnscrParam > 0) {
		_score += scene->_scnscrParam;
	}
}

// Script functions: ScnNxtFrm
void Game::sceneNxtfrm(Scene *scene) {
}

// debug methods
void Game::debug_drawZoneRects() {
	if (_debug_drawRects || debugChannelSet(1, Alg::kAlgDebugGraphics)) {
		if (_inMenu) {
			for (auto rect : _submenzone->_rects) {
				_screen->drawLine(rect->left, rect->top, rect->right, rect->top, 1);
				_screen->drawLine(rect->left, rect->top, rect->left, rect->bottom, 1);
				_screen->drawLine(rect->right, rect->bottom, rect->right, rect->top, 1);
				_screen->drawLine(rect->right, rect->bottom, rect->left, rect->bottom, 1);
			}
		} else if (_curScene != "") {
			Scene *targetScene = _sceneInfo->findScene(_curScene);
			for (auto &zone : targetScene->_zones) {
				for (auto rect : zone->_rects) {
					_screen->drawLine(rect->left, rect->top, rect->right, rect->top, 1);
					_screen->drawLine(rect->left, rect->top, rect->left, rect->bottom, 1);
					_screen->drawLine(rect->right, rect->bottom, rect->right, rect->top, 1);
					_screen->drawLine(rect->right, rect->bottom, rect->left, rect->bottom, 1);
				}
			}
		}
	}
}

bool Game::debug_dumpLibFile() {
	Common::DumpFile dumpFile;
	for (auto &entry : _libFileEntries) {
		_libFile.seek(entry._value, SEEK_SET);
		uint32 size = _libFile.readUint32LE();
		Common::Path dumpFileName(Common::String::format("libDump/%s", entry._key.c_str()));
		dumpFile.open(dumpFileName, true);
		assert(dumpFile.isOpen());
		dumpFile.writeStream(_libFile.readStream(size));
		dumpFile.flush();
		dumpFile.close();
	}
	return true;
}

} // End of namespace Alg
