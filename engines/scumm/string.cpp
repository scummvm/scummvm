/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */



#include "common/config-manager.h"
#include "audio/mixer.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/imuse_digi/dimuse_engine.h"
#ifdef ENABLE_HE
#include "scumm/he/intern_he.h"
#include "scumm/he/localizer.h"
#endif
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v6.h"
#include "scumm/scumm_v7.h"
#include "scumm/verbs.h"
#include "scumm/he/sound_he.h"

#include "scumm/ks_check.h"

namespace Scumm {



#pragma mark -
#pragma mark --- "High level" message code ---
#pragma mark -


void ScummEngine::printString(int m, const byte *msg) {
	switch (m) {
	case 0:
		// WORKAROUND bug #12734: The script tries to clear the currently
		// displayed message after Rapp gives you the map, but that means
		// you'll never see Guybrush's reaction to finding a map piece.
		//
		// It's a bit hard to pin down the exact case, since it happens
		// at a few different points during the script. We limit it to
		// when the player has the map piece.
		//
		// We have to do it here, because we don't want to delay the
		// animation of Rapp turning back to Ashes.
		if (_game.id == GID_MONKEY2 && _roomResource == 19 &&
			vm.slot[_currentScript].number == 203 &&
			_actorToPrintStrFor == 255 && strcmp((const char *)msg, " ") == 0 &&
			getOwner(200) == VAR(VAR_EGO) && VAR(VAR_HAVE_MSG) &&
			_enableEnhancements) {
			return;
		}

		// WORKAROUND bug #13378: In the German CD version, Sam's
		// reactions to Max beating up the scientist run much too quick
		// for the animation to match. We get around this by slowing
		// down that animation.
 		//
		// In the italian CD version, the whole scene is sped up to
		// keep up with Sam's speech. We compensate for this by slowing
		// down the other animations.
		if (_game.id == GID_SAMNMAX && vm.slot[_currentScript].number == 65 && _enableEnhancements) {
			Actor *a;

			if (_language == Common::DE_DEU && strcmp(_game.variant, "Floppy") != 0) {
				if (memcmp(msg + 16, "Ohh!", 4) == 0) {
					a = derefActorSafe(2, "printString");
					if (a)
						a->setAnimSpeed(3);
				}
			} else if (_language == Common::IT_ITA && strcmp(_game.variant, "Floppy") != 0) {
				if (memcmp(msg + 16, "Ooh.", 4) == 0) {
					a = derefActorSafe(3, "printString");
					if (a)
						a->setAnimSpeed(2);
					a = derefActorSafe(10, "printString");
					if (a)
						a->setAnimSpeed(2);
				}
			}
		}

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
	default:
		break;
	}
}

void ScummEngine::debugMessage(const byte *msg) {
	byte buffer[500];
	convertMessageToString(msg, buffer, sizeof(buffer));

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
		// and setting VAR_V6_SOUNDMODE beforehand. See patch #8051.
		if (_game.id == GID_SAMNMAX)
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

	if (isUsingOriginalGUI()) {
		if (_game.version > 4)
			VAR(VAR_KEYPRESS) = showBannerAndPause(0, -1, (const char *)msg).ascii;
		else
			VAR(VAR_KEYPRESS) = showOldStyleBannerAndPause((const char *)msg, _string[3].color, -1).ascii;
	} else {
		InfoDialog dialog(this, Common::U32String((char *)buf));
		VAR(VAR_KEYPRESS) = runDialog(dialog);
	}

}

#pragma mark -
#pragma mark --- Core message/subtitle code ---
#pragma mark -


bool ScummEngine::handleNextCharsetCode(Actor *a, int *code) {
	uint32 talk_sound_a = 0;
	uint32 talk_sound_b = 0;
	int color, frme, c = 0, oldy;
	bool endLoop = false;
	byte *buffer = _charsetBuffer + _charsetBufPos;
	while (!endLoop) {
		c = *buffer++;
		if (!(c == 0xFF || (_game.version <= 6 && c == 0xFE))) {
			break;
		}
		c = *buffer++;

		if (_newLineCharacter != 0 && c == _newLineCharacter) {
			c = 13;
			break;
		}

		switch (c) {
		case 1:
			c = 13; // new line
			_msgCount = _screenWidth;
			endLoop = true;
			break;
		case 2:
			_haveMsg = 0;
			_keepText = true;
			endLoop = true;
			break;
		case 3:
			_haveMsg = _game.version == 7 && !(_game.id == GID_FT && _game.features & GF_DEMO) ? 1 : 0xFF;
			_keepText = false;
			_msgCount = 0;
			endLoop = true;

			// WORKAROUND bug #13378: Some of the speech is badly
			// synced to the subtitles, particularly in the
			// localized versions. This happens because a single
			// speech line is used for a text that's broken up by
			// one or more embedded "wait" codes. Rather than
			// relying on the calculated talk delay, hard-code
			// better ones.
			if (_game.id == GID_SAMNMAX && _enableEnhancements && isScriptRunning(65)) {
				typedef struct {
					const char *str;
					const int16 talkDelay;
					const byte action;
				} TimingAdjustment;

				TimingAdjustment *adjustments;
				int numAdjustments;

				// We identify the broken up strings that need
				// adjustment by the upcoming text.

				TimingAdjustment timingAdjustmentsEN[] = {
					{ "It's just that",   100, 0 },
					{ "you're TOO nice",  90,  0 },
					{ "^unpredictable.",  170, 0 },
					{ "Yikes!",           120, 0 },
					{ "Huh?",             90,  0 },
					{ "Why do you",       110, 0 },
					{ "Maybe we can",     75,  0 },
					{ "Mind if I drive?", 160, 0 }
				};

				TimingAdjustment timingAdjustmentsDEFloppy[] = {
					{ "Und daf\x81r^",    110, 0 },
					{ "Es ist blo\xe1^",  120, 0 },
					{ "Hey.",             50,  0 },
					{ "Klasse Schlag!",   30,  0 },
					{ "Uiii!",            80,  0 },
					{ "H\x84h?",          60,  0 },
					{ "Kann ich seine",   110, 0 },
					{ "Warum, glaubst",   110, 0 },
					{ "La\xe1 uns von",   220, 0 },
					{ "Vielleicht",       90,  0 },
					{ "Kann ich fahren?", 220, 0 }
				};

				TimingAdjustment timingAdjustmentsDECD[] = {
					{ "Und daf\x81r^",    110, 0 },
					{ "Es ist blo\xe1^",  120, 0 },
					{ "Hey.",             130, 0 },
					{ "Klasse Schlag!",   150, 0 },
					{ "Uiii!",            185, 1 },
					{ "H\x84h?",          150, 0 },
					{ "Kann ich seine",   110, 0 },
					{ "Warum, glaubst",   110, 0 },
					{ "Vielleicht",       90,  0 },
					{ "Kann ich fahren?", 240, 0 }
				};

				TimingAdjustment timingAdjustmentsITFloppy[] = {
					{ "E per questo^",    140, 0 },
					{ "E' che^ecco^",     100, 0 },
					{ "^imprevedibile.",  170, 0 },
					{ "Huh?",             110, 0 },
					{ "Perch\x82 pensi",  90,  0 },
					{ "Andiamocene da",   230, 0 },
					{ "Forse possiamo",   75,  0 },
					{ "Ti dispiace",      160, 0 }
				};

				TimingAdjustment timingAdjustmentsITCD[] = {
					{ "E per questo^",    120, 0 },
					{ "Forse sei",        75,  0 },
					{ "^imprevedibile.",  170, 0 },
					{ "Oh.",              20,  0 },
					{ "Ehi, bel colpo.",  30,  0 },
					{ "Yikes!",           90,  0 },
					{ "Huh?",             50,  0 },
					{ "Posso tenere",     100, 0 },
					{ "Perch\x82 pensi",  120, 0 },
					{ "Andiamocene",      250, 0 },
					{ "Forse possiamo",   90,  0 },
					{ "Ti dispiace",      200, 0 }
				};

				TimingAdjustment timingAdjustmentsFRFloppy[] = {
					{ "Et pour me",       120, 0 },
					{ "C'est que^euh^",   100, 0 },
					{ "vous \x88tes",     65,  0 },
					{ "^impr\x82visible", 170, 0 },
					{ "Pourquoi est-ce",  100, 0 },
					{ "Filons de cet",    190, 0 },
					{ "Nous pourrons",    65,  0 },
					{ "Je peux conduire", 170, 0 },
					{ "Je n'oublierai",   90,  0 }
				};

				TimingAdjustment timingAdjustmentsFRCD[] = {
					{ "Oh.",              85,  0 },
					{ "H\x82, pas mal.",  80,  0 },
					{ "Yiik!",            110, 0 },
					{ "Je peux garder",   130, 0 },
					{ "Pourquoi est-ce",  120, 0 },
					{ "Nous pourrons",    80,  0 },
					{ "Je peux conduire", 220, 0 }
				};

				TimingAdjustment timingAdjustmentsES[] = {
					{ "Y por eso^",       130, 0 },
					{ "es simplemente",   100, 0 },
					{ "eres DEMASIADO",   90,  0 },
					{ "\xa8Hug?",         110, 0 },
					{ "\xa8Por qu\x82",   110, 0 },
					{ "Tal vez podamos",  75,  0 },
					{ "\xa8Te importa",   160, 0 }
				};

				switch (_language) {
				case Common::EN_ANY:
					adjustments = timingAdjustmentsEN;
					numAdjustments = ARRAYSIZE(timingAdjustmentsEN);
					break;
				case Common::DE_DEU:
					if (strcmp(_game.variant, "Floppy") == 0) {
						adjustments = timingAdjustmentsDEFloppy;
						numAdjustments = ARRAYSIZE(timingAdjustmentsDEFloppy);
					} else {
						adjustments = timingAdjustmentsDECD;
						numAdjustments = ARRAYSIZE(timingAdjustmentsDECD);
					}
					break;
				case Common::IT_ITA:
					if (strcmp(_game.variant, "Floppy") == 0) {
						adjustments = timingAdjustmentsITFloppy;
						numAdjustments = ARRAYSIZE(timingAdjustmentsITFloppy);
					} else {
						adjustments = timingAdjustmentsITCD;
						numAdjustments = ARRAYSIZE(timingAdjustmentsITCD);
					}
					break;
				case Common::FR_FRA:
					if (strcmp(_game.variant, "Floppy") == 0) {
						adjustments = timingAdjustmentsFRFloppy;
						numAdjustments = ARRAYSIZE(timingAdjustmentsFRFloppy);
					} else {
						adjustments = timingAdjustmentsFRCD;
						numAdjustments = ARRAYSIZE(timingAdjustmentsFRCD);
					}
					break;
				case Common::ES_ESP:
					adjustments = timingAdjustmentsES;
					numAdjustments = ARRAYSIZE(timingAdjustmentsES);
					break;
				default:
					adjustments = nullptr;
					numAdjustments = 0;
					break;
				}

				byte action = 0;

				for (int i = 0; i < numAdjustments; i++) {
					int len = strlen(adjustments[i].str);
					if (memcmp(buffer, adjustments[i].str, len) == 0) {
						_talkDelay = adjustments[i].talkDelay;
						action = adjustments[i].action;
						break;
					}
				}

				if (_language == Common::DE_DEU) {
					Actor *act;

					switch (action) {
					case 1:
						act = derefActorSafe(2, "handleNextCharsetCode");
						if (act)
							act->setAnimSpeed(2);

						// The actor speaks so slowly that the background
						// animations have run their course. Try to restart
						// them, even though it won't be quite seamless.

						int actors[] = { 3, 10 };

						for (int i = 0; i < ARRAYSIZE(actors); i++) {
							act = derefActorSafe(actors[i], "handleNextCharsetCode");
							if (act) {
								act->startAnimActor(act->_initFrame);
								act->animateActor(249);
							}
						}
						break;
					}
				}
			}

			break;
		case 8:
			// Ignore this code here. Occurs e.g. in MI2 when you
			// talk to the carpenter on scabb island. It works like
			// code 1 (=newline) in verb texts, but is ignored in
			// spoken text (i.e. here). Used for very long verb
			// sentences.
			break;
		case 9:
			frme = buffer[0] | (buffer[1] << 8);
			buffer += 2;
			if (a)
				a->startAnimActor(frme);
			break;
		case 10:
			// Note the similarity to the code in debugMessage()
			talk_sound_a = buffer[0] | (buffer[1] << 8) | (buffer[4] << 16) | (buffer[5] << 24);
			talk_sound_b = buffer[8] | (buffer[9] << 8) | (buffer[12] << 16) | (buffer[13] << 24);
			buffer += 14;
			if (_game.heversion >= 60) {
#ifdef ENABLE_HE
				((SoundHE *)_sound)->startHETalkSound(_localizer ? _localizer->mapTalk(talk_sound_a) : talk_sound_a);
#else
				((SoundHE *)_sound)->startHETalkSound(talk_sound_a);
#endif
			} else {
				_sound->talkSound(talk_sound_a, talk_sound_b, 2);
			}
			_haveActorSpeechMsg = false;
			break;
		case 12:
			color = buffer[0] | (buffer[1] << 8);
			buffer += 2;
			if (color == 0xFF)
				_charset->setColor(_charsetColor);
			else
				_charset->setColor(color);
			break;
		case 13:
			debug(0, "handleNextCharsetCode: Unknown opcode 13 %d", READ_LE_UINT16(buffer));
			buffer += 2;
			break;
		case 14:
			oldy = _charset->getFontHeight();
			_charset->setCurID(*buffer++);
			buffer += 2;
			memcpy(_charsetColorMap, _charsetData[_charset->getCurID()], 4);
			_nextTop -= _charset->getFontHeight() - oldy;
			break;
		default:
			error("handleNextCharsetCode: invalid code %d", c);
		}
	}
	_charsetBufPos = buffer - _charsetBuffer;
	*code = c;
	return (c != 2 && c != 3);
}

#ifdef ENABLE_HE
bool ScummEngine_v72he::handleNextCharsetCode(Actor *a, int *code) {
	const int charsetCode = (_game.heversion >= 80) ? 127 : 64;
	uint32 talk_sound_a = 0;
	//uint32 talk_sound_b = 0;
	int i, c = 0;
	char value[32];
	bool endLoop = false;
	bool endText = false;
	byte *buffer = _charsetBuffer + _charsetBufPos;
	while (!endLoop) {
		c = *buffer++;
		if (c != charsetCode) {
			break;
		}
		c = *buffer++;
		switch (c) {
		case 84:
			i = 0;
			c = *buffer++;
			while (c != 44) {
				value[i] = c;
				c = *buffer++;
				i++;
			}
			value[i] = 0;
			talk_sound_a = atoi(value);
			i = 0;
			c = *buffer++;
			while (c != charsetCode) {
				value[i] = c;
				c = *buffer++;
				i++;
			}
			value[i] = 0;
			//talk_sound_b = atoi(value);
			((SoundHE *)_sound)->startHETalkSound(_localizer ? _localizer->mapTalk(talk_sound_a) : talk_sound_a);
			break;
		case 104:
			_haveMsg = 0;
			_keepText = true;
			endLoop = endText = true;
			break;
		case 110:
			c = 13; // new line
			endLoop = true;
			break;
		case 116:
			i = 0;
			memset(value, 0, sizeof(value));
			c = *buffer++;
			while (c != charsetCode) {
				value[i] = c;
				c = *buffer++;
				i++;
			}
			value[i] = 0;
			talk_sound_a = atoi(value);
			//talk_sound_b = 0;
			((SoundHE *)_sound)->startHETalkSound(_localizer ? _localizer->mapTalk(talk_sound_a) : talk_sound_a);
			break;
		case 119:
			_haveMsg = 0xFF;
			_keepText = false;
			endLoop = endText = true;
			break;
		default:
			error("handleNextCharsetCode: invalid code %d", c);
		}
	}
	_charsetBufPos = buffer - _charsetBuffer;
	*code = c;
	return (endText == 0);
}
#endif

bool ScummEngine::newLine() {
	_nextLeft = _string[0].xpos;
	if (_charset->_center) {
		_nextLeft -= _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos) / 2;
		if (_nextLeft < 0)
			// The commented out part of the next line was meant as a fix for Kanji text glitches in DIG.
			// But these glitches couldn't be reproduced in recent tests. So the underlying issue might
			// have been taken care of in a different manner. And the fix actually caused other text glitches
			// (FT/German, if you look at the sign on the container at game start). After counterchecking
			// the original code it seems that setting _nextLeft to 0 is the right thing to do here.
			_nextLeft = /*_game.version >= 6 ? _string[0].xpos :*/ 0;
	} else if (_isRTL) {
		if (_game.id == GID_MANIAC || ((_game.id == GID_MONKEY || _game.id == GID_MONKEY2) && _charset->getCurID() == 4)) {
			_nextLeft = _screenWidth - _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos) - _nextLeft;
		} else if (_game.id == GID_MONKEY2 && _charset->getCurID() == 5) {
			_nextLeft += _screenWidth - 210 - _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos);
		}
	}
	if (_game.version == 0) {
		return false;
	} else if (!(_game.platform == Common::kPlatformFMTowns) && _string[0].height) {
		_nextTop += _string[0].height;
	} else {
		bool useCJK = _useCJKMode;
		// SCUMM5 FM-Towns doesn't use the height of the ROM font here.
		if (_game.platform == Common::kPlatformFMTowns && _game.version == 5)
			_useCJKMode = false;
		_nextTop += _charset->getFontHeight();
		_useCJKMode = useCJK;
	}
	if (_game.version > 3) {
		// FIXME: is this really needed?
		_charset->_disableOffsX = true;
	}
	return true;
}

