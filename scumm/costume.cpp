/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "actor.h"
#include "costume.h"
#include "scumm/sound.h"

const byte revBitMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

void CostumeRenderer::ignorePakCols(int a) {
	int n;

	n = _height;
	if (a > 1)
		n *= a;

	do {
		_repcolor = *_srcptr++;
		_replen = _repcolor & _maskval;
		if (_replen == 0) {
			_replen = *_srcptr++;
		}
		do {
			if (!--n) {
				_repcolor >>= _shrval;
				return;
			}
		} while (--_replen);
	} while (1);
}

const byte cost_scaleTable[256] = {
	255, 253, 125, 189, 61, 221, 93, 157, 29, 237,
	109, 173, 45, 205, 77, 141, 13, 245, 117, 181,
	53, 213, 85, 149, 21, 229, 101, 165, 37, 197, 69,
	133, 5, 249, 121, 185, 57, 217, 89, 153, 25, 233,
	105, 169, 41, 201, 73, 137, 9, 241, 113, 177, 49,
	209, 81, 145, 17, 225, 97, 161, 33, 193, 65, 129,
	1, 251, 123, 187, 59, 219, 91, 155, 27, 235, 107,
	171, 43, 203, 75, 139, 11, 243, 115, 179, 51, 211,
	83, 147, 19, 227, 99, 163, 35, 195, 67, 131, 3,
	247, 119, 183, 55, 215, 87, 151, 23, 231, 103,
	167, 39, 199, 71, 135, 7, 239, 111, 175, 47, 207,
	79, 143, 15, 223, 95, 159, 31, 191, 63, 127, 0,
	128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208,
	48, 176, 112, 240, 8, 136, 72, 200, 40, 168, 104,
	232, 24, 152, 88, 216, 56, 184, 120, 248, 4, 132,
	68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52,
	180, 116, 244, 12, 140, 76, 204, 44, 172, 108,
	236, 28, 156, 92, 220, 60, 188, 124, 252, 2, 130,
	66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50,
	178, 114, 242, 10, 138, 74, 202, 42, 170, 106,
	234, 26, 154, 90, 218, 58, 186, 122, 250, 6, 134,
	70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54,
	182, 118, 246, 14, 142, 78, 206, 46, 174, 110,
	238, 30, 158, 94, 222, 62, 190, 126, 254
};

