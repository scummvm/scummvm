/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
#include "scumm/sound.h"

int CharsetRenderer::getStringWidth(int arg, byte *text, int pos)
{
	byte *ptr;
	int width;
	byte chr;

	width = 1;
	ptr = _vm->getResourceAddress(rtCharset, _curId) + 29;
	if (_vm->_features & GF_SMALL_HEADER)
		ptr -= 12;

	while ((chr = text[pos++]) != 0) {
		if (chr == 0xD)
			break;
		if (chr == '@')
			continue;
		if (chr == 254)
			chr = 255;
		if (chr == 255) {
			chr = text[pos++];
			if (chr == 3)	// 'WAIT'
				break;
			if (chr == 8) { // 'Verb on next line'
				if (arg == 1)
					break;
				while (text[pos] == ' ')
					text[pos++] = '@';
				continue;
			}
			if (chr == 10 || chr == 21 || chr == 12 || chr == 13) {
				pos += 2;
				continue;
			}
			if (chr == 9 || chr == 1 || chr == 2) // 'Newline'
				break;
			if (chr == 14) {
				int set = text[pos] | (text[pos + 1] << 8);
				pos += 2;
				ptr = _vm->getResourceAddress(rtCharset, set) + 29;
				if (_vm->_features & GF_SMALL_HEADER)
					ptr -= 12;
				continue;
			}
		}
		width += getSpacing(chr, ptr);
	}

	return width;
}

void CharsetRenderer::addLinebreaks(int a, byte *str, int pos, int maxwidth)
{
	int lastspace = -1;
	int curw = 1;
	byte *ptr;
	byte chr;

	ptr = _vm->getResourceAddress(rtCharset, _curId) + 29;
	if (_vm->_features & GF_SMALL_HEADER)
		ptr -= 12;

	while ((chr = str[pos++]) != 0) {
		if (chr == '@')
			continue;
		if (chr == 254)
			chr = 255;
		if (chr == 255) {
			chr = str[pos++];
			if (chr == 3) // 'Wait'
				break;
			if (chr == 8) { // 'Verb on next line'
				if (a == 1) {
					curw = 1;
				} else {
					while (str[pos] == ' ')
						str[pos++] = '@';
				}
				continue;
			}
			if (chr == 10 || chr == 21 || chr == 12 || chr == 13) {
				pos += 2;
				continue;
			}
			if (chr == 1) { // 'Newline'
				curw = 1;
				continue;
			}
			if (chr == 2) // 'Don't terminate with \n'
				break;
			if (chr == 14) {
				int set = str[pos] | (str[pos + 1] << 8);
				pos += 2;
				ptr = _vm->getResourceAddress(rtCharset, set) + 29;
				if (_vm->_features & GF_SMALL_HEADER)
					ptr -= 12;
				continue;
			}
		}

		if (chr == ' ')
			lastspace = pos - 1;

		curw += getSpacing(chr, ptr);
		if (lastspace == -1)
			continue;
		if (curw > maxwidth) {
			str[lastspace] = 0xD;
			curw = 1;
			pos = lastspace + 1;
			lastspace = -1;
		}
	}
}

