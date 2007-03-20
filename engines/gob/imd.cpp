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

	_frameDataSize = 0;
	_vidBufferSize = 0;
	_frameData = 0;
	_vidBuffer = 0;

	_frontSurf = 21;
	_backSurf = 21;
	_frontMem = 0;
	_frameDelay = 0;
}

ImdPlayer::~ImdPlayer() {
	if (_curImd) {
		_curImd->surfDesc = 0;
		delete[] _curImd->palette;
		delete[] _curImd->framesPos;
		delete[] _curImd->frameCoords;
	}
	delete[] _frameData;
	delete[] _vidBuffer;
	delete[] _frontMem;
	delete _curImd;
}

// flagsBit: 0 = read and set palette
//           1 = read palette
ImdPlayer::Imd *ImdPlayer::loadImdFile(const char *path, SurfaceDesc *surfDesc, int8 flags) {
	int i;
	Imd *imdPtr;
	int16 handle;
	bool setAllPalBak;
	char buf[18];
	Video::Color *palBak;

	int32 byte_31449 = 0;
	int32 byte_3144D = 0;

	buf[0] = 0;
	strcpy(buf, path);
	strcat(buf, ".IMD");

	handle = _vm->_dataIO->openData(buf);

	if (handle < 0) {
		warning("Can't open IMD \"%s\"", buf);
		return 0;
	}

	imdPtr = new Imd;
	memset(imdPtr, 0, sizeof(Imd));

	_vm->_dataIO->readData(handle, buf, 18);

	// "fileHandle" holds the major version while loading
	imdPtr->fileHandle = READ_LE_UINT16(buf);
	imdPtr->verMin = READ_LE_UINT16(buf + 2);
	imdPtr->framesCount = READ_LE_UINT16(buf + 4);
	imdPtr->x = READ_LE_UINT16(buf + 6);
	imdPtr->y = READ_LE_UINT16(buf + 8);
	imdPtr->width = READ_LE_UINT16(buf + 10);
	imdPtr->height = READ_LE_UINT16(buf + 12);
	imdPtr->field_E = READ_LE_UINT16(buf + 14);
	imdPtr->curFrame = READ_LE_UINT16(buf + 16);

	if (imdPtr->fileHandle != 0)
		imdPtr->verMin = 0;

	if ((imdPtr->verMin & 0xFF) < 2) {
		warning("IMD version incorrect (%d,%d)", imdPtr->fileHandle, imdPtr->verMin);
		_vm->_dataIO->closeData(handle);
		delete imdPtr;
		return 0;
	}

	imdPtr->surfDesc = surfDesc;
	imdPtr->firstFramePos = imdPtr->curFrame;
	
/*
	imdPtr->field_3A = 0;
	if ((imdPtr->verMin & 0x800) && ((flags & 3) != 3))
		imdPtr->field_3A = new Video::Color[256];
*/

	if (flags & 3) {
		imdPtr->palette = new Video::Color[256];
		_vm->_dataIO->readData(handle, (char *) imdPtr->palette, 768);
	} else {
		imdPtr->palette = 0;
		_vm->_dataIO->seekData(handle, 768, 1);
	}

	if ((flags & 3) == 1) {
		palBak = _vm->_global->_pPaletteDesc->vgaPal;
		setAllPalBak = _vm->_global->_setAllPalette;

		_vm->_global->_pPaletteDesc->vgaPal = imdPtr->palette;
		_vm->_global->_setAllPalette = true;
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

		_vm->_global->_setAllPalette = setAllPalBak;
		_vm->_global->_pPaletteDesc->vgaPal = palBak;
	}

	if ((imdPtr->verMin & 0xFF) >= 3) {
		_vm->_dataIO->readData(handle, buf, 2);
		imdPtr->stdX = READ_LE_UINT16(buf);
		if (imdPtr->stdX > 1) {
			warning("IMD ListI incorrect (%d)", imdPtr->stdX);
			_vm->_dataIO->closeData(handle);
			delete imdPtr;
			return 0;
		}
		if (imdPtr->stdX != 0) {
			_vm->_dataIO->readData(handle, buf, 8);
			imdPtr->stdX = READ_LE_UINT16(buf);
			imdPtr->stdY = READ_LE_UINT16(buf + 2);
			imdPtr->stdWidth = READ_LE_UINT16(buf + 4);
			imdPtr->stdHeight = READ_LE_UINT16(buf + 6);
		} else
			imdPtr->stdX = -1;
	} else
		imdPtr->stdX = -1;

	imdPtr->framesPos = 0;
	if ((imdPtr->verMin & 0xFF) >= 4) {
		_vm->_dataIO->readData(handle, buf, 4);
		byte_31449 = READ_LE_UINT32(buf);
		if (byte_31449 != 0)
			imdPtr->framesPos = new int32[imdPtr->framesCount];
	}

	if (imdPtr->verMin & 0x8000) {
		_vm->_dataIO->readData(handle, buf, 4);
		byte_3144D = READ_LE_UINT32(buf);
	}

	if (imdPtr->verMin & 0x4000) { // loc_29C4F
		int16 word_3145B[3];

		flags &= 0x7F;
		_vm->_dataIO->readData(handle, buf, 6);
		word_3145B[0] = READ_LE_UINT16(buf);
		word_3145B[1] = READ_LE_UINT16(buf + 2);
		word_3145B[2] = READ_LE_UINT16(buf + 4);

		if (word_3145B[0] < 0)
			word_3145B[0] = -word_3145B[0];
		if (word_3145B[2] < 0)
			word_3145B[2] = -word_3145B[2];

		warning("Imd Stub! Sound stuff!");
		return 0;
	} // loc_29E43

	if (imdPtr->verMin & 0x2000) {
		_vm->_dataIO->readData(handle, buf, 2);
		imdPtr->frameDataSize = READ_LE_UINT16(buf);
		if (imdPtr->frameDataSize == 0) {
			_vm->_dataIO->readData(handle, buf, 8);
			imdPtr->frameDataSize = READ_LE_UINT32(buf);
			imdPtr->vidBufferSize = READ_LE_UINT32(buf + 4);
		} else {
			_vm->_dataIO->readData(handle, buf, 2);
			imdPtr->vidBufferSize = READ_LE_UINT16(buf);
		}
	} else {
		imdPtr->frameDataSize = imdPtr->width * imdPtr->height + 500;
		imdPtr->vidBufferSize = 0;
		if (!(imdPtr->field_E & 0x100) || (imdPtr->field_E & 0x1000))
			imdPtr->vidBufferSize = imdPtr->width * imdPtr->height + 500;
	}
	warning("-> %d, %d", imdPtr->frameDataSize, imdPtr->vidBufferSize);

	if (imdPtr->framesPos != 0) {
		_vm->_dataIO->seekData(handle, byte_31449, 0);
		for (i = 0; i < imdPtr->framesCount; i++) {
			_vm->_dataIO->readData(handle, buf, 4);
			imdPtr->framesPos[i] = READ_LE_UINT32(buf);
		}
	}

	if (imdPtr->verMin & 0x8000) {
		_vm->_dataIO->seekData(handle, byte_3144D, 0);
		imdPtr->frameCoords = new ImdCoord[imdPtr->framesCount];
		for (i = 0; i < imdPtr->framesCount; i++) {
			_vm->_dataIO->readData(handle, buf, 8);
			imdPtr->frameCoords[i].left = READ_LE_UINT16(buf);
			imdPtr->frameCoords[i].top = READ_LE_UINT16(buf + 2);
			imdPtr->frameCoords[i].right = READ_LE_UINT16(buf + 4);
			imdPtr->frameCoords[i].bottom = READ_LE_UINT16(buf + 6);
		}
	} else
		imdPtr->frameCoords = 0;

	_vm->_dataIO->seekData(handle, imdPtr->firstFramePos, 0);
	imdPtr->curFrame = 0;
	imdPtr->fileHandle = handle;
	imdPtr->filePos = imdPtr->firstFramePos;
	_frameDataSize = imdPtr->frameDataSize;
	_vidBufferSize = imdPtr->vidBufferSize;
	if (flags & 0x80) {
		imdPtr->verMin |= 0x1000;
		warning("Imd Stub! loadImdFile(), flags & 0x80");
	}

	return imdPtr;
}

