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
 * $URL: https://www.switchlink.se/svn/picture/resource.cpp $
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

	uint32 chunkCount, frameCount, subtitleSlot;
	byte moviePalette[768];
	
	memset(moviePalette, 0, sizeof(moviePalette));

	_vm->_screen->finishTextDrawItems();
	_vm->_screen->clearSprites();

	_vm->_arc->openResource(resIndex);

	frameCount = _vm->_arc->readUint32LE();
	chunkCount = _vm->_arc->readUint32LE();
	subtitleSlot = kMaxScriptSlots - 1;
	
	// TODO: Read/figure out rest of the header
	_vm->_arc->seek(0x10, SEEK_CUR);
	_vm->_sceneWidth = 640;
	_vm->_sceneHeight = 400;
	_vm->_cameraHeight = 400;
	_vm->_cameraX = 0;
	_vm->_cameraY = 0;

	while (chunkCount--) {

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
			_vm->updateInput();
			_vm->updateScreen();
			g_system->delayMillis(80);
			break;
		case 2: // palette data
			unpackPalette(chunkBuffer, moviePalette, 256, 3);
			_vm->_palette->setFullPalette(moviePalette);
			break;
		//case 3: -- what is this type
		case 4: // audio data
			// TODO: Handle audio data
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
			_vm->_screen->_talkTextY = READ_LE_UINT16(&chunkBuffer[0]);
			_vm->_screen->_talkTextX = READ_LE_UINT16(&chunkBuffer[2]);
			_vm->_screen->_talkTextFontColor = chunkBuffer[4];
			debug(0, "_talkTextX = %d; _talkTextY = %d; _talkTextFontColor = %d",
				_vm->_screen->_talkTextX, _vm->_screen->_talkTextY, _vm->_screen->_talkTextFontColor);
			break;
		case 8: // stop subtitles
			_vm->_script->getSlotData(subtitleSlot)[0] = 0xFF;
			_vm->_screen->finishTextDrawItems();
			break;
		default:
			error("Unknown chunk type %d at %08X", chunkType, _vm->_arc->pos() - 5 - chunkSize);
		}

		delete[] chunkBuffer;

		_vm->_arc->seek(movieOffset, SEEK_SET);

	}

	_vm->_arc->closeResource();

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


} // End of namespace Picture