void ScummEngine::fakeBidiString(byte *ltext, bool ignoreVerb) const {
	// Provides custom made BiDi mechanism.
	// Reverses texts on each line marked by control characters (considering different control characters used in verbs panel)
	// While preserving original order of numbers (also negative numbers and comma separated)
	int32 ll = 0;
	if ((_game.id == GID_INDY4 && ltext[ll] == 0x7F) || (_game.id == GID_MONKEY2 && ltext[ll] == 0x07)) {
		ll++;
	}
	while (ltext[ll] == 0xFF) {
		ll += 4;
	}

	if (_game.id == GID_MONKEY2 && ltext[0] == 0x07) {
		for (int i = 1; i < ll; i++)
			ltext[i - 1] = ltext[i];
		ltext[--ll] = 0x07;
	}

	int32 ipos = 0;
	int32 start = 0;
	byte *text = ltext + ll;
	byte *current = text;

	int32 bufferSize = 384;
	byte * const buff = (byte *)calloc(sizeof(byte), bufferSize);
	assert(buff);
	byte * const stack = (byte *)calloc(sizeof(byte), bufferSize);
	assert(stack);

	while (1) {
		if (*current == 0x0D || *current == 0 || *current == 0xFF || *current == 0xFE) {

			// ignore the line break for verbs texts
			if (ignoreVerb && *current && (*(current + 1) ==  8)) {
				*(current + 1) = *current;
				*current = 0x08;
				ipos += 2;
				current += 2;
				continue;
			}

			memset(buff, 0, bufferSize);
			memset(stack, 0, bufferSize);

			// Reverse string on current line (between start and ipos).
			int32 sthead = 0;
			byte last = 0;
			for (int32 j = 0; j < ipos; j++) {
				byte *curr = text + start + ipos - j - 1;
				// Special cases to preserve original ordering (numbers).
				if (Common::isDigit(*curr) ||
						(*curr == (byte)',' && j + 1 < ipos && Common::isDigit(*(curr - 1)) && Common::isDigit(last)) ||
						(*curr == (byte)'-' && (j + 1 == ipos || Common::isSpace(*(curr - 1))) && Common::isDigit(last))) {
					++sthead;
					stack[sthead] = *curr;
				} else {
					while (sthead > 0) {
						buff[j - sthead] = stack[sthead];
						--sthead;
					}
					buff[j] = *curr;
				}
				last = *curr;
			}
			while (sthead > 0) {
				buff[ipos - sthead] = stack[sthead];
				--sthead;
			}
			memcpy(text + start, buff, ipos);
			start += ipos + 1;
			ipos = -1;
			if (*current == 0xFF || *current == 0xFE) {
				current++;
				if (*current == 0x03 || *current == 0x02) {
					break;
				}
				if (*current == 0x0A || *current == 0x0C) {
					start += 2;
					current += 2;
				}
				start++;
				ipos++;
				current++;
				continue;
			}
		}
		if (*current) {
			ipos++;
			current++;
			continue;
		}
		break;
	}
	if (!ignoreVerb) {
		if (_game.id == GID_INDY4 && ltext[0] == 0x7F) {
			ltext[start + ipos + ll] = 0x80;
			ltext[start + ipos + ll + 1] = 0;
		} else if (_game.id == GID_MONKEY2) {
			// add non-printable character to end to avoid space trimming
			ltext[start + ipos + ll] = '@';
			ltext[start + ipos + ll + 1] = 0;
		}
	}

	free(buff);
	free(stack);
}

