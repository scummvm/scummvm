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
#include "charset.h"
#include "dialogs.h"
#include "verbs.h"
#include "scumm/sound.h"

void Scumm::unkMessage1() {
	byte buffer[100];
	_msgPtrToAdd = buffer;
	_messagePtr = addMessageToStack(_messagePtr);

	if ((_gameId == GID_CMI) && _debugMode) {	// In CMI, unkMessage1 is used for printDebug output
		warning("%s", buffer);
		return;
	}

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

void Scumm::unkMessage2() {
	byte buf[100], *tmp;

	_msgPtrToAdd = buf;
	tmp = _messagePtr = addMessageToStack(_messagePtr);

	if (_string[3].color == 0)
		_string[3].color = 4;

	InfoDialog* dialog = new InfoDialog(_newgui, this, (char*)buf);
	// FIXME: I know this is the right thing to do for MI1 and MI2. For
	// all other games it's just a guess.
	_vars[VAR_KEYPRESS] = runDialog (dialog);
	delete dialog;

	_messagePtr = tmp;
}

void Scumm::CHARSET_1() {
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
				_string[0].ypos = (int)_vars[VAR_V5_TALK_STRING_Y];
			}
			if (_string[0].ypos < 1)
				_string[0].ypos = 1;

			if (_string[0].xpos < 80)
				_string[0].xpos = 80;
			if (_string[0].xpos > _realWidth - 80)
				_string[0].xpos = _realWidth - 80;
		} else {
			s = a->scaley * a->talkPosX / 0xFF;
			_string[0].ypos = ((a->talkPosX - s) >> 1) + s - a->elevation + a->y;
			if (_string[0].ypos < 1)
				_string[0].ypos = 1;

			if (_string[0].ypos < camera._cur.y - (_realHeight / 2))
				_string[0].ypos = camera._cur.y - (_realHeight / 2);

			s = a->scalex * a->talkPosY / 0xFF;
			_string[0].xpos = ((a->talkPosY - s) >> 1) + s + a->x - camera._cur.x + (_realWidth / 2);
			if (_string[0].xpos < 80)
				_string[0].xpos = 80;
			if (_string[0].xpos > _realWidth - 80)
				_string[0].xpos = _realWidth - 80;
		}
	}

	_charset->_top = _string[0].ypos;
	_charset->_startLeft = _charset->_left = _string[0].xpos;

	if (a && a->charset)
		_charset->setCurID(a->charset);
	else
		_charset->setCurID(_string[0].charset);

	_charset->_center = _string[0].center;
	_charset->_right = _string[0].right;
	_charset->_color = _charsetColor;

	if (!(_features & GF_OLD256))	// FIXME
		for (i = 0; i < 4; i++)
			_charsetColorMap[i] = _charsetData[_charset->getCurID()][i];

	if (_keepText) {
		_charset->_strLeft = gdi._mask_left;
		_charset->_strRight = gdi._mask_right;
		_charset->_strTop = gdi._mask_top;
		_charset->_strBottom = gdi._mask_bottom;
	}

	if (_talkDelay)
		return;

	if (_haveMsg != 0xFF && _haveMsg != 0xFE) {
		// FIXME: DIG and CMI never set sfxMode or any actor talk data...
		// This hack will force the backup cutoff system to be used instead,
		// unless the talkChannel is null (eg, this string has no sound attached)
		if ((_gameId == GID_CMI || _gameId == GID_DIG) && (_sound->_talkChannel > 0))
			return;

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
	}

	t = _charset->_right - _string[0].xpos - 1;
	if (_charset->_center) {
		if (t > _charset->_nextLeft)
			t = _charset->_nextLeft;
		t <<= 1;
	}

	buffer = _charsetBuffer + _charsetBufPos;
	_charset->addLinebreaks(0, buffer, 0, t);

	if (_charset->_center) {
		_charset->_nextLeft -= _charset->getStringWidth(0, buffer) >> 1;
		if (_charset->_nextLeft < 0)
			_charset->_nextLeft = 0;
	}

	_charset->_disableOffsX = _charset->_firstChar = !_keepText;

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
				_charset->_nextTop = 8;
				_charset->_nextLeft = 0;
				continue;
			} else {
				_charset->_nextLeft = _string[0].xpos;
				if (_charset->_center) {
					_charset->_nextLeft -= _charset->getStringWidth(0, buffer) >> 1;
				}
				_charset->_nextTop += _charset->getFontHeight();
				_charset->_disableOffsX = true;
				continue;
			}
		}

		if (c == 0xFE)
			c = 0xFF;

		if (c != 0xFF) {
			_charset->_left = _charset->_nextLeft;
			_charset->_top = _charset->_nextTop;
			if (_features & GF_OLD256) {
				_charset->printChar(c);
			} else if (_features & GF_AFTER_V6) {
				if (!_noSubtitles || (_haveMsg != 0xFE && _haveMsg != 0xFF))
					_charset->printChar(c);
			} else {
				if (!_noSubtitles || _haveMsg != 0xFE)
					_charset->printChar(c);
			}

			_charset->_nextLeft = _charset->_left;
			_charset->_nextTop = _charset->_top;
			_talkDelay += (int)_vars[VAR_CHARINC];
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
				_charset->_color = _charsetColor;
			else
				_charset->_color = color;
			break;
		case 13:
			buffer += 2;
			break;
		case 14: {
			int oldy = _charset->getFontHeight();

			_charset->setCurID(*buffer++);
			buffer += 2;
			for (i = 0; i < 4; i++)
				_charsetColorMap[i] = _charsetData[_charset->getCurID()][i];
			_charset->_nextTop -= _charset->getFontHeight() - oldy;
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

	_charsetBufPos = buffer - _charsetBuffer;

	gdi._mask_left = _charset->_strLeft;
	gdi._mask_right = _charset->_strRight;
	gdi._mask_top = _charset->_strTop;
	gdi._mask_bottom = _charset->_strBottom;
}

void Scumm::drawDescString(byte *msg) {
	byte c, *buf, buffer[256];

	buf = _msgPtrToAdd = buffer;
	addMessageToStack(msg);

	_charsetBufPos = 0;
	_string[0].ypos += camera._cur.y - (_realHeight / 2);
	_charset->_top = _string[0].ypos;
	_charset->_startLeft = _charset->_left = _string[0].xpos;
	_charset->_right = _realWidth - 1;
	_charset->_center = _string[0].center;
	_charset->_color = _string[0].color;
	_charset->_disableOffsX = _charset->_firstChar = true;
	_charset->setCurID(_string[0].charset);
	_charset->_nextLeft = _string[0].xpos;
	_charset->_nextTop = _string[0].ypos;

	// Center text
	_charset->_nextLeft -= _charset->getStringWidth(0, buffer) >> 1;
	if (_charset->_nextLeft < 0)
		_charset->_nextLeft = 0;

	_talkDelay = 1;

	if (_string[0].ypos + _charset->getFontHeight() > 0)
		restoreBG(0, _string[0].ypos, _realWidth - 1, _string[0].ypos + _charset->getFontHeight());

	_charset->_nextLeft = _string[0].xpos;
	_charset->_nextTop = _string[0].ypos;

	do {
		c = *buf++;
		if (c != 0 && c != 0xFF) {
			_charset->_left = _charset->_nextLeft;
			_charset->_top = _charset->_nextTop;
			_charset->printChar(c);
			_charset->_nextLeft = _charset->_left;
			_charset->_nextTop = _charset->_top;
		}
	} while (c);
	_haveMsg = 1;

	// hack: more 8 pixels at width redraw before and after text
	// for proper description redraw while scrolling room
	gdi._mask_left = _charset->_strLeft - 8;
	gdi._mask_right = _charset->_strRight + 8;
	gdi._mask_top = _charset->_strTop;
	gdi._mask_bottom = _charset->_strBottom;
}

void Scumm::drawString(int a) {
	byte buf[256];
	byte *space;
	int i;
	byte fontHeight = 0, chr;
	uint color;

	_msgPtrToAdd = buf;
	_messagePtr = addMessageToStack(_messagePtr);

	_charset->_top = _string[a].ypos;
	_charset->_startLeft = _charset->_left = _string[a].xpos;
	_charset->_right = _string[a].right;
	_charset->_center = _string[a].center;
	_charset->_color = _string[a].color;
	_charset->_disableOffsX = _charset->_firstChar = true;
	_charset->setCurID(_string[a].charset);

	if (!(_features & GF_OLD256)) {
		for (i = 0; i < 4; i++)
			_charsetColorMap[i] = _charsetData[_charset->getCurID()][i];

		fontHeight = _charset->getFontHeight();
	}

	_msgPtrToAdd = buf;

	// trim from the right
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
	if (_charset->_center) {
		_charset->_left -= _charset->getStringWidth(a, buf) >> 1;
	}

	if (!(_features & GF_AFTER_V7))
		_charset->_ignoreCharsetMask = true;


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
		if (chr == 0xFE || chr == 0xFF) {
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
				if (_charset->_center) {
					_charset->_left = _charset->_startLeft - _charset->getStringWidth(a, buf + i);
				} else {
					_charset->_left = _charset->_startLeft;
				}
				_charset->_top += fontHeight;
				break;
			case 12:
				color = buf[i] + (buf[i + 1] << 8);
				i += 2;
				if (color == 0xFF)
					_charset->_color = _string[a].color;
				else
					_charset->_color = color;
				break;
			}
		} else {
			if (a == 1 && (_features & GF_AFTER_V6)) {
				if (_string[a].no_talk_anim == 0)
					_charset->_blitAlso = true;
			}
			_charset->printChar(chr);
			_charset->_blitAlso = false;
		}
	}

	_charset->_ignoreCharsetMask = false;

	if (a == 0) {
		_charset->_nextLeft = _charset->_left;
		_charset->_nextTop = _charset->_top;
	} 


	_string[a].xpos = _charset->_strRight + 8;	// Indy3: Fixes Grail Diary text positioning

	if (_features & GF_AFTER_V7) {
		_charset->_hasMask = true;
		// FIXME - how is this supposed to ever work, since gdi._mask_left is by default set
		// to -1 to mark it as invalid. Hence this comparision will always leave it at -1,
		// which implies that if the mask was marked invalid, it will always stay so. 
		// That seems odd and not at all to be the intended thing... or is it?
		if (_charset->_strLeft < gdi._mask_left)
			gdi._mask_left = _charset->_strLeft;
		if (_charset->_strRight > gdi._mask_right)
			gdi._mask_right = _charset->_strRight;
		if (_charset->_strTop < gdi._mask_top)
			gdi._mask_top = _charset->_strTop;
		if (_charset->_strBottom > gdi._mask_bottom)
			gdi._mask_bottom = _charset->_strBottom;
	} 
}

