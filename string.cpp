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

int CharsetRenderer::getStringWidth(int arg, byte *text, int pos) {
	byte *ptr;
	int width,offs,w;
	byte chr;

	width = 1;
	ptr = _vm->getResourceAddress(rtCharset, _curId) + 29;
        if(_vm->_features & GF_SMALL_HEADER)
                ptr-=12;

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
			if (chr==10 || chr==21 || chr==12 || chr==13) {
				pos += 2;
				continue;
			}
			if (chr==9 || chr==1 || chr==2)
				break;
			if (chr==14) {
				int set = text[pos] | (text[pos+1]<<8);
				pos+=2;
				ptr = _vm->getResourceAddress(rtCharset, set) + 29;
                                if(_vm->_features & GF_SMALL_HEADER)
                                        ptr-=12;
				continue;
			}
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

	ptr = _vm->getResourceAddress(rtCharset, _curId) + 29;
        if(_vm->_features & GF_SMALL_HEADER)
              ptr-=12;

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
			if (chr==10 || chr==21 || chr==12 || chr==13) {
				pos += 2;
				continue;
			}
			if (chr==1) {
				curw = 1;
				continue;
			}
			if (chr==2)
				break;
			if (chr==14) {
				int set = str[pos] | (str[pos+1]<<8);
				pos+=2;
				ptr = _vm->getResourceAddress(rtCharset, set) + 29;
                                if(_vm->_features & GF_SMALL_HEADER)
                                        ptr-=12;
				continue;
			}
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
	byte buffer[100];
	_msgPtrToAdd = buffer;
	_messagePtr = addMessageToStack(_messagePtr);

	if (buffer[0] == 0xFF && buffer[1]==10) {
		uint32 a,b;

		a = buffer[2] | (buffer[3]<<8) | (buffer[6]<<16) | (buffer[7]<<24);
		b = buffer[10] | (buffer[11]<<8) | (buffer[14]<<16) | (buffer[15]<<24);
		talkSound(a,b,1);
	}

//	warning("unkMessage1(\"%s\")", buf);
}

void Scumm::unkMessage2() {
	byte buf[100], *tmp;

	_msgPtrToAdd = buf;
	tmp = _messagePtr = addMessageToStack(_messagePtr);

	if (string[3].color==0)
		string[3].color = 4;

	warning("unkMessage2(\"%s\")", buf);
	_messagePtr = tmp;
}


