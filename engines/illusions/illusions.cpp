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

#include "illusions/illusions.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/cursor.h"
#include "illusions/dictionary.h"
#include "illusions/resources/fontresource.h"
#include "illusions/graphics.h"
#include "illusions/input.h"
#include "illusions/resources/actorresource.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/resources/soundresource.h"
#include "illusions/resources/talkresource.h"
#include "illusions/resourcesystem.h"
#include "illusions/screen.h"
#include "illusions/screentext.h"
#include "illusions/sound.h"
#include "illusions/specialcode.h"
#include "illusions/thread.h"
#include "illusions/time.h"
#include "illusions/updatefunctions.h"

#include "illusions/threads/talkthread.h"

#include "audio/audiostream.h"
#include "video/video_decoder.h"
#include "video/avi_decoder.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Illusions {

IllusionsEngine::IllusionsEngine(OSystem *syst, const IllusionsGameDescription *gd) :
	Engine(syst), _gameDescription(gd) {
	
	_random = new Common::RandomSource("illusions");

	_rerunThreads = false;
	
	_isSaveAllowed = true; // TODO
	_resumeFromSavegameRequested = false;
	_savegameSceneId = 0;
	_savegameThreadId = 0;
	_nextTempThreadId = 0;

	Engine::syncSoundSettings();

}

IllusionsEngine::~IllusionsEngine() {

	delete _random;

}

void IllusionsEngine::updateEvents() {
	Common::Event event;
	while (_eventMan->pollEvent(event)) {
		_input->processEvent(event);
		switch (event.type) {
		case Common::EVENT_QUIT:
			quitGame();
			break;
		default:
			break;
		}
	}
}

void IllusionsEngine::runUpdateFunctions() {
	_updateFunctions->update();
}

Common::Point *IllusionsEngine::getObjectActorPositionPtr(uint32 objectId) {
	Control *control = getObjectControl(objectId);
	if (control && control->_actor)
		return &control->_actor->_position;
	return 0;
}

uint32 IllusionsEngine::getElapsedUpdateTime() {
	uint32 result = 0;
	uint32 currTime = getCurrentTime();
	if (_resGetCtr <= 0 ) {
		if (_unpauseControlActorFlag) {
			_unpauseControlActorFlag = false;
			result = 0;
		} else {
			result = currTime - _lastUpdateTime;
		}
		_lastUpdateTime = currTime;
	} else {
		result = _resGetTime - _lastUpdateTime;
		_lastUpdateTime = _resGetTime;
	}
	return result;
}

int IllusionsEngine::updateActors(uint flags) {
	// TODO Move to Controls class
	uint32 deltaTime = getElapsedUpdateTime();
	for (Controls::ItemsIterator it = _controls->_controls.begin(); it != _controls->_controls.end(); ++it) {
		Control *control = *it;
		if (control->_pauseCtr == 0 && control->_actor && control->_actor->_controlRoutine)
			control->_actor->runControlRoutine(control, deltaTime);
	}
	return kUFNext;
}

int IllusionsEngine::updateSequences(uint flags) {
	// TODO Move to Controls class
	for (Controls::ItemsIterator it = _controls->_controls.begin(); it != _controls->_controls.end(); ++it) {
		Control *control = *it;
		if (control->_pauseCtr == 0 && control->_actor && control->_actor->_seqCodeIp) {
			control->sequenceActor();
		}
	}
	return kUFNext;
}

int IllusionsEngine::updateGraphics(uint flags) {
	Common::Point panPoint(0, 0);

	uint32 currTime = getCurrentTime();
	
	_camera->update(currTime);
	updateFader();

	// TODO Move to BackgroundInstanceList class
	BackgroundInstance *backgroundItem = _backgroundInstances->findActiveBackgroundInstance();
	if (backgroundItem) {
		BackgroundResource *bgRes = backgroundItem->_bgRes;
		for (uint i = 0; i < bgRes->_bgInfosCount; ++i) {
			BgInfo *bgInfo = &bgRes->_bgInfos[i];
			uint32 priority = getPriorityFromBase(bgInfo->_priorityBase);
			_screen->_drawQueue->insertSurface(backgroundItem->_surfaces[i],
				bgInfo->_surfInfo._dimensions, backgroundItem->_panPoints[i], priority);
			if (bgInfo->_flags & 1)
				panPoint = backgroundItem->_panPoints[i];
		}
	}

	// TODO Move to Controls class
	for (Controls::ItemsIterator it = _controls->_controls.begin(); it != _controls->_controls.end(); ++it) {
		Control *control = *it;
		Actor *actor = control->_actor;
		if (control->_pauseCtr == 0 && actor && (actor->_flags & 1) && !(actor->_flags & 0x0200)) {
			Common::Point drawPosition = control->calcPosition(panPoint);
			if (actor->_flags & 0x2000) {
				Frame *frame = &(*actor->_frames)[actor->_frameIndex - 1];
				_screen->_decompressQueue->insert(&actor->_drawFlags, frame->_flags,
					frame->_surfInfo._pixelSize, frame->_surfInfo._dimensions,
					frame->_compressedPixels, actor->_surface);
				actor->_flags &= ~0x2000;
			}
			/* Unused
			if (actor->_flags & 0x4000) {
				nullsub_1(&actor->drawFlags);
				actor->flags &= ~0x4000;
			}
			*/
			if (actor->_surfInfo._dimensions._width && actor->_surfInfo._dimensions._height) {
				uint32 priority = control->getDrawPriority();
				_screen->_drawQueue->insertSprite(&actor->_drawFlags, actor->_surface,
					actor->_surfInfo._dimensions, drawPosition, control->_position,
					priority, actor->_scale, actor->_spriteFlags);
			}
		}
	}

	if (_screenText->_surface) {
		// TODO Make nicer
		uint32 priority = getGameId() == kGameIdDuckman ? getPriorityFromBase(19) : getPriorityFromBase(99);
		_screen->_drawQueue->insertTextSurface(_screenText->_surface, _screenText->_dimensions,
			_screenText->_position, priority);
	}

	return kUFNext;
}

