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

const byte cost_scaleTable[256] = {
	0xFF, 0xFD, 0x7D, 0xBD, 0x3D, 0xDD, 0x5D, 0x9D,
	0x1D, 0xED, 0x6D, 0xAD, 0x2D, 0xCD, 0x4D, 0x8D,
	0x0D, 0xF5, 0x75, 0xB5, 0x35, 0xD5, 0x55, 0x95,
	0x15, 0xE5, 0x65, 0xA5, 0x25, 0xC5, 0x45, 0x85,
	0x05, 0xF9, 0x79, 0xB9, 0x39, 0xD9, 0x59, 0x99,
	0x19, 0xE9, 0x69, 0xA9, 0x29, 0xC9, 0x49, 0x89,
	0x09, 0xF1, 0x71, 0xB1, 0x31, 0xD1, 0x51, 0x91,
	0x11, 0xE1, 0x61, 0xA1, 0x21, 0xC1, 0x41, 0x81,
	0x01, 0xFB, 0x7B, 0xBB, 0x3B, 0xDB, 0x5B, 0x9B,
	0x1B, 0xEB, 0x6B, 0xAB, 0x2B, 0xCB, 0x4B, 0x8B,
	0x0B, 0xF3, 0x73, 0xB3, 0x33, 0xD3, 0x53, 0x93,
	0x13, 0xE3, 0x63, 0xA3, 0x23, 0xC3, 0x43, 0x83,
	0x03, 0xF7, 0x77, 0xB7, 0x37, 0xD7, 0x57, 0x97,
	0x17, 0xE7, 0x67, 0xA7, 0x27, 0xC7, 0x47, 0x87,
	0x07, 0xEF, 0x6F, 0xAF, 0x2F, 0xCF, 0x4F, 0x8F,
	0x0F, 0xDF, 0x5F, 0x9F, 0x1F, 0xBF, 0x3F, 0x7F,
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE
};

