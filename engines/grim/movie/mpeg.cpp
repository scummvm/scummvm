/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "graphics/surface.h"

#include "engines/grim/movie/mpeg.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"

#ifdef USE_MPEG2

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

MpegPlayer::MpegPlayer() : MoviePlayer() {
	g_movie = this;
	_speed = 50;
	_videoBase = new MpegHandler(this, g_system, MWIDTH, MHEIGHT);
}

void MpegPlayer::init() {
	MoviePlayer::init();

	// FIXME, deal with pixelformat differently when we get this properly tested.
	Graphics::PixelFormat format = Graphics::PixelFormat(16, 5, 6, 5, 0, 11, 5, 0, 0);
	_externalSurface->create(MWIDTH, MHEIGHT, format);

	g_system->getTimerManager()->installTimerProc(&timerCallback, _speed, this, "mpeg loop");
}

void MpegPlayer::deinit() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);

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
		g_grim->setMode(GrimEngine::NormalMode);
		return;
	}
	//else
	//bas->updateScreen();
}

void MpegPlayer::deliverFrameFromDecode(int width, int height, uint16 *dat) {
	memcpy(_externalSurface->pixels, dat, _externalSurface->w * _externalSurface->h * 2);
	_frame++;
	_updateNeeded = true;
}

bool MpegPlayer::loadFile(Common::String filename) {
	_videoBase->init(_fname.c_str());
	return true; // FIXME
}

} // end of namespace Grim

#endif // USE_MPEG2
