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

#include "made/mpegplayer.h"
#include "made/made.h"
#include "made/screen.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#ifdef USE_MPEG2
#include "made/magical_mpeg.h"
#include "video/mpegps_decoder.h"
#endif

namespace Made {

MpegPlayer::MpegPlayer(MadeEngine *vm) :
		_vm(vm), _decoder(nullptr), _looping(false), _passComplete(false), _paused(false),
		_videoLayer(nullptr), _startTime(0), _framesDrawn(0), _loopFrames(0), _loops(0),
		_playToken(0), _frameDurNum(1001), _frameDurDen(30) {
}

MpegPlayer::~MpegPlayer() {
	stop();

	// stop() leaves the last picture attached to the screen, but the surface is
	// about to be freed.
	_vm->_screen->setVideoLayer(nullptr);

	if (_videoLayer) {
		_videoLayer->free();
		delete _videoLayer;
	}
}

#ifdef USE_MPEG2

/** Expose the protected video-track EOF query needed by these assets. */
class MadeMpegDecoder : public Video::MPEGPSDecoder {
public:
	bool videoTracksEnded() const { return endOfVideoTracks(); }
};

void MpegPlayer::drawFrame(const Graphics::Surface *frame) {
	if (!_videoLayer)
		return;

	const uint bytesPerPixel = _videoLayer->format.bytesPerPixel;
	const int width = MIN<int>(_videoLayer->w, frame->w);

	// The common path is a 320x240 frame in the active screen format.
	if (_videoLayer->w == frame->w && _videoLayer->h == frame->h &&
		_videoLayer->pitch == frame->pitch) {
		memcpy(_videoLayer->getPixels(), frame->getPixels(), frame->pitch * frame->h);
		return;
	}

	_videoLayer->fillRect(_videoLayer->getRect(), 0);
	int srcY = 0;
	for (int y = 0; y < _videoLayer->h && srcY < frame->h; y++) {
		memcpy(_videoLayer->getBasePtr(0, y), frame->getBasePtr(0, srcY), width * bytesPerPixel);

		srcY++;
		if (!_vm->_screen->isGraphicsStretched() && (srcY % 6) == 5)
			srcY++;
	}
}

bool MpegPlayer::openDecoder() {
	if (!_loopData.empty()) {
		return openDecoderFrom(new Common::MemoryReadStream(
			&_loopData[0], _loopData.size(), DisposeAfterUse::NO));
	}

	Common::File *file = new Common::File();
	if (!file->open(_filename.c_str())) {
		warning("MpegPlayer: could not open '%s'", _filename.c_str());
		delete file;
		return false;
	}

	// Program streams need their audio padding repaired even when their video
	// headers are not scrambled. Bare elementary streams carry no audio.
	const bool magical = MagicalMpeg::isMagical(*file);
	const bool programStream = MagicalMpeg::isProgramStream(*file);

	Common::SeekableReadStream *stream = file;
	if (magical || programStream) {
		stream = MagicalMpeg::unlock(*file, MagicalMpeg::kDefaultMagicKey, magical);
		delete file;
		if (!stream) {
			warning("MpegPlayer: could not prepare '%s'", _filename.c_str());
			return false;
		}
	}

	// Cache a prepared looping stream, avoiding disk I/O and both repair passes
	// at every loop boundary.
	if (_looping && _loopData.empty()) {
		if (stream->size() <= 0 || stream->size() > 0xFFFFFFFFU) {
			warning("MpegPlayer: invalid stream size for '%s'", _filename.c_str());
			delete stream;
			return false;
		}

		_loopData.resize((uint32)stream->size());
		stream->seek(0);
		if (stream->read(&_loopData[0], _loopData.size()) == _loopData.size()) {
			delete stream;
			stream = new Common::MemoryReadStream(
				&_loopData[0], _loopData.size(), DisposeAfterUse::NO);
		} else {
			_loopData.clear();
			stream->seek(0);
		}
	}

	return openDecoderFrom(stream);
}

bool MpegPlayer::openDecoderFrom(Common::SeekableReadStream *stream) {
	// Pace from the sequence header. The PTS values in these assets do not map
	// reliably to the displayed pictures.
	_frameDurNum = 1001;
	_frameDurDen = 30;
	MagicalMpeg::getFrameDuration(*stream, _frameDurNum, _frameDurDen);
	stream->seek(0);

	_decoder = new MadeMpegDecoder();
	_decoder->setPrebufferedPackets(600);
	_decoder->setStopAtFirstFrame(true);
	_decoder->setAudioLeadTime(750);

	if (!_decoder->loadStream(stream)) {
		warning("MpegPlayer: could not load '%s'", _filename.c_str());
		delete _decoder;
		_decoder = nullptr;
		return false;
	}

	// Frames are copied directly into the screen's video layer, so the formats
	// must match exactly rather than merely use the same bytes per pixel.
	if (!_decoder->setOutputPixelFormat(_vm->_system->getScreenFormat())) {
		warning("MpegPlayer: unsupported screen format for '%s'", _filename.c_str());
		delete _decoder;
		_decoder = nullptr;
		return false;
	}

	_decoder->start();
	return true;
}

bool MpegPlayer::start(const char *filename, int16 playMode) {
	if (!filename || !*filename) {
		warning("MpegPlayer: no filename supplied");
		return false;
	}
	if (playMode < kPlayOnce || playMode > kPlayBlocking) {
		warning("MpegPlayer: invalid play mode %d for '%s'", playMode, filename);
		return false;
	}

	stop();

	if (!_loopData.empty() && _filename != filename)
		_loopData.clear();

	_filename = filename;
	_looping = (playMode == kPlayLoop);
	_passComplete = false;
	_paused = false;
	_playToken++;

	if (!openDecoder()) {
		_looping = false;
		return false;
	}

	if (!_videoLayer) {
		_videoLayer = new Graphics::Surface();
		_videoLayer->create(320, _vm->_screen->getOutputHeight(),
			_vm->_system->getScreenFormat());
	}

	// The first cutscene after the opening credits finishes them, as in PmvPlayer.
	if (scumm_stricmp(filename, "FWIZ01XX.MPG") == 0)
		_vm->_openingCreditsOpen = false;

	_vm->_screen->setVideoLayer(_videoLayer);

	_startTime = _vm->_system->getMillis();
	_framesDrawn = 0;
	_loopFrames = 0;
	_loops = 0;

	// Prime the decoder before the mixer starts consuming its empty audio queue.
	update();
	return true;
}

bool MpegPlayer::restartForLoop() {
	if (_framesDrawn == _loopFrames)
		return false;

	delete _decoder;
	_decoder = nullptr;
	if (!openDecoder())
		return false;

	// Exclude decoder restart time from the frame schedule.
	_startTime = _vm->_system->getMillis() -
		(uint32)((uint64)_framesDrawn * _frameDurNum / _frameDurDen);
	_loopFrames = _framesDrawn;
	_loops++;
	return true;
}

void MpegPlayer::update() {
	if (!_decoder)
		return;

	if (!_decoder->isPlaying() || _decoder->videoTracksEnded()) {
		_passComplete = true;
		_paused = true;
		if (_looping && restartForLoop())
			return;

		stop();
		return;
	}

	const uint32 now = _vm->_system->getMillis();
	const uint32 due = _startTime +
		(uint32)((uint64)_framesDrawn * _frameDurNum / _frameDurDen);
	if (now < due)
		return;

	const uint32 kMaxCatchUpFrames = 10;
	uint32 backlog = (uint32)(((uint64)(now - _startTime) * _frameDurDen) / _frameDurNum);
	backlog = (backlog > _framesDrawn) ? backlog - _framesDrawn : 0;
	backlog = MIN(backlog, kMaxCatchUpFrames);

	while (backlog > 1) {
		backlog--;
		if (!_decoder->decodeNextFrame() || _decoder->videoTracksEnded())
			break;
		_framesDrawn++;
	}

	const Graphics::Surface *frame = _decoder->decodeNextFrame();
	if (!frame)
		return;

	_framesDrawn++;
	drawFrame(frame);
	_vm->_screen->presentWorkScreen();
	_vm->_system->updateScreen();
}

void MpegPlayer::stop() {
	if (!_decoder)
		return;

	const uint32 elapsed = _vm->_system->getMillis() - _startTime;
	debug(1, "MpegPlayer: '%s' %s, %u frames in %u ms, %u loop(s)",
		_filename.c_str(), _looping ? "looping" : "once", _framesDrawn, elapsed, _loops);

	delete _decoder;
	_decoder = nullptr;

	// The card retains the current picture when playback stops.
	_vm->_screen->setVideoLayer(_videoLayer);
}

void MpegPlayer::pause() {
	if (!_decoder)
		return;

	_paused = true;
	_passComplete = true;
	stop();
}

void MpegPlayer::close() {
	_paused = false;
	stop();
}

int16 MpegPlayer::playState() const {
	if (_decoder && !_passComplete)
		return kStatePlaying;
	return _paused ? kStatePaused : kStateClosed;
}

#else

bool MpegPlayer::start(const char *filename, int16 playMode) {
	warning("MpegPlayer: cannot play '%s'; ScummVM was built without MPEG-2 support",
		filename ? filename : "");
	return false;
}

void MpegPlayer::update() {
}

void MpegPlayer::stop() {
}

void MpegPlayer::pause() {
}

void MpegPlayer::close() {
}

int16 MpegPlayer::playState() const {
	return kStateClosed;
}

#endif // USE_MPEG2

} // End of namespace Made