byte CostumeRenderer::mainRoutine(int xmoveCur, int ymoveCur) {
	int i, skip;
	byte drawFlag = 1;
	uint scal;
	bool use_scaling;
	byte startScaleIndexX;
	int ex1, ex2;
	int y_top, y_bottom;
	int x_left, x_right;
	int step;
	
	CHECK_HEAP

	v1.scaletable = cost_scaleTable;

	if (_loaded._numColors == 32) {
		v1.mask = 7;
		v1.shr = 3;
	} else {
		v1.mask = 0xF;
		v1.shr = 4;
	}

	switch (_loaded._format) {
	case 0x60:
	case 0x61:
		// This format is used e.g. in the Sam&Max intro
		ex1 = _srcptr[0];
		ex2 = _srcptr[1];
		_srcptr += 2;
		if (ex1 != 0xFF || ex2 != 0xFF) {
			ex1 = READ_LE_UINT16(_loaded._frameOffsets + ex1 * 2);
			_srcptr = _loaded._baseptr + READ_LE_UINT16(_loaded._baseptr + ex1 + ex2 * 2) + 14;
		}
	}

	v1.x = _actorX;
	v1.y = _actorY;

	use_scaling = (_scaleX != 0xFF) || (_scaleY != 0xFF);

	skip = 0;

	if (use_scaling) {
		v1.scaleXstep = -1;
		if (xmoveCur < 0) {
			xmoveCur = -xmoveCur;
			v1.scaleXstep = 1;
		}

		if (_mirror) {
			startScaleIndexX = _scaleIndexX = 128 - xmoveCur;
			for (i = 0; i < xmoveCur; i++) {
				if (cost_scaleTable[_scaleIndexX++] < _scaleX)
					v1.x -= v1.scaleXstep;
			}
			x_right = x_left = v1.x;
			_scaleIndexX = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (x_right < 0) {
					skip++;
					startScaleIndexX = _scaleIndexX;
				}
				scal = cost_scaleTable[_scaleIndexX++];
				if (scal < _scaleX)
					x_right++;
			}
		} else {
			startScaleIndexX = _scaleIndexX = xmoveCur + 128;
			for (i = 0; i < xmoveCur; i++) {
				scal = cost_scaleTable[_scaleIndexX--];
				if (scal < _scaleX)
					v1.x += v1.scaleXstep;
			}
			x_right = x_left = v1.x;
			_scaleIndexX = startScaleIndexX;
			for (i = 0; i < _width; i++) {
				if (x_left > (_vm->_screenWidth - 1)) {
					skip++;
					startScaleIndexX = _scaleIndexX;
				}
				scal = cost_scaleTable[_scaleIndexX--];
				if (scal < _scaleX)
					x_left--;
			}
		}
		_scaleIndexX = startScaleIndexX;
		if (skip)
			skip--;

		step = -1;
		if (ymoveCur < 0) {
			ymoveCur = -ymoveCur;
			step = 1;
		}
		_scaleIndexY = 128 - ymoveCur;
		for (i = 0; i < ymoveCur; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY)
				v1.y -= step;
		}
		y_top = y_bottom = v1.y;
		_scaleIndexY = 128 - ymoveCur;
		for (i = 0; i < _height; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY)
				y_bottom++;
		}
		_scaleIndexY = 128 - ymoveCur;
	} else {
		if (!_mirror)
			xmoveCur = -xmoveCur;
		v1.x += xmoveCur;
		v1.y += ymoveCur;
		if (_mirror) {
			x_left = v1.x;
			x_right = v1.x + _width;
		} else {
			x_left = v1.x - _width;
			x_right = v1.x;
		}
		y_top = v1.y;
		y_bottom = y_top + _height;
	}

	v1.skip_width = _width;
	v1.scaleXstep = _mirror ? 1 : -1;

	_vm->updateDirtyRect(0, x_left, x_right + 1, y_top, y_bottom, _dirty_id);

	if (y_top >= (int)_outheight || y_bottom <= 0)
		return 0;

	if (x_left >= _vm->_screenWidth || x_right <= 0)
		return 0;

	v1.replen = 0;

	if (_mirror) {
		if (!use_scaling)
			skip = -v1.x;
		if (skip > 0) {
			v1.skip_width -= skip;

			if (_vm->_version == 1)
				c64_ignorePakCols(skip);
			else
				codec1_ignorePakCols(skip);
			v1.x = 0;
		} else {
			skip = x_right - _vm->_screenWidth;
			if (skip <= 0) {
				drawFlag = 2;
			} else {
				v1.skip_width -= skip;
			}
		}
	} else {
		if (!use_scaling)
			skip = x_right - _vm->_screenWidth;
		if (skip > 0) {
			v1.skip_width -= skip;

			if (_vm->_version == 1)
				c64_ignorePakCols(skip);
			else
				codec1_ignorePakCols(skip);
			v1.x = _vm->_screenWidth - 1;
		} else {
			skip = -1 - x_left;
			if (skip <= 0)
				drawFlag = 2;
			else
				v1.skip_width -= skip;
		}
	}

	if (v1.skip_width <= 0)
		return 0;

	if (x_left < 0)
		x_left = 0;

	if ((uint) y_top > _outheight)
		y_top = 0;

	if ((uint) y_bottom > _outheight)
		y_bottom = _outheight;

	if (_draw_top > y_top)
		_draw_top = y_top;
	if (_draw_bottom < y_bottom)
		_draw_bottom = y_bottom;

	if (_height + y_top >= 256) {
		CHECK_HEAP
		return 2;
	}

	v1.destptr = _outptr + v1.y * _outwidth + v1.x;

	v1.mask_ptr = _vm->getMaskBuffer(0, v1.y, 0);
	v1.imgbufoffs = _vm->gdi._imgBufOffs[_zbuf];

	CHECK_HEAP

	if (_vm->_version == 1)
		procC64();
	else if (_vm->_features & GF_AMIGA && (_vm->_gameId == GID_INDY4 || _vm->_gameId == GID_MONKEY2))
		proc3_ami();
	else
		proc3();

	CHECK_HEAP
	return drawFlag;
}

// FIXME: Call this something sensible, make sure it works
void CostumeRenderer::c64_ignorePakCols(int num) {

#if 1
	// Fingolfin seez: this code is equivalent to codec1_ignorePakCols.
	// And I wonder if that can be true, considering the different
	// compressor used in procC64 vs. proc3 ?
	codec1_ignorePakCols(num);
#else
	int n = _height * num;
	do {
		v1.repcolor = *_srcptr >> 4;
		v1.replen = *_srcptr++ & 0x0F;

		if (v1.replen == 0)
			v1.replen = *_srcptr++;

		do {
			if (!--n) return;
		} while (--v1.replen);
	} while (1);
#endif
}

