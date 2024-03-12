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

#include "mohawk/mohawk.h"
#include "mohawk/resource.h"
#include "mohawk/video.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/textconsole.h"
#include "common/system.h"

#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "video/qt_decoder.h"


namespace Mohawk {

VideoEntry::VideoEntry() : _video(nullptr), _id(-1), _x(0), _y(0), _loop(false), _enabled(true) {
}

VideoEntry::VideoEntry(Video::VideoDecoder *video, const Common::Path &fileName) : _video(video), _fileName(fileName), _id(-1), _x(0), _y(0), _loop(false), _enabled(true) {
}

VideoEntry::VideoEntry(Video::VideoDecoder *video, int id) : _video(video), _id(id), _x(0), _y(0), _loop(false), _enabled(true) {
}

VideoEntry::~VideoEntry() {
	close();
}

void VideoEntry::close() {
	delete _video;
	_video = nullptr;

	if (_subtitles.isLoaded()) {
		g_system->hideOverlay();
	}
}

bool VideoEntry::endOfVideo() const {
	return !isOpen() || _video->endOfVideo();
}

int VideoEntry::getCurFrame() const {
	assert(_video);
	return _video->getCurFrame();
}

uint32 VideoEntry::getFrameCount() const {
	assert(_video);
	return _video->getFrameCount();
}

uint32 VideoEntry::getTime() const {
	assert(_video);
	return _video->getTime();
}

Audio::Timestamp VideoEntry::getDuration() const {
	assert(_video);
	return _video->getDuration();
}

Common::Rational VideoEntry::getRate() const {
	assert(_video);
	return _video->getRate();
}

void VideoEntry::center() {
	assert(_video);
	_x = (g_system->getWidth() - _video->getWidth()) / 2;
	_y = (g_system->getHeight() - _video->getHeight()) / 2;
}

void VideoEntry::setBounds(const Audio::Timestamp &startTime, const Audio::Timestamp &endTime) {
	assert(_video);
	_start = startTime;
	_video->setEndTime(endTime);
	_video->seek(startTime);
}

void VideoEntry::seek(const Audio::Timestamp &time) {
	assert(_video);
	_video->seek(time);
}

void VideoEntry::setRate(const Common::Rational &rate) {
	assert(_video);
	_video->setRate(rate);
}

void VideoEntry::pause(bool isPaused) {
	assert(_video);
	_video->pauseVideo(isPaused);

	if (_subtitles.isLoaded()) {
		if (isPaused) {
			g_system->hideOverlay();
		} else {
			g_system->showOverlay(false);
			g_system->clearOverlay();
			_subtitles.drawSubtitle(_video->getTime(), true);
		}
	}
}

void VideoEntry::start() {
	assert(_video);
	_video->start();

	if (_subtitles.isLoaded()) {
		const int16 h = g_system->getOverlayHeight(),
			        w = g_system->getOverlayWidth();
		_subtitles.setBBox(Common::Rect(20, h - 120, w - 20, h - 20));
		_subtitles.setColor(0xff, 0xff, 0xff);
		_subtitles.setFont("FreeSans.ttf");

		g_system->showOverlay(false);
		g_system->clearOverlay();
	}
}

void VideoEntry::stop() {
	assert(_video);
	_video->stop();

	if (_subtitles.isLoaded()) {
		g_system->hideOverlay();
	}
}

bool VideoEntry::isPlaying() const {
	assert(_video);
	return _video->isPlaying();
}

int VideoEntry::getVolume() const {
	assert(_video);
	return _video->getVolume();
}

void VideoEntry::setVolume(int volume) {
	assert(_video);
	_video->setVolume(CLIP(volume, 0, 255));
}

VideoManager::VideoManager(MohawkEngine *vm) : _vm(vm) {
	// Set dithering enabled, if required
	_enableDither = (_vm->getGameType() == GType_MYST || _vm->getGameType() == GType_MAKINGOF) && !_vm->isGameVariant(GF_ME);
}

VideoManager::~VideoManager() {
	stopVideos();
}

void VideoManager::pauseVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->pause(true);
}

