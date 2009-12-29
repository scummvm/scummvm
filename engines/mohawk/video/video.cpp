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
 * $URL$
 * $Id$
 *
 */

#include "mohawk/file.h"
#include "mohawk/video/video.h"
#include "mohawk/video/qt_player.h"

// Codecs
#include "mohawk/video/cinepak.h"
#include "mohawk/video/qtrle.h"
#include "mohawk/video/rpza.h"
#include "mohawk/video/smc.h"

namespace Mohawk {

////////////////////////////////////////////
// QueuedAudioStream 
////////////////////////////////////////////

QueuedAudioStream::QueuedAudioStream(int rate, int channels, bool autofree) {
	_rate = rate;
	_channels = channels;
	_autofree = autofree;
	_finished = false;
}

QueuedAudioStream::~QueuedAudioStream() {
	if (_autofree)
		while (!_queue.empty())
			delete _queue.pop();
	_queue.clear();
}

void QueuedAudioStream::queueAudioStream(Audio::AudioStream *audStream) {
	if (audStream->getRate() != getRate() && audStream->isStereo() && isStereo())
		error("QueuedAudioStream::queueAudioStream: audStream has mismatched parameters");
		
	_queue.push(audStream);
}

int QueuedAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesDecoded = 0;

	while (samplesDecoded < numSamples && !_queue.empty()) {
		samplesDecoded += _queue.front()->readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);

		if (_queue.front()->endOfData()) {
			Audio::AudioStream *temp = _queue.pop();
			if (_autofree)
				delete temp;
		}
	}

	return samplesDecoded;
}

////////////////////////////////////////////
// Video 
////////////////////////////////////////////

Video::Video() {
	_videoCodec = NULL;
	_noCodecFound = false;
	_curFrame = -1;
	_lastFrameStart = _nextFrameStart = 0;
	_audHandle = Audio::SoundHandle();
}

Video::~Video() {
}

void Video::stop() {
	stopAudio();
	
	if (!_noCodecFound)
		delete _videoCodec;
	
	closeFile();
}

void Video::reset() {
	delete _videoCodec; _videoCodec = NULL;
	_noCodecFound = false;
	_curFrame = -1;
	_lastFrameStart = _nextFrameStart = 0;

	stopAudio();
	resetInternal();
	startAudio();
}

Graphics::Codec *Video::createCodec(uint32 codecTag, byte bitsPerPixel) {
	if (codecTag == MKID_BE('cvid')) {
		// Cinepak: As used by most Myst and all Riven videos as well as some Myst ME videos. "The Chief" videos also use this.
		return new CinepakDecoder();
	} else if (codecTag == MKID_BE('rpza')) {
		// Apple Video ("Road Pizza"): Used by some Myst videos.
		return new RPZADecoder(getWidth(), getHeight());
	} else if (codecTag == MKID_BE('rle ')) {
		// QuickTime RLE: Used by some Myst ME videos.
		return new QTRLEDecoder(getWidth(), getHeight(), bitsPerPixel);
	} else if (codecTag == MKID_BE('smc ')) {
		// Apple SMC: Used by some Myst videos.
		return new SMCDecoder(getWidth(), getHeight());
	} else if (codecTag == MKID_BE('SVQ1')) {
		// Sorenson Video 1: Used by some Myst ME videos.
		warning ("Sorenson Video 1 not yet supported");
	} else if (codecTag == MKID_BE('SVQ3')) {
		// Sorenson Video 3: Used by some Myst ME videos.
		warning ("Sorenson Video 3 not yet supported");
	} else if (codecTag == MKID_BE('jpeg')) {
		// Motion JPEG: Used by some Myst ME videos.
		warning ("Motion JPEG not yet supported");
	} else if (codecTag == MKID_BE('QkBk')) {
		// CDToons: Used by most of the Broderbund games. This is an unknown format so far.
		warning ("CDToons not yet supported");
	} else {
		warning ("Unsupported codec \'%s\'", tag2str(codecTag));
	}
	
	return NULL;
}