byte CostumeRenderer::mainRoutine(int slot, int frame) {
	int xmove, ymove, i, b, s;
	uint scal;
	byte scaling;
	byte charsetmask, masking;
	byte unk19;
	byte newAmiCost;
	int ex1, ex2;
	
	newAmiCost = (_vm->_features & GF_AMIGA) || (_vm->_features & GF_OLD_BUNDLE);

	CHECK_HEAP
	_maskval = 0xF;
	_shrval = 4;
	if (_loaded._numColors == 32) {
		_maskval = 7;
		_shrval = 3;
	}

	_width2 = _srcptr[0];
	_width = _width2;
	_height2 = _srcptr[2];
	_height = _height2;
	xmove = (int16)READ_LE_UINT16(_srcptr + 4) + _xmove;
	ymove = (int16)READ_LE_UINT16(_srcptr + 6) + _ymove;
	_xmove += (int16)READ_LE_UINT16(_srcptr + 8);
	_ymove -= (int16)READ_LE_UINT16(_srcptr + 10);
	_srcptr += 12;

	switch (_loaded._ptr[7] & 0x7F) {
	case 0x60:
	case 0x61:
		ex1 = _srcptr[0];
		ex2 = _srcptr[1];
		_srcptr += 2;
		if (ex1 != 0xFF || ex2 != 0xFF) {
			ex1 = READ_LE_UINT16(_loaded._ptr + _loaded._numColors + 10 + ex1 * 2);
			_srcptr = _loaded._baseptr + READ_LE_UINT16(_loaded._ptr + ex1 + ex2 * 2) + 14;
		}
	}

	_xpos = _actorX;
	_ypos = _actorY;

	scaling = _scaleX == 255 && _scaleY == 255 ? 0 : 1;
	s = 0;

	if (scaling) {
		_scaleIndexXStep = -1;
		if (xmove < 0) {
			xmove = -xmove;
			_scaleIndexXStep = 1;
		}

		if (_mirror) {
			unk19 = _scaleIndexX = 128 - xmove;
			for (i = 0; i < xmove; i++) {
				scal = cost_scaleTable[_scaleIndexX++];
				if (scal < _scaleX)
					_xpos -= _scaleIndexXStep;
			}
			_right = _left = _xpos;
			_scaleIndexX = unk19;
			for (i = 0; i < _width; i++) {
				if (_right < 0) {
					s++;
					unk19 = _scaleIndexX;
				}
				scal = cost_scaleTable[_scaleIndexX++];
				if (scal < _scaleX)
					_right++;
			}
		} else {
			unk19 = _scaleIndexX = xmove + 128;
			for (i = 0; i < xmove; i++) {
				scal = cost_scaleTable[_scaleIndexX--];
				if (scal < _scaleX)
					_xpos += _scaleIndexXStep;
			}
			_right = _left = _xpos;
			_scaleIndexX = unk19;
			for (i = 0; i < _width; i++) {
				if (_left > (_vm->_realWidth - 1)) {
					s++;
					unk19 = _scaleIndexX;
				}
				scal = cost_scaleTable[_scaleIndexX--];
				if (scal < _scaleX)
					_left--;
			}
		}
		_scaleIndexX = unk19;
		if (s)
			s--;
		_scaleIndexYStep = -1;
		if (ymove < 0) {
			ymove = -ymove;
			_scaleIndexYStep = 1;
		}
		_scaleIndexY = 128 - ymove;
		for (i = 0; i < ymove; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY)
				_ypos -= _scaleIndexYStep;
		}
		_top = _bottom = _ypos;
		_scaleIndexY = 128 - ymove;
		for (i = 0; i < _height; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY)
				_bottom++;
		}
		_scaleIndexY = _scaleIndexYTop = 128 - ymove;
	} else {
		if (_mirror == 0)
			xmove = -xmove;
		_xpos += xmove;
		_ypos += ymove;
		if (_mirror) {
			_left = _xpos;
			_right = _xpos + _width;
		} else {
			_left = _xpos - _width;
			_right = _xpos;
		}
		_top = _ypos;
		_bottom = _top + _height;
	}

	_scaleIndexXStep = -1;
	if (_mirror)
		_scaleIndexXStep = 1;
	_ypostop = _ypos;

	_vm->updateDirtyRect(0, _left, _right + 1, _top, _bottom, _dirty_id);

	if (_top >= (int)_outheight || _bottom <= 0)
		return 0;

	_ypitch = _height * _vm->_realWidth;
	_docontinue = 0;
	b = 1;
	if (_left >= _vm->_realWidth || _right <= 0)
		return 1;
	if (_mirror) {
		_ypitch--;
		if (scaling == 0)
			s = -_xpos;
		if (s > 0) {
			_width2 -= s;
			ignorePakCols(s);
			_xpos = 0;
			_docontinue = 1;
		} else {
			s = _right - _vm->_realWidth;
			if (s <= 0) {
				b = 2;
			} else {
				_width2 -= s;
			}
		}
	} else {
		_ypitch++;
		if (scaling == 0)
			s = _right - _vm->_realWidth;
		if (s > 0) {
			_width2 -= s;
			ignorePakCols(s);
			_xpos = _vm->_realWidth - 1;
			_docontinue = 1;
		} else {
			s = -1 - _left;
			if (s <= 0)
				b = 2;
			else
				_width2 -= s;
		}
	}

	if (_width2 == 0)
		return 0;

	if ((uint) _top > (uint) _outheight)
		_top = 0;

	if (_left < 0)
		_left = 0;

	if ((uint) _bottom > _outheight)
		_bottom = _outheight;

	if (_draw_top > _top)
		_draw_top = _top;

	if (_draw_bottom < _bottom)
		_draw_bottom = _bottom;

	if (_height2 + _top >= 256) {
		CHECK_HEAP
		return 2;
	}

	_bgbak_ptr = _vm->getResourceAddress(rtBuffer, 5) + _vm->virtscr[0].xstart + _ypos * _vm->_realWidth + _xpos;
	_backbuff_ptr = _vm->virtscr[0].screenPtr + _vm->virtscr[0].xstart + _ypos * _vm->_realWidth + _xpos;
	charsetmask =
		_vm->hasCharsetMask(_left, _top + _vm->virtscr[0].topline, _right,
												_vm->virtscr[0].topline + _bottom);
	masking = 0;

	if (_vm->_features & GF_SMALL_HEADER)
		masking = (_zbuf != 0);
	else
		masking =
			_vm->isMaskActiveAt(_left, _top, _right, _bottom,
													_vm->getResourceAddress(rtBuffer, 9) +
													_vm->gdi._imgBufOffs[_zbuf] +
													_vm->_screenStartStrip);

	if (masking || charsetmask) {
		_mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + _ypos * _numStrips + _vm->_screenStartStrip;
		_imgbufoffs = _vm->gdi._imgBufOffs[_zbuf];
		if (!charsetmask && _zbuf != 0)
			_mask_ptr += _imgbufoffs;
		_mask_ptr_dest = _mask_ptr + _xpos / 8;
	}

	CHECK_HEAP
	if (_shadow_mode) {
		proc_special((masking << 1) + charsetmask);
		return b;
	}

	switch ((newAmiCost << 3) | (scaling << 2) | (masking << 1) | charsetmask) {
	case 0:
		proc6();										// no scaling, no masking, no charsetmask
		break;
	case 1:
	case 2:
		proc5();										// no scaling, masking or charsetmask
		break;
	case 3:
		proc4();										// no scaling, masking and charsetmask
		break;
	case 4:
		proc1();										// scaling, no masking, no charsetmask
		break;
	case 5:
	case 6:
		proc2();										// scaling, masking or charsetmask
		break;
	case 7:
		proc3();										// scaling, masking and charsetmask
		break;
	case 8:
		proc6_ami();								// no scaling, no masking, no charsetmask (Amiga)
		break;
	case 9:
	case 10:
		proc5_ami();								// no scaling, masking or charsetmask (Amiga)
		break;
	case 11:
		proc4_ami();								// no scaling, masking and charsetmask (Amiga)
		break;
	case 12:
		proc1_ami();								// scaling, no masking, no charsetmask (Amiga)
		break;
	case 13:
	case 14:
		proc2_ami();								// scaling, masking or charsetmask (Amiga)
		break;
	case 15:
		proc3_ami();								// scaling, masking and charsetmask (Amiga)
		break;
	}

	CHECK_HEAP
	return b;
}