void ScummEngine::CHARSET_1() {
	Actor *a;
	if (_game.heversion >= 70 && _haveMsg == 3) {
		stopTalk();
		return;
	}

	if (!_haveMsg)
		return;

	if (_game.version >= 4 && _game.version <= 6) {
		// Do nothing while the camera is moving
		if ((camera._dest.x / 8) != (camera._cur.x / 8) || camera._cur.x != camera._last.x)
			return;
	}

	a = nullptr;
	if (getTalkingActor() != 0xFF)
		a = derefActorSafe(getTalkingActor(), "CHARSET_1");

	if (a && _string[0].overhead) {
		int s;

		_string[0].xpos = a->getPos().x - _virtscr[kMainVirtScreen].xstart;
		_string[0].ypos = a->getPos().y - a->getElevation() - _screenTop;

		if (_game.version <= 5) {
			if (VAR(VAR_V5_TALK_STRING_Y) < 0) {
				if (_game.version == 4) {
					_string[0].ypos = (int)VAR(VAR_V5_TALK_STRING_Y) + a->getPos().y + a->getElevation();
				} else {
					s = (a->_scaley * (int)VAR(VAR_V5_TALK_STRING_Y)) / 0xFF;
					_string[0].ypos += (int)(((VAR(VAR_V5_TALK_STRING_Y) - s) / 2) + s);
				}
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

	if (_game.version >= 5)
		memcpy(_charsetColorMap, _charsetData[_charset->getCurID()], 4);

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_game.platform == Common::kPlatformFMTowns && (_keepText || _haveMsg == 0xFF))
		memcpy(&_charset->_str, &_curStringRect, sizeof(Common::Rect));
#endif

	if (_talkDelay)
		return;

	if ((_game.version <= 6 && _haveMsg == 1) ||
	    (_game.version == 7 && _haveMsg != 1)) {

		if (_game.heversion >= 60) {
			if (_sound->isSoundRunning(1) == 0)
				stopTalk();
		} else {
			if ((_sound->_sfxMode & 2) == 0)
				stopTalk();
		}
		return;
	}

	// The second check is from LOOM DOS EGA disasm. It prevents weird speech animations
	// with empty strings (bug #990). The same code is present in actorTalk(). The FM-Towns
	// versions don't have such code, but I do not get the weird speech animations either.
	// So apparently it is not needed there.
	if (a && !_string[0].no_talk_anim && !(_game.id == GID_LOOM && _game.platform != Common::kPlatformFMTowns && !_charsetBuffer[_charsetBufPos])) {
		a->runActorTalkScript(a->_talkStartFrame);
		_useTalkAnims = true;
	}

	_talkDelay = (VAR_DEFAULT_TALK_DELAY != 0xFF) ? VAR(VAR_DEFAULT_TALK_DELAY) : 60;

	if (!_keepText) {
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
			if (_game.platform == Common::kPlatformFMTowns)
				towns_restoreCharsetBg();
			else
#endif
				restoreCharsetBg();
		_msgCount = 0;
	} else if (_game.version <= 2) {
		_talkDelay += _msgCount * _defaultTextSpeed;
	}

	if (_game.version > 3) {
		int maxwidth = _charset->_right - _string[0].xpos - 1;
		if (_charset->_center) {
			if (maxwidth > _nextLeft)
				maxwidth = _nextLeft;
			maxwidth *= 2;
		}

		_charset->addLinebreaks(0, _charsetBuffer + _charsetBufPos, 0, maxwidth);
	}

	if (_charset->_center) {
		_nextLeft -= _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos) / 2;
		if (_nextLeft < 0)
			_nextLeft = _game.version >= 6 ? _string[0].xpos : 0;
	} else if (_isRTL) {
		if (_game.id == GID_MANIAC || ((_game.id == GID_MONKEY || _game.id == GID_MONKEY2) && _charset->getCurID() == 4)) {
			_nextLeft = _screenWidth - _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos) - _nextLeft;
		} else if (_game.id == GID_MONKEY2 && _charset->getCurID() == 5) {
			_nextLeft += _screenWidth - 210 - _charset->getStringWidth(0, _charsetBuffer + _charsetBufPos);
		}
	}

	_charset->_disableOffsX = _charset->_firstChar = !_keepText;

	int c = 0;

	if (_isRTL)
		fakeBidiString(_charsetBuffer + _charsetBufPos, true);

	bool createTextBox = (_macScreen && _game.id == GID_INDY3);
	bool drawTextBox = false;

	while (handleNextCharsetCode(a, &c)) {
		if (c == 0) {
			// End of text reached, set _haveMsg accordingly
			_haveMsg = 1;
			_keepText = false;
			_msgCount = 0;
			break;
		}

		if (c == 13) {
			if (!newLine())
				break;
			continue;
		}

		// Handle line overflow for V3. See also bug #2213.
		if (_game.version == 3 && _nextLeft >= _screenWidth) {
			_nextLeft = _screenWidth;
		}
		// Handle line breaks for V1-V2
		if (_game.version <= 2 && _nextLeft >= _screenWidth) {
			if (!newLine())
				break;	// FIXME: Is this necessary? Only would be relevant for v0 games
		}

		_charset->_left = _nextLeft;
		_charset->_top = _nextTop;

		if (createTextBox) {
			if (!_keepText)
				mac_createIndy3TextBox(a);
			createTextBox = false;
			drawTextBox = true;
		}

		if (c & 0x80 && _useCJKMode) {
			if (is2ByteCharacter(_language, c)) {
				byte *buffer = _charsetBuffer + _charsetBufPos;
				c += *buffer++ * 256; //LE
				_charsetBufPos = buffer - _charsetBuffer;
			}
		}
		if (_game.version <= 3) {
			_charset->printChar(c, false);
			_msgCount += 1;
		} else {
			if (_game.features & GF_16BIT_COLOR) {
				// HE games which use sprites for subtitles
			} else if (_game.heversion >= 60 && !ConfMan.getBool("subtitles") && _sound->isSoundRunning(1)) {
				// Special case for HE games
			} else if (_game.id == GID_LOOM && !ConfMan.getBool("subtitles") && (_sound->pollCD())) {
				// Special case for Loom (CD), since it only uses CD audio.for sound
			} else if (!ConfMan.getBool("subtitles") && (!_haveActorSpeechMsg || _mixer->isSoundHandleActive(*_sound->_talkChannelHandle))) {
				// Subtitles are turned off, and there is a voice version
				// of this message -> don't print it.
			} else {
				_charset->printChar(c, false);
			}
		}
		_nextLeft = _charset->_left;
		_nextTop = _charset->_top;

		if (drawTextBox)
			mac_drawIndy3TextBox();

		if (_game.version <= 2) {
			_talkDelay += _defaultTextSpeed;
			VAR(VAR_CHARCOUNT)++;
		} else {
			_talkDelay += (int)VAR(VAR_CHARINC);
		}
	}

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	if (_game.platform == Common::kPlatformFMTowns && (c == 0 || c == 2 || c == 3))
		memcpy(&_curStringRect, &_charset->_str, sizeof(Common::Rect));
#endif
}

