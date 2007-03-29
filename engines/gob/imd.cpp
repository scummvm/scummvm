/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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

#include "gob/gob.h"
#include "gob/imd.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/inter.h"
#include "gob/palanim.h"
#include "gob/sound.h"
#include "gob/video.h"

namespace Gob {

ImdPlayer::ImdPlayer(GobEngine *vm) : _vm(vm) {
	_curImd = 0;
	_curFile[0] = 0;

	_curX = 0;
	_curY = 0;
	_left = 0;
	_top = 0;
	_right = 0;
	_bottom = 0;

	_frameData = 0;
	_vidBuffer = 0;

	_frontSurf = 21;
	_backSurf = 21;
	_frontMem = 0;
	_frameDelay = 0;

	_noSound = true;
	_soundBuffer = 0;

	_soundFreq = 0;
	_soundSliceSize = 0;
	_soundSlicesCount = 0;

	_soundSliceLength = 0;
	_curSoundSlice = 0;
	_soundStage = 0;
}

ImdPlayer::~ImdPlayer() {
	if (_curImd) {
		delete[] _curImd->palette;
		delete[] _curImd->framesPos;
		delete[] _curImd->frameCoords;
		delete[] _curImd->extraPalette;
	}
	delete[] _frameData;
	delete[] _vidBuffer;
	delete[] _frontMem;
	delete[] _soundBuffer;
	delete _curImd;
}

// flag bits: 0 = read and set palette
//            1 = read palette
ImdPlayer::Imd *ImdPlayer::loadImdFile(const char *path, SurfaceDesc *surfDesc, int8 flags) {
	Imd *imdPtr;
	int16 handle;
	char buf[18];
	uint32 framesPosPos = 0;
	uint32 framesCordsPos = 0;

	strcpy(buf, path);
	strcat(buf, ".IMD");

	handle = _vm->_dataIO->openData(buf);

	if (handle < 0) {
		warning("Can't open IMD \"%s\"", buf);
		return 0;
	}

	imdPtr = new Imd;
	assert(imdPtr);
	memset(imdPtr, 0, sizeof(Imd));

	imdPtr->handle = _vm->_dataIO->readUint16(handle);
	imdPtr->verMin = _vm->_dataIO->readUint16(handle);
	imdPtr->framesCount = _vm->_dataIO->readUint16(handle);
	imdPtr->x = _vm->_dataIO->readUint16(handle);
	imdPtr->y = _vm->_dataIO->readUint16(handle);
	imdPtr->width = _vm->_dataIO->readUint16(handle);
	imdPtr->height = _vm->_dataIO->readUint16(handle);
	imdPtr->field_E = _vm->_dataIO->readUint16(handle);
	imdPtr->curFrame = _vm->_dataIO->readUint16(handle);

	if ((imdPtr->handle != 0) || ((imdPtr->verMin & 0xFF) < 2)) {
		warning("%s: Version incorrect (%d,%X)", buf, imdPtr->handle, imdPtr->verMin);
		_vm->_dataIO->closeData(handle);
		delete imdPtr;
		return 0;
	}

	imdPtr->handle = handle;
	imdPtr->surfDesc = surfDesc;
	imdPtr->firstFramePos = imdPtr->curFrame;
	imdPtr->curFrame = 0;
	
	if ((imdPtr->verMin & 0x800) && ((flags & 3) != 3))
		imdPtr->extraPalette = new Video::Color[256];

	if (flags & 3) {
		imdPtr->palette = new Video::Color[256];
		assert(imdPtr->palette);
		_vm->_dataIO->readData(handle, (char *) imdPtr->palette, 768);
	} else
		_vm->_dataIO->seekData(handle, 768, SEEK_CUR);

	if ((flags & 3) == 1)
		_vm->_video->setPalette(imdPtr->palette);

	if ((imdPtr->verMin & 0xFF) >= 3) {
		imdPtr->stdX = _vm->_dataIO->readUint16(handle);
		if (imdPtr->stdX > 1) {
			warning("%s: More than one standard coordinate quad found (%d)",
					buf, imdPtr->stdX);
			finishImd(imdPtr);
			return 0;
		}
		if (imdPtr->stdX != 0) {
			imdPtr->stdX = _vm->_dataIO->readUint16(handle);
			imdPtr->stdY = _vm->_dataIO->readUint16(handle);
			imdPtr->stdWidth = _vm->_dataIO->readUint16(handle);
			imdPtr->stdHeight = _vm->_dataIO->readUint16(handle);
		} else
			imdPtr->stdX = -1;
	} else
		imdPtr->stdX = -1;

	if ((imdPtr->verMin & 0xFF) >= 4) {
		framesPosPos = _vm->_dataIO->readUint32(handle);
		if (framesPosPos != 0) {
			imdPtr->framesPos = new int32[imdPtr->framesCount];
			assert(imdPtr->framesPos);
		}
	}

	if (imdPtr->verMin & 0x8000)
		framesCordsPos = _vm->_dataIO->readUint32(handle);

	_noSound = true;
	if (imdPtr->verMin & 0x4000) {
		_soundFreq = _vm->_dataIO->readUint16(handle);
		_soundSliceSize = _vm->_dataIO->readUint16(handle);
		_soundSlicesCount = _vm->_dataIO->readUint16(handle);

		if (_soundFreq < 0)
			_soundFreq = -_soundFreq;

		if (_soundSlicesCount < 0)
			_soundSlicesCount = -_soundSlicesCount - 1;

		if (_soundSlicesCount >= 40) {
			warning("%s: More than 40 sound slices found (%d)",
					buf, _soundSlicesCount + 1);
			finishImd(imdPtr);
			return 0;
		}

		_soundSliceLength = 1000 / (_soundFreq / _soundSliceSize);

		delete[] _soundBuffer;
		_soundBuffer = new byte[_soundSliceSize * _soundSlicesCount];
		assert(_soundBuffer);
		memset(_soundBuffer, 0, _soundSliceSize * _soundSlicesCount);

		_vm->_snd->stopSound(0);
		_soundDesc.set(SOUND_SND, SOUND_TOT, _soundBuffer,
				_soundSliceSize * _soundSlicesCount);

		_curSoundSlice = 0;
		_soundStage = 1;
		_noSound = false;
	}

	if (imdPtr->verMin & 0x2000) {
		imdPtr->frameDataSize = _vm->_dataIO->readUint16(handle);
		if (imdPtr->frameDataSize == 0) {
			imdPtr->frameDataSize = _vm->_dataIO->readUint32(handle);
			imdPtr->vidBufferSize = _vm->_dataIO->readUint32(handle);
		} else
			imdPtr->vidBufferSize = _vm->_dataIO->readUint16(handle);
	} else {
		imdPtr->frameDataSize = imdPtr->width * imdPtr->height + 500;
		if (!(imdPtr->field_E & 0x100) || (imdPtr->field_E & 0x1000))
			imdPtr->vidBufferSize = imdPtr->frameDataSize;
	}

	if (imdPtr->framesPos) {
		_vm->_dataIO->seekData(handle, framesPosPos, SEEK_SET);
		for (int i = 0; i < imdPtr->framesCount; i++)
			imdPtr->framesPos[i] = _vm->_dataIO->readUint32(handle);
	}

	if (imdPtr->verMin & 0x8000) {
		_vm->_dataIO->seekData(handle, framesCordsPos, SEEK_SET);
		imdPtr->frameCoords = new ImdCoord[imdPtr->framesCount];
		assert(imdPtr->frameCoords);
		for (int i = 0; i < imdPtr->framesCount; i++) {
			imdPtr->frameCoords[i].left = _vm->_dataIO->readUint16(handle);
			imdPtr->frameCoords[i].top = _vm->_dataIO->readUint16(handle);
			imdPtr->frameCoords[i].right = _vm->_dataIO->readUint16(handle);
			imdPtr->frameCoords[i].bottom = _vm->_dataIO->readUint16(handle);
		}
	}

	_vm->_dataIO->seekData(handle, imdPtr->firstFramePos, SEEK_SET);
	return imdPtr;
}

void ImdPlayer::finishImd(ImdPlayer::Imd *&imdPtr) {
	if (!imdPtr)
		return;

	_soundDesc.free();
	if (_soundStage == 2)
		_vm->_snd->stopSound(0);

	_vm->_dataIO->closeData(imdPtr->handle);

	delete[] imdPtr->frameCoords;
	delete[] imdPtr->palette;
	delete[] imdPtr->framesPos;
	delete[] imdPtr->extraPalette;

	delete imdPtr;
}

int8 ImdPlayer::openImd(const char *path, int16 x, int16 y,
		int16 startFrame, int16 flags) {
	const char *src;
	byte *vidMem;
	SurfaceDesc *surfDesc;

	if (!_curImd)
		_curFile[0] = 0;

	src = strrchr(path, '\\');
	src = !src ? path : src + 1;

	if ((path[0] != 0) && scumm_stricmp(_curFile, src)) {
		closeImd();

		_curImd = loadImdFile(path, 0, 3);
		if (!_curImd)
			return 0;

		_curX = _curImd->x;
		_curY = _curImd->y;
		strcpy(_curFile, src);

		delete[] _frameData;
		_frameData = new byte[_curImd->frameDataSize + 500];
		assert(_frameData);
		memset(_frameData, 0, _curImd->frameDataSize + 500);

		delete[] _vidBuffer;
		_vidBuffer = new byte[_curImd->vidBufferSize + 500];
		assert(_vidBuffer);
		memset(_vidBuffer, 0, _curImd->vidBufferSize + 500);

		if (!(flags & 0x100)) {

			if (_vm->_global->_videoMode == 0x14) {

				_backSurf = (flags & 0x80) ? 20 : 21;
				if (!(_curImd->field_E & 0x100) || (_curImd->field_E & 0x2000)) {
					setXY(_curImd, 0, 0);
					_curImd->surfDesc =
						_vm->_video->initSurfDesc(0x13,
								_curImd->width, _curImd->height, 0);
				} else {
					_curImd->surfDesc = _vm->_draw->_spritesArray[_frontSurf];
					if ((x != -1) || (y != -1)) {
						_curX = x != -1 ? x : _curX;
						_curY = y != -1 ? y : _curY;
						setXY(_curImd, _curX, _curY);
					}
				}

				if (flags & 0x40) {
					_curX = x != -1 ? x : _curX;
					_curY = y != -1 ? y : _curY;
					if (_curImd->surfDesc->_vidMode == 0x14) {
						surfDesc = _vm->_video->initSurfDesc(0x13,
								_curImd->width, _curImd->height, 0);
						_vm->_video->drawSprite(_vm->_draw->_spritesArray[21],
								surfDesc, _curX, _curY,
								_curX + _curImd->width - 1, _curY + _curImd->height - 1,
								0, 0, 0);

						vidMem = _curImd->surfDesc->getVidMem();
						for (int i = 0; i < _curImd->height; i++)
							for (int j = 0; j < _curImd->width; j++, vidMem++) {
								*(vidMem) = *(surfDesc->getVidMem() +
										(j / 4) + (surfDesc->getWidth() / 4 * i));
							}
						surfDesc = 0;
					}
				}

			} else {
				if ((x != -1) || (y != -1)) {
					_curX = (x != -1) ? x : _curX;
					_curY = (y != -1) ? y : _curY;
					setXY(_curImd, _curX, _curY);
				}
				_backSurf = (flags & 0x80) ? 20 : 21;
				_curImd->surfDesc = _vm->_draw->_spritesArray[_backSurf];
			}

		}
	}

	if (!_curImd)
		return 0;

	if (startFrame == -1) {
		closeImd();
		return 0;
	}

	_curX = (x != -1) ? x : _curX;
	_curY = (y != -1) ? y : _curY;

	WRITE_VAR(7, _curImd->framesCount);

	return 1;
}

void ImdPlayer::closeImd(void) {
	finishImd(_curImd);

	delete[] _frameData;
	delete[] _vidBuffer;
	delete[] _soundBuffer;
	_frameData = 0;
	_vidBuffer = 0;
	_soundBuffer = 0;

	_curImd = 0;
}

void ImdPlayer::setXY(ImdPlayer::Imd *imdPtr, int16 x, int16 y) {
	int i;

	if (imdPtr->stdX != -1) {
		imdPtr->stdX = imdPtr->stdX - imdPtr->x + x;
		imdPtr->stdY = imdPtr->stdY - imdPtr->y + y;
	}
	
	if (imdPtr->frameCoords) {
		for (i = 0; i < imdPtr->framesCount; i++) {
			if (imdPtr->frameCoords[i].left != -1) {
				imdPtr->frameCoords[i].left -=
					imdPtr->frameCoords[i].left - imdPtr->x + x;
				imdPtr->frameCoords[i].top -=
					imdPtr->frameCoords[i].top - imdPtr->y + y;
				imdPtr->frameCoords[i].right -=
					imdPtr->frameCoords[i].right - imdPtr->x + x;
				imdPtr->frameCoords[i].bottom -=
					imdPtr->frameCoords[i].bottom - imdPtr->y + y;
			}
		}
	}

	imdPtr->x = x;
	imdPtr->y = y;
}

void ImdPlayer::drawFrame(Imd *imdPtr, int16 frame, int16 x, int16 y,
		SurfaceDesc *dest) {
	if (!dest)
		dest = _vm->_draw->_frontSurface;

	if (frame == 0)
		_vm->_video->drawSprite(imdPtr->surfDesc, dest, 0, 0,
				imdPtr->width - 1, imdPtr->height - 1, x, y, 1);
	else if (imdPtr->frameCoords && (imdPtr->frameCoords[frame].left != -1))
		_vm->_video->drawSprite(imdPtr->surfDesc, dest,
				imdPtr->frameCoords[frame].left, imdPtr->frameCoords[frame].top,
				imdPtr->frameCoords[frame].right, imdPtr->frameCoords[frame].bottom,
				imdPtr->frameCoords[frame].left, imdPtr->frameCoords[frame].top, 1);
	else if (imdPtr->stdX != -1)
		_vm->_video->drawSprite(imdPtr->surfDesc, dest,
				imdPtr->stdX, imdPtr->stdY, imdPtr->stdX + imdPtr->stdWidth - 1,
				imdPtr->stdY + imdPtr->stdHeight - 1, x + imdPtr->stdX,
				y + imdPtr->stdY, 1);
	else
		_vm->_video->drawSprite(imdPtr->surfDesc, dest, 0, 0,
				imdPtr->width - 1, imdPtr->height - 1, x, y, 0);
}

void ImdPlayer::renderFrame(Imd *imdPtr) {
	int16 imdX, imdY;
	int16 imdW, imdH;
	int16 sW;
	uint16 pixCount, pixWritten;
	uint8 type;
	byte *imdVidMem;
	byte *imdVidMemBak;
	byte *dataPtr = 0;
	byte *srcPtr = 0;

	dataPtr = (byte *) _frameData;
	imdX = imdPtr->x;
	imdY = imdPtr->y;
	imdW = imdPtr->width;
	imdH = imdPtr->height;
	sW = imdPtr->surfDesc->getWidth();
	imdVidMem = imdPtr->surfDesc->getVidMem() + sW * imdY + imdX;

	type = *dataPtr++;
	srcPtr = dataPtr;

	if (type & 0x10) { // Palette data
		type ^= 0x10;
		dataPtr += 49;
	}

	srcPtr = dataPtr;
	if (type & 0x80) { // Frame data is compressed
		srcPtr = (byte *) _vidBuffer;
		type &= 0x7F;
		if ((type == 2) && (imdW == sW)) {
			frameUncompressor(imdVidMem, dataPtr);
			return;
		} else
			frameUncompressor(srcPtr, dataPtr);
	}

	if (type == 2) { // Whole block
		for (int i = 0; i < imdH; i++) {
			memcpy(imdVidMem, srcPtr, imdW);
			srcPtr += imdW;
			imdVidMem += sW;
		}
	} else if (type == 1) { // Sparse block
		imdVidMemBak = imdVidMem;
		for (int i = 0; i < imdH; i++) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) { // data
					pixCount = MIN((pixCount & 0x7F) + 1, imdW - pixWritten);
					memcpy(imdVidMem, srcPtr, pixCount);

					pixWritten += pixCount;
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else { // "hole"
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			imdVidMemBak += sW;
			imdVidMem = imdVidMemBak;
		}
	} else if (type == 0x42) { // Whole quarter-wide block
		for (int i = 0; i < imdH; i++) {
			imdVidMemBak = imdVidMem;

			for (int j = 0; j < imdW; j += 4, imdVidMem += 4, srcPtr++)
				memset(imdVidMem, *srcPtr, 4);

			imdVidMemBak += sW;
			imdVidMem = imdVidMemBak;
		}
	} else if ((type & 0xF) == 2) { // Whole half-high block
		for (; imdH > 1; imdH -= 2, imdVidMem += sW + sW, srcPtr += imdW) {
			memcpy(imdVidMem, srcPtr, imdW);
			memcpy(imdVidMem + sW, srcPtr, imdW);
		}
		if (imdH == -1)
			memcpy(imdVidMem, srcPtr, imdW);
	} else { // Sparse half-high block
		imdVidMemBak = imdVidMem;
		for (int i = 0; i < imdH; i += 2) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) { // data
					pixCount = MIN((pixCount & 0x7F) + 1, imdW - pixWritten);
					memcpy(imdVidMem, srcPtr, pixCount);
					memcpy(imdVidMem + sW, srcPtr, pixCount);

					pixWritten += pixCount;
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else { // "hole"
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			imdVidMemBak += sW + sW;
			imdVidMem = imdVidMemBak;
		}
	}
}

void ImdPlayer::frameUncompressor(byte *dest, byte *src) {
	int i;
	byte buf[4370];
	uint16 chunkLength;
	uint16 frameLength;
	uint16 bufPos1;
	uint16 bufPos2;
	uint16 tmp;
	uint8 chunkBitField;
	uint8 chunkCount;
	bool mode;

	frameLength = READ_LE_UINT16(src);
	src += 4;

	if ((READ_LE_UINT16(src) == 0x1234) && (READ_LE_UINT16(src + 2) == 0x5678)) {
		src += 4;
		bufPos1 = 273;
		mode = 1; // 123Ch (cmp al, 12h)
	} else {
		bufPos1 = 4078;
		mode = 0; // 275h (jnz +2)
	}

	memset(buf, 32, bufPos1);
	chunkCount = 1;
	chunkBitField = 0;

	while (frameLength > 0) {
		chunkCount--;
		if (chunkCount == 0) {
			tmp = *src++;
			chunkCount = 8;
			chunkBitField = tmp;
		}
		if (chunkBitField % 2) {
			chunkBitField >>= 1;
			buf[bufPos1] = *src;
			*dest++ = *src++;
			bufPos1 = (bufPos1 + 1) % 4096;
			frameLength--;
			continue;
		}
		chunkBitField >>= 1;

		tmp = READ_LE_UINT16(src);
		src += 2;
		chunkLength = ((tmp & 0xF00) >> 8) + 3;

		if ((mode && ((chunkLength & 0xFF) == 0x12)) ||
				(!mode && (chunkLength == 0)))
			chunkLength = *src++ + 0x12;

		bufPos2 = (tmp & 0xFF) + ((tmp >> 4) & 0x0F00);
		if (((tmp + chunkLength) >= 4096) ||
				((chunkLength + bufPos1) >= 4096)) {

			for (i = 0; i < chunkLength; i++, dest++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
				bufPos1 = (bufPos1 + 1) % 4096;
				bufPos2 = (bufPos2 + 1) % 4096;
			}

		} else if (((tmp + chunkLength) < bufPos1) ||
				((chunkLength + bufPos1) < bufPos2)) {

			memcpy(dest, buf + bufPos2, chunkLength);
			memmove(buf + bufPos1, buf + bufPos2, chunkLength);

			dest += chunkLength;
			bufPos1 += chunkLength;
			bufPos2 += chunkLength;

		} else {

			for (i = 0; i < chunkLength; i++, dest++, bufPos1++, bufPos2++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
			}

		}
		frameLength -= chunkLength;

	}
}

void ImdPlayer::play(const char *path, int16 x, int16 y, bool interruptible) {
	int16 mouseX;
	int16 mouseY;
	int16 buttons;

	_vm->_util->setFrameRate(12);
	if(!openImd(path, x, y, 0, 2))
		return;

	_vm->_video->fillRect(_vm->_draw->_frontSurface, x, y,
			x + _curImd->width - 1, y + _curImd->height - 1, 0);

	for (int i = 0; i < _curImd->framesCount; i++) {
		play(i, 4, 0, 255, 0, _curImd->framesCount - 1);

		if (_vm->_quitRequested || (interruptible &&
		     (_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B)))
			break;
	}

	closeImd();
}

void ImdPlayer::play(const char *path, int16 x, int16 y, int16 startFrame,
		int16 frames, bool fade, bool interruptible) {
	int16 mouseX;
	int16 mouseY;
	int16 buttons = 0;
	int endFrame;

	_vm->_util->setFrameRate(12);
	if(!openImd(path, x, y, 0, 0))
		return;

	_vm->_video->fillRect(_vm->_draw->_frontSurface, x, y,
			x + _curImd->width - 1, y + _curImd->height - 1, 0);

	if (fade)
		_vm->_palAnim->fade(0, -2, 0);

	endFrame = frames > 0 ? frames : _curImd->framesCount;
	for (int i = startFrame; i < endFrame; i++) {
		view(_curImd, i);
		drawFrame(_curImd, i, x, y);
		if (fade) {
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			fade = false;
		}
		_vm->_video->waitRetrace();

		if (_vm->_quitRequested || (interruptible &&
		     (_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B))) {
			_vm->_palAnim->fade(0, -2, 0);
			_vm->_video->clearSurf(_vm->_draw->_frontSurface);
			memset((char *) _vm->_draw->_vgaPalette, 0, 768);

			WRITE_VAR(4, buttons);
			WRITE_VAR(0, 0x11B);
			WRITE_VAR(57, (uint32) -1);
			break;
		}

		_vm->_util->waitEndFrame();
	}

	if (frames < 0) {
		endFrame = _curImd->framesCount + frames;
		for (int i = _curImd->framesCount - 1; i >= endFrame; i--) {
			seekFrame(_curImd, i, SEEK_SET, true);
			drawFrame(_curImd, i, x, y);
			_vm->_video->waitRetrace();

			if (_vm->_quitRequested || (interruptible &&
			     (_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B))) {
				_vm->_palAnim->fade(0, -2, 0);
				_vm->_video->clearSurf(_vm->_draw->_frontSurface);
				memset((char *) _vm->_draw->_vgaPalette, 0, 768);

				WRITE_VAR(4, buttons);
				WRITE_VAR(0, 0x11B);
				WRITE_VAR(57, (uint32) -1);
				break;
			}

			_vm->_util->waitEndFrame();
		}
	}

	closeImd();
}

void ImdPlayer::play(int16 frame, uint16 palCmd,
		int16 palStart, int16 palEnd, int16 palFrame, int16 lastFrame) {
	uint32 viewRet = 0;
	SurfaceDesc *surfDescBak;
	bool modifiedPal = false;

	_vm->_draw->_showCursor = 0;

  if ((frame < 0) || (frame > lastFrame))
		return;

	palCmd &= 0x3F;
	if ((frame == palFrame) || ((frame == lastFrame) && (palCmd == 8))) {
		modifiedPal = true;
		_vm->_draw->_applyPal = true;

		if (palCmd >= 4)
			copyPalette(palStart, palEnd);
	}

	if (modifiedPal && (palCmd == 8) && (_backSurf == 20))
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	if (_curImd->surfDesc) {
		if (_curImd->surfDesc->_vidMode == 0x14) {

			if ((_frontMem == _vm->_draw->_frontSurface->getVidMem()) &&
			    (_frontSurf == 20)) {
				_vm->_draw->_frontSurface->swap(_vm->_draw->_backSurface);
				viewRet = view(_curImd, frame);
				_vm->_draw->_frontSurface->swap(_vm->_draw->_backSurface);
			} else
				viewRet = view(_curImd, frame);

			if (_frontSurf == 21)
				_vm->_draw->invalidateRect(_left, _top, _right, _bottom);

		} else {
			if ((_curImd->field_E & 0x100) &&
					(_vm->_global->_videoMode == 0x14) &&
					(_frontSurf == 20) &&
					(checkFrameType(_curImd, frame) & 0x8000) &&
					(_backSurf == 21)) {

				surfDescBak = _curImd->surfDesc;
				if (_frontMem == _vm->_draw->_spritesArray[20]->getVidMem())
					_curImd->surfDesc = _vm->_draw->_spritesArray[21];
				else
					_curImd->surfDesc = _vm->_draw->_spritesArray[20];
				setXY(_curImd, _curX, _curY);
				viewRet = view(_curImd, frame);
				_curImd->surfDesc = surfDescBak;
				setXY(_curImd, 0, 0);

			} else {
				viewRet = view(_curImd, frame);
				if (!(viewRet & 0x800))
					drawFrame(frame);
			}
		}
	} else
		viewRet = view(_curImd, frame);

	if (modifiedPal && (palCmd == 16)) {
		if (_backSurf == 21)
			_vm->_draw->forceBlit();
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
		_vm->_draw->_noInvalidated = true;
	}

	if (viewRet & 0x10) {
		copyPalette(palStart, palEnd);

		if (_backSurf == 20)
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		else
			_vm->_draw->_applyPal = true;
	}

	if (modifiedPal && (palCmd == 8) && (_backSurf == 21))
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	if (!(viewRet & 0x800)) {
		if (_vm->_draw->_cursorIndex == -1) {
			if (_frontSurf == 20)
				flipFrontMem();
			else
				_vm->_draw->blitInvalidated();
		} else
			_vm->_draw->animateCursor(-1);
	}

	if (modifiedPal && ((palCmd == 2) || (palCmd == 4)))
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);