void ImdPlayer::finishImd(ImdPlayer::Imd *imdPtr) {
	if (imdPtr == 0)
		return;

/*
	if (dword_31345 != 0) {
		_vm->_sound->stopSound(0);
		dword_31345 = 0;
		delete off_31461;
		byte_31344 = 0;
	}
*/

	_vm->_dataIO->closeData(imdPtr->fileHandle);

	if (imdPtr->frameCoords != 0)
		delete[] imdPtr->frameCoords;
	if (imdPtr->palette != 0)
		delete[] imdPtr->palette;
	if (imdPtr->framesPos != 0)
		delete[] imdPtr->framesPos;

	delete imdPtr;
	imdPtr = 0;
}

int8 ImdPlayer::openImd(const char *path, int16 x, int16 y, int16 repeat, int16 flags) {
	int i;
	int j;
	const char *src;
	byte *vidMem;
	SurfaceDesc *surfDesc;

	if (path[0] != 0) {
		if (!_curImd)
			_curFile[0] = 0;

		src = strrchr(path, '\\');
		src = src == 0 ? path : src+1;

		if (strcmp(_curFile, src) != 0) {
			closeImd();

			_curImd = loadImdFile(path, 0, 2);
			if (!_curImd)
				return 0;

			_curX = _curImd->x;
			_curY = _curImd->y;
			strcpy(_curFile, src);
			_frameData = new byte[_frameDataSize + 500];
			_vidBuffer = new byte[_vidBufferSize + 500];
			memset(_frameData, 0, _frameDataSize + 500);
			memset(_vidBuffer, 0, _vidBufferSize + 500);

			if (_vm->_global->_videoMode == 0x14) {
				_backSurf = (flags & 0x80) ? 20 : 21;
				if (!(_curImd->field_E & 0x100) || (_curImd->field_E & 0x2000)) {
					setXY(_curImd, 0, 0);
					_curImd->surfDesc =
						_vm->_video->initSurfDesc(0x13, _curImd->width, _curImd->height, 0);
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
						surfDesc = _vm->_video->initSurfDesc(0x13, _curImd->width, _curImd->height, 0);
						_vm->_video->drawSprite(_vm->_draw->_spritesArray[21], surfDesc, _curX, _curY,
								_curX + _curImd->width - 1, _curY + _curImd->height - 1, 0, 0, 0);
						vidMem = _curImd->surfDesc->getVidMem();
						for (i = 0; i < _curImd->height; i++)
							for (j = 0; j < _curImd->width; j++, vidMem++) {
								*(vidMem) = *(surfDesc->getVidMem() +
										(j / 4) + (surfDesc->getWidth() / 4 * i));
							}
						surfDesc = 0;
					}
				}
			} else {
				if ((x != -1) || (y != -1)) {
					_curX = x != -1 ? x : _curX;
					_curY = y != -1 ? y : _curY;
					setXY(_curImd, _curX, _curY);
				}
				_backSurf = (flags & 0x80) ? 20 : 21;
				_curImd->surfDesc = _vm->_draw->_spritesArray[_backSurf];
			}
		}
	}

	if (!_curImd)
		return 0;

	if (repeat == -1) {
		closeImd();
		return 0;
	}

	_curX = x != -1 ? x : _curX;
	_curY = y != -1 ? y : _curY;

	WRITE_VAR(7, _curImd->framesCount);

	return 1;
}