void CostumeRenderer::procC64() {
	const byte *src;
	byte *dst;
	byte color;
	byte len = 0;

	int y = 0;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;

	for (int x = 0; x < (_width >> 3); x++) {
		color = *src++;
		if (color & 0x80) {
			len = color & 0x7f;
			color = *src++;
			for (int z = 0; z < len; z++) {
				dst[0] = dst[1] = _vm->gdi._C64Colors[(color >> 6) & 3];
				dst[2] = dst[3] = _vm->gdi._C64Colors[(color >> 4) & 3];
				dst[4] = dst[5] = _vm->gdi._C64Colors[(color >> 2) & 3];
				dst[6] = dst[7] = _vm->gdi._C64Colors[(color >> 0) & 3];
				dst += _outwidth;
				y++;
				if (y >= (_height >> 3)) {
					y = 0;
					dst = v1.destptr + (x << 3);
				}
			}
		} else {
			len = color;
			for (int z = 0; z < len; z++) {
				color = *src++;
				dst[0] = dst[1] = _vm->gdi._C64Colors[(color >> 6) & 3];
				dst[2] = dst[3] = _vm->gdi._C64Colors[(color >> 4) & 3];
				dst[4] = dst[5] = _vm->gdi._C64Colors[(color >> 2) & 3];
				dst[6] = dst[7] = _vm->gdi._C64Colors[(color >> 0) & 3];
				dst += _outwidth;
				y++;
				if (y >= (_height >> 3)) {
					y = 0;
					dst = v1.destptr + (x << 3);
				}
			}
		}
	}
}

void CostumeRenderer::proc3() {
	const byte *mask, *src;
	byte *dst;
	byte len, maskbit;
	uint y, color, height, pcolor;
	const byte *scaleytab;
	bool masked;

	y = v1.y;
	src = _srcptr;
	dst = v1.destptr;
	len = v1.replen;
	color = v1.repcolor;
	height = _height;

	scaleytab = &v1.scaletable[_scaleIndexY];
	maskbit = revBitMask[v1.x & 7];
	mask = v1.mask_ptr + (v1.x >> 3);

	if (len)
		goto StartPos;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;

		do {
			if (_scaleY == 255 || *scaleytab++ < _scaleY) {
				masked = (y < _outheight) && v1.mask_ptr && ((mask[0] | mask[v1.imgbufoffs]) & maskbit);
				
				if (color && y < _outheight && !masked) {
					// FIXME: Fully implement _shadow_mode.
					// For now, it's enough for Sam & Max
					// transparency.
					if (_shadow_mode & 0x20) {
						pcolor = _vm->_proc_special_palette[*dst];
					} else {
						pcolor = _palette[color];
						if (pcolor == 13 && _shadow_table)
							pcolor = _shadow_table[*dst];
					}
					*dst = pcolor;
				}
				dst += _outwidth;
				mask += _numStrips;
				y++;
			}
			if (!--height) {
				if (!--v1.skip_width)
					return;
				height = _height;
				y = v1.y;

				scaleytab = &v1.scaletable[_scaleIndexY];

				if (_scaleX == 255 || v1.scaletable[_scaleIndexX] < _scaleX) {
					v1.x += v1.scaleXstep;
					if (v1.x < 0 || v1.x >= _vm->_screenWidth)
						return;
					maskbit = revBitMask[v1.x & 7];
					v1.destptr += v1.scaleXstep;
				}
				_scaleIndexX += v1.scaleXstep;
				dst = v1.destptr;
				mask = v1.mask_ptr + (v1.x >> 3);
			}
		StartPos:;
		} while (--len);
	} while (1);
}

