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

#include "mohawk/mohawk.h"
#include "mohawk/resource.h"
#include "mohawk/video.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/textconsole.h"
#include "common/system.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "video/qt_decoder.h"


namespace Mohawk {

VideoEntry::VideoEntry() : _video(0), _id(-1), _x(0), _y(0), _loop(false), _enabled(true) {
}

VideoEntry::VideoEntry(Video::VideoDecoder *video, const Common::String &fileName) : _video(video), _fileName(fileName), _id(-1), _x(0), _y(0), _loop(false), _enabled(true) {
}

VideoEntry::VideoEntry(Video::VideoDecoder *video, int id) : _video(video), _id(id), _x(0), _y(0), _loop(false), _enabled(true) {
}

VideoEntry::~VideoEntry() {
	close();
}

void VideoEntry::close() {
	delete _video;
	_video = 0;
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
}

void VideoEntry::start() {
	assert(_video);
	_video->start();
}

void VideoEntry::stop() {
	assert(_video);
	_video->stop();
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

VideoHandle::VideoHandle(VideoEntryPtr ptr) : _ptr(ptr) {
}

VideoHandle::VideoHandle(const VideoHandle &handle) : _ptr(handle._ptr) {
}

VideoManager::VideoManager(MohawkEngine* vm) : _vm(vm) {
	// Set dithering enabled, if required
	_enableDither = _vm->getGameType() == GType_MYST && !(_vm->getFeatures() & GF_ME);
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

void VideoManager::playMovieBlocking(const Common::String &fileName, uint16 x, uint16 y, bool clearScreen) {
	VideoEntryPtr ptr = open(fileName);
	if (!ptr)
		return;

	ptr->moveTo(x, y);

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

	ptr->start();
	waitUntilMovieEnds(ptr);
}

void VideoManager::playMovieBlockingCentered(const Common::String &fileName, bool clearScreen) {
	VideoEntryPtr ptr = open(fileName);
	if (!ptr)
		return;

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

	ptr->center();
	ptr->start();
	waitUntilMovieEnds(ptr);
}

void VideoManager::waitUntilMovieEnds(VideoHandle videoHandle) {
	if (!videoHandle)
		return;

	// Sanity check
	if (videoHandle._ptr->isLooping())
		error("Called waitUntilMovieEnds() on a looping video");

	bool continuePlaying = true;

	while (!videoHandle->endOfVideo() && !_vm->shouldQuit() && continuePlaying) {
		if (updateMovies())
			_vm->_system->updateScreen();

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				continuePlaying = false;
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_SPACE:
					_vm->pauseGame();
					break;
				case Common::KEYCODE_ESCAPE:
					continuePlaying = false;
					_vm->doVideoTimer(videoHandle, true);
					break;
				default:
					break;
			}
			default:
				break;
			}
		}

		// Cut down on CPU usage
		_vm->_system->delayMillis(10);
	}

	// Ensure it's removed
	removeEntry(videoHandle._ptr);
}

void VideoManager::delayUntilMovieEnds(VideoHandle videoHandle) {
	// FIXME: Why is this separate from waitUntilMovieEnds?
	// It seems to only cut out the event loop (which is bad).

	if (!videoHandle)
		return;

	// Sanity check
	if (videoHandle._ptr->isLooping())
		error("Called delayUntilMovieEnds() on a looping video");

	while (!videoHandle->endOfVideo() && !_vm->shouldQuit()) {
		if (updateMovies())
			_vm->_system->updateScreen();

		// Cut down on CPU usage
		_vm->_system->delayMillis(10);
	}

	// Ensure it's removed
	removeEntry(videoHandle._ptr);
}

VideoHandle VideoManager::playMovie(const Common::String &fileName) {
	VideoEntryPtr ptr = open(fileName);
	if (!ptr)
		return VideoHandle();

	ptr->start();
	return ptr;
}

VideoHandle VideoManager::playMovie(uint16 id) {
	VideoEntryPtr ptr = open(id);
	if (!ptr)
		return VideoHandle();

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
			const Graphics::Surface *frame = video->decodeNextFrame();
			Graphics::Surface *convertedFrame = 0;

			if (frame && (*it)->isEnabled()) {
				Graphics::PixelFormat pixelFormat = _vm->_system->getScreenFormat();

				if (frame->format != pixelFormat) {
					// We don't support downconverting to 8bpp without having
					// support in the codec. Set _enableDither if shows up.
					if (pixelFormat.bytesPerPixel == 1) {
						warning("Cannot convert high color video frame to 8bpp");
						(*it)->close();
						it = _videos.erase(it);
						continue;
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

				// Clip the width/height to make sure we stay on the screen (Myst does this a few times)
				uint16 width = MIN<int32>(video->getWidth(), _vm->_system->getWidth() - (*it)->getX());
				uint16 height = MIN<int32>(video->getHeight(), _vm->_system->getHeight() - (*it)->getY());
				_vm->_system->copyRectToScreen(frame->getPixels(), frame->pitch, (*it)->getX(), (*it)->getY(), width, height);

				// We've drawn something to the screen, make sure we update it
				updateScreen = true;

				// Delete 8bpp conversion surface
				if (convertedFrame) {
					convertedFrame->free();
					delete convertedFrame;
				}
			}
		}

		// Check the video time
		_vm->doVideoTimer(*it, false);

		// Remember to increase the iterator
		it++;
	}

	// Return true if we need to update the screen
	return updateScreen;
}