void ScummEngine::drawString(int a, const byte *msg) {
	byte buf[270];
	byte *space;
	int i, c;
	byte fontHeight = 0;
	uint color;
	int code = (_game.heversion >= 80) ? 127 : 64;

	// drawString is not used in SCUMM v7 and v8
	assert(_game.version < 7);

	convertMessageToString(msg, buf, sizeof(buf));

	if (_isRTL)
		fakeBidiString(buf, false);

	_charset->_top = _string[a].ypos + _screenTop;
	_charset->_startLeft = _charset->_left = _string[a].xpos;
	_charset->_right = _string[a].right;
	_charset->_center = _string[a].center;
	_charset->setColor(_string[a].color);
	_charset->_disableOffsX = _charset->_firstChar = true;
	_charset->setCurID(_string[a].charset);

	// HACK: Correct positions of text in books in Indy3 Mac.
	// See also bug #8759. Not needed when using the Mac font.
	if (_game.id == GID_INDY3 && _game.platform == Common::kPlatformMacintosh && a == 1 && !_macScreen) {
		if (_currentRoom == 75) {
			// Grail Diary Page 1 (Library)
			if (_charset->_startLeft < 160)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 22;
			else if (_charset->_startLeft < 200)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 10;
		} else if (_currentRoom == 90) {
			// Grail Diary Page 2 (Catacombs - Engravings)
			if (_charset->_startLeft < 160)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 21;
			else if (_charset->_startLeft < 200)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 15;
		} else if (_currentRoom == 69) {
			// Grail Diary Page 3 (Catacombs - Music)
			if (_charset->_startLeft < 160)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 15;
			else if (_charset->_startLeft < 200)
				_charset->_startLeft = _charset->_left = _string[a].xpos - 10;
		} else if (_currentRoom == 74) {
			// Biplane Manual
			_charset->_startLeft = _charset->_left = _string[a].xpos - 35;
		}
	}

	if (_game.version >= 5)
		memcpy(_charsetColorMap, _charsetData[_charset->getCurID()], 4);

	fontHeight = _charset->getFontHeight();

	if (_game.version >= 4) {
		// trim from the right
		byte *tmp = buf;
		space = nullptr;
		while (*tmp) {
			if (*tmp == ' ') {
				if (!space)
					space = tmp;
			} else {
				space = nullptr;
			}
			tmp++;
		}
		if (space)
			*space = '\0';
	}

	if (_charset->_center) {
		_charset->_left -= _charset->getStringWidth(a, buf) / 2;
	} else if (_isRTL && _game.id != GID_SAMNMAX && _game.id != GID_MANIAC) {
		// Ignore INDY4 verbs (but allow dialogue)
		if (_game.id != GID_INDY4 || buf[0] == 127) {
			if (_game.id == GID_INDY4)
				buf[0] = 32;
			_charset->_left = _screenWidth - _charset->_startLeft - _charset->getStringWidth(1, buf);
		}
	}

	if (!buf[0]) {
		if (_game.version >= 5) {
			buf[0] = ' ';
			buf[1] = 0;
		} else {
			_charset->_str.left = _charset->_left;
			_charset->_str.top = _charset->_top;
			_charset->_str.right = _charset->_left;
			_charset->_str.bottom = _charset->_top;
		}
	}

	for (i = 0; (c = buf[i++]) != 0;) {
		if (_game.heversion >= 72 && c == code) {
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
			default:
				break;
			}
		} else if ((c == 0xFF || (_game.version <= 6 && c == 0xFE)) && (_game.heversion <= 71)) {
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
				} else if (_isRTL) {
					_charset->_left = _screenWidth - _charset->_startLeft - _charset->getStringWidth(1, buf + i);
				} else {
					_charset->_left = _charset->_startLeft;
				}
				if (!(_game.platform == Common::kPlatformFMTowns) && _string[0].height) {
					_nextTop += _string[0].height;
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
			default:
				break;
			}
		} else {
			if (a == 1 && _game.version >= 6) {
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
					//debug(0, "Would have set _charset->_blitAlso = true (wanted to print '%c' = %d)", c, c);
					_charset->_blitAlso = true;
				}
			}
			if (c & 0x80 && _useCJKMode) {
				if (is2ByteCharacter(_language, c))
					c += buf[i++] * 256;
			}
			_charset->printChar(c, true);
			_charset->_blitAlso = false;
		}
	}

	if (a == 0) {
		_nextLeft = _charset->_left;
		_nextTop = _charset->_top;
	}

	_string[a].xpos = _charset->_str.right;

	if (_game.heversion >= 60) {
		_string[a]._default.xpos = _string[a].xpos;
		_string[a]._default.ypos = _string[a].ypos;
	}
}