void Scumm::CHARSET_1() {
	int s, i, t, c;
	int frme;
	Actor *a;
	byte *buffer;	

#if !defined(FULL_THROTTLE)
	if (!_haveMsg || (camera._dest.x>>3) != (camera._cur.x>>3) ||
			camera._cur.x != camera._last.x 
		) return;
#else
	if (!_haveMsg)
		return;
#endif
	a = NULL;
	if (_vars[VAR_TALK_ACTOR] != 0xFF)
		a = derefActorSafe(_vars[VAR_TALK_ACTOR], "CHARSET_1");

	if (a && string[0].overhead!=0) {
		if (!(_features & GF_AFTER_V6)) {
			string[0].xpos = a->x - camera._cur.x + 160;

			if (_vars[VAR_V5_TALK_STRING_Y] < 0) {
				s = (a->scaley * (int)_vars[VAR_V5_TALK_STRING_Y]) / 0xFF;
				string[0].ypos = ((_vars[VAR_V5_TALK_STRING_Y]-s)>>1) + s - a->elevation + a->y;
			} else {
				string[0].ypos = _vars[VAR_V5_TALK_STRING_Y];
			}
			if (string[0].ypos < 1)
				string[0].ypos = 1;

			if (string[0].xpos < 80)
				string[0].xpos = 80;
			if (string[0].xpos > 240)
				string[0].xpos = 240;
		} else {
			s = a->scaley * a->new_1 / 0xFF;
			string[0].ypos = ((a->new_1 - s)>>1) + s - a->elevation + a->y;
			if (string[0].ypos<1)
				string[0].ypos = 1;
			
			s = a->scalex * a->new_2 / 0xFF;
			string[0].xpos = ((a->new_2 - s)>>1) + s + a->x - camera._cur.x + 160;
			if (string[0].xpos < 80)
				string[0].xpos = 80;
			if (string[0].xpos > 240)
				string[0].xpos = 240;
		}
	}

	charset._top = string[0].ypos;
	charset._left = string[0].xpos;
	charset._left2 = string[0].xpos;
	charset._curId = string[0].charset;

	if (a && a->charset)
		charset._curId = a->charset;

	charset._center = string[0].center;
	charset._right = string[0].right;
	charset._color = _charsetColor;
	_bkColor = 0;
	
	if(!(_features & GF_OLD256))	// FIXME
		for (i=0; i<4; i++)
                if(_features & GF_SMALL_HEADER)
                        charset._colorMap[i] = _charsetData[charset._curId][i-12];
                else
                        charset._colorMap[i] = _charsetData[charset._curId][i];
	
	if (_keepText) {
		charset._strLeft = gdi._mask_left;
		charset._strRight = gdi._mask_right;
		charset._strTop = gdi._mask_top;
		charset._strBottom = gdi._mask_bottom;
	}

	if (!_haveMsg || _talkDelay)
		return;
	
	if (_haveMsg!=0xFF) {
		if (_sfxMode==0)
			stopTalk();
		return;
	}

	if (a && !string[0].no_talk_anim) {
		startAnimActor(a, a->talkFrame1);
		_useTalkAnims = true;
	}

	_talkDelay = _defaultTalkDelay;

	if (!_keepText) {
		restoreCharsetBg();
		charset._xpos2 = string[0].xpos;
		charset._ypos2 = string[0].ypos;
	}
	
	t = charset._right - string[0].xpos - 1;
	if (charset._center) {
		if (t > charset._xpos2)
			t = charset._xpos2;
		t <<= 1;
	}

	buffer = charset._buffer + charset._bufPos;
	if(_features & GF_OLD256) {
		debug(1, "CHARSET_1: %s", buffer);
		return;
	}
	charset.addLinebreaks(0, buffer,0, t);

	_lastXstart = virtscr[0].xstart;
	if (charset._center) {
		charset._xpos2 -= charset.getStringWidth(0, buffer,0) >> 1;
		if (charset._xpos2<0)
			charset._xpos2 = 0;
	}

	charset._disableOffsX = charset._unk12 = !_keepText;

	do {
		c = *buffer++;
		if (c==0) {
			_haveMsg = 1;
			_keepText = false;
			break;
		}
		if (c == 13) {
newLine:;
			charset._xpos2 = string[0].xpos;
			if (charset._center) {
				charset._xpos2 -= charset.getStringWidth(0, buffer, 0)>>1;
			}
			if(_features & GF_SMALL_HEADER)
				charset._ypos2 += getResourceAddress(rtCharset,charset._curId)[18];
			else
				charset._ypos2 += getResourceAddress(rtCharset,charset._curId)[30];
			charset._disableOffsX = 1;
			continue;
		}

		if (c==0xFE) c=0xFF;

		if (c!=0xFF) {
			charset._left = charset._xpos2;
			charset._top = charset._ypos2;
			
			if (!(_features&GF_AFTER_V6)) {
//                                if (!_vars[VAR_V5_CHARFLAG]) { /* FIXME */
					charset.printChar(c);
//                                }
			} else {
				charset.printChar(c);
			}

			charset._xpos2 = charset._left;
			charset._ypos2 = charset._top;

			_talkDelay += _vars[VAR_CHARINC];
			continue;
		}

		c = *buffer++;
		if (c==3) {
			_haveMsg = 0xFF;
			_keepText = false;
			break;
		} else if (c==1) { 
			goto newLine;
		} else if (c==2) {
			_haveMsg = 0;
			_keepText = true;
			break;
		} else if (c==9) {
			frme = *buffer++;
			frme |= *buffer++<<8;
			if (a)
				startAnimActor(a, frme);
		} else if (c==10) {
			uint32 a,b;

			a = buffer[0] | (buffer[1]<<8) | (buffer[4]<<16) | (buffer[5]<<24);
			b = buffer[8] | (buffer[9]<<8) | (buffer[12]<<16) | (buffer[13]<<24);
			talkSound(a,b,2);
			buffer += 14;
		} else if (c==14) {
			int oldy = getResourceAddress(rtCharset,charset._curId)[30];

			charset._curId = *buffer++;
			buffer += 2;
			for (i=0; i<4; i++)
                                if(_features & GF_SMALL_HEADER)
                                        charset._colorMap[i] = _charsetData[charset._curId][i-12];
                                else
                                        charset._colorMap[i] = _charsetData[charset._curId][i];
			charset._ypos2 -= getResourceAddress(rtCharset,charset._curId)[30] - oldy;
		} else if (c==12) {
			int color;
			color = *buffer++;
			color |= *buffer++<<8;
			if (color==0xFF)
				charset._color = _charsetColor;
			else
				charset._color = color;
		} else if (c==13) {
			buffer += 2;
		} else {
			warning("CHARSET_1: invalid code %d", c);
		}
	} while (1);

	charset._bufPos = buffer - charset._buffer;

	gdi._mask_left = charset._strLeft;
	gdi._mask_right = charset._strRight;
	gdi._mask_top = charset._strTop;
	gdi._mask_bottom = charset._strBottom;
}

