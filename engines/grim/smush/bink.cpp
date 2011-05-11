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
 * $URL$
 * $Id$
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_chdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd
#define FORBIDDEN_SYMBOL_EXCEPTION_getwd
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

#include "engines/grim/smush/bink.h"

#include "engines/grim/grim.h"
#include "engines/grim/colormap.h"

#define MWIDTH 640
#define MHEIGHT 400

namespace Grim {

VideoPlayer* CreateBinkPlayer(){
	return new Bink_Player();
}

void Bink_Player::timerCallback(void *) {
	((Bink_Player*)g_video)->handleFrame();
}

Bink_Player::Bink_Player()  : VideoPlayer() {
	g_video = this;
	_isPlaying = false;
	_speed = 50;
}

Bink_Player::~Bink_Player() {
	deinit();
}

void Bink_Player::init() {
	_frame = 0;
	_movieTime = 0;
	_isPlaying = false;
	_videoPause = false;
	_updateNeeded = false;
	_width = MWIDTH;
	_height = MHEIGHT;

	//	assert(!_internalBuffer);
	//  assert(!_externalBuffer);

/*	if (!(g_grim->getGameFlags() & GF_DEMO)) {
		_internalBuffer = new byte[_width * _height * 2];
		_externalBuffer = new byte[_width * _height * 2];
	}
	warning("Trying to play %s",_fname.c_str());
	bas->init(_fname.c_str());*/
	g_system->getTimerManager()->installTimerProc(&timerCallback, _speed, NULL);
}

void Bink_Player::deinit() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);

	if (_internalBuffer) {
		delete[] _internalBuffer;
		_internalBuffer = NULL;
	}
	if (_externalBuffer) {
		delete[] _externalBuffer;
		_externalBuffer = NULL;
	}
	if (_videoLooping && _startPos) {
		delete[] _startPos->tmpBuf;
		delete[] _startPos;
		_startPos = NULL;
	}
/*	if (_stream) {
		_stream->finish();
		_stream = NULL;
		g_system->getMixer()->stopHandle(_soundHandle);
	}*/
	_videoLooping = false;
	_isPlaying = true;
	_videoPause = true;

}

void Bink_Player::handleWave(const byte *src, uint32 size) {

}

void Bink_Player::handleFrame() {

	if (_videoPause)
		return;

	if (_isPlaying) {
		_videoPause = true;
		return;
	}
	_isPlaying = false;
	_videoFinished = true;
	g_grim->setMode(ENGINE_MODE_NORMAL);
	return;
}
	
void Bink_Player::deliverFrameFromDecode(int width, int height, uint16 *dat){
	int wTest = _width;
	int hTest = _height;
	memcpy(_externalBuffer,dat,_width*_height*2);
	_frame++;
	_updateNeeded = true;
}

static byte delta_color(byte org_color, int16 delta_color) {
	int t = (org_color * 129 + delta_color) / 128;
	return CLIP(t, 0, 255);
}

void Bink_Player::handleDeltaPalette(byte *src, int32 size) {
}

void Bink_Player::handleFrameDemo() {
}

void Bink_Player::handleFramesHeader() {
}

bool Bink_Player::setupAnimDemo(const char *file) {
	return true;
}

bool Bink_Player::setupAnim(const char *file, bool looping, int x, int y) {
		return true;
}

void Bink_Player::stop() {
	deinit();
	g_grim->setMode(ENGINE_MODE_NORMAL);
}

bool Bink_Player::play(const char *filename, bool looping, int x, int y) {
/*	deinit();
	_fname = filename;

	if (gDebugLevel == DEBUG_SMUSH)
		printf("Playing video '%s'.\n", filename);

	init();*/
	
	return true;
}

void Bink_Player::saveState(SaveGame *state) {
}

void Bink_Player::restoreState(SaveGame *state) {

}
	

} // end of namespace Grim
