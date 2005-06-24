/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"

#include "common/config-manager.h"

#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/intern.h"
#include "scumm/verbs.h"
#include "scumm/sound.h"
#include "scumm/util.h"

namespace Scumm {

void ScummEngine::printString(int m, const byte *msg) {
	switch (m) {
	case 0:
		actorTalk(msg);
		break;
	case 1:
		drawString(1, msg);
		break;
	case 2:
		debugMessage(msg);
		break;
	case 3:
		showMessageDialog(msg);
		break;
	}
}


void ScummEngine::debugMessage(const byte *msg) {
	byte buffer[500];
	convertMessageToString(msg, buffer, sizeof(buffer));

//	if ((_gameId == GID_CMI) && _debugMode) {	// In CMI, debugMessage is used for printDebug output
	if ((buffer[0] != 0xFF) && _debugMode) {
		debug(0, "DEBUG: %s", buffer);
		return;
	}

	if (buffer[0] == 0xFF && buffer[1] == 10) {
		uint32 a, b;
		int channel = 0;

		a = buffer[2] | (buffer[3] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
		b = buffer[10] | (buffer[11] << 8) | (buffer[14] << 16) | (buffer[15] << 24);

		// Sam and Max uses a caching system, printing empty messages
		// and setting VAR_V6_SOUNDMODE beforehand. See patch 609791.
		if (_gameId == GID_SAMNMAX)
			channel = VAR(VAR_V6_SOUNDMODE);

		 if (channel != 2)
			_sound->talkSound(a, b, 1, channel);
	}
}

void ScummEngine::showMessageDialog(const byte *msg) {
	// Original COMI used different code at this point.
	// Seemed to use blastText for the messages
	byte buf[500];

	convertMessageToString(msg, buf, sizeof(buf));

	if (_string[3].color == 0)
		_string[3].color = 4;

	InfoDialog dialog(this, (char*)buf);
	VAR(VAR_KEYPRESS) = runDialog(dialog);
}

void ScummEngine::CHARSET_1() {
	uint32 talk_sound_a = 0;
	uint32 talk_sound_b = 0;
	int i, t, c;
	int frme;
	Actor *a;
	byte *buffer;
	int code = (_heversion >= 80) ? 127 : 64;
	char value[32];

	bool cmi_pos_hack = false;

	if (!_haveMsg)
		return;

	if (!(_features & GF_NEW_CAMERA) && !(_gameId == GID_ZAK256 && getTalkingActor() == 0xFF)) {
		if ((camera._dest.x / 8) != (camera._cur.x / 8) || camera._cur.x != camera._last.x)
			return;
	}

	a = NULL;
	if (getTalkingActor() != 0xFF)
		a = derefActorSafe(getTalkingActor(), "CHARSET_1");

	if (a && _string[0].overhead != 0) {
		int s;

		_string[0].xpos = a->_pos.x - virtscr[0].xstart;
		_string[0].ypos = a->_pos.y - a->getElevation() - _screenTop;

		if (_version <= 5) {

			if (VAR(VAR_V5_TALK_STRING_Y) < 0) {
				s = (a->_scaley * (int)VAR(VAR_V5_TALK_STRING_Y)) / 0xFF;
				_string[0].ypos += (int)(((VAR(VAR_V5_TALK_STRING_Y) - s) / 2) + s);
			} else {
				_string[0].ypos = (int)VAR(VAR_V5_TALK_STRING_Y);
			}

		} else {
			s = a->_scalex * a->_talkPosX / 0xFF;
			_string[0].xpos += ((a->_talkPosX - s) / 2) + s;

			s = a->_scaley * a->_talkPosY / 0xFF;
			_string[0].ypos += ((a->_talkPosY - s) / 2) + s;

			if (_string[0].ypos > _screenHeight - 40)
				_string[0].ypos = _screenHeight - 40;
		}

		if (_string[0].ypos < 1)
			_string[0].ypos = 1;

		if (_string[0].xpos < 80)
			_string[0].xpos = 80;
		if (_string[0].xpos > _screenWidth - 80)
			_string[0].xpos = _screenWidth - 80;
	}

	_charset->_top = _string[0].ypos + _screenTop;
	_charset->_startLeft = _charset->_left = _string[0].xpos;
	_charset->_right = _string[0].right;
	_charset->_center = _string[0].center;
	_charset->setColor(_charsetColor);

	if (a && a->_charset)
		_charset->setCurID(a->_charset);
	else
		_charset->setCurID(_string[0].charset);

	if (_version >= 5)
		for (i = 0; i < 4; i++)
			_charsetColorMap[i] = _charsetData[_charset->getCurID()][i];

	if (_talkDelay)
		return;

	if ((_version <= 7 && _haveMsg == 1) || (_version == 8 && VAR(VAR_HAVE_MSG))) {
		if ((_sound->_sfxMode & 2) == 0)
			stopTalk();
		return;
	}

	if (a && !_string[0].no_talk_anim) {
		a->runActorTalkScript(a->_talkStartFrame);
		_useTalkAnims = true;
	}

	// Always set to 60
	if (_version <= 6)
		_talkDelay = 60;
	else
		_talkDelay = VAR(VAR_DEFAULT_TALK_DELAY);
		
	if (!_keepText) {
		_charset->restoreCharsetBg();
	}

	t = _charset->_right - _string[0].xpos - 1;
	if (_charset->_center) {
		if (t > _charset->_nextLeft)
			t = _charset->_nextLeft;
		t *= 2;
	}

	buffer = _charsetBuffer + _charsetBufPos;

	if (_version > 3)
		_charset->addLinebreaks(0, buffer, 0, t);

	if (_charset->_center) {
		_charset->_nextLeft -= _charset->getStringWidth(0, buffer) / 2;
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
		
		// FIXME: This is a workaround for bug #864030: In COMI, some text
		// contains ASCII character 11 = 0xB. It's not quite clear what it is
		// good for; so for now we just ignore it, which seems to match the
		// original engine (BTW, traditionally, this is a 'vertical tab').
		if (c == 0x0B)
			continue;

		if (c == 13) {
		newLine:;
			_charset->_nextLeft = _string[0].xpos;
			if (_charset->_center) {
				_charset->_nextLeft -= _charset->getStringWidth(0, buffer) / 2;
			}
			if (!(_platform == Common::kPlatformFMTowns) && _string[0].height) {
				_charset->_nextTop += _string[0].height;
			} else {
				_charset->_nextTop += _charset->getFontHeight();
			}
			if (_version > 3) {
				// FIXME - is this really needed?
				_charset->_disableOffsX = true;
			}
			continue;
		}

		if (_heversion >= 72 && c == code) {
			c = *buffer++;
			switch (c) {
			case 84:
				i = 0;
				memset(value, 0, 32);
				c = *buffer++;
				while (c != 44) {
					value[i] = c;
					c = *buffer++;
					i++;
				}
				value[i] = 0;
				talk_sound_a = atoi(value);

				i = 0;
				memset(value, 0, 32);
				c = *buffer++;
				while (c != code) {
					value[i] = c;
					c = *buffer++;
					i++;
				}
				value[i] = 0;
				talk_sound_b = atoi(value);

				_sound->startHETalkSound(talk_sound_a);
				break;
			case 104:
				_haveMsg = 0;
				_keepText = true;
				break;
			case 110:
				goto newLine;
			case 116:
				i = 0;
				memset(value, 0, 32);
				c = *buffer++;
				while (c != code) {
					value[i] = c;
					c = *buffer++;
					i++;
				}
				value[i] = 0;
				talk_sound_a = atoi(value);
				talk_sound_b = 0;

				_sound->startHETalkSound(talk_sound_a);
				break;
			case 119:
				if (_haveMsg != 0xFE)
					_haveMsg = 0xFF;
				_keepText = false;
				break;
			default:
				warning("CHARSET_1: invalid code %d", c);
			}
		} else if (c == 0xFE || c == 0xFF) {
			// WORKAROUND to avoid korean code 0xfe treated as charset message code.
			if (c == 0xFE && checkKSCode(*(buffer + 1), c) && _useCJKMode) {
				goto loc_avoid_ks_fe;
			}
			c = *buffer++;
			switch (c) {
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
				if (a) 
					a->startAnimActor(frme);
				break;
			case 10:
				// Note the similarity to the code in debugMessage()
				talk_sound_a = buffer[0] | (buffer[1] << 8) | (buffer[4] << 16) | (buffer[5] << 24);
				talk_sound_b = buffer[8] | (buffer[9] << 8) | (buffer[12] << 16) | (buffer[13] << 24);
				buffer += 14;
	
				if (_heversion >= 60) {
					_sound->startHETalkSound(talk_sound_a);
				} else {
					_sound->talkSound(talk_sound_a, talk_sound_b, 2);
				}

				// Set flag that speech variant exist of this msg.
				// This is actually a hack added by ScummVM; the original did
				// subtitle hiding in some other way. I am not sure exactly
				// how, though.
				// FIXME: This is actually a rather ugly hack, and we should consider
				// replacing it with something better; problem is that _haveMsg is saved,
				// so we need to cope with old save games if we ever change this.
				// And BTW Fingolfin was responsible for this silly bad hack. Stupid me! :-).
				if (_haveMsg == 0xFF)
					_haveMsg = 0xFE;
				break;
			case 12:
				int color;
				color = *buffer++;
				color |= *buffer++ << 8;
				if (color == 0xFF)
					_charset->setColor(_charsetColor);
				else
					_charset->setColor(color);
				break;
			case 13:
				warning("CHARSET_1: Unknown opcode 13 %d", READ_LE_UINT16(buffer));
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
		} else {
loc_avoid_ks_fe:
			_charset->_left = _charset->_nextLeft;
			_charset->_top = _charset->_nextTop;
			if (c & 0x80 && _useCJKMode)
				if (_language == Common::JA_JPN && !checkSJISCode(c)) {
					c = 0x20; //not in S-JIS
				} else {
					c += *buffer++ * 256; //LE
					if (_gameId == GID_CMI) { //HACK: This fixes korean text position in COMI (off by 6 pixel)
						cmi_pos_hack = true;
						_charset->_top += 6;
					}
				}
			if (_version <= 3) {
				_charset->printChar(c);
			} else {
				if ((_imuseDigital && _sound->isSoundRunning(kTalkSoundID)) && (!ConfMan.getBool("subtitles") || VAR(VAR_VOICE_MODE) == 0)) {
					// Special case for games using imuse digital.for sound
				} else if (_heversion >= 60 && !ConfMan.getBool("subtitles") && _sound->isSoundRunning(1)) {
					// Special case for HE games
				} else if ((_gameId == GID_LOOM256) && !ConfMan.getBool("subtitles") && (_sound->pollCD())) {
					// Special case for loomcd, since it only uses CD audio.for sound
				} else if (!ConfMan.getBool("subtitles") && (_haveMsg == 0xFE || _mixer->isSoundHandleActive(_sound->_talkChannelHandle))) {
					// Subtitles are turned off, and there is a voice version
					// of this message -> don't print it. 
				} else
					_charset->printChar(c);
			}
			if (cmi_pos_hack) {
				cmi_pos_hack = false;
				_charset->_top -= 6;
			}

			_charset->_nextLeft = _charset->_left;
			_charset->_nextTop = _charset->_top;
			if (_version <= 2) {
				_talkDelay += _defaultTalkDelay;
				VAR(VAR_CHARCOUNT)++;
			} else
				_talkDelay += (int)VAR(VAR_CHARINC);
			
			// Handle line breaks for V1-V3
			if (_version <= 3 && _charset->_nextLeft > _screenWidth) {
				goto newLine;
			}
		}
	} while (c != 2 && c != 3);

	_charsetBufPos = buffer - _charsetBuffer;

	// TODO Verify this is correct spot
	if (_version == 8)
		VAR(VAR_HAVE_MSG) = (_string[0].no_talk_anim) ? 2 : 1;
}

void ScummEngine::drawString(int a, const byte *msg) {
	byte buf[270];
	byte *space;
	int i, c;
	byte fontHeight = 0;
	uint color;
	int code = (_heversion >= 80) ? 127 : 64;

	bool cmi_pos_hack = false;

	convertMessageToString(msg, buf, sizeof(buf));

	_charset->_top = _string[a].ypos + _screenTop;
	_charset->_startLeft = _charset->_left = _string[a].xpos;
	_charset->_right = _string[a].right;
	_charset->_center = _string[a].center;
	_charset->setColor(_string[a].color);
	_charset->_disableOffsX = _charset->_firstChar = true;
	_charset->setCurID(_string[a].charset);

	if (_version >= 5) {
		for (i = 0; i < 4; i++)
			_charsetColorMap[i] = _charsetData[_charset->getCurID()][i];
	}

	fontHeight = _charset->getFontHeight();

	// trim from the right
	byte *tmp = buf;
	space = NULL;
	while (*tmp) {
		if (*tmp == ' ') {
			if (!space)
				space = tmp;
		} else {
			space = NULL;
		}
		tmp++;
	}
	if (space)
		*space = '\0';
	if (_charset->_center) {
		_charset->_left -= _charset->getStringWidth(a, buf) / 2;
	}

	if (_version < 7)
		_charset->_ignoreCharsetMask = true;


	if (!buf[0]) {
		buf[0] = ' ';
		buf[1] = 0;
	}

	for (i = 0; (c = buf[i++]) != 0;) {
		if (_heversion >= 72 && c == code) {
			c = buf[i++];
			switch (c) {
			case 110:
				if (_charset->_center) {
					_charset->_left = _charset->_startLeft - _charset->getStringWidth(a, buf + i);
				} else {
					_charset->_left = _charset->_startLeft;
				}
				_charset->_top += fontHeight;
				break;
			}
		} else if (c == 0xFE || c == 0xFF) {
			c = buf[i++];
			switch (c) {
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
				if (!(_platform == Common::kPlatformFMTowns) && _string[0].height) {
					_charset->_nextTop += _string[0].height;
				} else {
					_charset->_top += fontHeight;
				}
				break;
			case 12:
				color = buf[i] + (buf[i + 1] << 8);
				i += 2;
				if (color == 0xFF)
					_charset->setColor(_string[a].color);
				else
					_charset->setColor(color);
				break;
			}
		} else {
			if (a == 1 && _version >= 6) {
				// FIXME: The following code is a bit nasty. It is used for the
				// Highway surfing game in Sam&Max; there, _blitAlso is set to
				// true when writing the highscore numbers. It is also in DOTT
				// for parts the intro and for drawing newspaper headlines. It
				// is also used for scores in bowling mini game in fbear and
				// for names in load/save screen of all HE games. Maybe it is
				// also being used in other places.
				//
				// A better name for _blitAlso might be _imprintOnBackground

				if (_string[a].no_talk_anim == false) {
					//warning("Would have set _charset->_blitAlso = true (wanted to print '%c' = %d)", c, c);
					_charset->_blitAlso = true;
				}
			}
			if (c & 0x80 && _useCJKMode) {
				if (_language == Common::JA_JPN && !checkSJISCode(c)) {
					c = 0x20; //not in S-JIS
				} else {
					c += buf[i++] * 256;
					if (_gameId == GID_CMI) {
						cmi_pos_hack = true;
						_charset->_top += 6;
					}
				}
			}
			_charset->printChar(c);
			_charset->_blitAlso = false;

			if (cmi_pos_hack) {
				cmi_pos_hack = false;
				_charset->_top -= 6;
			}
		}
	}

	_charset->_ignoreCharsetMask = false;

	if (a == 0) {
		_charset->_nextLeft = _charset->_left;
		_charset->_nextTop = _charset->_top;
	}

	_string[a].xpos = _charset->_str.right + 8;	// Indy3: Fixes Grail Diary text positioning
}

int ScummEngine::convertMessageToString(const byte *msg, byte *dst, int dstSize) {
	uint num = 0;
	uint32 val;
	byte chr;
	const byte *src;
	byte *end;
	byte transBuf[384];

	assert(dst);
	end = dst + dstSize;

	if (msg == NULL) {
		warning("Bad message in convertMessageToString, ignoring");
		return 0;
	}

	if (_version >= 7) {
		translateText(msg, transBuf);
		src = transBuf;
	} else {
		src = msg;
	}

	num = 0;

	while (1) {
		chr = src[num++];
		if ((_gameId == GID_FREDDI2) && (src[num - 1] == '(' && src[num] == 'P' && src[num + 1] == 'U')) {
			while (src[num++] != ')');
			continue;
		}
		if ((_features & GF_HE_LOCALIZED) && chr == '[') {
			while (src[num++] != ']');
			continue;
		}

		if (chr == 0)
			break;
		if (chr == 0xFF) {
			chr = src[num++];

			// WORKAROUND for bug #985948, a script bug in Indy3. Apparently,
			// a german 'sz' was encoded incorrectly as 0xFF2E. We replace
			// this by the correct encoding here. See also ScummEngine::resStrLen().
			if (_gameId == GID_INDY3 && chr == 0x2E) {
				*dst++ = 0xE1;
				continue;
			}

			if (chr == 1 || chr == 2 || chr == 3 || chr == 8) {
				// Simply copy these special codes
				*dst++ = 0xFF;
				*dst++ = chr;
			} else {
				val = (_version == 8) ? READ_LE_UINT32(src + num) : READ_LE_UINT16(src + num);
				switch (chr) {
				case 4:
					dst += convertIntMessage(dst, end - dst, val);
					break;
				case 5:
					dst += convertVerbMessage(dst, end - dst, val);
					break;
				case 6:
					dst += convertNameMessage(dst, end - dst, val);
					break;
				case 7:
					dst += convertStringMessage(dst, end - dst, val);
					break;
				case 9:
				case 10:
				case 12:
				case 13:
				case 14:
					// Simply copy these special codes
					*dst++ = 0xFF;
					*dst++ = chr;
					*dst++ = src[num+0];
					*dst++ = src[num+1];
					if (_version == 8) {
						*dst++ = src[num+2];
						*dst++ = src[num+3];
					}
					break;
				default:
					warning("convertMessageToString(): string escape sequence %d unknown", chr);
					break;
				}
				num += (_version == 8) ? 4 : 2;
			}
		} else {
			if (!(chr == '@' && _heversion <= 71)) {
				*dst++ = chr;
			}
		}
	
		// Check for a buffer overflow
		if (dst >= end)
			error("convertMessageToString: buffer overflow!");
	}
	*dst = 0;

	return dstSize - (end - dst);
}

int ScummEngine::convertIntMessage(byte *dst, int dstSize, int var) {
	int num;

	num = readVar(var);
	return snprintf((char *)dst, dstSize, "%d", num);
}

int ScummEngine::convertVerbMessage(byte *dst, int dstSize, int var) {
	int num, k;

	num = readVar(var);
	if (num) {
		for (k = 1; k < _numVerbs; k++) {
			if (num == _verbs[k].verbid && !_verbs[k].type && !_verbs[k].saveid) {
				const byte *ptr = getResourceAddress(rtVerb, k);
				return convertMessageToString(ptr, dst, dstSize);
			}
		}
	}
	return 0;
}

int ScummEngine::convertNameMessage(byte *dst, int dstSize, int var) {
	int num;

	num = readVar(var);
	if (num) {
		const byte *ptr = getObjOrActorName(num);
		if (ptr) {
			return convertMessageToString(ptr, dst, dstSize);
		}
	}
	return 0;
}

int ScummEngine::convertStringMessage(byte *dst, int dstSize, int var) {
	const byte *ptr;

	if (_version <= 2) {
		byte chr;
		int i = 0;
		while ((chr = (byte)_scummVars[var++])) {
			if (chr != '@') {
				*dst++ = chr;
				i++;
			}
		}

		return i;
	}

	if (_version == 3 || (_version >= 6 && _heversion < 72))
		var = readVar(var);

	if (var) {
		ptr = getStringAddress(var);
		if (ptr) {
			return convertMessageToString(ptr, dst, dstSize);
		}
	}
	return 0;
}

#ifndef DISABLE_HE
void ScummEngine_v80he::initCharset(int charsetno) {
	ScummEngine::initCharset(charsetno);
	VAR(VAR_CURRENT_CHARSET) = charsetno;
}
#endif

void ScummEngine::initCharset(int charsetno) {
	int i;

	if (_gameId == GID_FT) {
		if (!res.isResourceLoaded(rtCharset, charsetno))
			loadCharset(charsetno);
	} else {
		if (!getResourceAddress(rtCharset, charsetno))
			loadCharset(charsetno);
	}

	_string[0]._default.charset = charsetno;
	_string[1]._default.charset = charsetno;

	for (i = 0; i < 16; i++)
		_charsetColorMap[i] = _charsetData[charsetno][i];

}

void ScummEngine_v6::enqueueText(const byte *text, int x, int y, byte color, byte charset, bool center) {
	BlastText &bt = _blastTextQueue[_blastTextQueuePos++];
	assert(_blastTextQueuePos <= ARRAYSIZE(_blastTextQueue));

	convertMessageToString(text, bt.text, sizeof(bt.text));
	bt.xpos = x;
	bt.ypos = y;
	bt.color = color;
	bt.charset = charset;
	bt.center = center;
}

void ScummEngine_v6::drawBlastTexts() {
	byte *buf;
	int c;
	int i;

	_charset->_ignoreCharsetMask = true;
	for (i = 0; i < _blastTextQueuePos; i++) {

		buf = _blastTextQueue[i].text;

		_charset->_top = _blastTextQueue[i].ypos + _screenTop;
		_charset->_right = _screenWidth - 1;
		_charset->_center = _blastTextQueue[i].center;
		_charset->setColor(_blastTextQueue[i].color);
		_charset->_disableOffsX = _charset->_firstChar = true;
		_charset->setCurID(_blastTextQueue[i].charset);

		do {
			_charset->_left = _blastTextQueue[i].xpos;

			// Center text if necessary
			if (_charset->_center) {
				_charset->_left -= _charset->getStringWidth(0, buf) / 2;
				if (_charset->_left < 0)
					_charset->_left = 0;
			}
	
			do {
				c = *buf++;
				if (c != 0 && c != 0xFF && c != '\n') {
					if (c & 0x80 && _useCJKMode) {
						if (_language == Common::JA_JPN && !checkSJISCode(c)) {
							c = 0x20; //not in S-JIS
						} else {
							c += *buf++ * 256;
						}
					}
					_charset->printChar(c);
				}
			} while (c && c != '\n');

			_charset->_top += _charset->getFontHeight();
		} while (c);

		_blastTextQueue[i].rect = _charset->_str;
	}
	_charset->_ignoreCharsetMask = false;
}

void ScummEngine_v6::removeBlastTexts() {
	int i;

	for (i = 0; i < _blastTextQueuePos; i++) {
		restoreBG(_blastTextQueue[i].rect);
	}
	_blastTextQueuePos = 0;
}

#ifndef DISABLE_SCUMM_7_8
int indexCompare(const void *p1, const void *p2) {
	const LangIndexNode *i1 = (const LangIndexNode *) p1;
	const LangIndexNode *i2 = (const LangIndexNode *) p2;

	return strcmp(i1->tag, i2->tag);
}

// Create an index of the language file.
void ScummEngine_v7::loadLanguageBundle() {
	ScummFile file;
	int32 size;

	if (_gameId == GID_DIG) {
		openFile(file, "language.bnd");
	} else if (_gameId == GID_CMI) {
		openFile(file, "language.tab");
	} else {
		return;
	}
	if (file.isOpen() == false) {
		_existLanguageFile = false;
		return;
	}
	
	_existLanguageFile = true;

	size = file.size();
	_languageBuffer = (char *)calloc(1, size+1);
	file.read(_languageBuffer, size);
	file.close();

	int32 i;
	char *ptr = _languageBuffer;

	// Count the number of lines in the language file.
	for (_languageIndexSize = 0; ; _languageIndexSize++) {
		ptr = strpbrk(ptr, "\n\r");
		if (ptr == NULL)
			break;
		while (*ptr == '\n' || *ptr == '\r')
			ptr++;
	}

	// Fill the language file index. This is just an array of
	// tags and offsets. I did consider using a balanced tree
	// instead, but the extra overhead in the node structure would
	// easily have doubled the memory consumption of the index.
	// And anyway, using qsort + bsearch gives us the exact same
	// O(log(n)) access time anyway ;-).

	_languageIndex = (LangIndexNode *)calloc(_languageIndexSize, sizeof(LangIndexNode));

	ptr = _languageBuffer;

	if (_gameId == GID_DIG) {
		int lineCount = _languageIndexSize;
		const char *baseTag = "";
		byte enc = 0;	// Initially assume the language file is not encoded
	
		// We'll determine the real index size as we go.
		_languageIndexSize = 0;
		for (i = 0; i < lineCount; i++) {
			if (*ptr == '!') {
				// Don't know what a line with '!' means, just ignore it
			} else if (*ptr == 'e') {
				// File is encoded!
				enc = 0x13;
			} else if (*ptr == '@') {
				// A new 'base tag'
				baseTag = ptr + 1;
			} else if (*ptr == '#') {
				// Number of subtags following a given basetag. We don't need that
				// information so we just skip it
			} else if (isdigit(*ptr)) {
				int idx = 0;
				// A number (up to three digits)...
				while (isdigit(*ptr)) {
					idx = idx * 10 + (*ptr - '0');
					ptr++;
				}
	
				// ...followed by a slash...
				assert(*ptr == '/');
				ptr++;
	
				// ...and then the translated message, possibly encoded
				_languageIndex[_languageIndexSize].offset = ptr - _languageBuffer;
	
				// Decode string if necessary.
				if (enc) {
					while (*ptr != '\n' && *ptr != '\r')
						*ptr++ ^= enc;
				}
	
				// The tag is the basetag, followed by a dot and then the index
				sprintf(_languageIndex[_languageIndexSize].tag, "%s.%03d", baseTag, idx);
	
				// That was another index entry
				_languageIndexSize++;
			} else {
				error("Unknwon languag.bnd entry found: '%s'\n", ptr);
			}

			// Skip over newlines (and turn them into null bytes)
			ptr = strpbrk(ptr, "\n\r");
			if (ptr == NULL)
				break;
			while (*ptr == '\n' || *ptr == '\r')
				*ptr++ = 0;
		}
	} else {
		for (i = 0; i < _languageIndexSize; i++) {
			// First 8 chars in the line give the string ID / 'tag'
			int j;
			for (j = 0; j < 8 && !isspace(*ptr); j++, ptr++)
				_languageIndex[i].tag[j] = toupper(*ptr);
			_languageIndex[i].tag[j] = 0;
	
			// After that follows a single space which we skip
			assert(isspace(*ptr));
			ptr++;

			// Then comes the translated string: we record an offset to that.
			_languageIndex[i].offset = ptr - _languageBuffer;
	
			// Skip over newlines (and turn them into null bytes)
			ptr = strpbrk(ptr, "\n\r");
			if (ptr == NULL)
				break;
			while (*ptr == '\n' || *ptr == '\r')
				*ptr++ = 0;

			// Convert '\n' code to a newline. See also bug #902415.
			char *src, *dst;
			src = dst = _languageBuffer + _languageIndex[i].offset;
			while (*src) {
				if (src[0] == '\\' && src[1] == 'n') {
					*dst++ = '\n';
					src += 2;
				} else {
					*dst++ = *src++;
				}
			}
			*dst = 0;
		}
	}

	// Sort the index nodes. We'll later use bsearch on it, which is just as efficient
	// as using a binary tree, speed wise.
	qsort(_languageIndex, _languageIndexSize, sizeof(LangIndexNode), indexCompare);
}

void ScummEngine_v7::playSpeech(const byte *ptr) {
	if ((_gameId == GID_DIG || _gameId == GID_CMI) && ptr[0]) {
		char pointer[20];
		strcpy(pointer, (const char *)ptr);

		// Play speech
		if (!(_features & GF_DEMO) && (_gameId == GID_CMI)) // CMI demo does not have .IMX for voice
			strcat(pointer, ".IMX");

		_sound->stopTalkSound();
		_imuseDigital->stopSound(kTalkSoundID);
		_imuseDigital->startVoice(kTalkSoundID, pointer);
		_sound->talkSound(0, 0, 2);
	}
}

void ScummEngine_v7::translateText(const byte *text, byte *trans_buff) {
	LangIndexNode target;
	LangIndexNode *found = NULL;
	int i;
	
	trans_buff[0] = 0;
	_lastStringTag[0] = 0;

	// WORKAROUND for bug #1172655.
	if (_gameId == GID_DIG && text[0] != '/') {
		if (!strcmp((const char *)text, "faint light"))
			text = (const byte *)"/NEW.007/faint light";
		else if (!strcmp((const char *)text, "glowing crystal"))
			text = (const byte *)"/NEW.008/glowing crystal";
		else if (!strcmp((const char *)text, "glowing crystals"))
			text = (const byte *)"/NEW.009/glowing crystals";
		else if (!strcmp((const char *)text, "pit"))
			text = (const byte *)"/NEW.010/pit";
		else if (!strcmp((const char *)text, "You wish."))
			text = (const byte *)"/NEW.011/You wish.";
		else if (!strcmp((const char *)text, "In your dreams."))
			text = (const byte *)"/NEW.012/In your dreams";
		else if (!strcmp((const char *)text, "left"))
			text = (const byte *)"/CATHPLAT.068/left";
		else if (!strcmp((const char *)text, "right"))
			text = (const byte *)"/CATHPLAT.070/right";
		else if (!strcmp((const char *)text, "right"))
			text = (const byte *)"/CATHPLAT.067/top";
		else if (!strcmp((const char *)text, "exit"))
			text = (const byte *)"/SKY.008/exit";
		else if (!strcmp((const char *)text, "unattached lens"))
			text = (const byte *)"/NEW.013/unattached lens";
		else if (!strcmp((const char *)text, "lens slot"))
			text = (const byte *)"/NEW.014/lens slot";
	}

	
	if (_version >= 7 && text[0] == '/') {
		// Extract the string tag from the text: /..../
		for (i = 0; (i < 12) && (text[i + 1] != '/'); i++)
			_lastStringTag[i] = target.tag[i] = toupper(text[i + 1]);
		_lastStringTag[i] = target.tag[i] = 0;
		text += i + 2;

		// If a language file was loaded, try to find a translated version
		// by doing a lookup on the string tag.
		if (_existLanguageFile) {
			// HACK: These are used for the object line in COMI when
			// using one object on another. I don't know if the
			// text in the language file is a placeholder or if
			// we're supposed to use it, but at least in the
			// English version things will work so much better if
			// we can't find translations for these.
			
			if (*text && strcmp(target.tag, "PU_M001") != 0 && strcmp(target.tag, "PU_M002") != 0)
				found = (LangIndexNode *)bsearch(&target, _languageIndex, _languageIndexSize, sizeof(LangIndexNode), indexCompare);
		}
	}

	if (found != NULL) {
		strcpy((char *)trans_buff, _languageBuffer + found->offset);

		if ((_gameId == GID_DIG) && !(_features & GF_DEMO)) {
			// Replace any '%___' by the corresponding special codes in the source text
			const byte *src = text;
			char *dst = (char *)trans_buff;

			while ((dst = strstr(dst, "%___"))) {
				// Search for a special code in the message.
				while (*src && *src != 0xFF) {
					src++;
				}
				
				// Replace the %___ by the special code. Luckily, we can do
				// that in-place.
				if (*src == 0xFF) {
					memcpy(dst, src, 4);
					src += 4;
					dst += 4;
				} else
					break;
			}
		}
	} else {
		// Default: just copy the string
		memcpy(trans_buff, text, resStrLen(text) + 1);
	}
}

#endif

void ScummEngine::translateText(const byte *text, byte *trans_buff) {
	// Default: just copy the string
	memcpy(trans_buff, text, resStrLen(text) + 1);
}

} // End of namespace Scumm
