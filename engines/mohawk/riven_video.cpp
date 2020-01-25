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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mohawk/riven_video.h"

#include "mohawk/cursors.h"
#include "mohawk/resource.h"
#include "mohawk/riven.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_scripts.h"
#include "mohawk/riven_stack.h"

#include "common/system.h"

#include "graphics/surface.h"

#include "video/qt_decoder.h"

namespace Mohawk {

RivenVideo::RivenVideo(MohawkEngine_Riven *vm, uint16 code) :
		_vm(vm),
		_id(0),
		_slot(code),
		_x(0),
		_y(0),
		_loop(false),
		_enabled(false),
		_video(nullptr),
		_playing(false) {
}

RivenVideo::~RivenVideo() {
	delete _video;
}

void RivenVideo::load(uint16 id) {
	if (id == _id && _video) {
		return;
	}

	close();

	_id = id;
	_video = new Video::QuickTimeDecoder();
	_video->setSoundType(Audio::Mixer::kSFXSoundType);
	_video->setChunkBeginOffset(_vm->getResourceOffset(ID_TMOV, id));
	_video->loadStream(_vm->getResource(ID_TMOV, id));
}

void RivenVideo::close() {
	stop();

	delete _video;
	_video = nullptr;
}

bool RivenVideo::endOfVideo() const {
	return !_video || _video->endOfVideo();
}

int RivenVideo::getCurFrame() const {
	assert(_video);
	return _video->getCurFrame();
}

uint32 RivenVideo::getFrameCount() const {
	assert(_video);
	return _video->getFrameCount();
}

uint32 RivenVideo::getTime() const {
	assert(_video);
	return _video->getTime();
}

uint32 RivenVideo::getDuration() const {
	assert(_video);
	return _video->getDuration().msecs();
}

void RivenVideo::seek(uint32 time) {
	assert(_video);

	if (time == 0) {
		// Fast path
		_video->rewind();
	} else {
		_video->seek(Audio::Timestamp(0, time, 600));
	}
}

void RivenVideo::pause(bool isPaused) {
	if (_video) {
		_video->pauseVideo(isPaused);
	}
}

void RivenVideo::stop() {
	if (_video) {
		_video->stop();
	}
	_playing = false;
}

bool RivenVideo::isPlaying() const {
	return _playing;
}

void RivenVideo::setVolume(int volume) {
	assert(_video);
	_video->setVolume(CLIP(volume, 0, 255));
}

RivenVideoManager::RivenVideoManager(MohawkEngine_Riven *vm) : _vm(vm) {
}

RivenVideoManager::~RivenVideoManager() {
	removeVideos();
}

void RivenVideoManager::pauseVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->pause(true);
}

void RivenVideoManager::resumeVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->pause(false);
}

void RivenVideoManager::closeVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++) {
		(*it)->close();
	}
}

void RivenVideoManager::removeVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++) {
		delete *it;
	}

	_videos.clear();
}

void RivenVideoManager::updateMovies() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++) {
		RivenVideo *video = *it;
		// Check of the video has reached the end
		if (video->endOfVideo()) {
			if (video->isPlaying() && video->isLooping()) {
				// Seek back if looping
				video->seek(0);
			} else {
				continue;
			}
		}

		// Check if we need to draw a frame
		if (video->needsUpdate()) {
			video->drawNextFrame();
		}
	}
}

void RivenVideo::drawNextFrame() {
	const Graphics::Surface *frame = _video->decodeNextFrame();

	if (!frame || !isEnabled()) {
		return;
	}

	Graphics::Surface *convertedFrame = nullptr;
	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();

	if (frame->format != pixelFormat) {
		// Convert to the current screen format
		convertedFrame = frame->convertTo(pixelFormat, _video->getPalette());
		frame = convertedFrame;
	}

	g_system->copyRectToScreen(frame->getPixels(), frame->pitch,
	                               _x, _y, _video->getWidth(), _video->getHeight());

	// Delete 8bpp conversion surface
	if (convertedFrame) {
		convertedFrame->free();
		delete convertedFrame;
	}
}

bool RivenVideo::needsUpdate() const {
	return _video && _video->isPlaying() && !_video->isPaused() && _video->needsUpdate();
}

void RivenVideo::playBlocking(int32 endTime) {
	_vm->_cursor->hideCursor();

	if (!_playing) {
		play();
	}

	// Sanity check
	if (isLooping())
		error("Called playBlocking() on a looping video");

	bool playTillEnd;
	if (endTime == -1) {
		playTillEnd = true;
	} else {
		playTillEnd = false;
		_video->setEndTime(Audio::Timestamp(0, endTime, 600));
	}

	if (playTillEnd) {
		enable();
	}

	bool continuePlaying = true;
	while (!endOfVideo() && !_vm->hasGameEnded() && continuePlaying) {
		// Draw a frame
		_vm->doFrame();

		// Handle skipping
		if (playTillEnd && _vm->getStack()->getAction() == kRivenActionSkip) {
			continuePlaying = false;

			// Seek to the last frame
			_video->seek(_video->getDuration().addMsecs(-1));

			_vm->getStack()->mouseForceUp();
			_vm->getStack()->resetAction();
		}
	}

	// Execute the stored opcode
	uint16 storedOpcodeMovieSlot = _vm->_scriptMan->getStoredMovieOpcodeSlot();
	uint32 storedOpcodeTime = _vm->_scriptMan->getStoredMovieOpcodeTime();
	if (_slot == storedOpcodeMovieSlot && getTime() >= storedOpcodeTime) { // CHECKME: Suspicious use of time units
		_vm->_scriptMan->runStoredMovieOpcode();
	}

	if (playTillEnd) {
		disable();
		stop();
		seek(0);
	}

	_vm->_cursor->showCursor();
}

void RivenVideo::play() {
	if (!_video) {
		load(_id);
	}

	if (_video->endOfVideo()) {
		_video->rewind();
	}

	_video->start();
	_playing = true;
}

void RivenVideo::enable() {
	_enabled = true;
}

void RivenVideo::disable() {
	if (needsUpdate()) {
		drawNextFrame();
	}

	if (_video) {
		Common::Rect targetRect = Common::Rect(_video->getWidth(), _video->getHeight());
		targetRect.translate(_x, _y);

		_vm->_gfx->copySystemRectToScreen(targetRect);
	}

	_enabled = false;
}

void RivenVideoManager::disableAllMovies() {
	debug(2, "Disabling all movies");
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->disable();
}

RivenVideo *RivenVideoManager::openSlot(uint16 slot) {
	// If this video is already playing, return that handle
	RivenVideo *oldHandle = getSlot(slot);
	if (oldHandle)
		return oldHandle;

	// Create the video
	RivenVideo *video = new RivenVideo(_vm, slot);

	// Add it to the video list
	_videos.push_back(video);

	return video;
}

RivenVideo *RivenVideoManager::getSlot(uint16 slot) {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		if ((*it)->getSlot() == slot)
			return *it;

	return nullptr;
}

} // End of namespace Mohawk