void CostumeRenderer::proc3_ami() {
	const byte *mask, *src;
	byte *dst;
	byte maskbit, len, height, width;
	int color;
	uint y;
	bool masked;
	int oldXpos, oldScaleIndexX;

	mask = v1.mask_ptr + (v1.x >> 3);
	dst = v1.destptr;
	height = _height;
	width = _width;
	src = _srcptr;
	maskbit = revBitMask[v1.x & 7];
	y = v1.y;
	oldXpos = v1.x;
	oldScaleIndexX = _scaleIndexX;

	do {
		len = *src++;
		color = len >> v1.shr;
		len &= v1.mask;
		if (!len)
			len = *src++;
		do {
			if (_scaleY == 255 || cost_scaleTable[_scaleIndexY] < _scaleY) {
				masked = (y < _outheight) && v1.mask_ptr && ((mask[0] | mask[v1.imgbufoffs]) & maskbit);
				
				if (color && v1.x >= 0 && v1.x < _vm->_screenWidth && !masked) {
					*dst = _palette[color];
				}

				if (_scaleX == 255 || cost_scaleTable[_scaleIndexX] < _scaleX) {
					v1.x += v1.scaleXstep;
					dst += v1.scaleXstep;
					maskbit = revBitMask[v1.x & 7];
				}
				_scaleIndexX += v1.scaleXstep;
				mask = v1.mask_ptr + (v1.x >> 3);
			}
			if (!--width) {
				if (!--height)
					return;

				if (y >= _outheight)
					return;

				if (v1.x != oldXpos) {
					dst += _vm->_screenWidth - (v1.x - oldXpos);
					v1.mask_ptr += _numStrips;
					mask = v1.mask_ptr;
					y++;
				}
				width = _width;
				v1.x = oldXpos;
				_scaleIndexX = oldScaleIndexX;
				_scaleIndexY++;
			}
		} while (--len);
	} while (1);
}

void LoadedCostume::loadCostume(int id) {
	_id = id;
	_ptr = _vm->getResourceAddress(rtCostume, id);

	if (_vm->_version >= 6)
		_ptr += 8;
	else if (_vm->_features & GF_OLD_BUNDLE)
		_ptr += -2;
	else if (_vm->_features & GF_SMALL_HEADER)
		_ptr += 0;
	else
		_ptr += 2;

	_baseptr = _ptr;

	_numAnim = _ptr[6];
	_format = _ptr[7] & 0x7F;
	_mirror = (_ptr[7] & 0x80) != 0;
	_palette = _ptr + 8;
	switch (_format) {
	case 0x57:				// Only used in V1 games
		_numColors = 0;
		break;
	case 0x58:
		_numColors = 16;
		break;
	case 0x59:
		_numColors = 32;
		break;
	case 0x60:				// New since version 6
		_numColors = 16;
		break;
	case 0x61:				// New since version 6
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
		_numColors = (_vm->_version == 1) ? 0 : 1;
		_baseptr += 2;
	}
	_ptr += 8 + _numColors;
	_frameOffsets = _ptr + 2;
	_dataptr = _baseptr + READ_LE_UINT16(_ptr);
}

byte CostumeRenderer::drawLimb(const CostumeData &cost, int limb) {
	int i;
	int code;
	const byte *frameptr;

	// If the specified limb is stopped or not existing, do nothing.
	if (cost.curpos[limb] == 0xFFFF || cost.stopped & (1 << limb))
		return 0;

	// Determine the position the limb is at
	i = cost.curpos[limb] & 0x7FFF;
	
	// Get the frame pointer for that limb
	frameptr = _loaded._baseptr + READ_LE_UINT16(_loaded._frameOffsets + limb * 2);
	
	// Determine the offset to the costume data for the limb at position i
	code = _loaded._dataptr[i] & 0x7F;

	// Code 0x7B indicates a limb for which there is nothing to draw
	if (code != 0x7B) {
		_srcptr = _loaded._baseptr + READ_LE_UINT16(frameptr + code * 2);
		if (!(_vm->_features & GF_OLD256) || code < 0x79) {
			const CostumeInfo *costumeInfo;
			int xmoveCur, ymoveCur;

			if (_vm->_version == 1) {
				_width = _srcptr[0];
				_height = _srcptr[1];
				xmoveCur = _xmove + _srcptr[2];
				ymoveCur = _ymove + _srcptr[3];
				_xmove += _srcptr[4];
				_ymove -= _srcptr[5];
				_srcptr += 6;
			} else {
				// FIXME: those are here just in case... you never now...
				assert(_srcptr[1] == 0);
				assert(_srcptr[3] == 0);
		
				costumeInfo = (const CostumeInfo *)_srcptr;
				_width = READ_LE_UINT16(&costumeInfo->width);
				_height = READ_LE_UINT16(&costumeInfo->height);
				xmoveCur = _xmove + (int16)READ_LE_UINT16(&costumeInfo->rel_x);
				ymoveCur = _ymove + (int16)READ_LE_UINT16(&costumeInfo->rel_y);
				_xmove += (int16)READ_LE_UINT16(&costumeInfo->move_x);
				_ymove -= (int16)READ_LE_UINT16(&costumeInfo->move_y);
				_srcptr += 12;
			}
//printf("costume %d, limb %d: _width %d, _height %d, xmoveCur %d, ymoveCur %d, _xmove %d, _ymove, %d\n", _loaded._id, limb, _width, _height, xmoveCur, ymoveCur, _xmove, _ymove);
			return mainRoutine(xmoveCur, ymoveCur);
		}
	}

	return 0;

}