	// To allow quitting, etc. during IMDs
	_vm->_util->processInput();
	if (_vm->_quitRequested) 
		return;

	if (_soundStage != 2) {
		if (viewRet & 0x800) {
			if (_frameDelay == 0)
				_vm->_util->delay(30);
			else {
				_frameDelay -= 30;
				if (_frameDelay < 0)
					_frameDelay = 0;
			}
		} else
			_vm->_util->waitEndFrame();
	}

	_vm->_inter->animPalette();
}

inline void ImdPlayer::drawFrame(int16 frame) {
	if (_backSurf == 21) {

		if (_vm->_global->_videoMode == 0x14) {
			if (_frontSurf == 21) {
				_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
				drawFrame(_curImd, frame, _curX, _curY);
				_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
				_vm->_draw->invalidateRect(_curX + _left, _curY + _top,
						_curX + _right, _curY + _bottom);
			} else {
				if (_frontMem == _vm->_draw->_spritesArray[20]->getVidMem()) {
					_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
					drawFrame(_curImd, frame, _curX, _curY);
					_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
				} else
					drawFrame(_curImd, frame, _curX, _curY);
			}
		} else
			_vm->_draw->invalidateRect(_left, _top, _right, _bottom);

	} else if (_vm->_global->_videoMode == 0x14)
		drawFrame(_curImd, frame, _curX, _curY);
}