void Scumm::unkMessage1()
{
	byte buffer[100];
	_msgPtrToAdd = buffer;
	_messagePtr = addMessageToStack(_messagePtr);

	if (buffer[0] == 0xFF && buffer[1] == 10) {
		uint32 a, b;

		a = buffer[2] | (buffer[3] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
		b = buffer[10] | (buffer[11] << 8) | (buffer[14] << 16) | (buffer[15] << 24);

		// Sam and Max uses a caching system, printing empty messages
		// and setting VAR_V6_SOUNDMODE beforehand. See patch 609791.
		// FIXME: There are other VAR_V6_SOUNDMODE states, as
		// mentioned in the patch. FIXME after iMUSE is done.
		if (_gameId != GID_SAMNMAX || (_vars[VAR_V6_SOUNDMODE] != 2))
			_sound->talkSound(a, b, 1, -1);
	}
}

void Scumm::unkMessage2()
{
	byte buf[100], *tmp;

	_msgPtrToAdd = buf;
	tmp = _messagePtr = addMessageToStack(_messagePtr);

	if (_string[3].color == 0)
		_string[3].color = 4;

	warning("unkMessage2(\"%s\")", buf);
	_messagePtr = tmp;
}


void Scumm::CHARSET_1()
{
	uint32 talk_sound_a = 0;
	uint32 talk_sound_b = 0;
	int s, i, t, c;
	int frme = -1;
	Actor *a;
	byte *buffer;
	bool has_talk_sound = false;
	bool has_anim = false;

	if (!_haveMsg)
		return;

	// FIXME: This Zak check fixes several hangs (Yak hang, and opening
	// 'secret room while walking' hang. It doesn't do the camera check
	// when the talk target isn't an actor. The question is, can we make
	// this a more general case? Does it really need to be Zak specific?
	if (!(_features & GF_AFTER_V7) && !(_gameId==GID_ZAK256 && _vars[VAR_TALK_ACTOR] == 0xFF)) {
		if ((camera._dest.x >> 3) != (camera._cur.x >> 3) || camera._cur.x != camera._last.x)
			return;
	}

	a = NULL;
	if (_vars[VAR_TALK_ACTOR] != 0xFF)
		a = derefActorSafe(_vars[VAR_TALK_ACTOR], "CHARSET_1");

	if (a && _string[0].overhead != 0) {
		if (!(_features & GF_AFTER_V6)) {
			_string[0].xpos = a->x - camera._cur.x + (_realWidth / 2);

			if (_vars[VAR_V5_TALK_STRING_Y] < 0) {
				s = (a->scaley * (int)_vars[VAR_V5_TALK_STRING_Y]) / 0xFF;
				_string[0].ypos = ((_vars[VAR_V5_TALK_STRING_Y] - s) >> 1) + s - a->elevation + a->y;
			} else {
				_string[0].ypos = _vars[VAR_V5_TALK_STRING_Y];
			}
			if (_string[0].ypos < 1)
				_string[0].ypos = 1;

			if (_string[0].xpos < 80)
				_string[0].xpos = 80;
			if (_string[0].xpos > 240)
				_string[0].xpos = 240;
		} else {
			s = a->scaley * a->new_1 / 0xFF;
			_string[0].ypos = ((a->new_1 - s) >> 1) + s - a->elevation + a->y;
			if (_string[0].ypos < 1)
				_string[0].ypos = 1;

			if (_string[0].ypos < camera._cur.y - (_realHeight / 2))
				_string[0].ypos = camera._cur.y - (_realHeight / 2);

			s = a->scalex * a->new_2 / 0xFF;
			_string[0].xpos = ((a->new_2 - s) >> 1) + s + a->x - camera._cur.x + (_realWidth / 2);
			if (_string[0].xpos < 80)
				_string[0].xpos = 80;
			if (_string[0].xpos > 240)
				_string[0].xpos = 240;
		}
	}

	charset._top = _string[0].ypos;
	charset._left = _string[0].xpos;
	charset._left2 = _string[0].xpos;
	charset._curId = _string[0].charset;

	if (a && a->charset)
		charset._curId = a->charset;

	charset._center = _string[0].center;
	charset._right = _string[0].right;
	charset._color = _charsetColor;
	_bkColor = 0;

	if (!(_features & GF_OLD256))	// FIXME
		for (i = 0; i < 4; i++)
			if (_features & GF_SMALL_HEADER)
				charset._colorMap[i] = _charsetData[charset._curId][i - 12];
			else
				charset._colorMap[i] = _charsetData[charset._curId][i];

	if (_keepText) {
		charset._strLeft = gdi._mask_left;
		charset._strRight = gdi._mask_right;
		charset._strTop = gdi._mask_top;
		charset._strBottom = gdi._mask_bottom;
	}

	if (_talkDelay)
		return;

	if (_haveMsg != 0xFF && _haveMsg != 0xFE) {
		if (_sound->_sfxMode == 0)
			stopTalk();
		return;
	}

	if (a && !_string[0].no_talk_anim) {
		has_anim = true;
		_useTalkAnims = true;
	}

	// HACK: Most of the audio sync in Loom is handled by the "MI1
	// timer", but some of it depends on text strings timing out at
	// the right moment.

	if (_gameId == GID_LOOM256) {
		_defaultTalkDelay = 100;
		_vars[VAR_CHARINC] = 5;
	}

	_talkDelay = _defaultTalkDelay;

	if (!_keepText) {
		if (_features & GF_OLD256) {
			gdi._mask_left = _string[0].xpos;
			gdi._mask_top = _string[0].ypos;
			gdi._mask_bottom = _string[0].ypos + 8;
			gdi._mask_right = _realWidth;
			if (_string[0].ypos <= 16)	// If we are cleaning the text line, clean 2 lines.
				gdi._mask_bottom = 16;
		}
		restoreCharsetBg();
		charset._xpos2 = _string[0].xpos;
		charset._ypos2 = _string[0].ypos;
	}

	t = charset._right - _string[0].xpos - 1;
	if (charset._center) {
		if (t > charset._xpos2)
			t = charset._xpos2;
		t <<= 1;
	}

	buffer = charset._buffer + charset._bufPos;
	charset.addLinebreaks(0, buffer, 0, t);

	_lastXstart = virtscr[0].xstart;
	if (charset._center) {
		charset._xpos2 -= charset.getStringWidth(0, buffer, 0) >> 1;
		if (charset._xpos2 < 0)
			charset._xpos2 = 0;
	}

	charset._disableOffsX = charset._unk12 = !_keepText;

	do {
		c = *buffer++;
		if (c == 0) {
			// End of text reached, set _haveMsg to 1 so that the text will be
			// removed next time CHARSET_1 is called.
			_haveMsg = 1;
			_keepText = false;
			break;
		}
		if (c == 13) {
		newLine:;
			if (_features & GF_OLD256) {
				charset._ypos2 = 8;
				charset._xpos2 = 0;
				continue;
			} else {
				charset._xpos2 = _string[0].xpos;
				if (charset._center) {
					charset._xpos2 -= charset.getStringWidth(0, buffer, 0) >> 1;
				}
				if (_features & GF_SMALL_HEADER)
					charset._ypos2 += getResourceAddress(rtCharset, charset._curId)[18];
				else
					charset._ypos2 += getResourceAddress(rtCharset, charset._curId)[30];
				charset._disableOffsX = 1;
				continue;
			}
		}

		if (c == 0xFE)
			c = 0xFF;

		if (c != 0xFF) {
			charset._left = charset._xpos2;
			charset._top = charset._ypos2;
			if (_features & GF_OLD256)
				charset.printCharOld(c);
			else if (!(_features & GF_AFTER_V6)) {
//                                if (!_vars[VAR_V5_CHARFLAG]) { /* FIXME */
				if (!(_haveMsg == 0xFE && _noSubtitles))
					charset.printChar(c);
//                                }
			} else {
				if (!(_haveMsg == 0xFE && _noSubtitles))
					charset.printChar(c);
			}

			charset._xpos2 = charset._left;
			charset._ypos2 = charset._top;
			_talkDelay += _vars[VAR_CHARINC];
			continue;
		}

		c = *buffer++;
		switch(c) {
		case 1:
			goto newLine;
		case 2:
			_haveMsg = 0;
			_keepText = true;
			break;
		case 3:		
			if (_haveMsg != 0xFE)
				_haveMsg = 0xFF;
			_keepText = false;
			break;		
		case 9:
			frme = *buffer++;
			frme |= *buffer++ << 8;
			has_anim = true;
			break;
		case 10:
			talk_sound_a = buffer[0] | (buffer[1] << 8) | (buffer[4] << 16) | (buffer[5] << 24);
			talk_sound_b = buffer[8] | (buffer[9] << 8) | (buffer[12] << 16) | (buffer[13] << 24);
			has_talk_sound = true;
			buffer += 14;

			// Set flag that speech variant exist of this msg
			if (_haveMsg == 0xFF)
				_haveMsg = 0xFE;
			break;		
		case 12:
			int color;
			color = *buffer++;
			color |= *buffer++ << 8;
			if (color == 0xFF)
				charset._color = _charsetColor;
			else
				charset._color = color;
			break;
		case 13:
			buffer += 2;
			break;
		case 14: {
			int oldy;
			if (_features & GF_SMALL_HEADER)
				oldy = getResourceAddress(rtCharset, charset._curId)[18];
			else
				oldy = getResourceAddress(rtCharset, charset._curId)[30];

			charset._curId = *buffer++;
			buffer += 2;
			for (i = 0; i < 4; i++)
				if (_features & GF_SMALL_HEADER)
					charset._colorMap[i] = _charsetData[charset._curId][i - 12];
				else
					charset._colorMap[i] = _charsetData[charset._curId][i];
			if (_features & GF_SMALL_HEADER)
				charset._ypos2 -= getResourceAddress(rtCharset, charset._curId)[18] - oldy;
			else
				charset._ypos2 -= getResourceAddress(rtCharset, charset._curId)[30] - oldy;
			break;
			}
		default:
			warning("CHARSET_1: invalid code %d", c);
		}
		if (c == 3 || c == 2)
			break;

	} while (1);

	// Even if talkSound() is called, we may still have to call
	// startAnimActor() since actorTalk() may already have caused the
	// wrong animation frame to be drawn, and the talkSound() won't be
	// processed until after the next screen update. Bleah.

	if (has_talk_sound)
		_sound->talkSound(talk_sound_a, talk_sound_b, 2, frme);
	if (a && has_anim)
		a->startAnimActor(frme != -1 ? frme : a->talkFrame1);

	charset._bufPos = buffer - charset._buffer;

	gdi._mask_left = charset._strLeft;
	gdi._mask_right = charset._strRight;
	gdi._mask_top = charset._strTop;
	gdi._mask_bottom = charset._strBottom;
}

void Scumm::description()
{
	int c;
	byte *buffer;

	buffer = charset._buffer;
	charset._bufPos = 0;
	_string[0].ypos = camera._cur.y + 88;
	_string[0].xpos = (_realWidth / 2) - (charset.getStringWidth(0, buffer, 0) >> 1);
	if (_string[0].xpos < 0)
		_string[0].xpos = 0;

	charset._top = _string[0].ypos;
	charset._left = _string[0].xpos;
	charset._left2 = _string[0].xpos;
	charset._right = _realWidth - 1;
	charset._xpos2 = _string[0].xpos;
	charset._ypos2 = _string[0].ypos;
	charset._disableOffsX = charset._unk12 = 1;
	charset._curId = 3;
	charset._center = false;
	charset._color = 15;
	_bkColor = 0;
	// FIXME: _talkdelay = 1 - display description, not correct ego actor talking,
	// 0 - no display, correct ego actor talking
	_talkDelay = 0;

	restoreCharsetBg();

	_lastXstart = virtscr[0].xstart;

	do {
		c = *buffer++;
		if (c == 0) {
			_haveMsg = 1;
			break;
		}
		if (c != 0xFF) {
			charset._left = charset._xpos2;
			charset._top = charset._ypos2;
			charset.printChar(c);
			charset._xpos2 = charset._left;
			charset._ypos2 = charset._top;
			continue;
		}

	} while (1);

	gdi._mask_left = charset._strLeft;
	gdi._mask_right = charset._strRight;
	gdi._mask_top = charset._strTop;
	gdi._mask_bottom = charset._strBottom;
}

void Scumm::drawDescString(byte *msg)
{
	byte c, *buf, buffer[256];

	buf = _msgPtrToAdd = buffer;
	addMessageToStack(msg);

	charset._bufPos = 0;
	charset._top = _string[0].ypos;
	charset._left = _string[0].xpos;
	charset._left2 = _string[0].xpos;
	charset._right = _realWidth - 1;
	charset._xpos2 = _string[0].xpos;
	charset._ypos2 = _string[0].ypos;
	charset._curId = _string[0].charset;
	charset._center = _string[0].center;
	charset._color = _string[0].color;

	// Center text
	charset._xpos2 -= charset.getStringWidth(0, buffer, 0) >> 1;
	if (charset._xpos2 < 0)
		charset._xpos2 = 0;

	charset._disableOffsX = charset._unk12 = 1;
	_bkColor = 0;
	_talkDelay = 1;

	restoreCharsetBg();

	do {
		c = *buf++;
		if (c == 0) {
			_haveMsg = 1;
			break;
		}
		if (c != 0xFF) {
			charset._left = charset._xpos2;
			charset._top = charset._ypos2;
			charset.printChar(c);
			charset._xpos2 = charset._left;
			charset._ypos2 = charset._top;
			continue;
		}
	} while (1);

	gdi._mask_left = charset._strLeft;
	gdi._mask_right = charset._strRight;
	gdi._mask_top = charset._strTop;
	gdi._mask_bottom = charset._strBottom;
}

void Scumm::drawString(int a)
{
	byte buf[256];
	byte *charsetptr, *space;
	int i;
	byte byte1 = 0, chr;
	uint color;

	_msgPtrToAdd = buf;
	_messagePtr = addMessageToStack(_messagePtr);

	charset._left2 = charset._left = _string[a].xpos;
	charset._top = _string[a].ypos;
	charset._curId = _string[a].charset;
	charset._center = _string[a].center;
	charset._right = _string[a].right;
	charset._color = _string[a].color;
	_bkColor = 0;
	charset._unk12 = 1;
	charset._disableOffsX = 1;

	if (!(_features & GF_OLD256)) {
		charsetptr = getResourceAddress(rtCharset, charset._curId);
		assert(charsetptr);
		charsetptr += 29;
		if (_features & GF_SMALL_HEADER)
			charsetptr -= 12;

		for (i = 0; i < 4; i++)
			if (_features & GF_SMALL_HEADER)
				charset._colorMap[i] = _charsetData[charset._curId][i - 12];
			else
				charset._colorMap[i] = _charsetData[charset._curId][i];

		byte1 = charsetptr[1];
	}

	_msgPtrToAdd = buf;

	/* trim from the right */
	space = NULL;
	while (*_msgPtrToAdd) {
		if (*_msgPtrToAdd == ' ') {
			if (!space)
				space = _msgPtrToAdd;
		} else {
			space = NULL;
		}
		_msgPtrToAdd++;
	}
	if (space)
		*space = '\0';
	if (charset._center) {
		charset._left -= charset.getStringWidth(a, buf, 0) >> 1;
	}

	if (!(_features & GF_AFTER_V7))
		charset._ignoreCharsetMask = 1;


	// In Full Throttle (and other games?), verb text should always mask
	// and never time out. We can't do it blindly for all games, because
	// it causes problem with the FOA intro.

	if (_gameId == GID_FT && a == 4)
		_talkDelay = -1;

	if (!buf[0]) {
		buf[0] = ' ';
		buf[1] = 0;
	}

	for (i = 0; (chr = buf[i++]) != 0;) {
		if (chr == 254)
			chr = 255;
		if (chr == 255) {
			chr = buf[i++];
			switch (chr) {
			case 9:
			case 10:
			case 13:
			case 14:
				i += 2;
				break;
			case 1:
			case 8:
				if (charset._center) {
					charset._left = charset._left2 - charset.getStringWidth(a, buf, i);
				} else {
					charset._left = charset._left2;
				}
				charset._top += byte1;
				break;
			case 12:
				color = buf[i] + (buf[i + 1] << 8);
				i += 2;
				if (color == 0xFF)
					charset._color = _string[a].color;
				else
					charset._color = color;
				break;
			}
		} else {
			if (a == 1 && (_features & GF_AFTER_V6)) {
				if (_string[a].no_talk_anim == 0)
					charset._blitAlso = true;
			}
			if (_features & GF_OLD256)
				charset.printCharOld(chr);
			else
				charset.printChar(chr);
			charset._blitAlso = false;
		}
	}

	charset._ignoreCharsetMask = 0;

	if (a == 0) {
		charset._xpos2 = charset._left;
		charset._ypos2 = charset._top;
	} 

	if (_features & GF_AFTER_V7) {
		charset._hasMask = true;
		if (charset._strLeft < gdi._mask_left)
			gdi._mask_left = charset._strLeft;
		if (charset._strRight > gdi._mask_right)
			gdi._mask_right = charset._strRight;
		if (charset._strTop < gdi._mask_top)
			gdi._mask_top = charset._strTop;
		if (charset._strBottom > gdi._mask_bottom)
			gdi._mask_bottom = charset._strBottom;
	} 
}

byte *Scumm::addMessageToStack(byte *msg)
{
	int num, numorg;
	unsigned char *ptr, chr;

	numorg = num = _numInMsgStack;
	ptr = getResourceAddress(rtTemp, 6);

	if (ptr == NULL)
		error("Message stack not allocated");

	if (msg == NULL) {
		warning("Bad message in addMessageToStack, ignoring");
		return NULL;
	}

while ((ptr[num++] = chr = *msg++) != 0) {
		if (num >= 500)
			error("Message stack overflow");

		if (chr == 0xff) {	// 0xff is an escape character			
			ptr[num++] = chr = *msg++;	// followed by a "command" code 
			if (chr != 1 && chr != 2 && chr != 3 && chr != 8) {
				ptr[num++] = *msg++;	// and some commands are followed by parameters to the functions below
				ptr[num++] = *msg++;	// these are numbers of names, strings, verbs, variables, etc
			}
		}
	}

	_numInMsgStack = num;
	num = numorg;

	while (1) {
		ptr = getResourceAddress(rtTemp, 6);
		chr = ptr[num++];
		if (chr == 0)
			break;
		if (chr == 0xFF) {
			chr = ptr[num++];
			switch (chr) {
			case 4:
				addIntToStack(READ_LE_UINT16(ptr + num));
				num += 2;
				break;
			case 5:
				addVerbToStack(READ_LE_UINT16(ptr + num));
				num += 2;
				break;
			case 6:
				addNameToStack(READ_LE_UINT16(ptr + num));
				num += 2;
				break;
			case 7:
				addStringToStack(READ_LE_UINT16(ptr + num));
				num += 2;
				break;
			case 3:
			case 9:
//#if defined(DOTT)
			case 10:
			case 12:
			case 13:
			case 14:
//#endif
				*_msgPtrToAdd++ = 0xFF;
				*_msgPtrToAdd++ = chr;
				*_msgPtrToAdd++ = ptr[num++];
				*_msgPtrToAdd++ = ptr[num++];
				break;
			default: 
				warning("addMessageToStack(): string escape sequence %d unknown", chr);
				*_msgPtrToAdd++ = 0xFF;
				*_msgPtrToAdd++ = chr;
				break;
			}
		} else {
			if (chr != '@') {
				*_msgPtrToAdd++ = chr;
			}
		}
	}
	*_msgPtrToAdd = 0;
	_numInMsgStack = numorg;

	return msg;
}

void Scumm::addIntToStack(int var)
{
	int num, max;
	byte flag;

	num = readVar(var);
	if (num < 0) {
		*_msgPtrToAdd++ = '-';
		num = -num;
	}

	flag = 0;
	max = 10000;
	do {
		if (num >= max || flag) {
			*_msgPtrToAdd++ = num / max + '0';
			num -= (num / max) * max;
			flag = 1;
		}
		max /= 10;
		if (max == 1)
			flag = 1;
	} while (max);
}

void Scumm::addVerbToStack(int var)
{
	int num, i;

	num = readVar(var);
	if (num) {
		for (i = 1; i < _maxVerbs; i++) {
			if (num == _verbs[i].verbid && !_verbs[i].type && !_verbs[i].saveid) {
				addMessageToStack(getResourceAddress(rtVerb, i));
				break;
			}
		}
	} else {
		addMessageToStack((byte *)"");
	}
}

void Scumm::addNameToStack(int var)
{
	int num;

	num = readVar(var);
	if (num) {
		addMessageToStack(getObjOrActorName(num));
	} else {
		addMessageToStack((byte *)"");
	}
}

void Scumm::addStringToStack(int var)
{
	byte *ptr;

	if (_features & GF_AFTER_V6 || _gameId == GID_INDY3_256)
		var = readVar(var);

	if (var) {
		ptr = getStringAddress(var);
		if (ptr) {
			addMessageToStack(ptr);
			return;
		}
	}
	addMessageToStack((byte *)"");
}

void Scumm::initCharset(int charsetno)
{
	int i;

	if (_features & GF_OLD256)
		charsetno = !charsetno;

	if (_features & GF_SMALL_HEADER)
		loadCharset(charsetno);
	else if (!getResourceAddress(rtCharset, charsetno))
		loadCharset(charsetno);

	_string[0].t_charset = charsetno;
	_string[1].t_charset = charsetno;

	for (i = 0; i < 0x10; i++)
		if (_features & GF_SMALL_HEADER)
			charset._colorMap[i] = _charsetData[charset._curId][i - 12];
		else
			charset._colorMap[i] = _charsetData[charset._curId][i];
}

void CharsetRenderer::printCharOld(int chr)
{																// Loom3 / Zak256
	VirtScreen *vs;
	byte *char_ptr, *dest_ptr;
	unsigned int buffer = 0, mask = 0, x = 0, y = 0;
	unsigned char color;

	_vm->checkRange(_vm->_maxCharsets - 1, 0, _curId, "Printing with bad charset %d");

	if ((vs = _vm->findVirtScreen(_top)) == NULL)
		return;

	if (chr == '@')
		return;

	byte *ptr = _vm->getResourceAddress(rtCharset, _curId) + 29;
	if (_vm->_features & GF_SMALL_HEADER)
		ptr -= 12;

	if (_unk12) {
		_strLeft = _left;
		_strTop = _top;
		_strRight = _left;
		_strBottom = _top;
		_unk12 = 0;
	}
	char_ptr = _vm->getResourceAddress(rtCharset, _curId) + 224 + (chr + 1) * 8;
	dest_ptr = vs->screenPtr + vs->xstart + (_top - vs->topline) * _vm->_realWidth + _left;
	_vm->updateDirtyRect(vs->number, _left, _left + 8, _top - vs->topline, _top - vs->topline + 8, 0);

	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			if ((mask >>= 1) == 0) {
				buffer = *char_ptr++;
				mask = 0x80;
			}
			color = ((buffer & mask) != 0);
			if (color)
				*(dest_ptr + y * _vm->_realWidth + x) = _color;
		}
	}

	// FIXME
	_left += getSpacing(chr, ptr);

	if (_left > _strRight)
		_strRight = _left;

	if (_top + 8 > _strBottom)
		_strBottom = _top + 8;

}


