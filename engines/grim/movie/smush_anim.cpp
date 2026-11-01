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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"

#include "engines/grim/movie/smush_anim.h"
#include "engines/grim/movie/smush_sound.h"
#include "engines/grim/movie/smush_timer.h"

namespace Grim {

// The audio ring holds 120 blocks, about 5.5 seconds
#define SMUSH_DEMO_AUDIO_BLOCKS 120

// The file ring is read 12 sectors per call and 64 as the first read;
// the biggest read is a frame plus 100 sectors
#define SMUSH_DEMO_STREAM_CHUNK (12 * SMUSH_DEMO_SECTOR_SIZE)
#define SMUSH_DEMO_STREAM_PREFETCH (64 * SMUSH_DEMO_SECTOR_SIZE)
#define SMUSH_DEMO_STREAM_SLACK (100 * SMUSH_DEMO_SECTOR_SIZE)

SmushAnimPlay::SmushAnimPlay(SmushTimer *timer, SmushSound *sound) {
	_timer = timer;
	_sound = sound;
	_frameTimer = 0;
	_syncTimer = 0;
	_finished = 0;
	_framesDue = 0;
	_lateTicks = 0;
	_nominalDelay = 0;
	_currentDelay = 0;
	_skipFlag = 0;
	_state = 0;
	_started = 0;
	_syncPaused = 0;
}

SmushAnimPlay::~SmushAnimPlay() {
	anmStop();
}

int SmushAnimPlay::anmStart(int fps) {
	_lateTicks = 0;
	_framesDue = 0;
	_skipFlag = 0;
	_started = 0;
	_state = 0;
	_nominalDelay = SMUSH_TIMER_FREQUENCY / fps;
	_currentDelay = SMUSH_TIMER_FREQUENCY / fps;

	_frameTimer = _timer->sysAlloc(frameTimer, 1, SYS_TIMER_ONE_SHOT, this);
	if (!_frameTimer)
		return -1;

	_syncTimer = _timer->sysAlloc(syncTimer, _nominalDelay, SYS_TIMER_EXT_SYNC, this);
	if (!_syncTimer) {
		_timer->sysFree(_frameTimer);
		_frameTimer = 0;
		return -1;
	}

	_finished = 0;
	return 0;
}

void SmushAnimPlay::anmStop() {
	if (!_frameTimer && !_syncTimer)
		return;

	_timer->sysFree(_frameTimer);
	_timer->sysFree(_syncTimer);
	_frameTimer = 0;
	_syncTimer = 0;

	if (_syncPaused)
		_sound->souEngineResume();
}

int SmushAnimPlay::anmRender(SmushAnimClient *client) {
	if (_finished)
		return -1;

	return anmRenderBody(client);
}

int SmushAnimPlay::anmRenderBody(SmushAnimClient *client) {
	if (!_framesDue)
		return 6;

	if (!_started)
		client->resetFrameCounter();

	// Frames are 66.67 ms long (15 fps); the result is in frames
	int drift = (int)(int64)(((double)_sound->souEngineQueryTime(0) - (double)client->getFrameCounter() * 66.66666666667) * 0.0149999999999925);
	_lateTicks--;
	_framesDue = 0;

	if (drift > 0 || _state == 3 || _state == 4) {
		if (_currentDelay > _nominalDelay)
			_currentDelay = _nominalDelay;

		int step = (drift == 1) ? _nominalDelay / 16 : _nominalDelay / 8;
		_currentDelay -= step;
		if (_currentDelay < 1)
			_currentDelay = 1;

		int minimum = 3 * _nominalDelay / 4;
		if (_currentDelay < minimum)
			_currentDelay = minimum;
	} else if (drift < 0) {
		if (_currentDelay < _nominalDelay)
			_currentDelay = _nominalDelay;

		int step = (drift == -1) ? _nominalDelay / 16 : _nominalDelay / 8;
		_currentDelay += step;

		int maximum = 4 * _nominalDelay / 3;
		if (_currentDelay > maximum)
			_currentDelay = maximum;
	}

	_timer->sysReset(_frameTimer, _currentDelay);

	if (drift <= 2) {
		if (drift <= 0)
			_skipFlag = 0;
	} else {
		_skipFlag = 1;
	}

	int flags = 0;
	if (_skipFlag)
		flags |= 0x8000;

	_state = client->animatorRender(flags);
	if (_state < 0)
		_finished = 1;
	if (_state == 1)
		_started = 1;

	return _state;
}

void SmushAnimPlay::frameTimer(int timer, void *context) {
	((SmushAnimPlay *)context)->_framesDue++;
}

void SmushAnimPlay::syncTimer(int timer, void *context) {
	SmushAnimPlay *anim = (SmushAnimPlay *)context;

	if (!anim->_started)
		return;

	if (!anim->_syncPaused && anim->_lateTicks >= 7) {
		anim->_syncPaused = 1;
		anim->_sound->souEnginePause();
	} else if (anim->_syncPaused && anim->_lateTicks < 0) {
		anim->_syncPaused = 0;
		anim->_sound->souEngineResume();
	}

	if (anim->_lateTicks < 20 && !anim->_syncPaused)
		anim->_lateTicks++;
}

SmushDemoPlayer::SmushDemoPlayer(SmushDemoVideo *video, SmushDemoDriver *driver) {
	_video = video;
	_driver = driver;
	_stream = new SmushDemoStream();
	_anim = new SmushDemoAnim(_stream, this);
	memset(&_audio, 0, sizeof(_audio));
	_useDriver = true;
	_playing = 0;
	memset(_staging, 0, sizeof(_staging));
	memset(_palette, 0, sizeof(_palette));
	_frameSource = nullptr;
	_frameWidth = 0;
	_frameHeight = 0;
	_positionX = 0;
	_positionY = 0;
	_sourceX = 0;
	_sourceY = 0;
	_paused = 0;
	_done = 0;
	_lastResult = 0;
	_frameReady = 0;
	_paletteFlag = 0;
	_staged = 0;
	_audioPaused = 0;
	_audioStartFrame = 0;
	_audioClockBase = 0;
	_audioClockLast = 0;
	_startTime = 0;
	_lastUpdate = 0;
	_fps = 12;
	_frame = 0;
	_framesShown = 0;
	_frameCount = 1;
	_dropCount = 0;
	_behind = 0;
	_firstUpdate = 1;
	_streamStop = 1;
}

SmushDemoPlayer::~SmushDemoPlayer() {
	delete _anim;
	delete _stream;
}

int SmushDemoPlayer::preFrame(int frame, int frameCount) {
	_frameCount = frameCount;
	return 0;
}

int SmushDemoPlayer::postFrame(int frame, int frameCount) {
	_frameCount = frameCount;
	_frame = frame;
	return 0;
}

int SmushDemoPlayer::frameCallback(int flags, bool paletteChanged, bool display, int paletteState) {
	if (paletteState) {
		const byte *palette = _anim->anmGetPalette();
		for (int i = 0; i < 256; i++)
			_palette[i] = palette[i * 3] | (palette[i * 3 + 1] << 8) | (palette[i * 3 + 2] << 16);

		_paletteFlag = (paletteChanged ? 1 : 0) + 1;
	}

	_frameReady = 1;
	return 0;
}

int SmushDemoPlayer::chunkCallback(const byte *chunk, int size, int flags) {
	if (READ_LE_UINT16(chunk) != 8)
		return 0;

	const byte *data = chunk + 18;
	int remaining = size - 18;

	// The disasm fades the palette with the other parts, which no movie has
	if (READ_LE_UINT16(chunk + 6))
		return 0;

	_audioStartFrame = _frame;
	if (!remaining)
		return 0;

	int staged = _staged;
	while (true) {
		if (staged >= 2) {
			int needed = (_staging[0] << 8) + _staging[1] + 2 - staged;
			if (needed > remaining) {
				memcpy(_staging + staged, data, remaining);
				staged += remaining;
				_staged = staged;
				return 0;
			}

			memcpy(_staging + staged, data, needed);
			decodeIACT(_staging);
			staged = 0;
			data += needed;
			_staged = 0;
			remaining -= needed;
			if (!remaining)
				return 0;

			continue;
		}

		if (remaining == 1) {
			if (staged) {
				_staging[1] = *data++;
				remaining--;
				staged = 2;
				_staged = 2;
				if (!remaining)
					return 0;

				continue;
			}

			_staging[0] = *data;
			_staged = 1;
			return 0;
		}

		if (!staged) {
			_staging[0] = *data++;
			remaining--;
		}

		_staging[1] = *data++;
		remaining--;
		staged = 2;
		_staged = 2;
		if (!remaining)
			return 0;
	}
}

void SmushDemoPlayer::setFrameRate(int fps) {
	_fps = (int16)fps;
	if (_startTime)
		_startTime = audioGetTime();
}

void SmushDemoPlayer::setFrameBuffer(byte *buffer, int x, int y, int sourceX, int sourceY, int width, int height) {
	_frameSource = buffer;
	_frameWidth = width;
	_frameHeight = height;
	_positionX = x;
	_positionY = y;
	_sourceX = sourceX;
	_sourceY = sourceY;
}

void SmushDemoPlayer::decodeIACT(const byte *block) {
	int shiftLeft = block[2] >> 4;
	int shiftRight = block[2] & 0xF;
	byte *destination = _audio.data + _audio.produced % _audio.size;
	const byte *cursor = block + 3;

	for (int i = 0; i < SMUSH_DEMO_AUDIO_BLOCK_SIZE / 4; i++) {
		int16 sample;

		if (*cursor == 128) {
			sample = (int16)((cursor[1] << 8) + cursor[2]);
			cursor += 3;
		} else {
			sample = (int16)((int8)*cursor << shiftLeft);
			cursor++;
		}

		WRITE_LE_UINT16(destination, sample);
		destination += 2;

		if (*cursor == 128) {
			sample = (int16)((cursor[1] << 8) + cursor[2]);
			cursor += 3;
		} else {
			sample = (int16)((int8)*cursor << shiftRight);
			cursor++;
		}

		WRITE_LE_UINT16(destination, sample);
		destination += 2;
	}

	_audio.produced += SMUSH_DEMO_AUDIO_BLOCK_SIZE;
}

int SmushDemoPlayer::audioOpen(int rate, int channels, int bits) {
	if (_useDriver)
		return _driver->sndDrivOpen(rate, channels, bits, &_audio);

	_audio.data = new byte[_audio.size]();
	return _audio.data == nullptr;
}

void SmushDemoPlayer::audioClose() {
	if (_useDriver) {
		_driver->sndDrivClose(&_audio);
		return;
	}

	delete[] _audio.data;
	_audio.data = nullptr;
}

int SmushDemoPlayer::audioPause() {
	if (!_useDriver)
		return 1;

	int result = _driver->sndDrivPause(&_audio);
	if (!result)
		_audioPaused = 1;

	return result;
}

int SmushDemoPlayer::audioResume() {
	if (!_useDriver)
		return 1;

	int result = _driver->sndDrivResume(&_audio);
	if (!result)
		_audioPaused = 0;

	return result;
}

void SmushDemoPlayer::audioFill() {
	if (!_useDriver) {
		int consumed = _audio.produced - 1024;
		_audio.consumed = consumed <= 0 ? 0 : consumed;
		return;
	}

	if (_frame - _audioStartFrame > 5) {
		int produced = _audio.produced;
		if (produced * _fps / SMUSH_DEMO_AUDIO_BYTES_PER_SECOND >= _frameCount - 1) {
			int space = _audio.size + _audio.consumed - produced;
			if (space > SMUSH_DEMO_AUDIO_BLOCK_SIZE) {
				int blocks = (space - 1) / SMUSH_DEMO_AUDIO_BLOCK_SIZE;
				do {
					memset(_audio.data + _audio.produced % _audio.size, 0, SMUSH_DEMO_AUDIO_BLOCK_SIZE);
					blocks--;
					_audio.produced += SMUSH_DEMO_AUDIO_BLOCK_SIZE;
				} while (blocks);
			}
		}
	}

	_driver->sndDrivFill(&_audio);
}

int SmushDemoPlayer::audioGetTime() {
	if (!_useDriver)
		return g_system->getMillis();

	if (_audioStartFrame || _frame <= 2)
		return _driver->sndDrivGetTime(&_audio);

	_audioClockLast = _driver->sndDrivGetTime(&_audio);
	if (!_audioClockBase)
		_audioClockBase = g_system->getMillis() - 1000 * _frame / _fps;

	return _audioClockLast - _audioClockBase + g_system->getMillis();
}

int SmushDemoPlayer::getFramesDue() {
	if (_firstUpdate)
		return 1;

	return (int)((uint32)(_fps * (audioGetTime() - _startTime)) / 1000u - (uint32)_framesShown);
}

int SmushDemoPlayer::stepAnimation() {
	int result;

	while (true) {
		int flags = (!_behind || _lastResult == 11) ? 0 : 16;
		result = _anim->animatorRender(flags);
		_lastResult = (int16)result;
		if ((int16)result != 11)
			break;

		if (!_stream->streamFill(SMUSH_DEMO_STREAM_CHUNK))
			break;
	}

	if (result & 0x8000)
		_done = 1;

	return result;
}

bool SmushDemoPlayer::streamStep() {
	if (_streamStop)
		return false;

	bool read = false;
	while (_stream->streamFill(SMUSH_DEMO_STREAM_CHUNK))
		read = true;

	return read;
}

int SmushDemoPlayer::initialize(int bufferBytes, int width, int height) {
	int maxRead = width * height + SMUSH_DEMO_STREAM_SLACK;
	if (_stream->streamInitialize(bufferBytes - maxRead, maxRead))
		return 0;

	if (_anim->anmInitialize(width, height)) {
		_stream->streamTerminate();
		return 0;
	}

	if (_anim->anmRegisterCodecs(width, height)) {
		_anim->anmTerminate();
		_stream->streamTerminate();
		return 0;
	}

	return 1;
}

void SmushDemoPlayer::terminate() {
	_anim->anmUnregisterCodecs();
	_anim->anmTerminate();
	_stream->streamTerminate();
}

int SmushDemoPlayer::open(const Common::String &name, int offset) {
	if (!Common::File::exists(Common::Path(name)))
		return 1;

	memset(&_audio, 0, sizeof(_audio));
	_audio.size = SMUSH_DEMO_AUDIO_BLOCKS * SMUSH_DEMO_AUDIO_BLOCK_SIZE;

	if (audioOpen(SMUSH_DEMO_AUDIO_RATE, 2, 16))
		return 1;

	_anim->animatorInitialize();
	_anim->animatorNewAnim(name, offset);
	_stream->streamFill(SMUSH_DEMO_STREAM_PREFETCH);

	memset(_palette, 0, sizeof(_palette));
	_paused = 0;
	_done = 0;
	_lastResult = 0;
	_frameReady = 0;
	_paletteFlag = 0;
	_staged = 0;
	_audioPaused = 0;
	_audioStartFrame = 0;
	_audioClockBase = 0;
	_startTime = 0;
	_lastUpdate = g_system->getMillis();
	_frameSource = nullptr;
	_frameWidth = 0;
	_frameHeight = 0;
	_fps = 12;
	_frame = 0;
	_framesShown = 0;
	_frameCount = 1;
	_dropCount = 0;
	_behind = 0;
	_firstUpdate = 1;
	_streamStop = 0;
	return 0;
}

int SmushDemoPlayer::close() {
	audioClose();
	memset(&_audio, 0, sizeof(_audio));

	_streamStop = 0;
	_done = 1;
	return 0;
}

int SmushDemoPlayer::update(int *frameDecoded, int *finished, int *paletteChanged, const uint32 **palette) {
	*finished = 0;
	*frameDecoded = 0;
	*paletteChanged = 0;
	*palette = nullptr;

	bool late = false;
	if (_paused)
		return 0;

	if (!_frameReady) {
		stepAnimation();
		audioFill();
	}

	uint32 time = g_system->getMillis();
	if (time - _lastUpdate > 200)
		late = true;

	int due;
	if (late)
		due = 1;
	else
		due = getFramesDue();

	if (due <= _fps / 2 || _audioPaused) {
		if (due < 1 && _audioPaused)
			audioResume();
	} else {
		audioPause();
	}

	if (due > 0) {
		if (_frameReady)
			*frameDecoded = 1;

		*paletteChanged = _paletteFlag;
		*palette = _palette;
		_paletteFlag = 0;
		_frameReady = 0;
		_lastUpdate = time;
		if (_firstUpdate) {
			_firstUpdate = 0;
			_startTime = audioGetTime();
		}
	}

	if (_done || _framesShown >= _frameCount - 1) {
		*finished = 1;
		return 0;
	}

	*finished = 0;
	return 0;
}

void SmushDemoPlayer::pause() {
	if (!_paused && _audio.data) {
		_paused = 1;
		audioPause();
	}
}

void SmushDemoPlayer::resume() {
	if (_paused && _audio.data) {
		_paused = 0;
		audioResume();
	}
}

void SmushDemoPlayer::getFrameBuffer(const byte **pixels, int *width, int *height) {
	*pixels = _frameSource;
	*width = _frameWidth;
	*height = _frameHeight;
}

void SmushDemoPlayer::getSize(int *x, int *y) {
	*x = _positionX;
	*y = _positionY;
}

int SmushDemoPlayer::smushPlayStartMovie(const Common::String &name, int flags, int bufferBytes, int width, int height, int offset) {
	_useDriver = (flags >= 0);

	if (_video->videoStart(width, height, 8))
		return 1;

	if (!initialize(bufferBytes, width, height)) {
		_video->videoStop();
		return 1;
	}

	if (open(name, offset)) {
		terminate();
		_video->videoStop();
		return 1;
	}

	_playing = 1;
	return 0;
}

int SmushDemoPlayer::smushPlayUpdateMovie(int *finished) {
	int frameDecoded;
	int paletteChanged;
	const uint32 *palette;

	update(&frameDecoded, finished, &paletteChanged, &palette);
	if (!*finished && frameDecoded) {
		if (paletteChanged)
			_video->videoSetPalette(palette, 0, 256);

		SmushDemoTarget target;
		SmushDemoRect rect;
		_video->videoLock(&target, &rect);
		smushPlayBlit(&target, &rect);
		_video->videoUnlock();
	}

	return 0;
}

int SmushDemoPlayer::smushPlayStopMovie() {
	close();
	terminate();
	_video->videoStop();
	_playing = 0;
	return 0;
}

int SmushDemoPlayer::smushPlayPauseMovie() {
	if (_playing) {
		pause();
		return 0;
	}

	return 0;
}

int SmushDemoPlayer::smushPlayResumeMovie() {
	if (_playing) {
		resume();
		return 0;
	}

	return 0;
}

int SmushDemoPlayer::smushPlayBlit(SmushDemoTarget *target, const SmushDemoRect *rect) {
	int due = getFramesDue();
	_framesShown = _frame;

	if (target && _frameSource) {
		if (due > 1 && _dropCount < 3) {
			_dropCount++;
		} else {
			_dropCount = 0;

			int width = _frameWidth;
			int height = _frameHeight;
			int x = _sourceX;
			int y = _sourceY;

			if (x < rect->x) {
				width += x - rect->x;
				x = rect->x;
			}

			if (x + width > rect->width + rect->x)
				width = rect->width + rect->x - x;

			if (y < rect->y) {
				height += y - rect->y;
				y = rect->y;
			}

			if (y + height > rect->height + rect->y)
				height = rect->height + rect->y - y;

			int alignedX = x & ~7;
			int alignedWidth = ((width + x + 7) & ~7) - alignedX;

			const byte *source = _frameSource + alignedX + y * _frameWidth;
			byte *destination = target->pixels + alignedX + y * target->width;

			smushPlayBlitCopy(source, _frameWidth, destination, target->width, alignedWidth, height);
		}
	}

	uint32 dueAfter = (uint32)getFramesDue();
	if (dueAfter < 2) {
		if (!dueAfter)
			_behind = 0;
	} else {
		_behind = 1;
	}

	return 0;
}

void SmushDemoPlayer::smushPlayBlitCopy(const byte *source, int sourcePitch, byte *destination, int destinationPitch, int width, int rows) {
	for (int row = 0; row < rows; row++) {
		memcpy(destination, source, (width >> 2) * 4);

		source += sourcePitch;
		destination += destinationPitch;
	}
}

} // End of namespace Grim