int Scumm::cost_frameToAnim(Actor *a, int frame) {
	return newDirToOldDir(a->facing) + frame * 4;
}

void Scumm::cost_decodeData(Actor *a, int frame, uint usemask) {
	const byte *r;
	uint mask, j;
	int i;
	byte extra, cmd;
	const byte *dataptr;
	int anim;
	LoadedCostume lc(this);

	lc.loadCostume(a->costume);

	anim = cost_frameToAnim(a, frame);

	if (anim > lc._numAnim) {
		return;
	}

	if (_version == 1) {
		// FIXME
//printf("Offset table:\n");
//hexdump(lc._ptr + anim * 2 + 34 - 0x10, 0x20);
		r = lc._baseptr + READ_LE_UINT16(lc._ptr + 20 + anim * 2);
	} else {
		r = lc._baseptr + READ_LE_UINT16(lc._ptr + 34 + anim * 2);
	}
//printf("actor %d, costum %d, frame %d, anim %d:\n", a->number, lc._id, frame, anim);
//hexdump(r, 0x20);

	if (r == lc._baseptr) {
		return;
	}

	dataptr = lc._dataptr;
	// FIXME: Maybe V1 only ready one byte here? At least it seems by comparing the
	// V1 and V2 data that there is a 1-byte len difference.
/*	if (_version == 1) {
		mask = *r++;
	} else {
*/
		mask = READ_LE_UINT16(r);
		r += 2;
//	}
	i = 0;
	do {
		if (mask & 0x8000) {
			if (_version <= 3) {
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
		if ((_vm->VAR(_vm->VAR_CURRENT_LIGHTS) & LIGHTMODE_actor_color)) {
			memcpy(_palette, palette, 16);
		} else {
			memset(_palette, 8, 16);
			_palette[12] = 0;
		}
		if (_vm->_version == 1)
			return;
		_palette[_loaded._palette[0]] = _palette[0];
	} else {
		if ((_vm->_features & GF_NEW_OPCODES) || (_vm->VAR(_vm->VAR_CURRENT_LIGHTS) & LIGHTMODE_actor_color)) {
			for (i = 0; i < _loaded._numColors; i++) {
				color = palette[i];
				if (color == 255)
					color = _loaded._palette[i];
				_palette[i] = color;
			}
		} else {
			memset(_palette, 8, _loaded._numColors);
			_palette[12] = 0;
		}
	}
}

void CostumeRenderer::setFacing(Actor *a) {
	_mirror = newDirToOldDir(a->facing) != 0 || (_loaded._mirror);
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
	
	if (_vm->_version <= 3) {
		if (_dataptr[i] & 0x80)
			a->cost.soundCounter++;
	}
	
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
			a->cost.animCounter++;
			if (a->cost.start[slot] != end)
				continue;
		} else {
			if (_vm->_version >= 6) {
				if (nc >= 0x71 && nc <= 0x78) {
					_vm->_sound->addSoundToQueue2(a->sound[nc - 0x71]);
					if (a->cost.start[slot] != end)
						continue;
				}
			} else {
				if (nc == 0x78) {
					a->cost.soundCounter++;
					if (a->cost.start[slot] != end)
						continue;
				}
			}
		}

		a->cost.curpos[slot] = i | highflag;
		return (_dataptr[i] & 0x7F) != code;
	} while (1);
}

bool Scumm::isCostumeInUse(int cost) const {
	int i;
	Actor *a;

	if (_roomResource != 0)
		for (i = 1; i < _numActors; i++) {
			a = derefActor(i);
			if (a->isInCurrentRoom() && a->costume == cost)
				return true;
		}

	return false;
}
