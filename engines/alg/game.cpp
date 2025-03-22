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
#include "graphics/paletteman.h"

#include "alg/graphics.h"
#include "alg/scene.h"

#include "alg/game.h"

namespace Alg {

Game::Game(AlgEngine *vm) {
	_vm = vm;
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

Game::~Game() {
	_libFile.close();
	_libFileEntries.clear();
	delete _rnd;
	delete[] _palette;
	delete _screen;
	delete _background;
	delete _videoDecoder;
	delete _sceneInfo;
}

Common::Error Game::run() {
	return Common::kNoError;
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
		error("Can't open library file '%s'", path.toString().c_str());
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
	_videoDecoder->setStream(_libFile.readStream(_libFile.size()));
}

bool Game::loadScene(Scene *scene) {
	Common::String sceneFileName = Common::String::format("%s.mm", scene->name.c_str());
	Common::HashMap<Common::String, uint32>::iterator it = _libFileEntries.find(sceneFileName);
	if (it != _libFileEntries.end()) {
		debug("loaded scene %s", scene->name.c_str());
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

uint32 Game::_GetMsTime() {
	return g_system->getMillis();
}

bool Game::__Fired(Common::Point *point) {
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

Rect *Game::_CheckZone(Zone *zone, Common::Point *point) {
	Common::Array<Rect>::iterator rect;
	for (rect = zone->rects.begin(); rect != zone->rects.end(); ++rect) {
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
Zone *Game::_CheckZonesV1(Scene *scene, Rect *&hitRect, Common::Point *point) {
	Common::Array<Zone *>::iterator zone;
	for (zone = scene->zones.begin(); zone != scene->zones.end(); ++zone) {
		unsigned long startFrame = (*zone)->startFrame - _videoFrameSkip + 1;
		unsigned long endFrame = (*zone)->endFrame + _videoFrameSkip - 1;
		if (_currentFrame >= startFrame && _currentFrame <= endFrame) {
			hitRect = _CheckZone(*zone, point);
			if (hitRect != nullptr) {
				return *zone;
			}
		}
	}
	return nullptr;
}

// This is used by later games
Zone *Game::_CheckZonesV2(Scene *scene, Rect *&hitRect, Common::Point *point) {
	Common::Array<Zone *>::iterator zone;
	for (zone = scene->zones.begin(); zone != scene->zones.end(); ++zone) {
		unsigned long startFrame = (*zone)->startFrame - (_videoFrameSkip + 1) + ((_difficulty - 1) * _videoFrameSkip);
		unsigned long endFrame = (*zone)->endFrame + (_videoFrameSkip - 1) - ((_difficulty - 1) * _videoFrameSkip);
		if (_currentFrame >= startFrame && _currentFrame <= endFrame) {
			hitRect = _CheckZone(*zone, point);
			if (hitRect != nullptr) {
				return *zone;
			}
		}
	}
	return nullptr;
}

// only used by earlier games
void Game::_AdjustDifficulty(uint8 newDifficulty, uint8 oldDifficulty) {
	Common::Array<Scene *> *scenes = _sceneInfo->getScenes();
	for (size_t i = 0; i < scenes->size(); i++) {
		Scene *scene = (*scenes)[i];
		if (!(scene->diff & 0x01)) {
			if (scene->preop == "PAUSE" || scene->preop == "PAUSFI" || scene->preop == "PAUSPR") {
				scene->dataParam1 = (scene->dataParam1 * _pausdifscal[newDifficulty - 1]) / _pausdifscal[oldDifficulty - 1];
			}
		}
		for (size_t j = 0; j < scene->zones.size(); j++) {
			Zone *zone = scene->zones[j];
			for (size_t k = 0; k < zone->rects.size(); k++) {
				Rect *rect = &zone->rects[k];
				if (!(scene->diff & 0x02)) {
					int16 cx = (rect->left + rect->right) / 2;
					int16 cy = (rect->top + rect->bottom) / 2;
					int32 w = (rect->width() * _rectdifscal[newDifficulty - 1]) / _rectdifscal[oldDifficulty - 1];
					int32 h = (rect->height() * _rectdifscal[newDifficulty - 1]) / _rectdifscal[oldDifficulty - 1];
					rect->center(cx, cy, w, h);
				}
			}
		}
	}
}

void Game::_RestoreCursor() {
}

uint32 Game::_GetFrame(Scene *scene) {
	if (_videoDecoder->getCurrentFrame() == 0) {
		return scene->startFrame;
	}
	return scene->startFrame + (_videoDecoder->getCurrentFrame() * _videoFrameSkip) - _videoFrameSkip;
}

void Game::_SetFrame() {
}

int8 Game::_SkipToNewScene(Scene *scene) {
	if (!_gameInProgress || _sceneSkipped) {
		return 0;
	}
    if (scene->dataParam2 == -1) {
		_sceneSkipped = true;
		return -1;
    } else if (scene->dataParam2 > 0) {
        uint32 startFrame = scene->dataParam3;
        if (startFrame == 0) {
            startFrame = scene->startFrame + 15;
        }
        uint32 endFrame = scene->dataParam4;
        if (_currentFrame < endFrame && _currentFrame > startFrame) {
			_sceneSkipped = true;
            return 1;
        }
    }
	return 0;
}

// Sound
Audio::SeekableAudioStream *Game::_LoadSoundFile(const Common::Path &path) {
	Common::File *file = new Common::File();
	if (!file->open(path)) {
		warning("Can't open sound file '%s'", path.toString().c_str());
		delete file;
		return nullptr;
	}
	return Audio::makeRawStream(new Common::SeekableSubReadStream(file, 0, file->size(), DisposeAfterUse::NO), 8000, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
}

void Game::_PlaySound(Audio::SeekableAudioStream *stream) {
	if (stream != nullptr) {
		stream->rewind();
		g_system->getMixer()->stopHandle(_sfxAudioHandle);
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_sfxAudioHandle, stream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}
}

void Game::_DoDiffSound(uint8 difficulty) {
	switch (difficulty) {
	case 1:
		return _PlaySound(_easySound);
	case 2:
		return _PlaySound(_avgSound);
	case 3:
		return _PlaySound(_hardSound);
	}
}

void Game::_DoSaveSound() {
	_PlaySound(_saveSound);
}

void Game::_DoLoadSound() {
	_PlaySound(_loadSound);
}

void Game::_DoSkullSound() {
	_PlaySound(_skullSound);
}

void Game::_DoShot() {
	_PlaySound(_shotSound);
}

// Timer
static void _cursorTimer(void *refCon) {
	Game *game = static_cast<Game *>(refCon);
	game->runCursorTimer();
}

void Game::_SetupCursorTimer() {
	g_system->getTimerManager()->installTimerProc(&_cursorTimer, 1000000 / 50, (void *)this, "newtimer");
}

void Game::_RemoveCursorTimer() {
	g_system->getTimerManager()->removeTimerProc(&_cursorTimer);
}

void Game::runCursorTimer() {
	if (_gameTimer & 1) {
		_gameTimer++;
	} else {
		_gameTimer += 3;
	}
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
void Game::_zone_globalhit(Common::Point *point) {
	// do nothing
}

// Script functions: RectHit
void Game::_rect_hit_donothing(Rect *rect) {
	// do nothing
}

void Game::_rect_newscene(Rect *rect) {
	_score += rect->score;
	if (!rect->scene.empty()) {
		_cur_scene = rect->scene;
	}
}

void Game::_rect_easy(Rect *rect) {
	_DoDiffSound(1);
	_difficulty = 1;
}

void Game::_rect_average(Rect *rect) {
	_DoDiffSound(2);
	_difficulty = 2;
}

void Game::_rect_hard(Rect *rect) {
	_DoDiffSound(3);
	_difficulty = 3;
}

void Game::_rect_exit(Rect *rect) {
	_vm->quitGame();
}

// Script functions: Scene PreOps
void Game::_scene_pso_drawrct(Scene *scene) {
}

void Game::_scene_pso_pause(Scene *scene) {
	_hadPause = false;
	_pauseTime = 0;
}

void Game::_scene_pso_drawrct_fadein(Scene *scene) {
	_scene_pso_drawrct(scene);
	_scene_pso_fadein(scene);
}

void Game::_scene_pso_fadein(Scene *scene) {
	// do nothing
}

void Game::_scene_pso_pause_fadein(Scene *scene) {
	_scene_pso_pause(scene);
	_scene_pso_fadein(scene);
}

void Game::_scene_pso_preread(Scene *scene) {
	// do nothing
}

void Game::_scene_pso_pause_preread(Scene *scene) {
	_scene_pso_pause(scene);
	_scene_pso_preread(scene);
}

// Script functions: Scene Scene InsOps
void Game::_scene_iso_donothing(Scene *scene) {
	// do nothing
}

void Game::_scene_iso_startgame(Scene *scene) {
	_startscene = scene->insopParam;
}

void Game::_scene_iso_pause(Scene *scene) {
	bool checkPause = true;
	if (_hadPause) {
		checkPause = false;
	}
	if (_currentFrame > scene->endFrame) {
		checkPause = false;
	}
	if (scene->dataParam1 <= 0) {
		checkPause = false;
	}
	if (checkPause) {
		unsigned long pauseStart = atoi(scene->insopParam.c_str());
		unsigned long pauseEnd = atoi(scene->insopParam.c_str()) + _videoFrameSkip + 1;
		if (_currentFrame >= pauseStart && _currentFrame < pauseEnd && !_hadPause) {
			_gameTimer = 0;
			unsigned long pauseDuration = scene->dataParam1 * 0x90FF / 1000;
			_pauseTime = pauseDuration;
			_nextFrameTime += pauseDuration;
			_pauseTime += _GetMsTime();
			_hadPause = true;
		}
	}
	if (_pauseTime != 0) {
		if (_GetMsTime() > _pauseTime) {
			_pauseTime = 0;
		}
	}
}

// Script functions: Scene NxtScn
void Game::_scene_nxtscn_donothing(Scene *scene) {
	// do nothing
}

void Game::_scene_default_nxtscn(Scene *scene) {
	_cur_scene = scene->next;
}

// Script functions: ShowMsg
void Game::_scene_sm_donothing(Scene *scene) {
	// do nothing
}

// Script functions: ScnScr
void Game::_scene_default_score(Scene *scene) {
	if (scene->scnscrParam > 0) {
		_score += scene->scnscrParam;
	}
}

// Script functions: ScnNxtFrm
void Game::_scene_nxtfrm(Scene *scene) {
}

// debug methods
void Game::debug_drawZoneRects() {
	if (_debug_drawRects || debugChannelSet(1, Alg::kAlgDebugGraphics)) {
		if (_inMenu) {
			for (uint8 i = 0; i < _submenzone->rects.size(); i++) {
				Rect rect = _submenzone->rects[i];
				_screen->drawLine(rect.left, rect.top, rect.right, rect.top, 1);
				_screen->drawLine(rect.left, rect.top, rect.left, rect.bottom, 1);
				_screen->drawLine(rect.right, rect.bottom, rect.right, rect.top, 1);
				_screen->drawLine(rect.right, rect.bottom, rect.left, rect.bottom, 1);
			}
		} else if (_cur_scene != "") {
			Scene *targetScene = _sceneInfo->findScene(_cur_scene);
			for (uint8 i = 0; i < targetScene->zones.size(); i++) {
				Zone *zone = targetScene->zones[i];
				for (uint8 j = 0; j < zone->rects.size(); j++) {
					Rect rect = zone->rects[j];
					_screen->drawLine(rect.left, rect.top, rect.right, rect.top, 1);
					_screen->drawLine(rect.left, rect.top, rect.left, rect.bottom, 1);
					_screen->drawLine(rect.right, rect.bottom, rect.right, rect.top, 1);
					_screen->drawLine(rect.right, rect.bottom, rect.left, rect.bottom, 1);
				}
			}
		}
	}
}

bool Game::debug_dumpLibFile() {
	Common::DumpFile dumpFile;
	Common::HashMap<Common::String, uint32>::iterator entry;
	for (entry = _libFileEntries.begin(); entry != _libFileEntries.end(); ++entry) {
		_libFile.seek(entry->_value, SEEK_SET);
		uint32 size = _libFile.readUint32LE();
		Common::Path dumpFileName(Common::String::format("libDump/%s", entry->_key.c_str()));
		dumpFile.open(dumpFileName, true);
		assert(dumpFile.isOpen());
		dumpFile.writeStream(_libFile.readStream(size));
		dumpFile.flush();
		dumpFile.close();
	}
	return true;
}

} // End of namespace Alg