int ScummEngine::convertMessageToString(const byte *msg, byte *dst, int dstSize) {
	uint num = 0;
	uint32 val;
	byte chr;
	byte lastChr = 0;
	const byte *src;
	byte *start, *end;
	byte transBuf[2048];

	assert(dst);
	start = dst;
	end = dst + dstSize;

	if (msg == nullptr) {
		debug(0, "Bad message in convertMessageToString, ignoring");
		return 0;
	}

	if (_game.version >= 7 || isScummvmKorTarget()) {
		translateText(msg, transBuf, sizeof(transBuf));
		src = transBuf;
	} else {
		src = msg;
	}

	num = 0;

	while (1) {
		chr = src[num++];
		if (chr == 0)
			break;

		if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
			// Code for TM character
			if (chr == 0x0F && src[num] == 0x20) {
				*dst++ = 0x5D;
				*dst++ = 0x5E;
				continue;
			// Code for (C) character
			} else if (chr == 0x1C && src[num] == 0x20) {
				*dst++ = 0x3E;
				*dst++ = 0x2A;
				continue;
			// Code for " character
			} else if (chr == 0x19) {
				*dst++ = 0x2F;
				continue;
			}
		}

		if (chr == 0xFF) {
			chr = src[num++];

			// WORKAROUND for bug #1675, a script bug in Indy3. Apparently,
			// a german 'sz' was encoded incorrectly as 0xFF2E. We replace
			// this by the correct encoding here. See also ScummEngine::resStrLen().
			if (_game.id == GID_INDY3 && chr == 0x2E) {
				*dst++ = 0xE1;
				continue;
			}

			// WORKAROUND for bug #2715: Yet another script bug in Indy3.
			// Once more a german 'sz' was encoded incorrectly, but this time
			// they simply encoded it as 0xFF instead of 0xE1. Happens twice
			// in script 71.
			if (_game.id == GID_INDY3 && chr == 0x20 && vm.slot[_currentScript].number == 71) {
				num--;
				*dst++ = 0xE1;
				continue;
			}

			if (chr == 1 || chr == 2 || chr == 3 || chr == 8) {
				// Simply copy these special codes
				*dst++ = 0xFF;
				*dst++ = chr;
			} else {
				val = (_game.version == 8) ? READ_LE_UINT32(src + num) : READ_LE_UINT16(src + num);
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

					// WORKAROUND: In some versions of Sam & Max, talking to Evelyn Morrison
					// while wearing the bigfoot costume misattributes some lines to Max even
					// though Sam is the one actually speaking. For example, the French and
					// German releases use `startAnim(8)` while the English release correctly
					// uses `startAnim(7)` for this.
					if (_game.id == GID_SAMNMAX && _currentRoom == 52 && vm.slot[_currentScript].number == 102 &&
						chr == 9 && readVar(0x8000 + 95) != 0 && (VAR(171) == 997 || VAR(171) == 998) &&
						dst[-2] == 8 && _enableEnhancements) {
						dst[-2] = 7;
					}

					if (_game.version == 8) {
						*dst++ = src[num+2];
						*dst++ = src[num+3];
					}
					break;
				default:
					error("convertMessageToString(): string escape sequence %d unknown", chr);
				}
				num += (_game.version == 8) ? 4 : 2;
			}
		} else {
			if ((chr != '@') || (_game.version >= 7 && is2ByteCharacter(_language, lastChr)) ||
				(_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine && _language == Common::JA_JPN) ||
				(_game.platform == Common::kPlatformFMTowns && _language == Common::JA_JPN && checkSJISCode(lastChr))) {
				*dst++ = chr;
			}
			lastChr = chr;
		}

		// Check for a buffer overflow
		if (dst >= end)
			error("convertMessageToString: buffer overflow");
	}

	// WORKAROUND bug #12249 (occurs also in original): Missing actor animation in German versions of SAMNMAX
	// Adding the missing animation escape sequence while copying the text fixes it.
	if (_game.id == GID_SAMNMAX && _currentRoom == 56 && vm.slot[_currentScript].number == 200 && _language == Common::DE_DEU) {
		// 0xE5E6 is the CD version, 0xE373 is for the floppy version
		if (vm.slot[_currentScript].offs == 0xE5E6 || vm.slot[_currentScript].offs == 0xE373) {
			*dst++ = 0xFF;
			*dst++ = 0x09;
			*dst++ = 0x0E;
			*dst++ = 0x00;
		}
	}

	// WORKAROUND: Russian The Dig pads messages with 03. No idea why
	// it does not work as is with our rendering code, thus fixing it
	// with a workaround.
	if (_game.id == GID_DIG) {
		while (dst > start && *(dst - 1) == 0x03)
			dst--;
	}
	*dst = 0;

	return dstSize - (end - dst);
}