void CharsetRenderer::printChar(int chr)
{
	int d, right;
	VirtScreen *vs;

	_vm->checkRange(_vm->_maxCharsets - 1, 1, _curId, "Printing with bad charset %d");
	
	if ((vs = _vm->findVirtScreen(_top)) == NULL)
		return;

	if (chr == '@')
		return;

	byte *ptr = _vm->getResourceAddress(rtCharset, _curId) + 29;
	if (_vm->_features & GF_SMALL_HEADER)
		ptr -= 12;

	_bpp = _unk2 = *ptr;
	_invNumBits = 8 - _bpp;
	_colorMap[1] = _color;

	_charOffs = READ_LE_UINT32(ptr + chr * 4 + 4);

	if (!_charOffs)
		return;

	assert(_charOffs < 0x10000);

	_charPtr = ptr + _charOffs;

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
		if (d >= 0x80)
			d -= 0x100;
		_offsX = d;
	}

	d = _charPtr[3];
	if (d >= 0x80)
		d -= 0x100;
	_offsY = d;

	_top += _offsY;
	_left += _offsX;

	right = _left + _width;

	if (right > _right + 1 || _left < 0) {
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
	if (_drawTop < 0)
		_drawTop = 0;

	_bottom = _drawTop + _height + _offsY;

	_vm->updateDirtyRect(vs->number, _left, right, _drawTop, _bottom, 0);

	if (vs->number != 0)
		_blitAlso = false;
	if (vs->number == 0 && _blitAlso == 0)
		_hasMask = true;

	_dest_ptr = _backbuff_ptr = vs->screenPtr + vs->xstart + _drawTop * _vm->_realWidth + _left;

	if (_blitAlso) {
		_dest_ptr = _bgbak_ptr = _vm->getResourceAddress(rtBuffer, vs->number + 5)
			+ vs->xstart + _drawTop * _vm->_realWidth + _left;
	}

	_mask_ptr = _vm->getResourceAddress(rtBuffer, 9)
		+ _drawTop * 40 + _left / 8 + _vm->_screenStartStrip;

	_virtScreenHeight = vs->height;
	_charPtr += 4;

	drawBits();

	if (_blitAlso)
		_vm->blit(_backbuff_ptr, _bgbak_ptr, _width, _height);

	_left += _width;
	if (_left > _strRight)
		_strRight = _left;

	if (_top + _height > _strBottom)
		_strBottom = _top + _height;

	_top -= _offsY;
}

