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
 * Change Log:
 * $Log$
 * Revision 1.1  2001/10/09 14:30:13  strigeus
 * Initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

int CharsetRenderer::getStringWidth(int arg, byte *text, int pos) {
	byte *ptr;
	int width,offs,w;
	byte chr;

	width = 1;
	ptr = _vm->getResourceAddress(6, _curId) + 29;

	while ( (chr = text[pos++]) != 0) {
		if (chr==0xD)
			break;
		if (chr=='@')
			continue;
		if (chr==254) chr=255;
		if (chr==255) {
			chr=text[pos++];
			if (chr==3)
				break;
			if (chr==8) {
				if (arg==1)
					break;
				while (text[pos]==' ')
					text[pos++] = '@';
				continue;
			}
			if (chr==1 || chr==2)
				break;
		}

		offs = READ_LE_UINT32(ptr + chr*4 + 4);
		if (offs) {
			if (ptr[offs+2]>=0x80) {
				w = ptr[offs+2] - 0x100;
			} else {
				w = ptr[offs+2];
			}
			width += ptr[offs] + w;
		}
	}
	return width;
}

void CharsetRenderer::addLinebreaks(int a, byte *str, int pos, int maxwidth) {
	int lastspace = -1;
	int curw = 1;
	int offs,w;
	byte *ptr;
	byte chr;

	ptr = _vm->getResourceAddress(6, _curId) + 29;

	while ( (chr=str[pos++]) != 0) {
		if (chr=='@')
			continue;
		if (chr==254) chr=255;
		if (chr==255) {
			chr = str[pos++];
			if (chr==3)
				break;
			if (chr==8) {
				if (a==1) {
					curw = 1;
				} else {
					while (str[pos]==' ')
						str[pos++] = '@';
				}
				continue;
			}
			if (chr==1) {
				curw = 1;
				continue;
			}
			if (chr==2)
				break;
		}

		if (chr==' ')
			lastspace = pos - 1;

		offs = READ_LE_UINT32(ptr + chr*4 + 4);
		if (offs) {
			if (ptr[offs+2]>=0x80) {
				w = ptr[offs+2] - 0x100;
			} else {
				w = ptr[offs+2];
			}
			curw += w + ptr[offs];
		}
		if (lastspace==-1)
				continue;
		if (curw > maxwidth) {
			str[lastspace] = 0xD;
			curw = 1;
			pos = lastspace + 1;
			lastspace = -1;
		}
	}
}

void Scumm::unkMessage1() {
	byte buf[100];
	_msgPtrToAdd = buf;
	_messagePtr = addMessageToStack(_messagePtr);
}

void Scumm::unkMessage2() {
	byte buf[100], *tmp;

	_msgPtrToAdd = buf;
	tmp = _messagePtr = addMessageToStack(_messagePtr);

	if (_stringColor[3]==0)
		_stringColor[3] = 4;

	error("unkMessage2: call to printScummMessage(%s)", buf);
	vm.vars[0] = 0;
	_messagePtr = tmp;
}