#ifdef ENABLE_HE
int ScummEngine_v72he::convertMessageToString(const byte *msg, byte *dst, int dstSize) {
	uint num = 0;
	byte chr;
	const byte *src;
	byte *end;

	assert(dst);
	end = dst + dstSize;

	if (msg == NULL) {
		debug(0, "Bad message in convertMessageToString, ignoring");
		return 0;
	}

	src = msg;
	num = 0;

	while (1) {
		chr = src[num++];
		if (_game.heversion >= 80 && src[num - 1] == '(' && (src[num] == 'p' || src[num] == 'P')) {
			// Filter out the following prefixes in subtitles
			// (pickup4)
			// (PU1)
			// (PU2)
			while (src[num++] != ')')
				;
			continue;
		}
		if ((_game.features & GF_HE_LOCALIZED) && chr == '[') {
			while (src[num++] != ']')
				;
			continue;
		}

		if (chr == 0)
			break;

		*dst++ = chr;

		// Check for a buffer overflow
		if (dst >= end)
			error("convertMessageToString: buffer overflow");
	}
	*dst = 0;

	return dstSize - (end - dst);
}
#endif

int ScummEngine::convertIntMessage(byte *dst, int dstSize, int var) {
	int num;

	num = readVar(var);
	return snprintf((char *)dst, dstSize, "%d", num);
}