void Scumm::drawString(int a) {
	byte buf[256];
	byte *charsetptr,*space;
	int i;
	byte byte1, chr;
	uint color;
	
	_msgPtrToAdd = buf;
	_messagePtr = addMessageToStack(_messagePtr);

	charset._left2 = charset._left = string[a].xpos;
	charset._top = string[a].ypos;
	charset._curId = string[a].charset;
	charset._center = string[a].center;
	charset._right = string[a].right;
	charset._color = string[a].color;
	_bkColor = 0;
	charset._unk12 = 1;
	charset._disableOffsX = 1;
	if(!(_features & GF_OLD256)) {
	charsetptr = getResourceAddress(rtCharset, charset._curId);
	assert(charsetptr);
	charsetptr += 29;
        if(_features & GF_SMALL_HEADER)
                charsetptr-=12;

	for(i=0; i<4; i++)
                if(_features & GF_SMALL_HEADER)
                        charset._colorMap[i] = _charsetData[charset._curId][i-12];
                else
                        charset._colorMap[i] = _charsetData[charset._curId][i];

	byte1 = charsetptr[1];
	}
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
	if(_features & GF_OLD256) {
		debug(1, "DRAWSTRING: %s", buf);
		return;
	}
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
			case 10: case 13: case 14:
				i += 2;
				break;
			case 1: case 8:
				if (charset._center) {
					charset._left = charset._left2 - charset.getStringWidth(a, buf, i);
				} else {
					charset._left = charset._left2;
				}
				charset._top += byte1;
				break;
			case 12:
				color = buf[i] + (buf[i+1]<<8);
				i+=2;
				if (color==0xFF)
					charset._color = string[a].color;
				else
					charset._color = color;
				break;
			}
		} else {
			if (a==1 && (_features&GF_AFTER_V6))
				charset._blitAlso = true;
			charset.printChar(chr);
			charset._blitAlso = false;
		}
	}

	charset._ignoreCharsetMask = 0;
	
	if (a==0) {
		charset._xpos2 = charset._left;
		charset._ypos2 = charset._top;
	}
}