void Scumm::CHARSET_1() {
	int s, i, t, c;
	int frme;
	Actor *a;

	if (!_haveMsg || (camera._destPos>>3) != (camera._curPos>>3) ||
			camera._curPos != camera._lastPos 
		) return;

	a = NULL;
	if (vm.vars[VAR_TALK_ACTOR] != 0xFF)
		a = derefActorSafe(vm.vars[VAR_TALK_ACTOR], "CHARSET_1");

	if (a && _stringOverhead[0]!=0) {
		_stringXpos[0] = a->x - camera._curPos + 160;

		if (vm.vars[VAR_TALK_STRING_Y] < 0) {
			s = (a->scaley * (int)vm.vars[VAR_TALK_STRING_Y]) / 0xFF;
			_stringYpos[0] = ((vm.vars[VAR_TALK_STRING_Y]-s)>>1) + s - a->elevation + a->y;
		} else {
			_stringYpos[0] = vm.vars[VAR_TALK_STRING_Y];
		}
		if (_stringYpos[0] < 1)
			_stringYpos[0] = 1;

		if (_stringXpos[0] < 80)
			_stringXpos[0] = 80;
		if (_stringXpos[0] > 240)
			_stringXpos[0] = 240;
	}

	charset._top = _stringYpos[0];
	charset._left = _stringXpos[0];
	charset._left2 = _stringXpos[0];
	charset._curId = _stringCharset[0];

	if (a && a->charset)
		charset._curId = a->charset;

	charset._center = _stringCenter[0];
	charset._right = _stringRight[0];
	charset._color = _charsetColor;
	dseg_4E3C = 0;

	for (i=0; i<4; i++)
		charset._colorMap[i] = _charsetData[charset._curId][i];
	
	if (_keepText) {
		charset._strLeft = charset._mask_left;
		charset._strRight = charset._mask_right;
		charset._strTop = charset._mask_top;
		charset._strBottom = charset._mask_bottom;
	}

	if (!_haveMsg || _talkDelay)
		return;
	
	if (_haveMsg!=0xFF) {
		stopTalk();
		return;
	}

	if (a) {
		startAnimActor(a, a->talkFrame1, a->facing);
	}

	_talkDelay = _defaultTalkDelay;

	if (!_keepText) {
		restoreCharsetBg();
		_stringXpos2[0] = _stringXpos[0];
		_stringYpos2[0] = _stringYpos[0];
	}
	
	t = charset._right - _stringXpos2[0] - 1;
	if (charset._center) {
		if (t > _stringXpos2[0])
			t = _stringXpos2[0];
		t <<= 1;
	}
	charset.addLinebreaks(0, charset._buffer, charset._bufPos, t);

	_lastXstart = virtscr[0].xstart;
	if (charset._center) {
		_stringXpos2[0] -= charset.getStringWidth(0, charset._buffer, charset._bufPos) >> 1;
	}

	charset._disableOffsX = charset._unk12 = !_keepText;

	do {
		c = charset._buffer[charset._bufPos++];
		if (c==0) {
			_haveMsg = 1;
			_keepText = false;
			break;
		}
		if (c != 13) {
			if (c==0xFE)
				c=0xFF;

			if (c!=0xFF) {
PrintChar:;
				charset._left = _stringXpos2[0];
				charset._top = _stringYpos2[0];
				
				if (!vm.vars[VAR_CHARFLAG]) {
					charset.printChar(c);
				}
				_stringXpos2[0] = charset._left;
				_stringYpos2[0] = charset._top;

				_talkDelay += vm.vars[VAR_CHARINC];
				continue;
			}

			c = charset._buffer[charset._bufPos++];
			if (c==3) {
				_haveMsg = 0xFF;
				_keepText = false;
				break;
			}
			if (c!=1) {
				if (c==2) {
					_haveMsg = 0;
					_keepText = true;
					break;
				}
				if (c==9) {
					frme = charset._buffer[charset._bufPos++];
					frme |= charset._buffer[charset._bufPos++]<<8;
					if (a)
						startAnimActor(a, frme, a->facing);
				}
				goto PrintChar;
			}
		}
		_stringXpos2[0] = _stringXpos[0];
		if (charset._center) {
			_stringXpos2[0] -= charset.getStringWidth(0, charset._buffer, charset._bufPos)>>1;
		}
		_stringYpos2[0] += getResourceAddress(6,charset._curId)[30];
		charset._disableOffsX = 1;
	} while (1);

	charset._mask_left = charset._strLeft;
	charset._mask_right = charset._strRight;
	charset._mask_top = charset._strTop;
	charset._mask_bottom = charset._strBottom;
}

void Scumm::drawString(int a) {
	byte buf[256];
	byte *charsetptr,*space;
	int i;
	byte byte1, chr;
	
	_msgPtrToAdd = buf;
	_messagePtr = addMessageToStack(_messagePtr);

	charset._left2 = charset._left = _stringXpos[a];
	charset._top = _stringYpos[a];
	charset._curId = _stringCharset[a];
	charset._center = _stringCenter[a];
	charset._right = _stringRight[a];
	charset._color = _stringColor[a];
	dseg_4E3C = 0;
	charset._unk12 = 1;
	charset._disableOffsX = 1;

	charsetptr = getResourceAddress(6, charset._curId);
	assert(charsetptr);
	charsetptr += 29;

	for(i=0; i<4; i++)
		charset._colorMap[i] = _charsetData[charset._curId][i];

	byte1 = charsetptr[1];
	
	_msgPtrToAdd = buf;

	/* trim from the right */
	space = NULL;
	while (*_msgPtrToAdd){
		if (*_msgPtrToAdd==' ') {
			if (!space) space = _msgPtrToAdd;
		} else {
			space = NULL;
		}
		_msgPtrToAdd++;
	}
	if(space) *space='\0';

	if (charset._center) {
		charset._left -= charset.getStringWidth(a, buf, 0) >> 1;
	}

	charset._ignoreCharsetMask = 1;

	if (!buf[0]) {
		buf[0] = ' ';
		buf[1] = 0;
	}

	for (i=0; (chr=buf[i++]) != 0; ) {
		if (chr==254) chr=255;
		if (chr==255) {
			chr = buf[i++];
			switch(chr) {
			case 9:
				i += 2;
				break;
			case 1: case 8:
				if (charset._center) {
					charset._left = charset._left2 - charset.getStringWidth(a, buf, i);
				} else {
					charset._left = charset._left2;
				}
				charset._top += byte1;			
			}
		} else {
			charset.printChar(chr);
		}
	}

	charset._ignoreCharsetMask = 0;
	_stringXpos2[a] = charset._left;
	_stringYpos2[a] = charset._top;
}