inline void ImdPlayer::copyPalette(int16 palStart, int16 palEnd) {
	if (palStart == -1)
		memcpy((char *) _vm->_global->_pPaletteDesc->vgaPal,
				(char *) _curImd->palette, 768);
	else
		memcpy(((char *) (_vm->_global->_pPaletteDesc->vgaPal)) +
				palStart * 3, ((char *) (_curImd->palette)) + palStart * 3,
				(palEnd - palStart + 1) * 3);
}

inline void ImdPlayer::flipFrontMem() {
	if (_frontMem == _vm->_draw->_frontSurface->getVidMem())
		_frontMem = _vm->_draw->_backSurface->getVidMem();
	else
		_frontMem = _vm->_draw->_frontSurface->getVidMem();
}

uint16 ImdPlayer::checkFrameType(Imd *imdPtr, int16 frame) {
	uint16 retVal = 0;
	uint32 posBak;
	uint32 tmp;
	uint16 cmd;
	int16 frameBak;

	if (!imdPtr)
		return 0x8000;

	posBak = _vm->_dataIO->getPos(imdPtr->handle);
	frameBak = imdPtr->curFrame;

	if (imdPtr->curFrame != frame) {
		retVal |= 0x2000;
		seekFrame(imdPtr, frame, SEEK_SET);
	}

	do {
		if (frame != 0) {
			if (imdPtr->stdX != -1)
				retVal |= 0x1000;
			if (imdPtr->frameCoords && (imdPtr->frameCoords[frame].left != -1))
				retVal |= 0x400;
		}

		cmd = _vm->_dataIO->readUint16(imdPtr->handle);

		if ((cmd & 0xFFF8) == 0xFFF0) {
			if (cmd == 0xFFF0) {
				_vm->_dataIO->seekData(imdPtr->handle, 2, SEEK_CUR);
				cmd = _vm->_dataIO->readUint16(imdPtr->handle);
			}

			if (cmd == 0xFFF1) {
				retVal = 0x8000;
				continue;
			} else if (cmd == 0xFFF2) { // Skip (16 bit)
				cmd = _vm->_dataIO->readUint16(imdPtr->handle);
				_vm->_dataIO->seekData(imdPtr->handle, cmd, SEEK_CUR);
				retVal = 0x8000;
				continue;
			} else if (cmd == 0xFFF3) { // Skip (32 bit)
				tmp = _vm->_dataIO->readUint32(imdPtr->handle);
				_vm->_dataIO->seekData(imdPtr->handle, cmd, SEEK_CUR);
				retVal = 0x8000;
				continue;
			}
		}

		// Jump to frame
		if (cmd == 0xFFFD) {
			frame = _vm->_dataIO->readUint16(imdPtr->handle);
			if (imdPtr->framesPos) {
				_vm->_dataIO->seekData(imdPtr->handle,
						imdPtr->framesPos[frame], SEEK_SET);
				retVal |= 0x200;
				continue;
			}
			break;
		}

		// Next sound slice data
		if (cmd == 0xFF00) {
			_vm->_dataIO->seekData(imdPtr->handle,
					_soundSliceSize, SEEK_CUR);
			cmd = _vm->_dataIO->readUint16(imdPtr->handle);
		// Initial sound data (all slices)
		} else if (cmd == 0xFF01) {
			_vm->_dataIO->seekData(imdPtr->handle,
					_soundSliceSize * _soundSlicesCount, SEEK_CUR);
			cmd = _vm->_dataIO->readUint16(imdPtr->handle);
		}

		// Frame video data
		if (cmd != 0) {
			_vm->_dataIO->readData(imdPtr->handle, (char *) _frameData, 5);
			retVal |= _frameData[0];
		} else
			retVal |= 0x800;

		break;

	} while (true);

	_vm->_dataIO->seekData(imdPtr->handle, posBak, SEEK_SET);
	imdPtr->curFrame = frameBak;
	return retVal;
}