byte *Scumm::addMessageToStack(byte *msg) {
	int num, numorg;
	byte *ptr, chr;

	numorg = num = _numInMsgStack;
	ptr = getResourceAddress(rtTemp, 6);

	if (ptr==NULL)
		error("Message stack not allocated");

        if (msg==NULL)
                error("Bad message in addMessageToStack");

	while ( (chr=*msg++) != 0) {
		if (num > 500)
			error("Message stack overflow");

		ptr[num++] = chr;
		
		if (chr==255) {
			ptr[num++] = chr = *msg++;

			if (chr!=1 && chr!=2 && chr!=3 && chr!=8) {
				ptr[num++] = chr = *msg++;
				ptr[num++] = chr = *msg++;
			}
		}
	}
	ptr[num++] = 0;
	
	_numInMsgStack = num;
	num = numorg;

	while (1) {
		ptr = getResourceAddress(rtTemp, 6);
		chr = ptr[num++];
		if (chr == 0) 
			break;
		if (chr == 0xFF) {
			chr = ptr[num++];
			switch(chr) {
			case 4:
				unkAddMsgToStack2(READ_LE_UINT16(ptr + num));
				num+=2;
				break;
			case 5:
				unkAddMsgToStack3(READ_LE_UINT16(ptr + num));
				num+=2;
				break;
			case 6:
				unkAddMsgToStack4(READ_LE_UINT16(ptr + num));
				num+=2;
				break;
			case 7:
				unkAddMsgToStack5(READ_LE_UINT16(ptr + num));
				num+=2;
				break;
			case 9: 
//#if defined(DOTT)
			case 10: case 12: case 13: case 14:
//#endif
				*_msgPtrToAdd++ = 0xFF;
				*_msgPtrToAdd++ = chr;
				*_msgPtrToAdd++ = ptr[num++];
				*_msgPtrToAdd++ = ptr[num++];
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
			if (num==_verbs[i].verbid && !_verbs[i].type && !_verbs[i].saveid) {
				addMessageToStack(getResourceAddress(rtVerb, i));
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

	if (_features&GF_AFTER_V6)
		var = readVar(var);
	
	if (var) {
		ptr = getStringAddress(var);
		if (ptr) {
			addMessageToStack(ptr);
			return;
		}
	}
	addMessageToStack((byte*)"");
}

void Scumm::initCharset(int charsetno) {
	int i;
	if(_features & GF_OLD256) return; // FIXME

        if (_features & GF_SMALL_HEADER)
                loadCharset(charsetno);
        else
                if (!getResourceAddress(rtCharset, charsetno))
                        loadCharset(charsetno);

	string[0].t_charset = charsetno;
	string[1].t_charset = charsetno;

	for (i=0; i<0x10; i++)
                if(_features & GF_SMALL_HEADER)
                        charset._colorMap[i] = _charsetData[charset._curId][i-12];
                else
                        charset._colorMap[i] = _charsetData[charset._curId][i];
}

void CharsetRenderer::printChar(int chr) {
	int d,right;
	VirtScreen *vs;
	
	_vm->checkRange(_vm->_maxCharsets-1, 1, _curId, "Printing with bad charset %d");
	if ((vs=_vm->findVirtScreen(_top)) == NULL)
		return;

	if (chr=='@')
		return;

	_ptr = _vm->getResourceAddress(rtCharset, _curId) + 29;
	if (_vm->_features & GF_SMALL_HEADER)
		_ptr -=12;
	
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
	if (_drawTop<0) _drawTop = 0;

	_bottom = _drawTop + _height + _offsY;

	_vm->updateDirtyRect(vs->number, _left, right, _drawTop, _bottom, 0);

#if defined(OLD)
	if (vs->number==0)
		_hasMask = true;
#else
	if (vs->number!=0)
		_blitAlso = false;
	if (vs->number==0 && _blitAlso==0)
		_hasMask = true;
#endif

	_dest_ptr = _backbuff_ptr = _vm->getResourceAddress(rtBuffer, vs->number+1) 
		+ vs->xstart + _drawTop * 320 + _left;

#if !defined(OLD)
	if (_blitAlso) {
#else
	if (1) {
#endif
		_dest_ptr = _bgbak_ptr = _vm->getResourceAddress(rtBuffer, vs->number+5)
			+ vs->xstart + _drawTop * 320 + _left;
	}

	_mask_ptr = _vm->getResourceAddress(rtBuffer, 9)
		+ _drawTop * 40 + _left/8 
		+ _vm->_screenStartStrip;

	_revBitMask = revBitMask[_left&7];

	_virtScreenHeight = vs->height;
	_charPtr += 4;

	drawBits();

#if !defined(OLD)
	if (_blitAlso)
		blit(_backbuff_ptr, _bgbak_ptr, _width, _height);
#endif

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

	usemask = (_vm->_curVirtScreen->number==0 && _ignoreCharsetMask==0);

	bits = *_charPtr++;
	numbits = 8;

	dst = _dest_ptr;
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
		dst = (_dest_ptr += 320);
		mask += 40;
		y++;
	}
}