void ImdPlayer::closeImd(void) {
	if (_curImd == 0)
		return;

	_curImd->surfDesc = 0;
	finishImd(_curImd);

	delete[] _frameData;
	delete[] _vidBuffer;
	_frameData = 0;
	_vidBuffer = 0;

	_curImd = 0;
}

void ImdPlayer::setXY(ImdPlayer::Imd *imdPtr, int16 x, int16 y) {
	int i;

	if (imdPtr->stdX != -1) {
		imdPtr->stdX = imdPtr->stdX - imdPtr->x + x;
		imdPtr->stdY = imdPtr->stdY - imdPtr->y + y;
	}
	
	if (imdPtr->frameCoords != 0) {
		for (i = 0; i < imdPtr->framesCount; i++) {
			imdPtr->frameCoords[i].left -= imdPtr->frameCoords[i].left - imdPtr->x + x;
			imdPtr->frameCoords[i].top -= imdPtr->frameCoords[i].top - imdPtr->y + y;
			imdPtr->frameCoords[i].right -= imdPtr->frameCoords[i].right - imdPtr->x + x;
			imdPtr->frameCoords[i].bottom -= imdPtr->frameCoords[i].bottom - imdPtr->y + y;
		}
	}

	imdPtr->x = x;
	imdPtr->y = y;
}

