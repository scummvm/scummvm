/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_unlink

#include "common/endian.h"
#include "common/timer.h"
#include "common/file.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/raw.h"

#include "engines/grim/movie/mpeg.h"

#include "engines/grim/grim.h"
#include "engines/grim/colormap.h"

#define MWIDTH 640
#define MHEIGHT 400

namespace Grim {

MoviePlayer *CreateMpegPlayer() {
	return new MpegPlayer();
}

class MpegHandler : public Video::BaseAnimationState {
public:
	MpegHandler(MpegPlayer *vid, OSystem *sys, int width, int height) : BaseAnimationState(sys, width, height) {
		_mpeg = vid;
	}
protected:
	MpegPlayer *_mpeg;
	virtual void drawYUV(int width, int height, byte *const *dat) {
		plotYUV(MWIDTH, MHEIGHT, dat);
		_mpeg->deliverFrameFromDecode(width, height, _overlay);
	}
};

void MpegPlayer::timerCallback(void *) {
	((MpegPlayer*)g_movie)->handleFrame();
}

MpegPlayer::MpegPlayer() : MoviePlayer() {
	g_movie = this;
	_speed = 50;
	_videoBase = new MpegHandler(this, g_system, MWIDTH, MHEIGHT);
}

MpegPlayer::~MpegPlayer() {
	deinit();
}

void MpegPlayer::init() {
	_frame = 0;
	_movieTime = 0;
	_videoPause = false;
	_updateNeeded = false;
	_width = MWIDTH;
	_height = MHEIGHT;

	assert(!_externalBuffer);

	_externalBuffer = new byte[_width * _height * 2];

	warning("Trying to play %s",_fname.c_str());
	_videoBase->init(_fname.c_str());
	g_system->getTimerManager()->installTimerProc(&timerCallback, _speed, NULL);
}

void MpegPlayer::deinit() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);

	if (_externalBuffer) {
		delete[] _externalBuffer;
		_externalBuffer = NULL;
	}
	if (_videoLooping && _startPos) {
		delete[] _startPos->tmpBuf;
		delete[] _startPos;
		_startPos = NULL;
	}
	if (_stream) {
		_stream->finish();
		_stream = NULL;
		g_system->getMixer()->stopHandle(_soundHandle);
	}
	_videoLooping = false;
	_videoPause = true;
}

void MpegPlayer::handleFrame() {
	if (_videoPause)
		return;

	if (!_videoBase->decodeFrame()) {
		_videoFinished = true;
		g_grim->setMode(ENGINE_MODE_NORMAL);
		return;
	}
	//else
	//bas->updateScreen();
}

void MpegPlayer::deliverFrameFromDecode(int width, int height, uint16 *dat) {
	memcpy(_externalBuffer, dat, _width * _height * 2);
	_frame++;
	_updateNeeded = true;
}

void MpegPlayer::stop() {
	deinit();
	g_grim->setMode(ENGINE_MODE_NORMAL);
}

bool MpegPlayer::play(const char *filename, bool looping, int x, int y) {
	deinit();
	_fname = filename;

	if (gDebugLevel == DEBUG_SMUSH)
		printf("Playing video '%s'.\n", filename);

	init();

	return true;
}

void MpegPlayer::saveState(SaveGame *state) {
}

void MpegPlayer::restoreState(SaveGame *state) {
}

} // end of namespace Grim
