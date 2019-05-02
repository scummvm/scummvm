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

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_videoplayer.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/screen.h"
#include "engines/util.h"

namespace Illusions {

// BBDOUVideoPlayer

BBDOUVideoPlayer::BBDOUVideoPlayer(IllusionsEngine_BBDOU *vm)
	: _vm(vm), _videoDecoder(0), _callingThreadId(0), _objectId(0) {
}

BBDOUVideoPlayer::~BBDOUVideoPlayer() {
	delete _videoDecoder;
}

void BBDOUVideoPlayer::start(uint32 videoId, uint32 objectId, uint32 priority, uint32 callingThreadId) {
	debug(0, "BBDOUVideoPlayer::play(%08X, %08X, %d, %08X)", videoId, objectId, priority, callingThreadId);
	notifyCallingThread();
	_objectId = objectId;
	_callingThreadId = callingThreadId;
	Control *videoControl = _vm->_dict->getObjectControl(objectId);
	videoControl->_flags |= 0x0008;
	_vm->_input->discardAllEvents();
	Common::String filename = Common::String::format("%08x.avi", videoId);
	_videoDecoder = new Video::AVIDecoder();
	if (!_videoDecoder->loadFile(filename)) {
		delete _videoDecoder;
		_videoDecoder = 0;
		warning("Unable to open video %s", filename.c_str());
		notifyCallingThread();
		return;
	}
	_videoDecoder->start();
}

void BBDOUVideoPlayer::stop() {
	_vm->_input->discardAllEvents();
	delete _videoDecoder;
	_videoDecoder = 0;
	notifyCallingThread();
	_objectId = 0;
}

void BBDOUVideoPlayer::update() {
	if (_vm->_input->pollEvent(kEventAbort) || _videoDecoder->endOfVideo()) {
		stop();
	} else if (_videoDecoder->needsUpdate()) {
		Control *videoControl = _vm->_dict->getObjectControl(_objectId);
		const Graphics::Surface *frame = _videoDecoder->decodeNextFrame();
		Graphics::Surface *backSurface = videoControl->_actor->_surface;
		if (frame->format.bytesPerPixel == g_system->getScreenFormat().bytesPerPixel) {
			const int width = MIN(frame->w, backSurface->w) * frame->format.bytesPerPixel;
			const int height = MIN(frame->h, backSurface->h);
			const byte *src = (const byte*)frame->getPixels();
			byte *dest = (byte*)backSurface->getPixels();
			for (int yc = 0; yc < height; ++yc) {
				memcpy(dest, src, width);
				src += frame->pitch;
				dest += backSurface->pitch;
			}
		}
		ActorType *actorType = _vm->_dict->findActorType(videoControl->_actorTypeId);
		videoControl->_actor->_frameIndex = 1;
		videoControl->_actor->_surfInfo = actorType->_surfInfo;
		videoControl->appearActor();
		videoControl->deactivateObject();
		videoControl->_actor->_flags &= ~0x2000;
	}
}

bool BBDOUVideoPlayer::isPlaying() const {
	return _videoDecoder != 0;
}

void BBDOUVideoPlayer::notifyCallingThread() {
	if (_callingThreadId != 0) {
		_vm->notifyThreadId(_callingThreadId);
		_callingThreadId = 0;
	}
}

} // End of namespace Illusions
