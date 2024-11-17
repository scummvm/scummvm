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

#include "engines/metaengine.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ext/wme_vlink.h"

#include "common/timer.h"
#include "common/substream.h"

#ifdef USE_BINK
#include "video/bink_decoder.h"
#endif

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXVlink, false)

BaseScriptable *makeSXVlink(BaseGame *inGame, ScStack *stack) {
	return new SXVlink(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXVlink::SXVlink(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	stack->correctParams(1);
	uint32 handle = (uint32)stack->pop()->getInt();
	if (handle != 'D3DH') {
		warning("SXVlink() Invalid D3D handle");
	}
	_volume = 100;
}

//////////////////////////////////////////////////////////////////////////
SXVlink::~SXVlink() {
	_videoDecoder = nullptr;
}

//////////////////////////////////////////////////////////////////////////
const char *SXVlink::scToString() {
	return "[binkvideo object]";
}

void SXVlink::timerCallback(void *instance) {
	SXVlink *movie = static_cast<SXVlink *>(instance);
	Common::StackLock lock(movie->_frameMutex);
	movie->prepareFrame();
}

void SXVlink::prepareFrame() {
	if (_videoDecoder->endOfVideo()) {
		_videoFinished = true;
		return;
	}

	if (_videoDecoder->getTimeToNextFrame() > 0)
		return;

	const Graphics::Surface *decodedFrame = _videoDecoder->decodeNextFrame();
	if (decodedFrame) {
		_surface = *decodedFrame;
		if (_frame != _videoDecoder->getCurFrame()) {
			_updateNeeded = true;
		}
		_frame = _videoDecoder->getCurFrame();
	}
}

//////////////////////////////////////////////////////////////////////////
bool SXVlink::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Play(string path)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Play") == 0) {
		stack->correctParams(1);
		const char *path = stack->pop()->getString();

#ifdef USE_BINK
		_gameRef->freeze();
		((WintermuteEngine *)g_engine)->savingEnable(false);

		Common::SeekableReadStream *file = BaseFileManager::getEngineInstance()->openFile(path);
		if (file) {
			Common::SeekableReadStream *bink = new Common::SeekableSubReadStream(file, 0, file->size(), DisposeAfterUse::NO);
			if (bink) {
				_videoDecoder = new Video::BinkDecoder();
				if (_videoDecoder && _videoDecoder->loadStream(bink) && _videoDecoder->isVideoLoaded()) {
					_videoDecoder->setOutputPixelFormat(Graphics::PixelFormat(_gameRef->_renderer->getPixelFormat()));
					BaseSurface *texture = _gameRef->_renderer->createSurface();
					texture->create(_videoDecoder->getWidth(), _videoDecoder->getHeight());

					_gameRef->_renderer->setup2D();

					_frame = -1;
					_updateNeeded = false;
					_videoFinished = false;

					_videoDecoder->start();
					_videoDecoder->setVolume(_volume);

					g_system->getTimerManager()->installTimerProc(&timerCallback, 10000, this, "movieLoop");

					do {
						if (_updateNeeded) {
							{
								Common::StackLock lock(_frameMutex);
								texture->startPixelOp();
								texture->putSurface(_surface, false);
								texture->endPixelOp();
							}
							texture->display(0, 0, Rect32(texture->getWidth(), texture->getHeight()));
							_updateNeeded = false;
							_gameRef->_renderer->flip();
						}
						g_system->delayMillis(10);

						Common::Event event;
						while (g_system->getEventManager()->pollEvent(event)) {
							if (event.type == Common::EVENT_KEYDOWN) {
								if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
									_videoFinished = true;
									g_system->getEventManager()->purgeKeyboardEvents();
								}
							} else if (event.type == Common::EVENT_LBUTTONDOWN) {
								_videoFinished = true;
							} else if (event.type == Common::EVENT_SCREEN_CHANGED) {
								_gameRef->_renderer->onWindowChange();
							}
						}
					} while (!g_engine->shouldQuit() && !_videoFinished);

					g_system->getTimerManager()->removeTimerProc(&timerCallback);

					{
						Common::StackLock lock(_frameMutex);
						_videoDecoder->stop();
						_videoDecoder->close();
					}

					delete texture;
				}
				delete _videoDecoder;
			}
			BaseFileManager::getEngineInstance()->closeFile(file);
		}

		((WintermuteEngine *)g_engine)->savingEnable(true);
		_gameRef->unfreeze();
#else
		warning("SXVlink::Play(%s) Bink playback not compiled in", path);
#endif

		stack->pushNULL();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetVolume(int level)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetVolume") == 0) {
		stack->correctParams(1);
		_volume = stack->pop()->getInt();

		stack->pushNULL();
		return STATUS_OK;
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXVlink::scGetProperty(const Common::String &name) {
	_scValue->setNULL();
	return _scValue;
}


//////////////////////////////////////////////////////////////////////////
bool SXVlink::scSetProperty(const char *name, ScValue *value) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool SXVlink::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	persistMgr->transferSint32(TMEMBER(_volume));
	return STATUS_OK;
}

} // End of namespace Wintermute
