/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/endian.h"
#include "common/system.h"

#include "graphics/cursorman.h"

#include "simon/animation.h"
#include "simon/intern.h"
#include "simon/simon.h"

#include "sound/audiostream.h"
#include "sound/wave.h"

namespace Simon {

MoviePlayer::MoviePlayer(SimonEngine *vm, Audio::Mixer *mixer)
	: DXAPlayer(), _vm(vm), _mixer(mixer) {
	_omniTV = false;

	_leftButtonDown = false;
	_rightButtonDown = false;

	memset(baseName, 0, sizeof(baseName));

	_sequenceNum = 0;
	_ticks = 0;
}

bool MoviePlayer::load(const char *filename) {
	char videoName[20];
	uint i;

	int baseLen = strlen(filename) - 4;
	memset(baseName, 0, sizeof(baseName));
	memcpy(baseName, filename, baseLen);

	// Change file extension to dxa
	sprintf(videoName, "%s.dxa", baseName);

	if (!loadFile(videoName)) {
		// Check short filename to work around
		// bug in a German Windows 2CD version.
		if (baseLen >= 8) {
			char shortName[20];
			memset(shortName, 0, sizeof(shortName));
			memcpy(shortName, filename, 6);
	
			sprintf(shortName, "%s~1", shortName);

			if (!loadFile(shortName))
				error("Failed to load video file %s or %s", videoName, shortName);
			
			memset(baseName, 0, sizeof(baseName));
			memcpy(baseName, shortName, 8);
			debug(0, "Playing video %s", shortName);
		} else {
			error("Failed to load video file %s", videoName);
		}
	} else {
		debug(0, "Playing video %s", videoName);
	}

	CursorMan.showMouse(false);

	if ((_vm->getPlatform() == Common::kPlatformAmiga || _vm->getPlatform() == Common::kPlatformMacintosh) &&
		_vm->_language != Common::EN_ANY) {
		_sequenceNum = 0;
		for (i = 0; i < 90; i++) {
			if (!scumm_stricmp(baseName, _sequenceList[i]))
				_sequenceNum = i;
		}
	}

	return true;
}

void MoviePlayer::playOmniTV() {
	// Load OmniTV video
	if (!_fd.isOpen()) {
		_vm->_variableArray[254] = 6747;
		return;
	} else {
		_vm->setBitFlag(42, false);
		_omniTV = true;
		startSound();
		return;
	}
}

void MoviePlayer::play() {
	// The OmniTV videos were not included with Amiga and Macintosh versions.
	if (_vm->getPlatform() == Common::kPlatformWindows && _vm->getBitFlag(40)) {
		playOmniTV();
		return;
	}

	if (!_fd.isOpen()) {
		return;
	}

	_leftButtonDown = false;
	_rightButtonDown = false;

	_mixer->stopAll();

	// Resolution is smaller in Amiga verison so always clear screen
	if (_width == 384 && _height == 280) {
		memset(_vm->_frontBuf, 0, _vm->_screenHeight * _vm->_screenWidth);
	}

	_ticks = _vm->_system->getMillis();

	startSound();

	while (_frameNum < _framesCount)
		handleNextFrame();

	closeFile();

	_vm->o_killAnimate();

	if (_vm->getBitFlag(41)) {
		memcpy(_vm->_backBuf, _vm->_frontBuf, _frameSize);
	} else {
		uint8 palette[1024];
		memset(palette, 0, sizeof(palette));
		_vm->dx_clear_surfaces(480);
		_vm->_system->setPalette(palette, 0, 256);
	}

	_vm->_fastFadeOutFlag = true;
}

void MoviePlayer::startSound() {
	byte *buffer;
	uint32 offset, size, tag;

	tag = _fd.readUint32BE();
	if (tag == MKID_BE('WAVE')) {
		size = _fd.readUint32BE();

		if (_sequenceNum) {
			Common::File in;

			_fd.seek(size, SEEK_CUR);

			in.open((const char *)"audio.wav");
			if (!in.isOpen()) {
				error("Can't read offset file 'audio.wav'");
			}

			in.seek(_sequenceNum * 8, SEEK_SET);
			offset = in.readUint32LE();
			size = in.readUint32LE();

			buffer = (byte *)malloc(size);
			in.seek(offset, SEEK_SET);
			in.read(buffer, size);
			in.close();
		} else {
			buffer = (byte *)malloc(size);
			_fd.read(buffer, size);
		}

		Common::MemoryReadStream stream(buffer, size);
		_bgSoundStream = Audio::makeWAVStream(stream);
		free(buffer);
	} else {
		_bgSoundStream = Audio::AudioStream::openStreamFile(baseName);
	}

	if (_bgSoundStream != NULL) {
		_mixer->stopHandle(_bgSound);
		_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_bgSound, _bgSoundStream);
	}
}