void ImdPlayer::seekFrame(Imd *imdPtr, int16 frame, int16 from, bool restart) {
	uint32 framePos;

	if (!imdPtr)
		return;

	if (from == SEEK_CUR)
		frame += imdPtr->curFrame;
	else if (from == SEEK_END)
		frame = imdPtr->framesCount - frame - 1;

	if (frame >= imdPtr->framesCount)
		return;

	if (frame == 0) {
		framePos = imdPtr->firstFramePos;
	} else if (frame == 1) {
		framePos = imdPtr->firstFramePos;
		_vm->_dataIO->seekData(imdPtr->handle, framePos, SEEK_SET);
		framePos += _vm->_dataIO->readUint16(imdPtr->handle) + 4;
	} else if (imdPtr->framesPos) {
		framePos = imdPtr->framesPos[frame];
	} else if (restart && (_soundStage == 0)) {
		for (int i = 0; i <= frame; i++)
			view(_curImd, i);
	} else
		error("%s: Frame %d is not directly accessible", _curFile, frame);

	_vm->_dataIO->seekData(imdPtr->handle, framePos, SEEK_SET);
	imdPtr->curFrame = frame;
}

uint32 ImdPlayer::view(Imd *imdPtr, int16 frame) {
	uint32 retVal = 0;
	uint32 cmd = 0;
	int16 xBak, yBak, heightBak, widthBak;
	bool hasNextCmd = false;
	bool startSound = false;

	if (!imdPtr)
		return 0x8000;

	if (frame != imdPtr->curFrame) {
		retVal |= 0x2000;
		seekFrame(imdPtr, frame, SEEK_SET);
	}

	_left = xBak = imdPtr->x;
	_top = yBak = imdPtr->y;
	_bottom = heightBak= imdPtr->height;
	_right = widthBak = imdPtr->width;

	if ((frame == 0) && (imdPtr->verMin & 0x800))
		_vm->_video->setPalette(imdPtr->palette);

	do {
		if (frame != 0) {
			if (imdPtr->stdX != -1) {
				_left = imdPtr->x = imdPtr->stdX;
				_top = imdPtr->y = imdPtr->stdY;
				_right = imdPtr->width = imdPtr->stdWidth;
				_bottom = imdPtr->height = imdPtr->stdHeight;
				_right += _left - 1;
				_bottom += _top - 1;
				retVal |= 0x1000;
			}
			if (imdPtr->frameCoords &&
					(imdPtr->frameCoords[frame].left != -1)) {
				_left = imdPtr->x = imdPtr->frameCoords[frame].left;
				_top = imdPtr->y = imdPtr->frameCoords[frame].top;
				_right = imdPtr->width =
					imdPtr->frameCoords[frame].right - imdPtr->x + 1;
				_bottom = imdPtr->height =
					imdPtr->frameCoords[frame].bottom - imdPtr->y + 1;
				_right += _left - 1;
				_bottom += _top - 1;
				retVal |= 0x400;
			}
		}

		cmd = _vm->_dataIO->readUint16(imdPtr->handle);

		if ((cmd & 0xFFF8) == 0xFFF0) {
			if (cmd == 0xFFF0) {
				_vm->_dataIO->seekData(imdPtr->handle, 2, SEEK_CUR);
				cmd = _vm->_dataIO->readUint16(imdPtr->handle);
			}

			if (cmd == 0xFFF1) {
				retVal = 0x8000;
				continue;
			} else if (cmd == 0xFFF2) { // Skip (16 bit)
				cmd = _vm->_dataIO->readUint16(imdPtr->handle);
				_vm->_dataIO->seekData(imdPtr->handle, cmd, SEEK_CUR);
				retVal = 0x8000;
				continue;
			} else if (cmd == 0xFFF3) { // Skip (32 bit)
				cmd = _vm->_dataIO->readUint32(imdPtr->handle);
				_vm->_dataIO->seekData(imdPtr->handle, cmd, SEEK_CUR);
				retVal = 0x8000;
				continue;
			}
		}

		if (_soundStage != 0) {
			char *soundBuf =
				(char *) (_soundBuffer + _curSoundSlice * _soundSliceSize);

			if (!hasNextCmd)
				waitEndSoundSlice();

			// Next sound slice data
			if (cmd == 0xFF00) {

				if (!hasNextCmd && !_noSound) {
					_vm->_dataIO->readData(imdPtr->handle, soundBuf,
							_soundSliceSize);
					_vm->_snd->convToSigned((byte *) soundBuf, _soundSliceSize);
				} else
					_vm->_dataIO->seekData(imdPtr->handle,
							_soundSliceSize, SEEK_CUR);

				cmd = _vm->_dataIO->readUint16(imdPtr->handle);

			// Initial sound data (all slices)
			} else if (cmd == 0xFF01) {
				int dataLength = _soundSliceSize * _soundSlicesCount;

				if (!hasNextCmd && !_noSound) {
					_vm->_dataIO->readData(imdPtr->handle,
							(char *) _soundBuffer, dataLength);
					_vm->_snd->convToSigned(_soundBuffer, dataLength);

					_curSoundSlice = _soundSlicesCount - 1;
					_soundStage = 1;
					startSound = true;
				} else
					_vm->_dataIO->seekData(imdPtr->handle, dataLength, SEEK_CUR);

				cmd = _vm->_dataIO->readUint16(imdPtr->handle);

			// Clear sound slice
			} else if (!hasNextCmd && (!_noSound))
				memset(soundBuf, 0, _soundSliceSize);

			if (!hasNextCmd)
				_curSoundSlice = (_curSoundSlice + 1) % _soundSlicesCount;
		}

		// Set palette
		if (cmd == 0xFFF4) {
			_vm->_dataIO->seekData(imdPtr->handle, 2, SEEK_CUR);
			retVal |= 0x10;
			if (imdPtr->extraPalette) {
				_vm->_dataIO->readData(imdPtr->handle,
						(char *) imdPtr->extraPalette, 768);
				_vm->_video->setPalette(imdPtr->extraPalette);
			} else if (imdPtr->palette)
				_vm->_dataIO->readData(imdPtr->handle,
						(char *) imdPtr->palette, 768);
			else
				_vm->_dataIO->readData(imdPtr->handle,
						(char *) _frameData, 768);

			cmd = _vm->_dataIO->readUint16(imdPtr->handle);
		}

		hasNextCmd = false;

		// Jump to frame
		if (cmd == 0xFFFD) {

			frame = _vm->_dataIO->readUint16(imdPtr->handle);
			if (imdPtr->framesPos) {
				imdPtr->curFrame = frame;
				_vm->_dataIO->seekData(imdPtr->handle,
						imdPtr->framesPos[frame], SEEK_SET);

				hasNextCmd = true;
				retVal |= 0x200;
			}

		} else if (cmd == 0xFFFC) {

			retVal |= 1;
			cmd = _vm->_dataIO->readUint32(imdPtr->handle);
			_vm->_dataIO->readData(imdPtr->handle,
					(char *) _frameData, cmd + 2);

			if (imdPtr->surfDesc) {
				int16 left = imdPtr->x;
				int16 top = imdPtr->y;
				int16 right = imdPtr->width + left;
				int16 bottom = imdPtr->height + top;

				if (imdPtr->surfDesc->getWidth() < right) {
					left = 0;
					right = imdPtr->width;
				}
				if (imdPtr->surfDesc->getWidth() < right)
					right = imdPtr->surfDesc->getWidth();
				if (imdPtr->surfDesc->getHeight() < bottom) {
					top = 0;
					bottom = imdPtr->height;
				}
				if (imdPtr->surfDesc->getHeight() < bottom)
					bottom = imdPtr->surfDesc->getHeight();

				imdPtr->x = left;
				imdPtr->y = top;
				imdPtr->height = bottom - top;
				imdPtr->width = right - left;

				renderFrame(imdPtr);
			}

			retVal |= _frameData[0];

		// Frame video data
		} else if (cmd != 0) {

			_vm->_dataIO->readData(imdPtr->handle, (char *) _frameData, cmd + 2);
			if (imdPtr->surfDesc)
				renderFrame(imdPtr);

			retVal |= _frameData[0];

		} else
			retVal |= 0x800;

	} while (hasNextCmd);

	if (startSound) {
		_vm->_snd->stopSound(0);
		_vm->_snd->playSample(_soundDesc, -1, _soundFreq);
		_soundStage = 2;
	}

	imdPtr->x = xBak;
	imdPtr->y = yBak;
	imdPtr->width = widthBak;
	imdPtr->height = heightBak;

	imdPtr->curFrame++;
	if ((imdPtr->curFrame == (imdPtr->framesCount - 1)) && (_soundStage == 2)) {
		// Clear the remaining sound buffer
		if (_curSoundSlice > 0)
			memset(_soundBuffer + _curSoundSlice * _soundSliceSize, 0,
					_soundSliceSize * _soundSlicesCount -
					_curSoundSlice * _soundSliceSize);

		_vm->_snd->setRepeating(0);
		_vm->_snd->waitEndPlay();
	}

	return retVal;
}

inline void ImdPlayer::waitEndSoundSlice() {
	_vm->_video->retrace();
	_vm->_util->delay(_soundSliceLength);
}

} // End of namespace Gob