void CostumeRenderer::proc6() {
	byte *src, *dst;
	byte len, height, pcolor, width;
	uint y;
	int color;
	uint scrheight;

	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	height = _height2;
	scrheight = _outheight;
	width = _width2;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (color && y < scrheight) {
				pcolor = _palette[color];
				if (pcolor == 13) {
					pcolor = _shadow_table[*dst];
				}
				*dst = pcolor;
			}

			dst += _vm->_realWidth;
			y++;
			if (!--height) {
				if (!--width)
					return;
				height = _height;
				dst -= _ypitch;
				y = _ypostop;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc5() {
	byte *mask, *src, *dst;
	byte maskbit, len, height, pcolor;
	uint y, scrheight;
	int color;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	height = _height2;
	scrheight = _outheight;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (color && y < scrheight && !(*mask & maskbit)) {
				pcolor = _palette[color];
				if (pcolor == 13)
					pcolor = _shadow_table[*dst];
				*dst = pcolor;
			}
			dst += _vm->_realWidth;
			y++;
			mask += _numStrips;
			if (!--height) {
				if (!--_width2)
					return;
				height = _height;
				dst -= _ypitch;
				y = _ypostop;
				if (_scaleIndexXStep != 1) {
					maskbit <<= 1;
					if (!maskbit) {
						maskbit = 1;
						_mask_ptr--;
					}
				} else {
					maskbit >>= 1;
					if (!maskbit) {
						maskbit = 0x80;
						_mask_ptr++;
					}
				}
				mask = _mask_ptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc4() {
	byte *mask, *src, *dst;
	byte maskbit, len, height, pcolor;
	uint y, scrheight;
	int color;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	height = _height2;
	scrheight = _outheight;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (color && y < scrheight && !((*mask | mask[_imgbufoffs]) & maskbit)) {
				pcolor = _palette[color];
				if (pcolor == 13)
					pcolor = _shadow_table[*dst];
				*dst = pcolor;
			}
			dst += _vm->_realWidth;
			y++;
			mask += _numStrips;
			if (!--height) {
				if (!--_width2)
					return;
				height = _height;
				dst -= _ypitch;
				y = _ypostop;
				if (_scaleIndexXStep != 1) {
					maskbit <<= 1;
					if (!maskbit) {
						maskbit = 1;
						_mask_ptr--;
					}
				} else {
					maskbit >>= 1;
					if (!maskbit) {
						maskbit = 0x80;
						_mask_ptr++;
					}
				}
				mask = _mask_ptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc3() {
	byte *mask, *src, *dst;
	byte maskbit, len, height, pcolor, width;
	int color, t;
	uint y;

	mask = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	height = _height2;
	width = _width2;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (cost_scaleTable[_scaleIndexY++] < _scaleY) {
				if (color && y < _outheight && !((*mask | mask[_imgbufoffs]) & maskbit)) {
					pcolor = _palette[color];
					if (pcolor == 13)
						pcolor = _shadow_table[*dst];
					*dst = pcolor;
				}
				dst += _vm->_realWidth;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--width)
					return;
				height = _height;
				y = _ypostop;
				_scaleIndexY = _scaleIndexYTop;
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					if (_xpos < 0 || _xpos >= _vm->_realWidth)
						return;
					maskbit = revBitMask[_xpos & 7];
					_backbuff_ptr += _scaleIndexXStep;
				}
				dst = _backbuff_ptr;
				mask = _mask_ptr + (_xpos >> 3);
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc2() {
	byte *mask, *src, *dst;
	byte maskbit, len, height, pcolor, width;
	int color, t;
	uint y;

	mask = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	height = _height2;
	width = _width2;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (cost_scaleTable[_scaleIndexY++] < _scaleY) {
				if (color && y < _outheight && !(*mask & maskbit)) {
					pcolor = _palette[color];
					if (pcolor == 13)
						pcolor = _shadow_table[*dst];
					*dst = pcolor;
				}
				dst += _vm->_realWidth;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--width)
					return;
				height = _height;
				y = _ypostop;
				_scaleIndexY = _scaleIndexYTop;
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					if (_xpos < 0 || _xpos >= _vm->_realWidth)
						return;
					maskbit = revBitMask[_xpos & 7];
					_backbuff_ptr += _scaleIndexXStep;
				}
				dst = _backbuff_ptr;
				mask = _mask_ptr + (_xpos >> 3);
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc1() {
	byte *mask, *src, *dst, *dstorg;
	byte maskbit, len, height, pcolor, width;
	int color, t;
	uint y;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;
	src = _srcptr;
	dstorg = dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	height = _height2;
	width = _width2;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (cost_scaleTable[_scaleIndexY++] < _scaleY) {
				if (color && y < _outheight) {
					pcolor = _palette[color];
					if (pcolor == 13)
						pcolor = _shadow_table[*dst];
					*dst = pcolor;
				}
				dst += _vm->_realWidth;
				y++;
			}
			if (!--height) {
				if (!--width)
					return;
				height = _height;
				y = _ypostop;
				_scaleIndexY = _scaleIndexYTop;
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					if (_xpos < 0 || _xpos >= _vm->_realWidth)
						return;
					_backbuff_ptr += _scaleIndexXStep;
				}
				dst = _backbuff_ptr;
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc6_ami() {
	byte len;
	byte *src, *dst;
	byte width, height, pcolor;
	int color;
	int step = _scaleIndexXStep;
	int x;
	int y;
	int scrheight;

	x = _xpos;
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	scrheight = _outheight;
	width = _width;
	height = _height2;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (color && x >= 0 && x < _vm->_realWidth) {
				pcolor = _palette[color];
/*				  if (pcolor == 13) {
					pcolor = _shadow_table[*dst];
				}*/
				*dst = pcolor;
			}

			dst += step;
			x += step;
			if (!--width) {
				if (!--height)
					return;
				width = _width;
				dst += _vm->_realWidth - step * _width;
				x = _xpos;
				y++;
				if (y >= scrheight)
					return;
			}
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc5_ami() {
	byte *mask, *src, *dst;
	byte maskbit, len, width, height, pcolor;
	uint y, scrheight;
	int color, x;
	int step = _scaleIndexXStep;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	x = _xpos;
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	scrheight = _outheight;
	width = _width;
	height = _height2;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (color && x >=0 && x < _vm->_realWidth && !(*mask & maskbit)) {
				pcolor = _palette[color];
/*				  if (pcolor == 13)
					pcolor = _shadow_table[*dst];*/
				*dst = pcolor;
			}
			dst += step;
			x += step;
			if (step != 1) {
				maskbit <<= 1;
				if (!maskbit)
				{
					maskbit = 1;
					mask--;
				}
			} else {
				maskbit >>= 1;
				if (!maskbit)
				{
					maskbit = 0x80;
					mask++;
				}
			}
			if (!--width) {
				if (!--height)
					return;
				width = _width;
				x = _xpos;
				y++;
				dst += _vm->_realWidth - step * _width;
				_mask_ptr += _numStrips;
				mask = _mask_ptr;
				maskbit = revBitMask[_xpos & 7];
				if (y >= scrheight)
					return;
			}
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc4_ami() {
	byte *mask, *src, *dst;
	byte maskbit, len, width, height, pcolor;
	uint y, scrheight;
	int color, x;
	int step = _scaleIndexXStep;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	x = _xpos;
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	scrheight = _outheight;
	height = _height2;
	width = _width;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (color && x >= 0 && x < _vm->_realWidth && !((*mask | mask[_imgbufoffs]) & maskbit)) {
				pcolor = _palette[color];
/*				  if (pcolor == 13)
					pcolor = _shadow_table[*dst];*/
				*dst = pcolor;
			}
			dst += step;
			x += step;
			if (step != 1) {
				maskbit <<= 1;
				if (!maskbit) {
					maskbit = 1;
					mask--;
				}
			} else {
				maskbit >>= 1;
				if (!maskbit) {
					maskbit = 0x80;
					mask++;
				}
			}
			if (!--width) {
				if (!--height)
					return;
				width = _width;
				y++;
				x = _xpos;
				dst += _vm->_realWidth - step * _width;
				_mask_ptr += _numStrips;
				mask = _mask_ptr;
				maskbit = revBitMask[_xpos & 7];
				if (y >= scrheight)
					return;
			}
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc3_ami() {
	byte *mask, *src, *dst;
	byte maskbit, len, height, pcolor, width;
	int color, t;
	uint y;
	int oldXpos, oldScaleIndexX;

	mask = _mask_ptr_dest;
	dst = _backbuff_ptr;
	height = _height2;
	width = _width;
	src = _srcptr;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;
	oldXpos = _xpos;
	oldScaleIndexX = _scaleIndexX;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;
		do {
			if (cost_scaleTable[_scaleIndexY] < _scaleY) {
				if (color && _xpos >= 0 && _xpos < _vm->_realWidth && !((*mask | mask[_imgbufoffs]) & maskbit)) {
					pcolor = _palette[color];
/*					  if (pcolor == 13)
						pcolor = _shadow_table[*dst];*/
					*dst = pcolor;
				}

				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					dst += _scaleIndexXStep;
					maskbit = revBitMask[_xpos & 7];
				}
				mask = _mask_ptr + (_xpos >> 3);
			}
			if (!--width) {
				if (!--height)
					return;

				if (y >= _outheight)
					return;

				if (_xpos != oldXpos) {
					dst += _vm->_realWidth - (_xpos - oldXpos);
					_mask_ptr += _numStrips;
					mask = _mask_ptr;
					y++;
				}
				width = _width;
				_xpos = oldXpos;
				_scaleIndexX = oldScaleIndexX;
				_scaleIndexY++;
			}
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc2_ami() {
	byte *mask, *src, *dst;
	byte maskbit, len, height, pcolor, width;
	int color, t;
	uint y;
	int oldXpos, oldScaleIndexX;

	mask = _mask_ptr_dest;
	dst = _backbuff_ptr;
	height = _height2;
	width = _width;
	src = _srcptr;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;

	oldXpos = _xpos;
	oldScaleIndexX = _scaleIndexX;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;
		do {
			if (cost_scaleTable[_scaleIndexY] < _scaleY) {
				if (color && _xpos >= 0 && _xpos < _vm->_realWidth && !(*mask & maskbit)) {
					pcolor = _palette[color];
/*					  if (pcolor == 13)
						pcolor = _shadow_table[*dst];*/
					*dst = pcolor;
				}
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					dst += _scaleIndexXStep;
					maskbit = revBitMask[_xpos & 7];
					_backbuff_ptr += _scaleIndexXStep;
				}
				mask = _mask_ptr + (_xpos >> 3);
			}
			if (!--width) {
				if (!--height)
					return;

				if (y >= _outheight)
					return;

				if (_xpos != oldXpos) {
					dst += _vm->_realWidth - (_xpos - oldXpos);
					_mask_ptr += _numStrips;
					mask = _mask_ptr;
					y++;
				}
				width = _width;
				_xpos = oldXpos;
				_scaleIndexX = oldScaleIndexX;
				_scaleIndexY++;
			}
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc1_ami() {
	byte *mask, *src, *dst;
	byte maskbit, len, height, pcolor, width;
	uint y;
	int color;
	int t, x;
	int oldXpos, oldScaleIndexX;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	x = _xpos;
	y = _ypos;

	dst = _backbuff_ptr;
	height = _height2;
	width = _width;
	src = _srcptr;

	oldXpos = _xpos;
	oldScaleIndexX = _scaleIndexX;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {
			if (cost_scaleTable[_scaleIndexY] < _scaleY) {
				if (color && _xpos >= 0 && _xpos < _vm->_realWidth) {
					pcolor = _palette[color];
/*					  if (pcolor == 13)
						pcolor = _shadow_table[*dst];*/
					*dst = pcolor;
				}

				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					dst += _scaleIndexXStep;
				}
			}
			if (!--width) {
				if (!--height)
					return;

				if (y >= _outheight)
					return;

				if (_xpos != oldXpos) {
					dst += _vm->_realWidth - (_xpos - oldXpos);
					y++;
				}
				width = _width;
				_xpos = oldXpos;
				_scaleIndexX = oldScaleIndexX;
				_scaleIndexY++;
			}
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc_special(byte mask2) {
	byte *mask, *src, *dst, *dstorg;
	byte maskbit, len, height, pcolor, width;
	uint y;
	int color;
	int t;

	byte shadow1;
	byte shadow2;
	byte shadow3;
	byte shadow4;
	byte shadow5;

	shadow1 = _shadow_mode & 0x80;
	shadow2 = _shadow_mode & 0x40;
	shadow3 = _shadow_mode & 0x20;
	shadow4 = _shadow_mode & 0x10;
	shadow5 = _shadow_mode & 0x0F;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos & 7];
	y = _ypos;

	mask = _mask_ptr_dest;
	dstorg = dst = _backbuff_ptr;
	height = _height2;
	width = _width2;
	len = _replen;
	color = _repcolor;
	src = _srcptr;

	if (_mirror == 0)
		shadow5 = -shadow5;

	maskbit = revBitMask[_xpos & 7];

	dst = _backbuff_ptr;

	if (mask2 != 0 && mask2 < 3)
		_imgbufoffs = 0;

	if (_docontinue)
		goto StartPos;

	do {
		len = *src++;
		color = len >> _shrval;
		len &= _maskval;
		if (!len)
			len = *src++;

		do {												// ok
			if (cost_scaleTable[_scaleIndexY++] < _scaleY) {
				if (color && y < _outheight) {
					if (!mask2 || (mask2 && !((*mask | mask[_imgbufoffs]) & maskbit))) {
						if (shadow3 == 0) {
							pcolor = _palette[color];
							if (pcolor != 13)
								goto proc_special_end;

						}
						if (shadow2 != 0) {
							warning("proc_special: shadow2 unimplemented");
							pcolor = 0;
						} else							// we don't need all the random stuff, just the background copy
						{
							pcolor = _vm->_proc_special_palette[*dst];
						}
					proc_special_end:;
						*dst = pcolor;
					}
				}
				dst += _vm->_realWidth;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--width)
					return;
				height = _height;
				y = _ypostop;
				_scaleIndexY = _scaleIndexYTop;
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					if (_xpos < 0 || _xpos >= _vm->_realWidth)
						return;
					maskbit = revBitMask[_xpos & 7];
					_backbuff_ptr += _scaleIndexXStep;
				}
				dst = _backbuff_ptr;
				mask = _mask_ptr + (_xpos >> 3);
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void LoadedCostume::loadCostume(int id) {
	_ptr = _vm->getResourceAddress(rtCostume, id);

	if (_vm->_features & GF_AFTER_V6)
		_ptr += 8;
	else if (_vm->_features & GF_OLD_BUNDLE)
		_ptr += -2;
	else if (_vm->_features & GF_SMALL_HEADER)
		_ptr += 0;
	else
		_ptr += 2;

	_baseptr = _ptr;

	switch (_ptr[7] & 0x7F) {
	case 0x58:
		_numColors = 16;
		break;
	case 0x59:
		_numColors = 32;
		break;
	case 0x60:										/* New since version 6 */
		_numColors = 16;
		break;
	case 0x61:										/* New since version 6 */
		_numColors = 32;
		break;
	default:
		error("Costume %d is invalid", id);
	}
	
	// In GF_OLD_BUNDLE games, there is no actual palette, just a single color byte. 
	// Don't forget, these games were designed around a fixed 16 color HW palette :-)
	// In addition, all offsets are shifted by 2; we accomodate that via a seperate
	// _baseptr value (instead of adding tons of if's throughout the code).
	if (_vm->_features & GF_OLD_BUNDLE) {
		_numColors = 1;
		_baseptr += 2;
	}
	_dataptr = _baseptr + READ_LE_UINT16(_ptr + _numColors + 8);
}

byte CostumeRenderer::drawLimb(const CostumeData &cost, int limb) {
	int i;
	int code;

	if (cost.curpos[limb] == 0xFFFF || cost.stopped & (1 << limb))
		return 0;

	i = cost.curpos[limb] & 0x7FFF;

	_frameptr = _loaded._baseptr + READ_LE_UINT16(_loaded._ptr + _loaded._numColors + limb * 2 + 10);

	code = _loaded._dataptr[i] & 0x7F;
	
	_srcptr = _loaded._baseptr + READ_LE_UINT16(_frameptr + code * 2);

	if (code != 0x7B) {
		if (!(_vm->_features & GF_OLD256) || code < 0x79)
			return mainRoutine(limb, code);
	}

	return 0;

}

int Scumm::cost_frameToAnim(Actor *a, int frame) {
	return newDirToOldDir(a->facing) + frame * 4;
}

void Scumm::cost_decodeData(Actor *a, int frame, uint usemask) {
	byte *r;
	uint mask, j;
	int i;
	byte extra, cmd;
	byte *dataptr;
	int anim;
	LoadedCostume lc(this);

	lc.loadCostume(a->costume);

	anim = cost_frameToAnim(a, frame);

	if (anim > lc._ptr[6]) {
		return;
	}

	r = lc._baseptr + READ_LE_UINT16(lc._ptr + anim * 2 + lc._numColors + 42);

	if (r == lc._baseptr) {
		return;
	}

	dataptr = lc._dataptr;
	mask = READ_LE_UINT16(r);
	r += 2;
	i = 0;
	do {
		if (mask & 0x8000) {
			if (_features & GF_AFTER_V3) {
				j = *r++;

				if (j == 0xFF)
					j = 0xFFFF;
			} else {
				j = READ_LE_UINT16(r);
				r += 2;
			}
			if (usemask & 0x8000) {
				if (j == 0xFFFF) {
					a->cost.curpos[i] = 0xFFFF;
					a->cost.start[i] = 0;
					a->cost.frame[i] = frame;
				} else {
					extra = *r++;
					cmd = dataptr[j];
					if (cmd == 0x7A) {
						a->cost.stopped &= ~(1 << i);
					} else if (cmd == 0x79) {
						a->cost.stopped |= (1 << i);
					} else {
						a->cost.curpos[i] = a->cost.start[i] = j;
						a->cost.end[i] = j + (extra & 0x7F);
						if (extra & 0x80)
							a->cost.curpos[i] |= 0x8000;
						a->cost.frame[i] = frame;
					}
				}
			} else {
				if (j != 0xFFFF)
					r++;
			}
		}
		i++;
		usemask <<= 1;
		mask <<= 1;
	} while ((uint16)mask);
}

void CostumeRenderer::setPalette(byte *palette) {
	int i;
	byte color;

	if (_vm->_features & GF_OLD_BUNDLE) {
		if ((_vm->_vars[_vm->VAR_CURRENT_LIGHTS] & LIGHTMODE_actor_color)) {
			memcpy(_palette, palette, 16);
		} else {
			memset(_palette, 8, 16);
			_palette[12] = 0;
		}
		// FIXME: is this actually anything more than a guess (i.e. verified by disassmbly) ?
		// A more "logical" thing would be if any 255 values in _palette are replaced
		// by this value, at least that would closely parallel the actor palette usage in
		// newer games (see below).
		_palette[_loaded._ptr[8]] = _palette[0];
	} else {
		for (i = 0; i < _loaded._numColors; i++) {
			if ((_vm->_vars[_vm->VAR_CURRENT_LIGHTS] & LIGHTMODE_actor_color) || (_vm->_features & GF_AFTER_V6)) {
				color = palette[i];
				if (color == 255)
					color = _loaded._ptr[8 + i];
			} else {
				color = (i == 12) ? 0 : 8;
			}
			_palette[i] = color;
		}
	}
}

void CostumeRenderer::setFacing(Actor *a) {
	_mirror = newDirToOldDir(a->facing) != 0 || (_loaded._ptr[7] & 0x80);
}

void CostumeRenderer::setCostume(int costume) {
	_loaded.loadCostume(costume);
}

byte LoadedCostume::increaseAnims(Actor *a) {
	int i;
	byte r = 0;

	for (i = 0; i != 16; i++) {
		if (a->cost.curpos[i] != 0xFFFF)
			r += increaseAnim(a, i);
	}
	return r;
}

byte LoadedCostume::increaseAnim(Actor *a, int slot) {
	int highflag;
	int i, end;
	byte code, nc;

	if (a->cost.curpos[slot] == 0xFFFF)
		return 0;

	highflag = a->cost.curpos[slot] & 0x8000;
	i = a->cost.curpos[slot] & 0x7FFF;
	end = a->cost.end[slot];
	code = _dataptr[i] & 0x7F;

	do {
		if (!highflag) {
			if (i++ >= end)
				i = a->cost.start[slot];
		} else {
			if (i != end)
				i++;
		}
		nc = _dataptr[i];

		if (nc == 0x7C) {
			a->cost.animCounter1++;
			if (a->cost.start[slot] != end)
				continue;
		} else {
			if (_vm->_features & GF_AFTER_V6) {
				if (nc >= 0x71 && nc <= 0x78) {
					_vm->_sound->addSoundToQueue2(a->sound[nc - 0x71]);
					if (a->cost.start[slot] != end)
						continue;
				}
			} else {
				if (nc == 0x78) {
					a->cost.animCounter2++;
					if (a->cost.start[slot] != end)
						continue;
				}
			}
		}

		a->cost.curpos[slot] = i | highflag;
		return (_dataptr[i] & 0x7F) != code;
	} while (1);
}

bool Scumm::isCostumeInUse(int cost) {
	int i;
	Actor *a;

	if (_roomResource != 0)
		for (i = 1; i < NUM_ACTORS; i++) {
			a = derefActor(i);
			if (a->isInCurrentRoom() && a->costume == cost)
				return true;
		}

	return false;
}