void CharsetRenderer::drawBits()
{
	bool usemask;
	byte *dst, *mask, maskmask;
	int y, x;
	int maskpos;
	int color;
	byte numbits, bits;

	usemask = (_vm->_curVirtScreen->number == 0 && _ignoreCharsetMask == 0);

	bits = *_charPtr++;
	numbits = 8;

	dst = _dest_ptr;
	mask = _mask_ptr;
	y = 0;

	for (y = 0; y < _height && y + _drawTop < _virtScreenHeight;) {
		maskmask = revBitMask[_left & 7];
		maskpos = 0;

		for (x = 0; x < _width; x++) {
			color = (bits >> _invNumBits) & 0xFF;
			if (color) {
				if (usemask) {
					mask[maskpos] |= maskmask;
				}
				*dst = _colorMap[color];
			}
			dst++;
			bits <<= _bpp;
			if ((numbits -= _bpp) == 0) {
				bits = *_charPtr++;
				numbits = 8;
			}
			if ((maskmask >>= 1) == 0) {
				maskmask = 0x80;
				maskpos++;
			}
		}
		dst = (_dest_ptr += _vm->_realWidth);
		mask += 40;
		y++;
	}
}

// do spacing for variable width old-style font
int CharsetRenderer::getSpacing(byte chr, byte *charset)
{
	int spacing = 0;
	
	if (_vm->_features & GF_OLD256) {
		spacing = *(charset - 11 + chr);
	} else {
		int offs = READ_LE_UINT32(charset + chr * 4 + 4);
		if (offs) {
			spacing = charset[offs];
			if (charset[offs + 2] >= 0x80) {
				spacing += charset[offs + 2] - 0x100;
			} else {
				spacing += charset[offs + 2];
			}
		}
	}
	
	return spacing;
}