void ImdPlayer::play(int16 frame, uint16 palCmd, int16 palStart, int16 palEnd, int16 palFrame, int16 lastFrame) {
	int16 viewRet = 0;
	bool modifiedPal = false;
	SurfaceDesc *surfDescBak;

	_vm->_draw->_showCursor = 0;

	int8 byte_31344 = 0;

	if ((frame < 0) || (frame > lastFrame))
		return;

	if ((frame == palFrame) || ((frame == lastFrame) && (palCmd == 8))) { // loc_1C3F0
		modifiedPal = true;
		_vm->_draw->_applyPal = false;
		if (palCmd >= 4) {
			if (palStart != -1)
				memcpy( ((char *) (_vm->_global->_pPaletteDesc->vgaPal)) + palStart * 3,
						((char *) (_curImd->palette)) + palStart * 3, (palEnd - palStart + 1) * 3);
			else
				memcpy((char *) _vm->_global->_pPaletteDesc->vgaPal, (char *) _curImd->palette, 768);
		}
	}

	if (modifiedPal && (palCmd == 8) && (_backSurf == 20))
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	if (_curImd->surfDesc->_vidMode == 0x14) {
		if ((_frontSurf == 20) && (_frontMem == _vm->_draw->_spritesArray[20]->getVidMem())) {
			_vm->_draw->_spritesArray[20]->swap(_vm->_draw->_spritesArray[21]);
			viewRet = view(_curImd, frame);
			_vm->_draw->_spritesArray[20]->swap(_vm->_draw->_spritesArray[21]);
		} else
			viewRet = view(_curImd, frame);
		if (_frontSurf == 21) {
			if ((_curImd->frameCoords == 0) || (_curImd->frameCoords[frame].left == -1))
				_vm->_draw->invalidateRect(_curX, _curY,
						_curX + _curImd->width - 1, _curY + _curImd->height - 1);
			else
				_vm->_draw->invalidateRect(_curImd->frameCoords[frame].left,
						_curImd->frameCoords[frame].top, _curImd->frameCoords[frame].right,
						_curImd->frameCoords[frame].bottom);
		}
	} else {
		if ((_curImd->field_E & 0x100) && (_vm->_global->_videoMode == 0x14) &&
				(_frontSurf == 20) && (sub_2C825(_curImd) & 0x8000) && (_backSurf == 21)) {
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
			if (!(viewRet & 0x800)) {
				if (_backSurf == 21) {
					if (_vm->_global->_videoMode == 0x14) {
						if (_frontSurf == 21) {
							_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
							drawFrame(_curImd, frame, _curX, _curY);
							_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
							if ((_curImd->frameCoords == 0) || (_curImd->frameCoords[frame].left == -1))
								_vm->_draw->invalidateRect(_curX, _curY, _curX + _curImd->width - 1,
										_curY + _curImd->height - 1);
							else
								_vm->_draw->invalidateRect(_curImd->frameCoords[frame].left,
										_curImd->frameCoords[frame].top, _curImd->frameCoords[frame].right,
										_curImd->frameCoords[frame].bottom);
						} else {
							if (_frontMem == _vm->_draw->_spritesArray[20]->getVidMem()) { // loc_1C68D
								_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
								drawFrame(_curImd, frame, _curX, _curY);
								_vm->_draw->_frontSurface->swap(_vm->_draw->_spritesArray[21]);
							} else
								drawFrame(_curImd, frame, _curX, _curY);
						}
					} else {
						if ((_curImd->frameCoords == 0) || (_curImd->frameCoords[frame].left == -1))
							_vm->_draw->invalidateRect(_curX, _curY, _curX + _curImd->width - 1,
									_curY + _curImd->height - 1);
						else
							_vm->_draw->invalidateRect(_curImd->frameCoords[frame].left,
									_curImd->frameCoords[frame].top, _curImd->frameCoords[frame].right,
									_curImd->frameCoords[frame].bottom);
					}
				} else
					if (_vm->_global->_videoMode == 0x14)
						drawFrame(_curImd, frame, _curX, _curY);
			}
		}
	}

	if (modifiedPal && (palCmd == 16)) {
		if ((_vm->_draw->_spritesArray[20] != _vm->_draw->_spritesArray[21]) && (_backSurf == 21))
			_vm->_video->drawSprite(_vm->_draw->_spritesArray[21],
					_vm->_draw->_spritesArray[20], 0, 0,
					_vm->_draw->_spritesArray[21]->getWidth() - 1,
					_vm->_draw->_spritesArray[21]->getHeight() - 1, 0, 0, 0);
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
		_vm->_draw->_noInvalidated = true;
	}
	if (modifiedPal && (palCmd == 8) && (_backSurf == 21))
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);

	if (!(viewRet & 0x800)) {
		if (_vm->_draw->_cursorIndex == -1) {
			if (_frontSurf == 20) {
				if (_frontMem == _vm->_draw->_spritesArray[20]->getVidMem())
					_frontMem = _vm->_draw->_spritesArray[21]->getVidMem();
				else
					_frontMem = _vm->_draw->_spritesArray[20]->getVidMem();
				warning("GOB2 Stub! sub_1BC3A(_frontMem);");
			} else
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

	if (byte_31344 != 2) {
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

int16 ImdPlayer::view(ImdPlayer::Imd *imdPtr, int16 frame) {
	int16 x;
	int16 y;
	int16 width;
	int16 height;
	int16 retVal;
	uint32 tmp;
	char buf[4];

	int8 var_4;
	int32 var_12 = 0;

	// .---
	int16 word_31451 = 0;
	int8 byte_31344 = 0;
	int8 byte_2DA60 = 0;
	int16 word_2DA61 = -1;
	// '---

	word_31451 = 0;

	if (imdPtr == 0)
		return (int16)0x8000;

	retVal = 0;
	var_4 = 0;

	if (frame != imdPtr->curFrame) {
		retVal |= 0x2000;
		if (frame == 0)
			imdPtr->filePos = imdPtr->firstFramePos;
		else if (frame == 1) {
			imdPtr->filePos = imdPtr->firstFramePos;
			_vm->_dataIO->seekData(imdPtr->fileHandle, imdPtr->filePos, 0);
			_vm->_dataIO->readData(imdPtr->fileHandle, buf, 2);
			tmp = READ_LE_UINT16(buf);
			imdPtr->filePos += tmp + 4;
		} else if (imdPtr->framesPos != 0)
			imdPtr->filePos = imdPtr->framesPos[frame];
		else
			error("Image %d inaccessible in IMD", frame);
		imdPtr->curFrame = frame;
		_vm->_dataIO->seekData(imdPtr->fileHandle, imdPtr->filePos, 0);
	}

	x = imdPtr->x;
	y = imdPtr->y;
	width = imdPtr->width;
	height = imdPtr->height;

	do {
		if (frame != 0) {
			if (imdPtr->stdX != -1) {
				imdPtr->x = imdPtr->stdX;
				imdPtr->y = imdPtr->stdY;
				imdPtr->width = imdPtr->stdWidth;
				imdPtr->height = imdPtr->stdHeight;
				retVal |= 0x1000;
			}
			if ((imdPtr->frameCoords != 0) && (imdPtr->frameCoords[frame].left != -1)) {
				var_4 |= 0x400;
				imdPtr->x = imdPtr->frameCoords[frame].left;
				imdPtr->y = imdPtr->frameCoords[frame].top;
				imdPtr->width = imdPtr->frameCoords[frame].right - imdPtr->x + 1;
				imdPtr->height = imdPtr->frameCoords[frame].bottom - imdPtr->y + 1;
			}
		}

		_vm->_dataIO->readData(imdPtr->fileHandle, buf, 2);
		tmp = READ_LE_UINT16(buf);

		imdPtr->filePos += 2;

		if ((tmp & 0xFFF8) == 0xFFF0) {
			if (tmp == 0xFFF0) {
				_vm->_dataIO->readData(imdPtr->fileHandle, buf, 2);
				tmp = READ_LE_UINT16(buf);
				if (var_4 == 0)
					word_31451 = tmp;
				_vm->_dataIO->readData(imdPtr->fileHandle, buf, 2);
				tmp = READ_LE_UINT16(buf);
				imdPtr->filePos += 4;
			} else if (tmp == 0xFFF1) {
				retVal = (int16)0x8000;
				continue;
			} else if (tmp == 0xFFF2) {
				_vm->_dataIO->readData(imdPtr->fileHandle, buf, 2);
				tmp = READ_LE_UINT16(buf);
				imdPtr->filePos += 2;
				_vm->_dataIO->seekData(imdPtr->fileHandle, tmp, 1);
				imdPtr->filePos += tmp;
				retVal = (int16)0x8000;
				continue;
			} else if (tmp == 0xFFF3) {
				_vm->_dataIO->readData(imdPtr->fileHandle, buf, 4);
				tmp = READ_LE_UINT32(buf);
				imdPtr->filePos += 4;
				_vm->_dataIO->seekData(imdPtr->fileHandle, tmp, 1);
				imdPtr->filePos += tmp;
				retVal = (int16)0x8000;
				continue;
			}
		}
		if (byte_31344 != 0) {
			if ((var_4 == 0) && (_vm->_global->_soundFlags & 0x14) && (byte_31344 == 2)) { // loc_2A503
				var_12 = _vm->_util->getTimeKey();
				warning("GOB2 Stub! view, IMD sound stuff");
			}
		}
		var_4 = 0;
		if (tmp == 0xFFFD) {
			_vm->_dataIO->readData(imdPtr->fileHandle, buf, 2);
			frame = READ_LE_UINT16(buf);
			if ((imdPtr->framesPos != 0) && (byte_2DA60 == 0)) {
				word_2DA61 = frame;
				imdPtr->filePos = imdPtr->framesPos[frame];
				_vm->_dataIO->seekData(imdPtr->fileHandle, imdPtr->filePos, 0);
				var_4 = 1;
				retVal |= 0x200;
				imdPtr->curFrame = frame;
			} else
				imdPtr->filePos += 2;
			continue;
		}
		if (tmp != 0) {
			imdPtr->filePos += tmp + 2;
			if (byte_2DA60 != 0) {
				_vm->_dataIO->seekData(imdPtr->fileHandle, tmp + 2, 1);
			} else {
				_vm->_dataIO->readData(imdPtr->fileHandle, (char *) _frameData, tmp + 2);
				retVal |= *_frameData;
				if (imdPtr->surfDesc == 0)
					continue;
				if (imdPtr->surfDesc->_vidMode != 0x14)
					renderframe(imdPtr);
				else
					warning("GOB2 Stub! viedImd, sub_2C69A(imdPtr);");
			}
		} else
			retVal |= 0x800;
	} while (var_4 != 0);

	if (byte_2DA60 != 0) {
		byte_2DA60 = 0;
		retVal |= 0x100;
	}

	imdPtr->x = x;
	imdPtr->y = y;
	imdPtr->width = width;
	imdPtr->height = height;
	imdPtr->curFrame++;
	
	return retVal;
}

void ImdPlayer::drawFrame(Imd *imdPtr, int16 frame, int16 x, int16 y,
		SurfaceDesc *dest) {
	if (!dest)
		dest = _vm->_draw->_frontSurface;

	if (frame == 0)
		_vm->_video->drawSprite(imdPtr->surfDesc, dest, 0, 0,
				imdPtr->width - 1, imdPtr->height - 1, x, y, 1);
	else if ((imdPtr->frameCoords != 0) && (imdPtr->frameCoords[frame].left != -1))
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

void ImdPlayer::renderframe(Imd *imdPtr) {
	int i;
	int16 imdX;
	int16 imdY;
	int16 imdW;
	int16 imdH;
	int16 sW;
	uint16 pixCount, pixWritten;
	uint8 type;
	byte *imdVidMem;
	byte *imdVidMemBak;
	byte *dataPtr = 0;
	byte *srcPtr = 0;
	byte *srcPtrBak = 0;

	dataPtr = (byte *) _frameData;
	imdX = imdPtr->x;
	imdY = imdPtr->y;
	imdW = imdPtr->width;
	imdH = imdPtr->height;
	sW = imdPtr->surfDesc->getWidth();
	imdVidMem = imdPtr->surfDesc->getVidMem() + sW * imdY + imdX;

	type = *dataPtr++;
	srcPtr = dataPtr;

	if (type & 0x10) {
		type ^= 0x10;
		dataPtr++; // => 0x3C8       |_ palette
		dataPtr += 48; // => 0x3C9   |  stuff
	}

	srcPtr = dataPtr;
	if (type & 0x80) {
		srcPtr = (byte *) _vidBuffer;
		type &= 0x7F;
		if ((type == 2) && (imdW == sW)) {
			frameUncompressor(imdVidMem, dataPtr);
			return;
		} else
			frameUncompressor(srcPtr, dataPtr);
	}

	if (type == 2) {
		for (i = 0; i < imdH; i++) {
			memcpy(imdVidMem, srcPtr, imdW);
			srcPtr += imdW;
			imdVidMem += sW;
		}
	} else if (type == 1) {
		imdVidMemBak = imdVidMem;
		for (i = 0; i < imdH; i++) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) {
					pixCount = (pixCount & 0x7F) + 1;
					// Just to be safe
					pixCount = (pixWritten + pixCount) > imdW ? imdW - pixWritten : pixCount;
					pixWritten += pixCount;
					memcpy(imdVidMem, srcPtr, pixCount);
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else {
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			imdVidMemBak += sW;
			imdVidMem = imdVidMemBak;
		}
	} else if (type == 0x42) { // loc_2AFC4
		warning("=> type == 0x42");
	} else if ((type & 0xF) == 2) { // loc_2AFEC
		warning("=> (type & 0xF) == 2");
	} else { // loc_2B021
		srcPtrBak = srcPtr;
		for (i = 0; i < imdH; i += 2) {
			pixWritten = 0;
			while (pixWritten < imdW) {
				pixCount = *srcPtr++;
				if (pixCount & 0x80) {
					pixCount = (pixCount & 0x7F) + 1;
					// Just to be safe
					pixCount = (pixWritten + pixCount) > imdW ? imdW - pixWritten : pixCount;
					pixWritten += pixCount;
					memcpy(imdVidMem, srcPtr, pixCount);
					memcpy(imdVidMem + sW, srcPtr, pixCount);
					imdVidMem += pixCount;
					srcPtr += pixCount;
				} else {
					pixCount = (pixCount + 1) % 256;
					pixWritten += pixCount;
					imdVidMem += pixCount;
				}
			}
			srcPtrBak += sW + sW;
			srcPtr = srcPtrBak;
		}
	}
}

void ImdPlayer::frameUncompressor(byte *dest, byte *src) {
	int i;
	byte buf[4370];
	int16 chunkLength;
	int16 frameLength;
	uint16 bufPos1;
	uint16 bufPos2;
	uint16 tmp;
	uint8 chunkBitField;
	uint8 chunkCount;
	bool mode;

	frameLength = READ_LE_UINT16(src);
	src += 4;
	bufPos1 = 4078;
	mode = 0; // 275h (jnz +2)
	if ((READ_LE_UINT16(src) == 0x1234) && (READ_LE_UINT16(src + 2) == 0x5678)) {
		src += 4;
		bufPos1 = 273;
		mode = 1; // 123Ch (cmp al, 12h)
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

		if ((mode && ((chunkLength & 0xFF) == 0x12)) || (!mode && (chunkLength == 0)))
			chunkLength = *src++ + 0x12;

		bufPos2 = (tmp & 0xFF) + ((tmp >> 4) & 0x0F00);
		if (((tmp + chunkLength) >= 4096) || ((chunkLength + bufPos1) >= 4096)) {
			for (i = 0; i < chunkLength; i++, dest++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
				bufPos1 = (bufPos1 + 1) % 4096;
				bufPos2 = (bufPos2 + 1) % 4096;
			}
			frameLength -= chunkLength;
		} else if (((tmp + chunkLength) < bufPos1) || ((chunkLength + bufPos1) < bufPos2)) {
			memcpy(dest, buf + bufPos2, chunkLength);
			dest += chunkLength;
			memmove(buf + bufPos1, buf + bufPos2, chunkLength);
			bufPos1 += chunkLength;
			bufPos2 += chunkLength;
			frameLength -= chunkLength;
		} else {
			for (i = 0; i < chunkLength; i++, dest++, bufPos1++, bufPos2++) {
				*dest = buf[bufPos2];
				buf[bufPos1] = buf[bufPos2];
			}
			frameLength -= chunkLength;
		}
	}
}

void ImdPlayer::play(const char *path, int16 x, int16 y, int16 startFrame, int16 frames,
		bool fade, bool interruptible) {
	int16 mouseX;
	int16 mouseY;
	int16 buttons = 0;
	int curFrame;
	int endFrame;
	int backFrame;

	_vm->_util->setFrameRate(12);
	openImd(path, 0, 0, 0, 0);
	_vm->_video->fillRect(_vm->_draw->_frontSurface, x, y, x + _curImd->width - 1,
			y + _curImd->height - 1, 0);

	if (fade)
		_vm->_palAnim->fade(0, -2, 0);

	endFrame = frames > 0 ? frames : _curImd->framesCount;
	for (curFrame = 0; curFrame < endFrame; curFrame++) {
		view(_curImd, curFrame);
		drawFrame(_curImd, curFrame, x, y);
		if (fade) {
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, -2, 0);
			fade = false;
		}
		_vm->_video->waitRetrace(_vm->_global->_videoMode);
		if ((interruptible && (_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B)) ||
				_vm->_quitRequested) {
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
		for (curFrame = _curImd->framesCount - 1; curFrame >= endFrame; curFrame--) {
			for (backFrame = 0; backFrame <= curFrame; backFrame++)
				view(_curImd, backFrame);
			drawFrame(_curImd, curFrame, x, y);
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
			if ((interruptible && (_vm->_game->checkKeys(&mouseX, &mouseY, &buttons, 0) == 0x11B)) ||
					_vm->_quitRequested) {
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

int16 ImdPlayer::sub_2C825(Imd *imdPtr) {
	warning("GOB2 Stub! sub_2C825()");
	return 0;
}

} // End of namespace Gob