void VideoManager::activateMLST(uint16 mlstId, uint16 card) {
	Common::SeekableReadStream *mlstStream = _vm->getResource(ID_MLST, card);
	uint16 recordCount = mlstStream->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		MLSTRecord mlstRecord;
		mlstRecord.index = mlstStream->readUint16BE();
		mlstRecord.movieID = mlstStream->readUint16BE();
		mlstRecord.code = mlstStream->readUint16BE();
		mlstRecord.left = mlstStream->readUint16BE();
		mlstRecord.top = mlstStream->readUint16BE();

		for (byte j = 0; j < 2; j++)
			if (mlstStream->readUint16BE() != 0)
				warning("u0[%d] in MLST non-zero", j);

		if (mlstStream->readUint16BE() != 0xFFFF)
			warning("u0[2] in MLST not 0xFFFF");

		mlstRecord.loop = mlstStream->readUint16BE();
		mlstRecord.volume = mlstStream->readUint16BE();
		mlstRecord.u1 = mlstStream->readUint16BE();

		if (mlstRecord.u1 != 1)
			warning("mlstRecord.u1 not 1");

		// We've found a match, add it
		if (mlstRecord.index == mlstId) {
			// Make sure we don't have any duplicates
			for (uint32 j = 0; j < _mlstRecords.size(); j++)
				if (_mlstRecords[j].index == mlstRecord.index || _mlstRecords[j].code == mlstRecord.code) {
					_mlstRecords.remove_at(j);
					j--;
				}

			_mlstRecords.push_back(mlstRecord);
			break;
		}
	}

	delete mlstStream;
}

void VideoManager::clearMLST() {
	_mlstRecords.clear();
}

VideoHandle VideoManager::playMovieRiven(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++) {
		if (_mlstRecords[i].code == id) {
			debug(1, "Play tMOV %d (non-blocking) at (%d, %d) %s, Volume = %d", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, _mlstRecords[i].loop != 0 ? "looping" : "non-looping", _mlstRecords[i].volume);

			VideoEntryPtr ptr = open(_mlstRecords[i].movieID);
			if (ptr) {
				ptr->moveTo(_mlstRecords[i].left, _mlstRecords[i].top);
				ptr->setLooping(_mlstRecords[i].loop != 0);
				ptr->setVolume(_mlstRecords[i].volume);
				ptr->start();
			}

			return ptr;
		}
	}

	return VideoHandle();
}

void VideoManager::playMovieBlockingRiven(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++) {
		if (_mlstRecords[i].code == id) {
			debug(1, "Play tMOV %d (blocking) at (%d, %d), Volume = %d", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, _mlstRecords[i].volume);
			VideoEntryPtr ptr = open(_mlstRecords[i].movieID);
			ptr->moveTo(_mlstRecords[i].left, _mlstRecords[i].top);
			ptr->setVolume(_mlstRecords[i].volume);
			ptr->start();
			waitUntilMovieEnds(ptr);
			return;
		}
	}
}

void VideoManager::stopMovieRiven(uint16 id) {
	debug(2, "Stopping movie %d", id);
	VideoHandle handle = findVideoHandleRiven(id);
	if (handle)
		removeEntry(handle._ptr);
}

void VideoManager::disableAllMovies() {
	debug(2, "Disabling all movies");
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		(*it)->setEnabled(false);
}

VideoEntryPtr VideoManager::open(uint16 id) {
	// If this video is already playing, return that handle
	VideoHandle oldHandle = findVideoHandle(id);
	if (oldHandle._ptr)
		return oldHandle._ptr;

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

VideoEntryPtr VideoManager::open(const Common::String &fileName) {
	// If this video is already playing, return that entry
	VideoHandle oldHandle = findVideoHandle(fileName);
	if (oldHandle._ptr)
		return oldHandle._ptr;

	// Otherwise, create a new entry
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);
	if (!stream)
		return VideoEntryPtr();

	Video::VideoDecoder *video = new Video::QuickTimeDecoder();
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

VideoHandle VideoManager::findVideoHandleRiven(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id)
			for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
				if ((*it)->getID() == _mlstRecords[i].movieID)
					return *it;

	return VideoHandle();
}

VideoHandle VideoManager::findVideoHandle(uint16 id) {
	if (id == 0)
		return VideoHandle();

	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		if ((*it)->getID() == id)
			return *it;

	return VideoHandle();
}

VideoHandle VideoManager::findVideoHandle(const Common::String &fileName) {
	if (fileName.empty())
		return VideoHandle();

	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		if ((*it)->getFileName().equalsIgnoreCase(fileName))
			return *it;

	return VideoHandle();
}

bool VideoManager::isVideoPlaying() {
	for (VideoList::iterator it = _videos.begin(); it != _videos.end(); it++)
		if (!(*it)->endOfVideo())
			return true;

	return false;
}

void VideoManager::drawVideoFrame(VideoHandle handle, const Audio::Timestamp &time) {
	// FIXME: This should be done separately from the "playing"
	// videos eventually.
	assert(handle);
	handle->seek(time);
	updateMovies();
	handle->close();
}

VideoManager::VideoList::iterator VideoManager::findEntry(VideoEntryPtr ptr) {
	return Common::find(_videos.begin(), _videos.end(), ptr);
}

void VideoManager::removeEntry(VideoEntryPtr ptr) {
	VideoManager::VideoList::iterator it = findEntry(ptr);
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
			error("Failed to set dither for video %s", entry->getFileName().c_str());
	}
}

} // End of namespace Mohawk