byte *Scumm::addMessageToStack(byte *msg) {
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
				if (_features & GF_AFTER_V8) {
					ptr[num++] = *msg++;
					ptr[num++] = *msg++;
				}
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
				if (_features & GF_AFTER_V8) {
					addIntToStack(READ_LE_UINT32(ptr + num));
					num += 4;
				} else {
					addIntToStack(READ_LE_UINT16(ptr + num));
					num += 2;
				}
				break;
			case 5:
				if (_features & GF_AFTER_V8) {
					addVerbToStack(READ_LE_UINT32(ptr + num));
					num += 4;
				} else {
					addVerbToStack(READ_LE_UINT16(ptr + num));
					num += 2;
				}
				break;
			case 6:
				if (_features & GF_AFTER_V8) {
					addNameToStack(READ_LE_UINT32(ptr + num));
					num += 4;
				} else {
					addNameToStack(READ_LE_UINT16(ptr + num));
					num += 2;
				}
				break;
			case 7:
				if (_features & GF_AFTER_V8) {
					addStringToStack(READ_LE_UINT32(ptr + num));
					num += 4;
				} else {
					addStringToStack(READ_LE_UINT16(ptr + num));
					num += 2;
				}
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
				if (_features & GF_AFTER_V8) {
					// FIXME - is this right?!?
					*_msgPtrToAdd++ = ptr[num++];
					*_msgPtrToAdd++ = ptr[num++];
				}
				break;
			default: 
				debug(2, "addMessageToStack(): string escape sequence %d unknown", chr);
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

void Scumm::addIntToStack(int var) {
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
	int num, k;

	num = readVar(var);
	if (num) {
		for (k = 1; k < _maxVerbs; k++) {
			if (num == _verbs[k].verbid && !_verbs[k].type && !_verbs[k].saveid) {
				byte *ptr = getResourceAddress(rtVerb, k);
				if ((_features & GF_AFTER_V8) && (ptr[0] == '/')) {
					char pointer[20];
					int i, j;

					translateText(ptr, _transText);

					for (i = 0, j = 0; (ptr[i] != '/' || j == 0) && j < 19; i++) {
							if (ptr[i] != '/')
							pointer[j++] = ptr[i];
					}
					pointer[j] = 0;
					_sound->_talkChannel = _sound->playBundleSound(pointer);
					addMessageToStack(_transText);
				} else {
					addMessageToStack(ptr);
				}
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
		byte *ptr = getObjOrActorName(num);
		if ((_features & GF_AFTER_V8) && (ptr[0] == '/')) {
			char pointer[20];
			int i, j;

			translateText(ptr, _transText);
			for (i = 0, j = 0; (ptr[i] != '/' || j == 0) && j < 19; i++) {
				if (ptr[i] != '/')
					pointer[j++] = ptr[i];
			}
			pointer[j] = 0;

			addMessageToStack(_transText);
		} else {
			addMessageToStack(ptr);
		}
	} else {
		addMessageToStack((byte *)"");
	}
}

void Scumm::addStringToStack(int var) {
	byte *ptr;

	if (_features & GF_AFTER_V6 || _gameId == GID_INDY3_256)
		var = readVar(var);

	if (var) {
		ptr = getStringAddress(var);
		if (ptr) {
			if ((_features & GF_AFTER_V8) && (ptr[0] == '/')) {
				char pointer[20];
				int i, j;

				translateText(ptr, _transText);

				for (i = 0, j = 0; (ptr[i] != '/' || j == 0) && j < 19; i++) {
						if (ptr[i] != '/')
								pointer[j++] = ptr[i];
				}
				pointer[j] = 0;

				addMessageToStack(_transText);
			} else {
				addMessageToStack(ptr);
			}
		}
	} else
		addMessageToStack((byte *)"");
}

void Scumm::initCharset(int charsetno) {
	int i;

	if (_features & GF_OLD256)
		charsetno = !charsetno;

	if (_features & GF_SMALL_HEADER)
		loadCharset(charsetno);
	else if (!getResourceAddress(rtCharset, charsetno))
		loadCharset(charsetno);

	_string[0].t_charset = charsetno;
	_string[1].t_charset = charsetno;

	for (i = 0; i < 16; i++)
		_charsetColorMap[i] = _charsetData[_charset->getCurID()][i];
}

int indexCompare(const void *p1, const void *p2) {
	struct langIndexNode *i1 = (struct langIndexNode *) p1;
	struct langIndexNode *i2 = (struct langIndexNode *) p2;

	return strcmp(i1->tag, i2->tag);
}

void Scumm::loadLanguageBundle() {
	File file;
	int32 size;

	if (_gameId == GID_DIG) {
		file.open("language.bnd", _gameDataPath);
	} else if (_gameId == GID_CMI) {
		file.open("language.tab", _gameDataPath);
	} else {
		return;
	}

	if(file.isOpen() == false) {
		_existLanguageFile = false;
		return;
	}

	size = file.size();
	_languageBuffer = (char *)calloc(1, size+1);
	file.read(_languageBuffer, size);
	file.close();

	// Create an index of the language file.
	// FIXME: Extend this mechanism to also cover The Dig?

	if (_gameId == GID_CMI) {
		int32 i;
		char *ptr = _languageBuffer;

		// Count the number of lines in the language file.

		_languageStrCount = 0;

		for (;;) {
			ptr = strpbrk(ptr, "\n\r");
			if (ptr == NULL)
				break;
			while (*ptr == '\n' || *ptr == '\r')
				ptr++;
			_languageStrCount++;
		}

		// Fill the language file index. This is just an array of
		// tags and offsets. I did consider using a balanced tree
		// instead, but the extra overhead in the node structure would
		// easily have doubled the memory consumption of the index.

		_languageIndex = (struct langIndexNode *) malloc(_languageStrCount * sizeof(struct langIndexNode));

		ptr = _languageBuffer;

		for (i = 0; i < _languageStrCount; i++) {
			int j;

			for (j = 0; j < 8 && !isspace(ptr[j]); j++)
				_languageIndex[i].tag[j] = toupper(ptr[j]);
			_languageIndex[i].tag[j] = 0;
			ptr += (j + 1);
			_languageIndex[i].offset = ptr - _languageBuffer;
			ptr = strpbrk(ptr, "\n\r");
			if (ptr == NULL)
				break;
			while (*ptr == '\n' || *ptr == '\r')
				ptr++;
		}

		// Conceptually, it may be more elegant to construct the
		// index so that it is sorted, or otherwise ordered, from the
		// start. However, this is less error-prone and likely to be
		// much more optimized than anything I might implement.

		qsort(_languageIndex, _languageStrCount, sizeof(struct langIndexNode), indexCompare);
		free(_languageBuffer);
	}
	
	_existLanguageFile = true;
}

void Scumm::translateText(byte *text, byte *trans_buff) {
	char name[20], tmp[500], tmp2[20], num_s[20], number[4], enc;
	int32 num, l, j, k, r, pos = 0;
	char *buf = _languageBuffer;

	if (_gameId == GID_CMI) {
		if ((text[0] == '/') && (_existLanguageFile == true)) {
			struct langIndexNode target;
			struct langIndexNode *found;

			// copy name from text /..../
			for (l = 0; (l < 8) && (text[l + 1] != '/'); l++)
				target.tag[l] = toupper(text[l + 1]);
			target.tag[l] = 0;

			found = (struct langIndexNode *)bsearch(&target, _languageIndex, _languageStrCount, sizeof(struct langIndexNode), indexCompare);

			if (found != NULL) {
				File file;

				file.open("language.tab", _gameDataPath);
				if (file.isOpen()) {
					byte *ptr = trans_buff;
					byte c;

					file.seek(found->offset, SEEK_SET);
					for (;;) {
						c = file.readByte();
						if (c == 10 || c == 13) {
							*ptr = 0;
							break;
						} else
							*ptr++ = c;
					}
					file.close();
					return;
				} else {
					// Some evil person removed the language file?
					_existLanguageFile = false;
				}
			}
		}
		byte *pointer = (byte *)strchr((char *)text + 1, '/');
		if (pointer != NULL) {
			pointer++;
			memcpy(trans_buff, pointer, resStrLen(pointer) + 1);
			return;
		}
	} else if (_gameId == GID_DIG) {
		if ((text[0] == '/') && (_existLanguageFile == true)) {
			// copy name from text /..../
			for (l = 0; (l < 20) && (text[l + 1] != '.'); l++) {
				name[l] = text[l + 1];
			}
			name[l] = 0;
			l++;
			// get number from text /..../
			number[0] = text[l + 1];
			number[1] = text[l + 2];
			number[2] = text[l + 3];
			number[3] = 0;
			num = atol(number);
			sprintf(num_s, "%d", num);

			// determine is file encoded
			if (buf[pos] == 'e') {
				enc = 0x13;
				pos += 3;
			} else {
				enc = 0;
			}

			// skip translation if flag 'h' exist
			if (buf[pos] == 'h') {
				pos += 3;
				byte *pointer = (byte *)strchr((char *)text + 1, '/');
				if (pointer != NULL)
					memcpy(trans_buff, pointer + 1, resStrLen(pointer + 1) + 1);
				else
					trans_buff[0] = '\0';
				return;
			}

			for(;;) {
				// search char @
				if (buf[pos++] == '@') {
					// copy name after @ to endline
					l = 0;
					do {
						tmp[l++] = buf[pos++];
					} while((buf[pos] != 0x0d) && (buf[pos + 1] != 0x0a) && (l < 19));
					tmp[l] = 0;
					pos += 2;
					// compare 'name' with above name
					if (strcmp(tmp, name) == 0) {
						// get number lines of 'name' after '#'
						l = 0;
						if (buf[pos++] == '#') {
							do {
								tmp[l++] = buf[pos++];
							} while((buf[pos] != 0x0d) && (buf[pos + 1] != 0x0a) && (l < 19));
							tmp[l] = 0;
							pos += 2;
							l = atol(tmp);
							// get number of line
							for(r = 0; r < l; r++) {
								j = 0;
								do {
									tmp2[j++] = buf[pos++];
								} while(buf[pos] != '/');
								tmp2[j] = 0;
								// compare if is right line
								if (strcmp(tmp2, num_s) == 0) {
									k = 0;
									pos++;
									// copy translated text to tran_buffer
									do {
										trans_buff[k++] = (buf[pos++]) ^ enc;
									} while((buf[pos] != 0x0d) && (buf[pos + 1] != 0x0a));
									trans_buff[k] = 0;
									return;
								}
								// goto next line
								do { 
									pos++;
								}	while((buf[pos] != 0x0d) && (buf[pos + 1] != 0x0a));
								pos += 2;
							}
						}
					}
				}
			}
		}
		byte *pointer = (byte *)strchr((char *)text + 1, '/');
		if (pointer != NULL) {
			pointer++;
			l = 0;
			while (*pointer != '/' && *pointer != 0xff && *pointer != 0) {
				trans_buff[l++] = *pointer++;
			}
			trans_buff[l] = '\0';
			return;
		}
	}
	memcpy(trans_buff, text, resStrLen(text) + 1);
}