void VideoManager::resumeVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->pause(false);
}

void VideoManager::stopVideos() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->close();

	_videos.clear();
}

VideoEntryPtr VideoManager::playMovie(const Common::Path &fileName, Audio::Mixer::SoundType soundType) {
	VideoEntryPtr ptr = open(fileName, soundType);
	if (!ptr)
		return VideoEntryPtr();

	Common::String baseName(fileName.baseName());

	Common::String subtitlesName = Common::String::format("%s.srt", baseName.substr(0, baseName.size() - 4).c_str());
	ptr->loadSubtitles(fileName.getParent().appendComponent(subtitlesName));

	ptr->start();
	return ptr;
}

VideoEntryPtr VideoManager::playMovie(uint16 id) {
	VideoEntryPtr ptr = open(id);
	if (!ptr)
		return VideoEntryPtr();

	ptr->start();
	return ptr;
}

bool VideoManager::updateMovies() {
	bool updateScreen = false;

	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); ) {
		// Check of the video has reached the end
		if ((*it)->endOfVideo()) {
			if ((*it)->isLooping()) {
				// Seek back if looping
				(*it)->seek((*it)->getStart());
			} else {
				// Done; close and continue on
				(*it)->close();
				it = _videos.erase(it);
				continue;
			}
		}

		Video::VideoDecoder *video = (*it)->_video;

		// Ignore paused videos
		if (video->isPaused()) {
			it++;
			continue;
		}

		// Check if we need to draw a frame
		if (video->needsUpdate()) {
			if (drawNextFrame(*it)) {
				updateScreen = true;
			}

			updateScreen |= (*it)->_subtitles.drawSubtitle(video->getTime(), false);
		}

		// Remember to increase the iterator
		it++;
	}

	// Return true if we need to update the screen
	return updateScreen;
}

bool VideoManager::drawNextFrame(VideoEntryPtr videoEntry) {
	Video::VideoDecoder *video = videoEntry->_video;
	const Graphics::Surface *frame = video->decodeNextFrame();

	if (!frame || !videoEntry->isEnabled()) {
		return false;
	}

	Graphics::Surface *convertedFrame = nullptr;
	Graphics::PixelFormat pixelFormat = _vm->_system->getScreenFormat();

	if (frame->format != pixelFormat) {
		// We don't support downconverting to 8bpp without having
		// support in the codec. Set _enableDither if shows up.
		if (pixelFormat.bytesPerPixel == 1) {
			warning("Cannot convert high color video frame to 8bpp");
			return false;
		}

		// Convert to the current screen format
		convertedFrame = frame->convertTo(pixelFormat, video->getPalette());
		frame = convertedFrame;
	} else if (pixelFormat.bytesPerPixel == 1 && video->hasDirtyPalette()) {
		// Set the palette when running in 8bpp mode only
		// Don't do this for Myst, which has its own per-stack handling
		if (_vm->getGameType() != GType_MYST)
			_vm->_system->getPaletteManager()->setPalette(video->getPalette(), 0, 256);
	}

	// Clip the video to make sure it stays on the screen (Myst does this a few times)
	Common::Rect targetRect = Common::Rect(video->getWidth(), video->getHeight());
	targetRect.translate(videoEntry->getX(), videoEntry->getY());

	Common::Rect frameRect = Common::Rect(video->getWidth(), video->getHeight());

	if (targetRect.left < 0) {
		frameRect.left -= targetRect.left;
		targetRect.left = 0;
	}

	if (targetRect.top < 0) {
		frameRect.top -= targetRect.top;
		targetRect.top = 0;
	}

	if (targetRect.right > _vm->_system->getWidth()) {
		frameRect.right -= targetRect.right - _vm->_system->getWidth();
		targetRect.right = _vm->_system->getWidth();
	}

	if (targetRect.bottom > _vm->_system->getHeight()) {
		frameRect.bottom -= targetRect.bottom - _vm->_system->getHeight();
		targetRect.bottom = _vm->_system->getHeight();
	}

	_vm->_system->copyRectToScreen(frame->getBasePtr(frameRect.left, frameRect.top), frame->pitch,
	                               targetRect.left, targetRect.top, targetRect.width(), targetRect.height());

	// Delete 8bpp conversion surface
	if (convertedFrame) {
		convertedFrame->free();
		delete convertedFrame;
	}

	// We've drawn something to the screen, make sure we update it
	return true;
}