void Video::startAudio() {
	Audio::AudioStream *audStream = getAudioStream();
	
	if (!audStream) // No audio/audio not supported
		return;
	
	g_system->getMixer()->playInputStream(Audio::Mixer::kPlainSoundType, &_audHandle, audStream);
}

void Video::pauseAudio() {
	g_system->getMixer()->pauseHandle(_audHandle, true);
}

void Video::resumeAudio() {
	g_system->getMixer()->pauseHandle(_audHandle, false);
}

void Video::stopAudio() {
	g_system->getMixer()->stopHandle(_audHandle);
}

Graphics::Surface *Video::getNextFrame() {	
	if (_noCodecFound || _curFrame >= (int32)getFrameCount() - 1)
		return NULL;
		
	if (_nextFrameStart == 0)
		_nextFrameStart = g_system->getMillis() * 100;
		
	_lastFrameStart = _nextFrameStart;
	_curFrame++;
	_nextFrameStart = getFrameDuration() + _lastFrameStart;
	
	Common::SeekableReadStream *frameData = getNextFramePacket();
	
	if (!_videoCodec) {
		_videoCodec = createCodec(getCodecTag(), getBitsPerPixel());
		// If we don't get it still, the codec is unsupported ;)
		if (!_videoCodec) {
			_noCodecFound = true;
			return NULL;
		}
	}

	if (frameData) {
		Graphics::Surface *frame = _videoCodec->decodeImage(frameData);
		delete frameData;
		return frame;
	}

	return NULL;
}

bool Video::endOfVideo() {
	QueuedAudioStream *audStream = getAudioStream();
		
	return (!audStream || audStream->endOfData()) && (_noCodecFound || _curFrame >= (int32)getFrameCount() - 1);
}

bool Video::needsUpdate() {
	if (endOfVideo())
		return false;

	if (_curFrame == -1)
		return true;

	return (g_system->getMillis() * 100 - _lastFrameStart) >= getFrameDuration();
}

////////////////////////////////////////////
// VideoManager 
////////////////////////////////////////////
	
VideoManager::VideoManager(MohawkEngine* vm) : _vm(vm) {
	if (_vm->getFeatures() & GF_HASBINK) {
		warning ("This game uses bink video. Playback is disabled.");
		_videoType = kBinkVideo;
	} else {
		_videoType = kQuickTimeVideo;
	}
	
	_pauseStart = 0;
}
	
VideoManager::~VideoManager() {
	_mlstRecords.clear();
	stopVideos();
}

void VideoManager::pauseVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		_videoStreams[i]->pauseAudio();
	_pauseStart = _vm->_system->getMillis() * 100;
}

void VideoManager::resumeVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++) {
		_videoStreams[i]->addPauseTime(_vm->_system->getMillis() * 100 - _pauseStart);
		_videoStreams[i]->resumeAudio();
	}
	
	_pauseStart = 0;
}

void VideoManager::stopVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		_videoStreams[i]->stop();
	_videoStreams.clear();
}

void VideoManager::playMovie(Common::String filename, uint16 x, uint16 y, bool clearScreen) {
	Common::File *file = new Common::File();
	if (!file->open(filename))
		return; // Return silently for now...

	VideoEntry entry;
	entry.video = createVideo();

	if (!entry.video)
		return;

	entry->loadFile(file);

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

	entry.x = x;
	entry.y = y;
	entry.loop = false;
	playMovie(entry);
}

void VideoManager::playMovieCentered(Common::String filename, bool clearScreen) {
	Common::File *file = new Common::File();
	if (!file->open(filename))
		return; // Return silently for now...

	VideoEntry entry;
	entry.video = createVideo();

	if (!entry.video)
		return;

	entry->loadFile(file);

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

	entry.x = (_vm->_system->getWidth() - entry->getWidth()) / 2;
	entry.y = (_vm->_system->getHeight() - entry->getHeight()) / 2;
	entry.loop = false;
	playMovie(entry);
}
	