void MoviePlayer::nextFrame() {
	if (!_omniTV)
		return;

	if (_vm->getBitFlag(42)) {
		_omniTV = false;
		closeFile();
		return;
	}

	if (_mixer->isSoundHandleActive(_bgSound) && (_mixer->getSoundElapsedTime(_bgSound) * _framesPerSec) / 1000 < _frameNum) {
		copyFrameToBuffer(_vm->getBackBuf(), 465, 222, _vm->_screenWidth);
		return;
	}

	if (_frameNum < _framesCount) {
		decodeNextFrame();
		copyFrameToBuffer(_vm->getBackBuf(), 465, 222, _vm->_screenWidth);
		_frameNum++;
	} else {
		_omniTV = false;
		closeFile();
		_vm->_variableArray[254] = 6747;
	}
}

void MoviePlayer::handleNextFrame() {
	decodeNextFrame();
	processFrame();

	_vm->_system->updateScreen();
	_frameNum++;

	OSystem::Event event;
	while (_vm->_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_KEYDOWN:
			if (event.kbd.ascii == 27) {
				_leftButtonDown = true;
				_rightButtonDown = true;
			}
			break;
		case OSystem::EVENT_LBUTTONDOWN:
			_leftButtonDown = true;
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			_rightButtonDown = true;
			break;
		case OSystem::EVENT_LBUTTONUP:
			_leftButtonDown = false;
			break;
		case OSystem::EVENT_RBUTTONUP:
			_rightButtonDown = false;
			break;
		case OSystem::EVENT_QUIT:
			_vm->_system->quit();
			break;
		default:
			break;
		}
	}

	if (_leftButtonDown && _rightButtonDown && !_vm->getBitFlag(41)) {
		_frameNum = _framesCount;
	}
}

void MoviePlayer::setPalette(byte *pal) {
	byte palette[1024];
	byte *p = palette;

	for (int i = 0; i <= 256; i++) {
		*p++ = *pal++;
		*p++ = *pal++;
		*p++ = *pal++;
		*p++ = 0;
	}

	_vm->_system->setPalette(palette, 0, 256);
}

void MoviePlayer::processFrame() {
	copyFrameToBuffer(_vm->getFrontBuf(), (_vm->_screenWidth - _width) / 2, (_vm->_screenHeight - _height) / 2, _vm->_screenWidth);
	_vm->_system->copyRectToScreen(_vm->getFrontBuf(), _vm->_screenWidth, 0, 0, _vm->_screenWidth, _vm->_screenHeight);

	if ((_bgSoundStream == NULL) || ((int)(_mixer->getSoundElapsedTime(_bgSound) * _framesPerSec) / 1000 < _frameNum + 1) ||
		_frameSkipped > _framesPerSec) {
		if (_frameSkipped > _framesPerSec) {
			warning("force frame %i redraw", _frameNum);
			_frameSkipped = 0;
		}

		if (_bgSoundStream && _mixer->isSoundHandleActive(_bgSound)) {
			while (_mixer->isSoundHandleActive(_bgSound) && (_mixer->getSoundElapsedTime(_bgSound) * _framesPerSec) / 1000 < _frameNum) {
				_vm->_system->delayMillis(10);
			}
			// In case the background sound ends prematurely, update
			// _ticks so that we can still fall back on the no-sound
			// sync case for the subsequent frames.
			_ticks = _vm->_system->getMillis();
		} else {
			_ticks += _frameTicks;
			while (_vm->_system->getMillis() < _ticks)
				_vm->_system->delayMillis(10);
		}
	} else {
		warning("dropped frame %i", _frameNum);
		_frameSkipped++;
	}
}

const char * MoviePlayer::_sequenceList[90] = {
	"agent32",
	"Airlock",
	"Badluck",
	"bentalk1",
	"bentalk2",
	"bentalk3",
	"BigFight",
	"BLOWLAB",
	"breakdown",
	"bridge",
	"button2",
	"cargo",
	"COACH",
	"Colatalk",
	"cygnus2",
	"dream",
	"escape2",
	"FASALL",
	"fbikewurb",
	"feebdel",
	"Feebohno",
	"feebpump",
	"feefone1",
	"feefone2",
	"founder2",
	"founder3",
	"founder4",
	"fxmadsam",
	"fxwakeup",
	"gate",
	"Get Car",
	"getaxe",
	"getlift",
	"icetrench",
	"intomb1",
	"intomb2",
	"Jackpot",
	"knockout",
	"labocto",
	"longfeeb",
	"Mainmin",
	"maznat",
	"meetsquid",
	"mflirt",
	"mfxHappy",
	"Mix_Feeb1",
	"Mix_Feeb2",
	"Mix_Feeb3",
	"Mix_Guardscn",
	"Mlights1",
	"MLights2",
	"MProtest",
	"mudman",
	"munlock",
	"MUS5P2",
	"MUSOSP1",
	"Omenter",
	"Omnicofe",
	"OUTMIN~1",
	"Readbook",
	"Rebelhq",
	"RebelHQ2",
	"Reedin",
	"rescue1",
	"rescue2",
	"samcar",
	"Samdead",
	"scanner",
	"Sleepy",
	"spitbrai",
	"statue1",
	"statue2",
	"sva1",
	"sva2",
	"Teeter",
	"Temple2",
	"Temple3",
	"Temple4",
	"Temple5",
	"Temple6",
	"Temple7",
	"Temple8",
	"Tic-tac2",
	"torture",
	"transmit",
	"Typey",
	"ventfall",
	"ventoff",
	"wasting",
	"wurbatak"
};

} // End of namespace Simon