int IllusionsEngine::updateSoundMan(uint flags) {
	_soundMan->update();
	return kUFNext;
}

int IllusionsEngine::updateSprites(uint flags) {
	_screen->updateSprites();
	_screenPalette->updatePalette();
	return kUFNext;
}

int IllusionsEngine::getRandom(int max) {
	return _random->getRandomNumber(max - 1);
}

int IllusionsEngine::convertPanXCoord(int16 x) {
	// TODO
	return 0;
}

bool IllusionsEngine::calcPointDirection(Common::Point &srcPt, Common::Point &dstPt, uint &facing) {
	facing = 0;
	uint xd = 0, yd = 0;
	if (srcPt.x < dstPt.x)
		xd = 0x40;
	else if (srcPt.x > dstPt.x)
		xd = 0x04;
	else
		xd = 0x00;
	if (srcPt.y < dstPt.y)
		yd = 0x01;
	else if (srcPt.y > dstPt.y)
		yd = 0x10;
	else
		yd = 0x00;
	if (!xd && !yd)
		facing = 0;
	else if (!yd && xd)
		facing = xd;
	else if (yd && !xd)
		facing = yd;
	else if (xd == 0x04 && yd == 0x01)
		facing = 0x02;
	else if (xd == 0x40 && yd == 0x01)
		facing = 0x80;
	else if (xd == 0x04 && yd == 0x10)
		facing = 0x08;
	else if (xd == 0x40 && yd == 0x10)
		facing = 0x20;
	return facing != 0;
}

void IllusionsEngine::playVideo(uint32 videoId, uint32 objectId, uint32 priority, uint32 threadId) {
	Video::VideoDecoder *videoDecoder = new Video::AVIDecoder();
	Common::String filename = Common::String::format("%08X.AVI", objectId);
	if (!videoDecoder->loadFile(filename)) {
		delete videoDecoder;
		warning("Unable to open video %s", filename.c_str());
		return;
	}

	videoDecoder->start();

	bool skipVideo = false;

	while (!shouldQuit() && !videoDecoder->endOfVideo() && !skipVideo) {
		if (videoDecoder->needsUpdate()) {
			const Graphics::Surface *frame = videoDecoder->decodeNextFrame();
			if (videoDecoder->hasDirtyPalette()) {
				const byte *palette = videoDecoder->getPalette();
				_system->getPaletteManager()->setPalette(palette, 0, 256);
			}

			if (frame) {
				_system->copyRectToScreen(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
				_system->updateScreen();
			}
		}

		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) ||
				event.type == Common::EVENT_LBUTTONUP)
				skipVideo = true;
		}
	}

	videoDecoder->close();
	delete videoDecoder;
}

bool IllusionsEngine::isSoundActive() {
	// TODO
	return true;
}

void IllusionsEngine::setCurrFontId(uint32 fontId) {
	_fontId = fontId;
}

bool IllusionsEngine::checkActiveTalkThreads() {
	return _threads->isActiveThread(kMsgQueryTalkThreadActive);
}

void IllusionsEngine::setTextDuration(int kind, uint32 duration) {
	_field8 = kind;
	switch (_field8) {
	case 1:
	case 2:
		_fieldA = 0;
		break;
	case 3:
	case 4:
		_fieldA = duration;
		break;
	default:
		break;
	}
}

uint32 IllusionsEngine::clipTextDuration(uint32 duration) {
	switch (_field8) {
	case 2:
		if (duration == 0)
			duration = 240;
		break;
	case 3:
		if (duration < _fieldA)
			duration = _fieldA;
		break;
	case 4:
		if (duration > _fieldA)
			duration = _fieldA;
		break;
	}
	return duration;
}

void IllusionsEngine::getDefaultTextDimensions(WidthHeight &dimensions) {
	dimensions = _defaultTextDimensions;
}

void IllusionsEngine::setDefaultTextDimensions(WidthHeight &dimensions) {
	_defaultTextDimensions = dimensions;
}

void IllusionsEngine::getDefaultTextPosition(Common::Point &position) {
	position = _defaultTextPosition;
}

void IllusionsEngine::setDefaultTextPosition(Common::Point &position) {
	_defaultTextPosition = position;
}

FramesList *IllusionsEngine::findActorSequenceFrames(Sequence *sequence) {
	return _actorInstances->findSequenceFrames(sequence);
}

} // End of namespace Illusions