int ScummEngine::convertVerbMessage(byte *dst, int dstSize, int var) {
	int num, k;

	bool isKorVerbGlue = false;

	if (isScummvmKorTarget() && _useCJKMode && var & (1 << 15)) {
		isKorVerbGlue = true;
		var &= ~(1 << 15);
	}

	num = readVar(var);
	if (num) {
		for (k = 1; k < _numVerbs; k++) {
			// Fix ZAK FM-TOWNS bug #1734 by emulating exact (inconsistant?) behavior of the original code
			if (num == _verbs[k].verbid && !_verbs[k].type && (!_verbs[k].saveid || (_game.version == 3 && _game.platform == Common::kPlatformFMTowns))) {
				// Process variation of Korean postpositions
				// Used by Korean fan translated games (monkey1, monkey2)
				if (isKorVerbGlue) {
					static const byte code0380[4] = {0xFF, 0x07, 0x03, 0x80};                                 // "eul/reul"
					static const byte code0480[4] = {0xFF, 0x07, 0x04, 0x80};                                 // "wa/gwa"
					static const byte codeWalkTo[9] = {0xFF, 0x07, 0x03, 0x80, 0x20, 0xC7, 0xE2, 0xC7, 0xD8}; // "eul/reul hyang-hae"
					byte _transText[10];
					memset(_transText, 0, sizeof(_transText));
					// WORKAROUND: MI Korean verb parser
					if (_game.id == GID_MONKEY_VGA) {
						switch (_verbs[k].verbid) {
							case 1:		// Open
							case 2:		// Close
							case 3:		// Give
							case 4:		// Turn on
							case 5:		// Turn off
							case 6:		// Push
							case 7:		// Pull
							case 8:		// Use
							case 9:		// Look at
							case 10:	// Walk to
								memcpy(_transText, codeWalkTo, 9);
								break;
							case 11:	// Pick up
								memcpy(_transText, code0380, 4);
								break;
							case 13:	// Talk to
								memcpy(_transText, code0480, 4);
								break;
						}
						return convertMessageToString(_transText, dst, dstSize);
					}
					// WORKAROUND: MI2 Korean verb parser
					if (_game.id == GID_MONKEY2) {
						if (_verbs[k].verbid <= 11) {
							switch (_verbs[k].verbid) {
								case 2:		// Open
								case 3:		// Close
								case 4:		// Give
								case 5:		// Push
								case 6:		// Pull
								case 7:		// Use
								case 8:		// Look at
								case 9:		// Pick up
									memcpy(_transText, code0380, 4);
									break;
								case 10:	// Talk to
									memcpy(_transText, code0480, 4);
									break;
								case 11:	// Walk to
									memcpy(_transText, codeWalkTo, 9);
									break;
							}
							return convertMessageToString(_transText, dst, dstSize);
						}
					}
				} else {
					const byte *ptr = getResourceAddress(rtVerb, k);
					return convertMessageToString(ptr, dst, dstSize);
				}
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
			int increment = convertMessageToString(ptr, dst, dstSize);
			// Save the final consonant (jongsung) of the last Korean character
			// Used by Korean fan translated games (monkey1, monkey2)
			if (isScummvmKorTarget() && _useCJKMode) {
				_krStrPost = 0;
				int len = resStrLen(ptr);
				if (len >= 2) {
					for (int i = len; i > 1; i--) {
						byte k1 = ptr[i - 2];
						byte k2 = ptr[i - 1];
						if (checkKSCode(k1, k2)) {
							int jongsung = checkJongsung(k1, k2);
							if (jongsung)
								_krStrPost |= 1;
							if (jongsung == 8) // 'ri-eul' final consonant
								_krStrPost |= (1 << 1);
							break;
						}
					}
				}
			}
			return increment;
		}
	}
	return 0;
}

int ScummEngine::convertStringMessage(byte *dst, int dstSize, int var) {
	const byte *ptr;

	if (_game.version <= 2) {
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

	if (_game.version == 3 || (_game.version >= 6 && _game.heversion < 72))
		var = readVar(var);

	// Process variation of Korean postpositions
	// Used by Korean fan translated games (monkey1, monkey2)
	if (isScummvmKorTarget() && _useCJKMode && (var & (1 << 15))) {
		int idx;
		static const byte codeIdx[] = {0x00, 0x00, 0xC0, 0xB8, 0x00, 0x00, 0xC0, 0xCC, 0xB0, 0xA1, 0xC0, 0xCC, 0xB8, 0xA6, 0xC0, 0xBB, 0xBF, 0xCD, 0xB0, 0xFA, 0xB4, 0xC2, 0xC0, 0xBA};

		if ((var & ~(1 << 15)) == 0)
			idx = (2 * (var & ~(1 << 15)) + (_krStrPost & 1) - bool(_krStrPost & 2)) * 2;
		else
			idx = (2 * (var & ~(1 << 15)) + (_krStrPost & 1)) * 2;

		byte _transText[3];
		const byte byteIdx[] = {codeIdx[idx], codeIdx[idx + 1]};

		memset(_transText, 0, sizeof(_transText));
		memcpy(_transText, byteIdx, 2);

		return convertMessageToString(_transText, dst, dstSize);
	} else if (var) {
		ptr = getStringAddress(var);
		if (ptr) {
			int increment = convertMessageToString(ptr, dst, dstSize);
			// Save the final consonant (jongsung) of the last Korean character
			// Used by Korean fan translated games (monkey1, monkey2)
			if (isScummvmKorTarget() && _useCJKMode) {
				_krStrPost = 0;
				for (int i = resStrLen(ptr); i > 1; i--) {
					byte k1 = ptr[i - 2];
					byte k2 = ptr[i - 1];
					if (checkKSCode(k1, k2)) {
						int jongsung = checkJongsung(k1, k2);
						if (jongsung)
							_krStrPost |= 1;
						if (jongsung == 8)	// 'ri-eul' final consonant
							_krStrPost |= (1 << 1);
						break;
					}
				}
			}
			return increment;
		}
	}
	return 0;
}


#pragma mark -
#pragma mark --- Charset initialisation ---
#pragma mark -


#ifdef ENABLE_HE
void ScummEngine_v80he::initCharset(int charsetno) {
	ScummEngine::initCharset(charsetno);
	VAR(VAR_CURRENT_CHARSET) = charsetno;
}
#endif

void ScummEngine::initCharset(int charsetno) {
	if (_game.id == GID_FT) {
		if (!_res->isResourceLoaded(rtCharset, charsetno))
			loadCharset(charsetno);
	} else {
		if (!getResourceAddress(rtCharset, charsetno))
			loadCharset(charsetno);
	}

	_string[0]._default.charset = charsetno;
	_string[1]._default.charset = charsetno;

	memcpy(_charsetColorMap, _charsetData[charsetno], sizeof(_charsetColorMap));
}


#pragma mark -
#pragma mark --- Translation/localization code ---
#pragma mark -


#ifdef ENABLE_SCUMM_7_8
static int indexCompare(const void *p1, const void *p2) {
	const ScummEngine_v7::LangIndexNode *i1 = (const ScummEngine_v7::LangIndexNode *) p1;
	const ScummEngine_v7::LangIndexNode *i2 = (const ScummEngine_v7::LangIndexNode *) p2;

	return strcmp(i1->tag, i2->tag);
}

// Create an index of the language file.
void ScummEngine_v7::loadLanguageBundle() {
	if (isScummvmKorTarget()) {
		// Support language bundle for FT
		ScummEngine::loadLanguageBundle();
		return;
	}
	ScummFile file;
	int32 size;

	if (_game.id == GID_DIG) {
		openFile(file, "language.bnd");
	} else if (_game.id == GID_CMI) {
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

	if (_game.id == GID_DIG) {
		int lineCount = _languageIndexSize;
		const char *baseTag = "";
		byte enc = 0;	// Initially assume the language file is not encoded

		// We'll determine the real index size as we go.
		_languageIndexSize = 0;
		for (i = 0; i < lineCount; i++) {
			if (*ptr == '!') {
				// Don't know what a line with '!' means, just ignore it
			} else if (*ptr == 'h') {
				// File contains Korean text (Hangul). just ignore it
			} else if (*ptr == 'j') {
				// File contains Japanese text. just ignore it
			} else if (*ptr == 'c') {
				// File contains Chinese text. just ignore it
			} else if (*ptr == 'e') {
				// File is encoded!
				enc = 0x13;
			} else if (*ptr == '@') {
				// A new 'base tag'
				baseTag = ptr + 1;
			} else if (*ptr == '#') {
				// Number of subtags following a given basetag. We don't need that
				// information so we just skip it
			} else if (Common::isDigit(*ptr)) {
				int idx = 0;
				// A number (up to three digits)...
				while (Common::isDigit(*ptr)) {
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
				error("Unknown language.bnd entry found: '%s'", ptr);
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
			for (j = 0; j < 8 && !Common::isSpace(*ptr); j++, ptr++)
				_languageIndex[i].tag[j] = toupper(*ptr);
			_languageIndex[i].tag[j] = 0;

			// After that follows a single space which we skip
			assert(Common::isSpace(*ptr));
			ptr++;

			// Then comes the translated string: we record an offset to that.
			_languageIndex[i].offset = ptr - _languageBuffer;

			// Skip over newlines (and turn them into null bytes)
			ptr = strpbrk(ptr, "\n\r");
			if (ptr == NULL)
				break;
			while (*ptr == '\n' || *ptr == '\r')
				*ptr++ = 0;

			// Convert '\n' code to a newline. See also bug #1487.
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
	if (_game.id == GID_DIG && (ConfMan.getBool("speech_mute") || VAR(VAR_VOICE_MODE) == 2))
		return;

	if ((_game.id == GID_DIG || _game.id == GID_CMI) && ptr[0]) {
		Common::String pointerStr((const char *)ptr);

		// Play speech
		if (!(_game.features & GF_DEMO) && (_game.id == GID_CMI)) // CMI demo does not have .IMX for voice
			pointerStr += ".IMX";

		_sound->stopTalkSound();
		_imuseDigital->stopSound(kTalkSoundID);
		_imuseDigital->startVoice(kTalkSoundID, pointerStr.c_str(), _actorToPrintStrFor);
		_sound->talkSound(0, 0, 2);
	}
}

void ScummEngine_v7::translateText(const byte *text, byte *trans_buff, int transBufferSize) {
	if (isScummvmKorTarget()) {
		// Support language bundle for FT
		ScummEngine::translateText(text, trans_buff, transBufferSize);
		return;
	}
	LangIndexNode target;
	LangIndexNode *found = NULL;
	int i;

	trans_buff[0] = 0;
	_lastStringTag[0] = 0;

	if (_game.version >= 7 && text[0] == '/') {
		// Extract the string tag from the text: /..../
		for (i = 0; (i < 12) && (text[i + 1] != '/'); i++)
			_lastStringTag[i] = toupper(text[i + 1]);
		_lastStringTag[i] = 0;
	}

	// WORKAROUND for bug #1977.
	if (_game.id == GID_DIG) {
		// Based on the second release of The Dig
		// Only applies to the subtitles and not speech
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
		else if (!strcmp((const char *)text, "top"))
			text = (const byte *)"/CATHPLAT.067/top";
		else if (!strcmp((const char *)text, "exit"))
			text = (const byte *)"/SKY.008/exit";
		else if (!strcmp((const char *)text, "unattached lens"))
			text = (const byte *)"/NEW.013/unattached lens";
		else if (!strcmp((const char *)text, "lens slot"))
			text = (const byte *)"/NEW.014/lens slot";
		else if (!strcmp((const char *)text, "Jonathon Jackson"))
			text = (const byte *)"Aram Gutowski";
		else if (!strcmp((const char *)text, "Brink"))
			text = (const byte *)"/CREVICE.049/Brink";
		else if (!strcmp((const char *)text, "Robbins"))
			text = (const byte *)"/NEST.061/Robbins";
	}


	if (_game.version >= 7 && text[0] == '/') {
		// Extract the string tag from the text: /..../
		for (i = 0; (i < 12) && (text[i + 1] != '/'); i++)
			target.tag[i] = toupper(text[i + 1]);
		target.tag[i] = 0;
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
		Common::strlcpy((char *)trans_buff, _languageBuffer + found->offset, transBufferSize);

		if ((_game.id == GID_DIG) && !(_game.features & GF_DEMO)) {
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

void ScummEngine::loadLanguageBundle() {
	if (!isScummvmKorTarget()) {
		_existLanguageFile = false;
		return;
	}

	ScummFile file;
	openFile(file, "korean.trs");

	if (!file.isOpen()) {
		_existLanguageFile = false;
		return;
	}

	_existLanguageFile = true;

	int size = file.size();

	uint32 magic1 = file.readUint32BE();
	uint32 magic2 = file.readUint32BE();

	if (magic1 != MKTAG('S', 'C', 'V', 'M') || magic2 != MKTAG('T', 'R', 'S', ' ')) {
		_existLanguageFile = false;
		return;
	}

	_numTranslatedLines = file.readUint16LE();
	_translatedLines = new TranslatedLine[_numTranslatedLines];
	_languageLineIndex = new uint16[_numTranslatedLines];

	// sanity check
	for (int i = 0; i < _numTranslatedLines; i++) {
		_languageLineIndex[i] = 0xffff;
	}

	for (int i = 0; i < _numTranslatedLines; i++) {
		int idx = file.readUint16LE();
		assert(idx < _numTranslatedLines);
		_languageLineIndex[idx] = i;
		_translatedLines[i].originalTextOffset = file.readUint32LE();
		_translatedLines[i].translatedTextOffset = file.readUint32LE();
	}

	// sanity check
	for (int i = 0; i < _numTranslatedLines; i++) {
		if (_languageLineIndex[i] == 0xffff) {
			error("Invalid language bundle file");
		}
	}

	// Room
	byte numTranslatedRoom = file.readByte();
	for (uint32 i = 0; i < numTranslatedRoom; i++) {
		byte roomId = file.readByte();

		TranslationRoom &room = _roomIndex.getOrCreateVal(roomId);

		uint16 numScript = file.readUint16LE();
		for (int sc = 0; sc < numScript; sc++) {
			uint32 scrpKey = file.readUint32LE();
			uint16 scrpLeft = file.readUint16LE();
			uint16 scrpRight = file.readUint16LE();

			room.scriptRanges.setVal(scrpKey, TranslationRange(scrpLeft, scrpRight));
		}
	}

	int bodyPos = file.pos();

	for (int i = 0; i < _numTranslatedLines; i++) {
		_translatedLines[i].originalTextOffset -= bodyPos;
		_translatedLines[i].translatedTextOffset -= bodyPos;
	}
	_languageBuffer = new byte[size - bodyPos];
	file.read(_languageBuffer, size - bodyPos);
	file.close();

	debug(2, "loadLanguageBundle: Loaded %d entries", _numTranslatedLines);
}

const byte *ScummEngine::searchTranslatedLine(const byte *text, const TranslationRange &range, bool useIndex) {
	int textLen = resStrLen(text);

	int left = range.left;
	int right = range.right;

	int dbgIterationCount = 0;

	while (left <= right) {
		dbgIterationCount++;
		debug(8, "searchTranslatedLine: Range: %d - %d", left, right);
		int mid = (left + right) / 2;
		int idx = useIndex ? _languageLineIndex[mid] : mid;
		const byte *originalText = &_languageBuffer[_translatedLines[idx].originalTextOffset];
		int originalLen = resStrLen(originalText);
		int compare = memcmp(text, originalText, MIN(textLen + 1, originalLen + 1));
		if (compare == 0) {
			debug(8, "searchTranslatedLine: Found in %d iteration", dbgIterationCount);
			const byte *translatedText = &_languageBuffer[_translatedLines[idx].translatedTextOffset];
			return translatedText;
		} else if (compare < 0) {
			right = mid - 1;
		} else if (compare > 0) {
			left = mid + 1;
		}
	}

	debug(8, "searchTranslatedLine: Not found in %d iteration", dbgIterationCount);

	return nullptr;
}

void ScummEngine::translateText(const byte *text, byte *trans_buff, int transBufferSize) {
	if (_existLanguageFile) {
		if (_currentScript == 0xff) {
			// used in drawVerb(), etc
			debug(7, "translateText: Room=%d, CurrentScript == 0xff", _currentRoom);
		} else {
			// Use series of heuristics to preserve "the context of the conversation",
			// since one English text can be translated differently depending on the context.
			ScriptSlot *slot = &vm.slot[_currentScript];
			debug(7, "translateText: Room=%d, Script=%d, WIO=%d", _currentRoom, slot->number, slot->where);

			byte roomKey = 0;
			if (slot->where != WIO_GLOBAL) {
				roomKey = _currentRoom;
			}

			uint32 scriptKey = slot->where << 16 | slot->number;
			if (slot->where == WIO_ROOM) {
				scriptKey = slot->where << 16;
			}

			// First search by _currentRoom and _currentScript
			Common::HashMap<byte, TranslationRoom>::const_iterator iterator = _roomIndex.find(roomKey);
			if (iterator != _roomIndex.end()) {
				const TranslationRoom &room = iterator->_value;
				TranslationRange scrpRange;
				if (room.scriptRanges.tryGetVal(scriptKey, scrpRange)) {
					const byte *translatedText = searchTranslatedLine(text, scrpRange, true);
					if (translatedText) {
						debug(7, "translateText: Found by heuristic #1");
						memcpy(trans_buff, translatedText, MIN<int>(resStrLen(translatedText) + 1, transBufferSize));
						return;
					}
				}
			}

			// If not found, search for current room
			roomKey = _currentRoom;
			scriptKey = WIO_ROOM << 16;
			iterator = _roomIndex.find(roomKey);
			if (iterator != _roomIndex.end()) {
				const TranslationRoom &room = iterator->_value;
				TranslationRange scrpRange;
				if (room.scriptRanges.tryGetVal(scriptKey, scrpRange)) {
					const byte *translatedText = searchTranslatedLine(text, scrpRange, true);
					if (translatedText) {
						debug(7, "translateText: Found by heuristic #2");
						memcpy(trans_buff, translatedText, MIN<int>(resStrLen(translatedText) + 1, transBufferSize));
						return;
					}
				}
			}
		}

		// Try full search
		const byte *translatedText = searchTranslatedLine(text, TranslationRange(0, _numTranslatedLines - 1), false);
		if (translatedText) {
			debug(7, "translateText: Found by full search");
			memcpy(trans_buff, translatedText, MIN<int>(resStrLen(translatedText) + 1, transBufferSize));
			return;
		}

		debug(7, "translateText: Not found");
	}

	// Default: just copy the string
	memcpy(trans_buff, text, MIN<int>(resStrLen(text) + 1, transBufferSize));
}

bool ScummEngine::reverseIfNeeded(const byte *text, byte *reverseBuf, int reverseBufSize) const {
	if (_language != Common::HE_ISR)
		return false;
	if (_game.id != GID_LOOM && _game.id != GID_ZAK)
		return false;
	Common::strlcpy(reinterpret_cast<char *>(reverseBuf), reinterpret_cast<const char *>(text), reverseBufSize);
	fakeBidiString(reverseBuf, true);
	return true;
}

Common::CodePage ScummEngine::getDialogCodePage() const {
	switch (_language) {
	case Common::KO_KOR:
		return Common::kWindows949;
	case Common::JA_JPN:
		return Common::kWindows932;
	case Common::ZH_TWN:
	case Common::ZH_CHN:
		return Common::kWindows950;
	case Common::RU_RUS:
		return Common::kDos866;
	case Common::HE_ISR:
		switch (_game.id) {
		case GID_LOOM:
		case GID_ZAK:
			return Common::kDos862;
		default:
			return Common::kWindows1255;
		}
	default:
		return (_game.version > 7) ? Common::kWindows1252 : Common::kDos850;
	}
}

} // End of namespace Scumm