void VideoManager::playMovie(VideoEntry videoEntry) {
	// Add video to the list
	_videoStreams.push_back(videoEntry);
	
	bool continuePlaying = true;
	videoEntry->startAudio();
	
	while (!videoEntry->endOfVideo() && !_vm->shouldQuit() && continuePlaying) {
		if (updateBackgroundMovies())
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
	
	videoEntry->stop();

	_videoStreams.clear();
}

void VideoManager::playBackgroundMovie(Common::String filename, int16 x, int16 y, bool loop) {
	Common::File *file = new Common::File();
	if (!file->open(filename))
		return; // Return silently for now...

	VideoEntry entry;
	entry.video = createVideo();
	
	if (!entry.video)
		return;
	
	entry->loadFile(file);
	
	// Center x if requested
	if (x < 0)
		x = (_vm->_system->getWidth() - entry->getWidth()) / 2;
	
	// Center y if requested
	if (y < 0)
		y = (_vm->_system->getHeight() - entry->getHeight()) / 2;
		
	entry.x = x;
	entry.y = y;
	entry.loop = loop;
	
	entry->startAudio();
	_videoStreams.push_back(entry);
}

bool VideoManager::updateBackgroundMovies() {
	bool updateScreen = false;

	for (uint32 i = 0; i < _videoStreams.size() && !_vm->shouldQuit(); i++) {
		// Remove any videos that are over
		if (_videoStreams[i]->endOfVideo()) {
			if (_videoStreams[i].loop) {
				_videoStreams[i]->reset();
			} else {
				delete _videoStreams[i].video;
				_videoStreams.remove_at(i);
				i--;
				continue;
			}
		}

		// Check if we need to draw a frame
		if (_videoStreams[i]->needsUpdate()) {
			Graphics::Surface *frame = _videoStreams[i]->getNextFrame();
			bool deleteFrame = false;

			if (frame) {			
				// Convert from 8bpp to the current screen format if necessary
				if (frame->bytesPerPixel == 1) {
					Graphics::Surface *newFrame = new Graphics::Surface();
					Graphics::PixelFormat pixelFormat = _vm->_system->getScreenFormat();
					byte *palette = _videoStreams[i]->getPalette();
					assert(palette);
					
					newFrame->create(frame->w, frame->h, pixelFormat.bytesPerPixel);
					
					for (uint16 j = 0; j < frame->h; j++) {
						for (uint16 k = 0; k < frame->w; k++) {
							byte palIndex = *((byte *)frame->getBasePtr(k, j));
							byte r = palette[palIndex * 4];
							byte g = palette[palIndex * 4 + 1];
							byte b = palette[palIndex * 4 + 2];
							if (pixelFormat.bytesPerPixel == 2)
								*((uint16 *)newFrame->getBasePtr(k, j)) = pixelFormat.RGBToColor(r, g, b);
							else
								*((uint32 *)newFrame->getBasePtr(k, j)) = pixelFormat.RGBToColor(r, g, b);
						}
					}
					
					frame = newFrame;
					deleteFrame = true;
				}

				// Check if we're drawing at a 2x or 4x resolution (because of 
				// evil QuickTime scaling it first).
				if (_videoStreams[i]->getScaleMode() == kScaleHalf || _videoStreams[i]->getScaleMode() == kScaleQuarter) {
					byte scaleFactor = (_videoStreams[i]->getScaleMode() == kScaleHalf) ? 2 : 4;

					Graphics::Surface scaledSurf;
					scaledSurf.create(_videoStreams[i]->getWidth() / scaleFactor, _videoStreams[i]->getHeight() / scaleFactor, frame->bytesPerPixel);

					for (uint32 j = 0; j < scaledSurf.h; j++)
						for (uint32 k = 0; k < scaledSurf.w; k++)
							memcpy(scaledSurf.getBasePtr(k, j), frame->getBasePtr(k * scaleFactor, j * scaleFactor), frame->bytesPerPixel);

					_vm->_system->copyRectToScreen((byte*)scaledSurf.pixels, scaledSurf.pitch, _videoStreams[i].x, _videoStreams[i].y, scaledSurf.w, scaledSurf.h);
					scaledSurf.free();
				} else {
					// Clip the width/height to make sure we stay on the screen (Myst does this a few times)
					uint16 width = MIN<int32>(_videoStreams[i]->getWidth(), _vm->_system->getWidth() - _videoStreams[i].x);
					uint16 height = MIN<int32>(_videoStreams[i]->getHeight(), _vm->_system->getHeight() - _videoStreams[i].y);
					_vm->_system->copyRectToScreen((byte*)frame->pixels, frame->pitch, _videoStreams[i].x, _videoStreams[i].y, width, height);
				}

				// We've drawn something to the screen, make sure we update it
				updateScreen = true;
				
				// Delete the frame if we're using the buffer from the 8bpp conversion
				if (deleteFrame) {
					frame->free();
					delete frame;
				}
			}
		}

		// Update the audio buffer too
		_videoStreams[i]->updateAudioBuffer();
	}

	// Return true if we need to update the screen
	return updateScreen;
}

void VideoManager::activateMLST(uint16 mlstId, uint16 card) {
	Common::SeekableReadStream *mlstStream = _vm->getRawData(ID_MLST, card);
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

		// Enable the record by default
		mlstRecord.enabled = true;

		if (mlstRecord.index == mlstId) {
			_mlstRecords.push_back(mlstRecord);
			break;
		}
	}
	
	delete mlstStream;
}