byte *Scumm::addMessageToStack(byte *msg) {
	int num, numorg;
	byte *ptr, chr;

	numorg = num = _numInMsgStack;
	ptr = getResourceAddress(0xC, 6);

	if (ptr==NULL)
		error("Message stack not allocated");

	while ( (chr=*msg++) != 0) {
		if (num > 500)
			error("Message stack overflow");

		ptr[num++] = chr;
		
		if (chr==255) {
			ptr[num++] = chr = *msg++;

			if (chr==0 || chr!=2 && chr!=3 && chr!=8) {
				ptr[num++] = chr = *msg++;
				ptr[num++] = chr = *msg++;
			}
		}
	}
	ptr[num++] = 0;
	
	_numInMsgStack = num;
	num = numorg;

	while (1) {
		ptr = getResourceAddress(0xC, 6);
		chr = ptr[num++];
		if (chr == 0) 
			break;
		if (chr == 0xFF) {
			ptr = getResourceAddress(0xC, 6);
			chr = ptr[num++];
			switch(chr) {
			case 4:
				unkAddMsgToStack2(
					READ_LE_UINT16(getResourceAddress(0xC, 6)+ num)
				);
				num+=2;
				break;
			case 5:
				unkAddMsgToStack3(
					READ_LE_UINT16(getResourceAddress(0xC, 6)+ num)
				);
				num+=2;
				break;
			case 6:
				unkAddMsgToStack4(
					READ_LE_UINT16(getResourceAddress(0xC, 6)+ num)
				);
				num+=2;
				break;
			case 7:
				unkAddMsgToStack5(
					READ_LE_UINT16(getResourceAddress(0xC, 6)+num)
				);
				num+=2;
				break;
			case 9:
				*_msgPtrToAdd++ = 0xFF;
				*_msgPtrToAdd++ = chr;
				*_msgPtrToAdd++ = getResourceAddress(0xC, 6)[num++];
				*_msgPtrToAdd++ = getResourceAddress(0xC, 6)[num++];
				break;
			default:
				*_msgPtrToAdd++ = 0xFF;
				*_msgPtrToAdd++ = chr;
			}
		} else {
			if (chr!='@') {
				*_msgPtrToAdd++ = chr;
			}
		}
	}
	*_msgPtrToAdd = 0;
	_numInMsgStack = numorg;
	
	return msg;
}

void Scumm::unkAddMsgToStack2(int var) {
	int num,max;	
	byte flag;
	
	num = readVar(var);
	if (num < 0) {
		*_msgPtrToAdd++ = '-';
		num = -num;
	}

	flag = 0;
	max = 10000;
	do {
		if (num>=max || flag) {
			*_msgPtrToAdd++ = num/max + '0';
			num -= (num/max)*max;
			flag=1;
		}
		max/=10;
		if (max==1) flag=1;
	} while (max);
}

void Scumm::unkAddMsgToStack3(int var) {
	int num,i;
	
	num = readVar(var);
	if (num) {
		for (i=1; i<_maxVerbs; i++) {
			if (num==verbs[i].verbid && !verbs[i].type && !verbs[i].saveid) {
				addMessageToStack(getResourceAddress(8, i));
				break;
			}
		}
	} else {
		addMessageToStack((byte*)"");
	}
}

void Scumm::unkAddMsgToStack4(int var) {
	int num;

	num = readVar(var);
	if (num) {
		addMessageToStack(getObjOrActorName(num));	
	} else {
		addMessageToStack((byte*)"");
	}
}

void Scumm::unkAddMsgToStack5(int var) {
	byte *ptr;
	if (var) {
		ptr = getResourceAddress(7, var);
		if (ptr) {
			addMessageToStack(ptr);
			return;
		}
	}
	addMessageToStack((byte*)"");
}