VideoEntryPtr VideoManager::open(uint16 id) {
	// If this video is already playing, return that handle
	VideoEntryPtr oldVideo = findVideo(id);
	if (oldVideo)
		return oldVideo;

	// Otherwise, create a new entry
	Video::QuickTimeDecoder *video = new Video::QuickTimeDecoder();
	video->setChunkBeginOffset(_vm->getResourceOffset(ID_TMOV, id));
	video->loadStream(_vm->getResource(ID_TMOV, id));

	// Create the entry
	VideoEntryPtr entry(new VideoEntry(video, id));

	// Enable dither if necessary
	checkEnableDither(entry);

	// Add it to the video list
	_videos.push_back(entry);

	return entry;
}

VideoEntryPtr VideoManager::open(const Common::Path &fileName, Audio::Mixer::SoundType soundType) {
	// If this video is already playing, return that entry
	VideoEntryPtr oldVideo = findVideo(fileName);
	if (oldVideo)
		return oldVideo;

	// Otherwise, create a new entry
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);
	if (!stream)
		return VideoEntryPtr();

	Video::VideoDecoder *video = new Video::QuickTimeDecoder();
	video->setSoundType(soundType);
	if (!video->loadStream(stream)) {
		// FIXME: Better error handling
		delete video;
		return VideoEntryPtr();
	}

	// Create the entry
	VideoEntryPtr entry(new VideoEntry(video, fileName));

	// Enable dither if necessary
	checkEnableDither(entry);

	// Add it to the video list
	_videos.push_back(entry);

	return entry;
}

VideoEntryPtr VideoManager::findVideo(uint16 id) {
	if (id == 0)
		return VideoEntryPtr();

	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		if ((*it)->getID() == id)
			return *it;

	return VideoEntryPtr();
}

VideoEntryPtr VideoManager::findVideo(const Common::Path &fileName) {
	if (fileName.empty())
		return VideoEntryPtr();

	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		if ((*it)->getFileName().equalsIgnoreCase(fileName))
			return *it;

	return VideoEntryPtr();
}

bool VideoManager::isVideoPlaying() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		if (!(*it)->endOfVideo())
			return true;

	return false;
}

void VideoManager::drawVideoFrame(const VideoEntryPtr &video, const Audio::Timestamp &time) {
	assert(video);
	video->seek(time);
	drawNextFrame(video);
	video->stop();
}

VideoManager::VideoList::iterator VideoManager::findEntry(VideoEntryPtr ptr) {
	return Common::find(_videos.begin(), _videos.end(), ptr);
}

void VideoManager::removeEntry(const VideoEntryPtr &video) {
	VideoManager::VideoList::iterator it = findEntry(video);
	if (it != _videos.end())
		_videos.erase(it);
}

void VideoManager::checkEnableDither(VideoEntryPtr &entry) {
	// If we're not dithering, bail out
	if (!_enableDither)
		return;

	// Set the palette
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	entry->_video->setDitheringPalette(palette);

	if (entry->_video->getPixelFormat().bytesPerPixel != 1) {
		if (entry->getFileName().empty())
			error("Failed to set dither for video tMOV %d", entry->getID());
		else
			error("Failed to set dither for video %s", entry->getFileName().toString().c_str());
	}
}

} // End of namespace Mohawk
