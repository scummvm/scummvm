/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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

const byte revBitMask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

void CostumeRenderer::ignorePakCols(int a) {
	int n;
	n = _height;
	if (a>1) n *= a;
	do {
		_repcolor = *_srcptr++;
		_replen = _repcolor&_maskval;
		if (_replen==0) {
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

byte CostumeRenderer::mainRoutine(Actor *a, int slot, int frame) {
	int xmove, ymove, i,b,s;
	uint scal;
	byte scaling;
	byte charsetmask, masking;
	byte unk19;
	int ex1,ex2;

	CHECK_HEAP

	_maskval = 0xF;
	_shrval = 4;
	if (_numColors == 32) {
		_maskval = 7;
		_shrval = 3;
	}
	
	_width2 = _srcptr[0];
	_width = _width2;
	_height2 = _srcptr[2];
	_height = _height2;
	xmove = (int16)READ_LE_UINT16(_srcptr+4) + _xmove;
	ymove = (int16)READ_LE_UINT16(_srcptr+6) + _ymove;
	_xmove += (int16)READ_LE_UINT16(_srcptr+8);
	_ymove -= (int16)READ_LE_UINT16(_srcptr+10);
	_srcptr += 12;

	switch(_ptr[7]&0x7F) {
	case 0x60: case 0x61:
		ex1 = _srcptr[0];
		ex2 = _srcptr[1];
		_srcptr += 2;
		if (ex1!=0xFF || ex2!=0xFF) {
			ex1=READ_LE_UINT16(_ptr + _numColors + 10 + ex1*2);
			_srcptr = _ptr + READ_LE_UINT16(_ptr + ex1 + ex2*2) + 14;
		}
	} 

	_xpos = _actorX;
	_ypos = _actorY;
		
	scaling = _scaleX==255 && _scaleY==255 ? 0 : 1;
	s = 0;

	if (scaling) {
		_scaleIndexXStep = -1;
		if (xmove < 0) {
			xmove = -xmove;
			_scaleIndexXStep = 1;
		}

		if(_mirror) {
			unk19 = _scaleIndexX = 128 - xmove;
			for (i=0; i<xmove; i++) {
				scal = cost_scaleTable[_scaleIndexX++];
				if (scal < _scaleX)
					_xpos -= _scaleIndexXStep;
			}
			_right = _left = _xpos;
			_scaleIndexX = unk19;
			for (i=0; i<_width; i++) {
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
			for (i=0; i<xmove; i++) {
				scal = cost_scaleTable[_scaleIndexX--];
				if (scal < _scaleX)
					_xpos += _scaleIndexXStep;
			}
			_right = _left = _xpos;
			_scaleIndexX = unk19;
			for (i=0; i<_width; i++) {
				if (_left > 319) {
					s++;
					unk19 = _scaleIndexX;
				}
				scal = cost_scaleTable[_scaleIndexX--];
				if(scal < _scaleX)
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
		for (i=0; i<ymove; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY)
				_ypos -= _scaleIndexYStep;
		}
		_top = _bottom = _ypos;
		_scaleIndexY = 128 - ymove;
		for (i=0; i<_height; i++) {
			scal = cost_scaleTable[_scaleIndexY++];
			if (scal < _scaleY) 
				_bottom++;
		}
		_scaleIndexY = _scaleIndexYTop = 128 - ymove;
	} else {
		if(_mirror==0)
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
	_vscreenheight = _vm->virtscr[0].height;
	_vm->updateDirtyRect(0, _left, _right+1,_top,_bottom,1<<a->number);

	if (_top >= (int)_vscreenheight || _bottom <= 0)
		return 0;

	_ypitch = _height * 320;
	_docontinue = 0;
	b = 1;
	if (_left > 319 || _right <= 0)
		return 0;
	if (_mirror) {
		_ypitch--;
		if (scaling==0) {
			s = -_xpos;
		}
		if (s > 0) {
			_width2 -= s;
			ignorePakCols(s);
			_xpos = 0;
			_docontinue = 1;
		} else {
			s = _right - 320;
			if (s<=0) {
				b = 2;
			} else {
				_width2 -= s;
			}
		}
	} else {
		_ypitch++;
		if(scaling==0)
			s = _right - 320;
		if (s > 0) {
			_width2 -= s;
			ignorePakCols(s);
			_xpos = 319;
			_docontinue = 1;
		} else {
			s = -1 - _left;
			if (s <= 0)
				b = 2;
			else
				_width2 -= s;
		}
	}

	if (_width2==0)
		return 0;

	if ((uint)_top > (uint)_vscreenheight)
		_top = 0;

	if (_left<0) _left=0;

	if ((uint)_bottom > _vscreenheight)
		_bottom = _vscreenheight;

	if (a->top > _top)
		a->top = _top;

	if (a->bottom < _bottom)
		a->bottom = _bottom;

	if (_height2 + _top >= 256) {
		CHECK_HEAP
		return 2;
	}


	_bgbak_ptr = _vm->getResourceAddress(rtBuffer, 5) + _vm->virtscr[0].xstart + _ypos*320 + _xpos;
	_backbuff_ptr = _vm->getResourceAddress(rtBuffer, 1) + _vm->virtscr[0].xstart + _ypos*320 + _xpos;
	charsetmask = _vm->hasCharsetMask(_left, _top + _vm->virtscr[0].topline, _right, _vm->virtscr[0].topline + _bottom);
	masking = 0;

	if (_zbuf) {
		masking = _vm->isMaskActiveAt(_left, _top, _right, _bottom,
			_vm->getResourceAddress(rtBuffer, 9) + _vm->gdi._imgBufOffs[_zbuf] + _vm->_screenStartStrip
		);
	}

	if (_zbuf || charsetmask) {
		_mask_ptr = _vm->getResourceAddress(rtBuffer, 9) + _ypos*40 + _vm->_screenStartStrip;

		_imgbufoffs = _vm->gdi._imgBufOffs[_zbuf];
		if (!charsetmask && _zbuf!=0)
			_mask_ptr += _imgbufoffs;
		_mask_ptr_dest = _mask_ptr + _xpos / 8;
	}

	CHECK_HEAP

	if (a->data8) {
		proc_special(a->data8);
		return b;
	}

	switch ((scaling<<2)|(masking<<1)|charsetmask) {
	case 0: 
		proc6();
		break;
	case 1: case 2:
		proc5();
		break;
	case 3:
		proc4();
		break;
	case 4:
		proc1();
		break;
	case 5:case 6:
		proc2();
		break;
	case 7:
		proc3();
		break;
	}

	CHECK_HEAP
	return b;
}

void CostumeRenderer::proc6() {
	byte len;
	byte *src, *dst;
	byte width,height,pcolor;
	int color;
	uint y;
	uint scrheight;

	y = _ypos;
	len = _replen;
	src = _srcptr;
	dst = _backbuff_ptr;
	color = _repcolor;
	scrheight = _vscreenheight;
	width = _width2;
	height = _height2;

	if (_docontinue) goto StartPos;

	do {
		len = *src++;
		color = len>>_shrval;
		len &= _maskval;
		if (!len) len = *src++;

		do {
			if (color && y < scrheight) {
				pcolor = _palette[color];
				if (pcolor==13) {
					pcolor = _transEffect[*dst];
				}
				*dst = pcolor;
			}

			dst += 320;
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
	byte *mask,*src,*dst;
	byte maskbit,len,height,pcolor;
	uint y,scrheight;
	int color;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos&7];
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	scrheight = _vscreenheight;
	height = _height2;

	if (_docontinue) goto StartPos;

	do {
		len = *src++;
		color = len>>_shrval;
		len &= _maskval;
		if (!len) len = *src++;

		do {
			if (color && y<scrheight && !(*mask&maskbit)) {
				pcolor = _palette[color];
				if (pcolor==13)
					pcolor = _transEffect[*dst];
				*dst = pcolor;
			}
			dst += 320;
			y++;
			mask += 40;
			if (!--height) {
				if(!--_width2)
					return;
				height = _height;
				dst -= _ypitch;
				y = _ypostop;
				if(_scaleIndexXStep!=1) {
					maskbit<<=1;
					if (!maskbit) {
						maskbit=1;
						_mask_ptr--;
					}
				} else {
					maskbit>>=1;
					if (!maskbit) {
						maskbit=0x80;
						_mask_ptr++;
					}
				}
				mask = _mask_ptr;
			}
StartPos:;
		} while (--len);
	} while(1);
}

void CostumeRenderer::proc4() {
	byte *mask,*src,*dst;
	byte maskbit,len,height,pcolor;
	uint y,scrheight;
	int color;
	
	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos&7];
	y = _ypos;
	src = _srcptr;
	dst = _backbuff_ptr;
	len = _replen;
	color = _repcolor;
	scrheight = _vscreenheight;
	height = _height2;

	if (_docontinue) goto StartPos;

	do {
		len = *src++;
		color = len>>_shrval;
		len &= _maskval;
		if (!len) len = *src++;
		
		do {
			if (color && y<scrheight && !((*mask|mask[_imgbufoffs])&maskbit)) {
				pcolor = _palette[color];
				if (pcolor==13)
					pcolor = _transEffect[*dst];
				*dst = pcolor;
			}
			dst += 320;
			y++;
			mask += 40;
			if (!--height) {
				if(!--_width2)
					return;
				height = _height;
				dst -= _ypitch;
				y = _ypostop;
				if(_scaleIndexXStep!=1) {
					maskbit<<=1;
					if (!maskbit) {
						maskbit=1;
						_mask_ptr--;
					}
				} else {
					maskbit>>=1;
					if (!maskbit) {
						maskbit=0x80;
						_mask_ptr++;
					}
				}
				mask = _mask_ptr;
			}
StartPos:;
		} while (--len);
	} while(1);
}

void CostumeRenderer::proc3() {
	byte *mask,*src,*dst;
	byte maskbit,len,height,pcolor,width;
	int color,t;
	uint y;
	
	mask = _mask_ptr_dest;
	dst = _backbuff_ptr;
	height = _height2;
	width = _width2;
	len = _replen;
	color = _repcolor;
	src = _srcptr;
	maskbit = revBitMask[_xpos&7];
	y = _ypos;

	if (_docontinue) goto StartPos;

	do {
		len = *src++;
		color = len>>_shrval;
		len &= _maskval;
		if (!len) len = *src++;
		do {
			if (cost_scaleTable[_scaleIndexY++] < _scaleY) {
				if (color && y < _vscreenheight && !((*mask|mask[_imgbufoffs])&maskbit)) {
					pcolor = _palette[color];
					if (pcolor==13)
						pcolor = _transEffect[*dst];
					*dst = pcolor;
				}
				dst += 320;
				mask += 40;
				y++;
			}
			if (!--height) {
				if(!--width)
					return;
				height = _height;
				y = _ypostop;
				_scaleIndexY = _scaleIndexYTop;
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					if (_xpos >= 320)
						return;
					maskbit = revBitMask[_xpos&7];
					_backbuff_ptr += _scaleIndexXStep;
				}
				dst = _backbuff_ptr;
				mask = _mask_ptr + (_xpos>>3);
			}
StartPos:;
		} while (--len);
	} while(1);
}

void CostumeRenderer::proc2() {
	byte *mask,*src,*dst;
	byte maskbit,len,height,pcolor,width;
	int color,t;
	uint y;
	
	mask = _mask_ptr_dest;
	dst = _backbuff_ptr;
	height = _height2;
	width = _width2;
	len = _replen;
	color = _repcolor;
	src = _srcptr;
	maskbit = revBitMask[_xpos&7];
	y = _ypos;

	if (_docontinue) goto StartPos;

	do {
		len = *src++;
		color = len>>_shrval;
		len &= _maskval;
		if (!len) len = *src++;
		do {
			if (cost_scaleTable[_scaleIndexY++] < _scaleY) {
				if (color && y < _vscreenheight && !(*mask&maskbit)) {
					pcolor = _palette[color];
					if (pcolor==13)
						pcolor = _transEffect[*dst];
					*dst = pcolor;
				}
				dst += 320;
				mask += 40;
				y++;
			}
			if (!--height) {
				if(!--width)
					return;
				height = _height;
				y = _ypostop;
				_scaleIndexY = _scaleIndexYTop;
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					if (_xpos >= 320)
						return;
					maskbit = revBitMask[_xpos&7];
					_backbuff_ptr += _scaleIndexXStep;
				}
				dst = _backbuff_ptr;
				mask = _mask_ptr + (_xpos>>3);
			}
StartPos:;
		} while (--len);
	} while(1);

}

void CostumeRenderer::proc1() {
	byte *mask,*src,*dst,*dstorg;
	byte maskbit,len,height,pcolor,width;
	uint y;
	int color;
	int t;

	mask = _mask_ptr = _mask_ptr_dest;
	maskbit = revBitMask[_xpos&7];
	y = _ypos;

	dstorg = dst = _backbuff_ptr;
	height = _height2;
	width = _width2;
	len = _replen;
	color = _repcolor;
	src = _srcptr;

	if (_docontinue) goto StartPos;

	do {
		len = *src++;
		color = len>>_shrval;
		len &= _maskval;
		if (!len) len = *src++;
		
		do {
			if (cost_scaleTable[_scaleIndexY++] < _scaleY) {
				if (color && y < _vscreenheight) {
					pcolor = _palette[color];
					if (pcolor==13)
						pcolor = _transEffect[*dst];
					*dst = pcolor;
				}
				dst += 320;
				y++;
			}
			if (!--height) {
				if(!--width)
					return;
				height = _height;
				y = _ypostop;
				_scaleIndexY = _scaleIndexYTop;
				t = _scaleIndexX;
				_scaleIndexX = t + _scaleIndexXStep;
				if (cost_scaleTable[t] < _scaleX) {
					_xpos += _scaleIndexXStep;
					if (_xpos >= 320)
						return;
					_backbuff_ptr += _scaleIndexXStep;
				}
				dst = _backbuff_ptr;
			}
StartPos:;
		} while (--len);
	} while(1);
}

void CostumeRenderer::proc_special(byte code) {
	warning("stub CostumeRenderer::proc_special(%d) not implemented");
}

void CostumeRenderer::loadCostume(int id) {
	_ptr = _vm->getResourceAddress(3, id);
	
	if (_vm->_majorScummVersion == 6) {
		_ptr += 8;
	} else {
		_ptr += 2;
	}

	switch(_ptr[7]&0x7F) {
	case 0x58:
		_numColors = 16;
		break;
	case 0x59:
		_numColors = 32;
		break;
	case 0x60: /* New since version 6 */
		_numColors = 16;
		break;
	case 0x61: /* New since version 6 */
		_numColors = 32;
		break;
	default:
		error("Costume %d is invalid", id);
	}

	_dataptr = _ptr + READ_LE_UINT16(_ptr + _numColors + 8);
}

byte CostumeRenderer::drawOneSlot(Actor *a, int slot) {
	int i;
	int code;
	CostumeData *cd = &a->cost;

	if (cd->a[slot]==0xFFFF || cd->hdr & (1<<slot))
		return 0;
	
	i = cd->a[slot]&0x7FFF;
	_frameptr = _ptr + READ_LE_UINT16(_ptr + _numColors + slot*2 + 10);
	code = _dataptr[i]&0x7F;
	_srcptr = _ptr + READ_LE_UINT16(_frameptr + code*2);

	if (code != 0x7B) {
		return mainRoutine(a, slot, code);
	}

	return 0;
}

byte CostumeRenderer::drawCostume(Actor *a) {
	int i;
	byte r = 0;

	_xmove = _ymove = 0;
	for (i=0; i<16; i++)
		r|=drawOneSlot(a, i);
	return r;
}

byte CostumeRenderer::animateOneSlot(Actor *a, int slot) {
	int highflag;
	int i,end;
	byte code,nc;

	if (a->cost.a[slot]==0xFFFF)
		return 0;

	highflag = a->cost.a[slot]&0x8000;
	i = a->cost.a[slot]&0x7FFF;
	end = a->cost.c[slot];
	code=_dataptr[i]&0x7F;

	do {
		if (!highflag) {
			if (i++ >= end)
				i = a->cost.b[slot];
		} else {
			if (i != end)
				i++;
		}

		nc = _dataptr[i];

		if (nc==0x7C) {
			a->cost.animCounter1++;
			if(a->cost.b[slot] != end)
				continue;
		} else {
			if (_vm->_majorScummVersion == 6) {
				if (nc>=0x71 && nc<=0x78) {
					_vm->addSoundToQueue2(a->sound[nc-0x71]);
					if(a->cost.b[slot] != end)
						continue;
				}
			} else {
				if (nc==0x78) {
					a->cost.animCounter2++;
					if(a->cost.b[slot] != end)
						continue;
				}
			}
		}

		a->cost.a[slot] = i|highflag;
		return (_dataptr[i]&0x7F) != code;
	} while(1);
}

byte CostumeRenderer::animate(Actor *a) {
	int i;
	byte r = 0;

	for (i=0; i<16; i++) {
		if(a->cost.a[i]!=0xFFFF)
			r+=animateOneSlot(a, i);
	}
	return r;
}

