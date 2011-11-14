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
 * $URL: https://www.switchlink.se/svn/picture/movie.cpp $
 * $Id: resource.cpp 2 2008-08-04 12:18:15Z johndoe $
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/movie.h"
#include "picture/palette.h"
#include "picture/resource.h"
#include "picture/screen.h"
#include "picture/script.h"

namespace Picture {

MoviePlayer::MoviePlayer(PictureEngine *vm) : _vm(vm) {
}

MoviePlayer::~MoviePlayer() {
}

void MoviePlayer::playMovie(uint resIndex) {

	uint32 subtitleSlot;
	byte moviePalette[768];
	
	_vm->_isSaveAllowed = false;

	memset(moviePalette, 0, sizeof(moviePalette));

	_vm->_screen->finishTalkTextItems();
	_vm->_screen->clearSprites();

	_vm->_arc->openResource(resIndex);

	subtitleSlot = kMaxScriptSlots - 1;

	_frameCount = _vm->_arc->readUint32LE();
	_chunkCount = _vm->_arc->readUint32LE();

	// TODO: Figure out rest of the header
	_vm->_arc->readUint32LE();
	_vm->_arc->readUint32LE();
	_framesPerSoundChunk = _vm->_arc->readUint32LE();
	_vm->_arc->readUint32LE();

	_vm->_sceneWidth = 640;
	_vm->_sceneHeight = 400;
	_vm->_cameraHeight = 400;
	_vm->_cameraX = 0;
	_vm->_cameraY = 0;

	_audioStream = Audio::makeAppendableAudioStream(22050, Audio::Mixer::FLAG_UNSIGNED);

	_vm->_mixer->playInputStream(Audio::Mixer::kPlainSoundType, &_audioStreamHandle, _audioStream);

	_soundChunkFramesLeft = 0;
	_lastPrefetchOfs = 0;
	fetchAudioChunks();

	uint32 lastTime = _vm->_mixer->getSoundElapsedTime(_audioStreamHandle);

	while (_chunkCount--) {

		byte chunkType;
		uint32 chunkSize;
		byte *chunkBuffer;
		uint32 movieOffset;

		chunkType = _vm->_arc->readByte();
		chunkSize = _vm->_arc->readUint32LE();

		debug(0, "chunkType = %d; chunkSize = %d", chunkType, chunkSize);

		chunkBuffer = new byte[chunkSize];
		_vm->_arc->read(chunkBuffer, chunkSize);

		movieOffset = _vm->_arc->pos();

		switch (chunkType) {
		case 0: // image data
		case 1:
		
			unpackRle(chunkBuffer, _vm->_screen->_backScreen);
			// TODO: Rework this
			_vm->_screen->updateShakeScreen();
			_vm->_screen->_fullRefresh = true;
			_vm->updateInput();
			_vm->updateScreen();

			_soundChunkFramesLeft--;
			if (_soundChunkFramesLeft <= _framesPerSoundChunk) {
				fetchAudioChunks();
			}

			while (_vm->_mixer->getSoundElapsedTime(_audioStreamHandle) < lastTime + 111) {
				g_system->delayMillis(0);
			}

			lastTime = _vm->_mixer->getSoundElapsedTime(_audioStreamHandle);

			break;
		case 2: // palette data
			unpackPalette(chunkBuffer, moviePalette, 256, 3);
			_vm->_palette->setFullPalette(moviePalette);
			break;
		//case 3: -- what is this type
		case 4: // audio data
			break;
		case 5:
			// TODO: Check if the text is a subtitle (last character == 0xFE).
			//	   If so, don't show it if text display is disabled.
			memcpy(_vm->_script->getSlotData(subtitleSlot), chunkBuffer, chunkSize);
			_vm->_screen->updateTalkText(subtitleSlot, 0);
			break;
		case 6: // start/stop shakescreen effect
			if (chunkBuffer[0] == 0xFF)
				_vm->_screen->stopShakeScreen();
			else
				_vm->_screen->startShakeScreen(chunkBuffer[0]);
			break;
		case 7: // setup subtitle parameters
			_vm->_screen->_talkTextY = READ_LE_UINT16(chunkBuffer + 0);
			_vm->_screen->_talkTextX = READ_LE_UINT16(chunkBuffer + 2);
			_vm->_screen->_talkTextFontColor = ((chunkBuffer[4] << 4) & 0xF0) | ((chunkBuffer[4] >> 4) & 0x0F);
			debug(0, "_talkTextX = %d; _talkTextY = %d; _talkTextFontColor = %d",
				_vm->_screen->_talkTextX, _vm->_screen->_talkTextY, _vm->_screen->_talkTextFontColor);
			break;
		case 8: // stop subtitles
			_vm->_script->getSlotData(subtitleSlot)[0] = 0xFF;
			_vm->_screen->finishTalkTextItems();
			break;
		default:
			error("MoviePlayer::playMovie(%04X) Unknown chunk type %d at %08X", resIndex, chunkType, _vm->_arc->pos() - 5 - chunkSize);
		}

		delete[] chunkBuffer;

		_vm->_arc->seek(movieOffset, SEEK_SET);
		
		if (!handleInput())
			break;

	}

	_audioStream->finish();
	_vm->_mixer->stopHandle(_audioStreamHandle);

	_vm->_arc->closeResource();
	
	debug(0, "playMovie() done");

	_vm->_isSaveAllowed = true;

}

void MoviePlayer::fetchAudioChunks() {

	uint32 startOfs = _vm->_arc->pos();
	uint32 chunkCount = _chunkCount;
	uint prefetchChunkCount = 0;

	if (_lastPrefetchOfs != 0)
		_vm->_arc->seek(_lastPrefetchOfs, SEEK_SET);

	while (chunkCount-- && prefetchChunkCount < _framesPerSoundChunk / 2) {
		byte chunkType = _vm->_arc->readByte();
		uint32 chunkSize = _vm->_arc->readUint32LE();
		if (chunkType == 4) {
			byte *chunkBuffer = new byte[chunkSize];
			_vm->_arc->read(chunkBuffer, chunkSize);
			_audioStream->queueBuffer(chunkBuffer, chunkSize);
			chunkBuffer = NULL;
			prefetchChunkCount++;
			_soundChunkFramesLeft += _framesPerSoundChunk;
			delete[] chunkBuffer;
		} else {
			_vm->_arc->seek(chunkSize, SEEK_CUR);
		}
	}

	_lastPrefetchOfs = _vm->_arc->pos();

	_vm->_arc->seek(startOfs, SEEK_SET);

}

void MoviePlayer::unpackPalette(byte *source, byte *dest, int elemCount, int elemSize) {
	int ofs = 0, size = elemCount * elemSize;
	while (ofs < size) {
		byte len;
		len = *source++;
		if (len == 0) {
			len = *source++;
		} else {
			byte value = *source++;
			memset(dest, value, len);
		}
		ofs += len;
		dest += len;
	}
}

void MoviePlayer::unpackRle(byte *source, byte *dest) {
	int size = 256000;
	while (size > 0) {
		byte a = *source++;
		byte b = *source++;
		if (a == 0) {
			dest += b;
			size -= b;
		} else {
			memset(dest, b, a);
			dest += a;
			size -= a;
		}
	}
}

bool MoviePlayer::handleInput() {
	Common::Event event;
	Common::EventManager *eventMan = g_system->getEventManager();
	while (eventMan->pollEvent(event)) {
	switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
				return false;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			return false;
		case Common::EVENT_QUIT:
			_vm->quitGame();
			return false;
			break;
		default:
			break;
		}
	}
	return !_vm->shouldQuit();
}

} // End of namespace Picture