void VideoManager::playMovie(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			warning("STUB: Play tMOV %d (non-blocking) at (%d, %d)", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top);
			return; // TODO: This will do a lot of things wrong if enabled right now ;)
			VideoEntry entry;
			entry.video = new QTPlayer();
			entry->loadFile(_vm->getRawData(ID_TMOV, _mlstRecords[i].movieID));
			entry.x = _mlstRecords[i].left;
			entry.y = _mlstRecords[i].top;
			entry.id = _mlstRecords[i].movieID;
			entry.loop = _mlstRecords[i].loop != 0;
			_videoStreams.push_back(entry);
			return;
		}
}

void VideoManager::playMovieBlocking(uint16 id) {
	// NOTE/TODO: playMovieBlocking can be called after playMovie, essentially
	// making it just a playMovieBlocking. It basically nullifies the first call.

	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			warning("STUB: Play tMOV %d (blocking) at (%d, %d)", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top);
			
			// TODO: See if a non-blocking movie has been activated with the same id,
			// and if so, block input until that movie is finished.
			
			VideoEntry entry;
			entry.video = new QTPlayer();
			entry->loadFile(_vm->getRawData(ID_TMOV, _mlstRecords[i].movieID));
			entry.x = _mlstRecords[i].left;
			entry.y = _mlstRecords[i].top;
			entry.id = _mlstRecords[i].movieID;
			entry.loop = false;
			playMovie(entry);
			return;
		}
}

void VideoManager::stopMovie(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			warning("STUB: Stop tMOV %d", _mlstRecords[i].movieID);
			return;
		}
}

void VideoManager::enableMovie(uint16 id) {
	debug(2, "Enabling movie %d", id);
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			_mlstRecords[i].enabled = true;
			return;
		}
}

void VideoManager::disableMovie(uint16 id) {
	debug(2, "Disabling movie %d", id);
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			_mlstRecords[i].enabled = false;
			return;
		}
}

void VideoManager::disableAllMovies() {
	debug(2, "Disabling all movies");
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		_mlstRecords[i].enabled = false;
}

Video *VideoManager::createVideo() {
	if (_videoType == kBinkVideo || _vm->shouldQuit())
		return NULL;

	return new QTPlayer();
}

} // End of namespace Mohawk