void Scumm::initCharset(int charsetno) {
	int i;

	if (!getResourceAddress(6, charsetno))
		loadCharset(charsetno);

	textslot.charset[0] = charsetno;
	textslot.charset[1] = charsetno;

	for (i=0; i<0x10; i++)
		charset._colorMap[i] = _charsetData[charsetno][i];
}

void CharsetRenderer::printChar(int chr) {
	int d,right;
	VirtScreen *vs;
	
	_vm->checkRange(_vm->_maxCharsets-1, 1, _curId, "Printing with bad charset %d");
	if (_vm->findVirtScreen(_top)==-1)
		return;

	vs = &_vm->virtscr[_vm->gdi.virtScreen];

	if (chr=='@')
		return;

	_ptr = _vm->getResourceAddress(6, _curId) + 29;

	_bpp = _unk2 = *_ptr;
	_invNumBits = 8 - _bpp;
	_bitMask = 0xFF << _invNumBits;
	_colorMap[1] = _color;

	_charOffs = READ_LE_UINT32(_ptr + chr*4 + 4);

	if (!_charOffs)
		return;

	assert(_charOffs < 0x10000);

	_charPtr = _ptr + _charOffs;

	_width = _charPtr[0];
	_height = _charPtr[1];
	if (_unk12) {
		_strLeft = 0;
		_strTop = 0;
		_strRight = 0;
		_strBottom = 0;
	}

	if (_disableOffsX) {
		_offsX = 0;
	} else {
		d = _charPtr[2];
		if (d>=0x80)
			d -= 0x100;
		_offsX = d;
	}

	d = _charPtr[3];
	if(d>=0x80)
		d -= 0x100;
	_offsY = d;

	_top += _offsY;
	_left += _offsX;

	right = _left + _width;

	if (right>_right+1 || _left < 0) {
		_left = right;
		_top -= _offsY;
		return;
	}

	_disableOffsX = 0;

	if (_unk12) {
		_strLeft = _left;
		_strTop = _top;
		_strRight = _left;
		_strBottom = _top;
		_unk12 = 0;
	}

	if (_left < _strLeft)
		_strLeft = _left;

	if (_top < _strTop)
		_strTop = _top;

	_drawTop = _top - vs->topline;
	_bottom = _drawTop + _height + _offsY;

	_vm->updateDirtyRect(_vm->gdi.virtScreen, _left, right, _drawTop, _bottom, 0);

	if (_vm->gdi.virtScreen==0)
		_hasMask = true;

	_bg_ptr = _vm->getResourceAddress(0xA, _vm->gdi.virtScreen+1) 
		+ vs->xstart + _drawTop * 320 + _left;
	
	_where_to_draw_ptr = _vm->getResourceAddress(0xA, _vm->gdi.virtScreen+5)
		+ vs->xstart + _drawTop * 320 + _left;

	_mask_ptr = _vm->getResourceAddress(0xA, 9)
		+ _drawTop * 40 + _left/8 
		+ _vm->_screenStartStrip;

	_revBitMask = revBitMask[_left&7];

	_virtScreenHeight = vs->height;
	_charPtr += 4;

	drawBits();
	
	_left += _width;
	if (_left  > _strRight)
		_strRight = _left;

	if (_top + _height > _strBottom)
		_strBottom = _top + _height;

	_top -= _offsY;
}


void CharsetRenderer::drawBits() {
	bool usemask;
	byte *dst, *mask,maskmask;
	int y,x;
	int maskpos;
	int color;
	byte numbits,bits;

	usemask = (_vm->gdi.virtScreen==0 && _ignoreCharsetMask==0);

	bits = *_charPtr++;
	numbits = 8;

	dst = _bg_ptr;
	mask = _mask_ptr;
	y = 0;

	for(y=0; y<_height && y+_drawTop < _virtScreenHeight;) {
		maskmask = _revBitMask;
		maskpos = 0;

		for (x=0; x<_width; x++) {
			color = (bits&_bitMask)>>_invNumBits;
			if (color) {
				if (usemask) {
					mask[maskpos] |= maskmask;
				}
				*dst = _colorMap[color];
			}
			dst++;
			bits <<= _bpp;
			if ((numbits -= _bpp)==0) {
				bits = *_charPtr++;
				numbits = 8;
			}
			if ((maskmask>>=1)==0) {
				maskmask = 0x80;
				maskpos++;
			}
		}
		dst = (_bg_ptr += 320);
		mask += 40;
		y++;
	}
}