void Scumm::loadLanguageBundle() {
	File file;

	file.open("language.bnd", _gameDataPath);
	if(file.isOpen() == false) {
		_existLanguageFile = false;
		return;
	}
	_languageBuffer = (char*)malloc(file.size());
	file.read(_languageBuffer, file.size());
	file.close();
	_existLanguageFile = true;
}

void Scumm::translateText(char * text, char * trans_buff) {
	if ((_existLanguageFile == true) && (text[0] == '/') && (text[1] != ' ')) {
		char name[20], tmp[20], tmp2[20], num_s[20];
		int32 num, l, j, k, r, pos;
		char enc;

		// copy name from text /..../
		for (l = 0; (l < 20) && (*(text + l + 1) != '.'); l++) {
			name[l] = *(text + l + 1);
		}
		name[l] = 0;
		l++;

		// get number from text /..../
		char number[4];
		number[0] = *(text + l + 1);
		number[1] = *(text + l + 2);
		number[2] = *(text + l + 3);
		number[3] = 0;
		num = atol(number);
		sprintf(num_s, "%ld", num);

		char * buf = _languageBuffer;
		// determine is file encoded
		if (*buf == 'e') {
			enc = 0x13;
			pos = 1;
		} else {
			enc = 0;
			pos = 0;
		}

		for(;;) {
			// search char @
			if (*(buf + pos++) == '@') {
				// copy name after @ to endline
				l = 0;
				do {
					tmp[l++] = *(buf + pos++);
				} while((*(buf + pos) != 0x0d) && (*(buf + pos + 1) != 0x0a) && (l < 19));
				tmp[l] = 0;
				pos += 2;
				// compare 'name' with above name
				if (strcmp(tmp, name) == 0) {
					// get number lines of 'name' after '#'
					l = 0;
					if (*(buf + pos++) == '#') {
						do {
							tmp[l++] = *(buf + pos++);
						} while((*(buf + pos) != 0x0d) && (*(buf + pos + 1) != 0x0a) && (l < 19));
						tmp[l] = 0;
						pos += 2;
						l = atol(tmp);
						// get number of line
						for(r = 0; r < l; r++) {
							j = 0;
							do {
								tmp2[j++] = *(buf + pos++);
							} while(*(buf + pos) != '/');
							tmp2[j] = 0;
							// compare if is right line
							if (strcmp(tmp2, num_s) == 0) {
								k = 0;
								pos++;
								// copy translated text to tran_buffer
								do {
									*(trans_buff + k++) = (*(buf + pos++)) ^ enc;
								} while((*(buf + pos) != 0x0d) && (*(buf + pos + 1) != 0x0a));
								*(trans_buff + k) = 0;
								return;
							}
							// goto next line
							do { 
								pos++;
							}	while((*(buf + pos) != 0x0d) && (*(buf + pos + 1) != 0x0a));
							pos += 2;
						}
					}
				}
			}
		}
	}

	if (text[0] == '/') {
		char *pointer = strchr((char*)text + 1, '/');
		if (pointer != NULL)
			strcpy(trans_buff, pointer + 1);
		else
			strcpy(trans_buff, "");

		return;
	}
	strcpy(trans_buff, text);
}

