/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "dragons/bigfile.h"
#include "dragons/actor.h"
#include "dragons/cursor.h"
#include "dragons/actorresource.h"
#include "dragons/talk.h"
#include "dragons/sound.h"
#include "dragons/dragons.h"
#include "dragons/dragonini.h"
#include "dragons/dragonimg.h"
#include "dragons/dragonobd.h"
#include "dragons/scene.h"
#include "dragons/font.h"
#include "dragons/scriptopcodes.h"

namespace Dragons {

Talk::Talk(DragonsEngine *vm, BigfileArchive *bigfileArchive): _vm(vm), _bigfileArchive(bigfileArchive) {
	_dat_800726ec_tfont_field0 = 1; //TODO source from font file
	_dat_800726f0_tfont_field2 = 1; // TODO source from font file

	_dat_8008e7e8_dialogBox_x1 = 0;
	_dat_8008e844_dialogBox_y1 = 0;
	_dat_8008e848_dialogBox_x2 = 0;
	_dat_8008e874_dialogBox_y2 = 0;
	_dat_800633f8_talkDialogFlag = 0;
}

void Talk::init() {
	memset(_defaultResponseTbl, 0, sizeof(_defaultResponseTbl));
	initDefaultResponseTable();
	_dat_800633f8_talkDialogFlag = 0;
}

bool Talk::loadText(uint32 textIndex, uint16 *textBuffer, uint16 bufferLength) {
	char filename[13] = "drag0000.txt";
	uint32 fileNo = (textIndex >> 0xc) & 0xffff;
	uint32 fileOffset = textIndex & 0xfff;

	sprintf(filename, "drag%04d.txt", fileNo);
	uint32 size;
	byte *data = _bigfileArchive->load(filename, size);
	debug(1, "DIALOG: %s, %s, %d", filename, data, fileOffset);
	printWideText(data + 10 + fileOffset);

	copyTextToBuffer(textBuffer, data + 10 + fileOffset, bufferLength);
	bool status = (READ_LE_INT16(data) != 0);
	free(data);
	return status;
}

void Talk::printWideText(byte *text) {
	char buf[2000];
	int i = 0;
	for (; READ_LE_INT16(text) != 0 && i < 1999; i++) {
		char c = *text;
		if (c < 0x20 && i < 1999-4) {
			buf[i++] = '0';
			buf[i++] = 'x';
			buf[i++] = (c & 0xF0 >> 4) + '0';
			buf[i] = (c & 0x0F) + '0';
		} else {
			buf[i] = *text;
		}
		text += 2;
	}
	buf[MIN(i, 1999)] = 0;
	debug(1, "TEXT: %s", buf);
}

void
Talk::FUN_8003239c(uint16 *dialog, int16 x, int16 y, int32 param_4, uint16 param_5, Actor *actor, uint16 startSequenceId,
				   uint16 endSequenceId, uint32 textId) {
	//TODO 0x800323a4
	uint16 stubDialog[] = {'1', '2', '3', 0};

	//TODO dragon_text_related(textId);
	_vm->_isLoadingDialogAudio = true;
	uint32 uVar4 = 0; //TODO FUN_8001d1ac(0, textId, 0);

	actor->updateSequence(startSequenceId);
	_vm->_sound->playSpeech(textId);
	conversation_related_maybe((*dialog != 0) ? dialog : &stubDialog[0], (int)x, (int)y, param_4 & 0xffff, (uint)param_5, textId, uVar4 & 0xffff);

	actor->updateSequence(endSequenceId);
}

//void Talk::conversation_related_maybe(uint16 *dialogText, uint16 x, uint16 y, uint16 param_4, int16 param_5, uint32 textId,
//								 int16 param_7) {
//	//TODO display dialog text here while we wait for audio stream to complete.
//	_vm->_fontManager->addText(x, y, dialogText, wideStrLen(dialogText), 0);
//	while (_vm->isFlagSet(ENGINE_FLAG_8000)) {
//		_vm->waitForFrames(1);
//	}
//	_vm->_fontManager->clearText();
//}

uint8 Talk::conversation_related_maybe(uint16 *dialogText, uint16 x, uint16 y, uint16 param_4, int16 param_5, uint32 textId, int16 param_7) {
	//uint32 *puVar1;
	//uint32 *puVar2;
	short sVar3;
	short sVar4;
	uint16 *tmpTextPtr;
	int iVar5;
	int iVar6;
	//int iVar7;
	int iVar8;
	uint uVar9;
	uint uVar10;
	uint16 uVar11;
	//uint32 uVar12;
	uint16 *puVar13;
	//uint32 uVar14;
	//uint32 uVar15;
	//uint32 *puVar16;
	//uint32 *puVar17;
	uint16 *dialogTextLinePtr;
	uint16 *puVar18;
	uint16 *curDialogTextPtr;
	int unaff_s4 = 0;
	uint16 uVar19;
	short sVar20;
	uint32 maxLineLengthMaybe;
	uint16 currentLine [44];
	uint16 asStack2592 [1208];
	//uint32 local_b0 [14];
	uint16 local_58;
	uint16 returnStatus;
	//byte *lineTblPtr;
	//uint oldEngineFlags;

	bool isFlag8Set = _vm->isFlagSet(ENGINE_FLAG_8);
	returnStatus = 0;
// TODO what does this do?
//		puVar1 = uint32_ARRAY_80011a60;
//		puVar2 = local_b0;
//		do {
//			puVar17 = puVar2;
//			puVar16 = puVar1;
//			uVar12 = puVar16[1];
//			uVar14 = puVar16[2];
//			uVar15 = puVar16[3];
//			*puVar17 = *puVar16;
//			puVar17[1] = uVar12;
//			puVar17[2] = uVar14;
//			puVar17[3] = uVar15;
//			puVar1 = puVar16 + 4;
//			puVar2 = puVar17 + 4;
//		} while (puVar16 + 4 != (uint32 *)&_dat_80011a80);
//		uVar12 = puVar16[5];
//		puVar17[4] = __dat_80011a80;
//		puVar17[5] = uVar12;
	if (param_5 != 0) {
		_vm->clearFlags(ENGINE_FLAG_8);
	}

	//Remove '\' chars from dialogText
	tmpTextPtr = findCharInU16Str(dialogText, 0x5c);
	while (tmpTextPtr != nullptr) {
		sVar3 = tmpTextPtr[1];
		*tmpTextPtr = sVar3;
		while (sVar3 != 0) {
			sVar3 = tmpTextPtr[2];
			tmpTextPtr[1] = sVar3;
			tmpTextPtr = tmpTextPtr + 1;
		}
		tmpTextPtr = findCharInU16Str(dialogText, 0x5c);
	}
	iVar5 = (uint)x << 0x10;
	if ((short)x < 0xf) {
		x = 0xf;
		iVar5 = 0xf0000;
	}
	if (0x19 < iVar5 >> 0x10) {
		x = 0x19;
	}
	iVar5 = (uint)y << 0x10;
	if ((int)((uint)y << 0x10) < 0) {
		y = 0;
		iVar5 = 0;
	}
	if (0x18 < iVar5 >> 0x10) {
		y = 0x18;
	}
	local_58 = x;
	if (0x13 < (short)x) {
		local_58 = 0x27 - x;
	}
	iVar5 = ((int)((uint)local_58 << 0x10) >> 0xf) + 2;
//		if (_dat_800726ec_tfont_field0 == 0) {
//			trap(0x1c00);
//		}
//		if (((uint)_dat_800726ec_tfont_field0 == 0xffffffff) && (iVar5 == -0x80000000)) {
//			trap(0x1800);
//		}
	sVar3 = (short)(iVar5 / (int)(uint)_dat_800726ec_tfont_field0);
	if (*dialogText != 0) {
		/* check for space or hyphen */
		curDialogTextPtr = dialogText;
		do {
			maxLineLengthMaybe = 0;
			iVar5 = 0;
			do {
				sVar20 = (short)maxLineLengthMaybe;
				uVar19 = *curDialogTextPtr;
				iVar6 = iVar5;
				if (uVar19 == 0) break;
				while (uVar19 == 0x20) {
					curDialogTextPtr = curDialogTextPtr + 1;
					uVar19 = *curDialogTextPtr;
				}
				iVar8 = 0;
				iVar6 = (int)sVar3 + -2;
				for (unaff_s4 = 0; unaff_s4 < iVar6; unaff_s4++) {
					currentLine[unaff_s4] = curDialogTextPtr[unaff_s4];
					if (currentLine[unaff_s4] == 0) {
						iVar8 = -1;
						break;
					}
				}
//					unaff_s4 = 0;
//					if (0 < iVar6) {
//						iVar7 = 0;
//						do {
//							currentLine[unaff_s4] = curDialogTextPtr[unaff_s4];
//							sVar4 = *(short *)((iVar7 >> 0xf) + (int)curDialogTextPtr);
//							*(short *)((iVar7 >> 0xf) + (int)currentLine) = sVar4;
//							if (sVar4 == 0) {
//								iVar8 = -1;
//								break;
//							}
//							unaff_s4 = unaff_s4 + 1;
//							iVar7 = unaff_s4 * 0x10000;
//						} while (unaff_s4 < iVar6);
//					}
				iVar6 = iVar5;
				if ((int)(short)unaff_s4 == 0)
					break;
				currentLine[(int)(short)unaff_s4] = 0;

				if (iVar8 == 0) {
					iVar8 = unaff_s4 - 1;
					for (; iVar8 >= 0 ; iVar8--) {
						if (currentLine[iVar8] == 0x20 || currentLine[iVar8] == 0x2d) {
							break;
						}
					}
					goto LAB_800328f8;
				}
//					iVar6 = (iVar8 << 0x10) >> 0x10;
//					if (iVar8 << 0x10 == 0) {
//						iVar8 = unaff_s4 + -1;
//						if (-1 < iVar8 * 0x10000) {
//							do {
//								/* check for space or hyphen */
//								sVar4 = *(short *)(((iVar8 << 0x10) >> 0xf) + (int)currentLine);
//								if ((sVar4 == 0x20) || (iVar6 = iVar8 + -1, sVar4 == 0x2d)) break;
//								iVar8 = iVar6;
//							} while (-1 < iVar6 * 0x10000);
//							iVar6 = (int)(short)iVar8;
//							goto LAB_800328f8;
//						}
//					} else
				{
					LAB_800328f8:
					if (-1 < iVar8) {
						if (currentLine[iVar8] == 0x20) {
							for (; iVar8 >= 0; iVar8--) {
								if (currentLine[iVar8] != 0x20) {
									break;
								}
								currentLine[iVar8] = 0;
							}

//								do {
//									if (*(short *)(((iVar8 << 0x10) >> 0xf) + (int)currentLine) != 0x20) break;
//									iVar6 = iVar8 + -1;
//									*(undefined2 *)(((iVar8 << 0x10) >> 0xf) + (int)currentLine) = 0;
//									iVar8 = iVar6;
//								} while (-1 < iVar6 * 0x10000);
						} else {
							currentLine[iVar8 + 1] = 0;
						}
					}
				}

				iVar6 = iVar5;
				if (currentLine[0] == 0) break;
				iVar6 = iVar5 + 1;
				UTF16ToUTF16Z(asStack2592 + (int)(short)iVar5 * 0x29, currentLine);
				uVar9 = strlenUTF16(currentLine);
				if ((int)sVar20 < (int)(uVar9 & 0xffff)) {
					maxLineLengthMaybe = strlenUTF16(currentLine);
				}
				sVar20 = (short)maxLineLengthMaybe;
				uVar9 = strlenUTF16(currentLine);
				curDialogTextPtr = curDialogTextPtr + (uVar9 & 0xffff);
				iVar5 = iVar6;
			} while (iVar6 * 0x10000 >> 0x10 < 5);
			sVar4 = (short)iVar6;
			iVar5 = iVar6 << 0x10;
			puVar18 = curDialogTextPtr;
			if (sVar4 == 5) {
				if (*curDialogTextPtr != 0) {
					uVar19 = *curDialogTextPtr;
					puVar13 = curDialogTextPtr;
					do {
						if (0x20 < uVar19) break;
						puVar13 = puVar13 + 1;
						uVar19 = *puVar13;
					} while (uVar19 != 0);
					iVar5 = iVar6 << 0x10;
					puVar18 = curDialogTextPtr;
					if (*puVar13 == 0) goto LAB_80032a70;
					uVar9 = strlenUTF16(currentLine);
//						curDialogTextPtr = curDialogTextPtr + (uVar9 & 0xffff) * 0x7fffffff; TODO why was this line generated?
					curDialogTextPtr -= uVar9;
					iVar6 = iVar6 + -1;
				}
				sVar4 = (short)iVar6;
				iVar5 = iVar6 << 0x10;
				puVar18 = curDialogTextPtr;
			}
			LAB_80032a70:
			iVar5 = (iVar5 >> 0x10) * (uint)_dat_800726f0_tfont_field2 + -1;
			uVar19 = y;
			if ((int)(short)y < iVar5) {
				uVar19 = (uint16)iVar5;
			}
			if (0x16 < (short)uVar19) {
				uVar19 = 0x16;
			}
			uVar11 = (sVar20 * _dat_800726ec_tfont_field0 + 3) & 0xfffe;
			sVar20 = (short)uVar11 >> 1;
			if ((short)x < 0x14) {
				_dat_8008e7e8_dialogBox_x1 = (x - sVar20) + 1;
			} else {
				_dat_8008e7e8_dialogBox_x1 = x - sVar20;
			}
			_dat_8008e848_dialogBox_x2 = (uVar11 + _dat_8008e7e8_dialogBox_x1) - 1;
			_dat_8008e844_dialogBox_y1 = (uVar19 - sVar4 * _dat_800726f0_tfont_field2) + 1;
			_dat_8008e874_dialogBox_y2 = _dat_8008e844_dialogBox_y1 + sVar4 * _dat_800726f0_tfont_field2 + 1;
			if (!_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_1) && ((!_vm->isFlagSet(ENGINE_FLAG_1000_SUBTITLES_DISABLED) || (param_7 != 0)))) {
				unaff_s4 = 0;
				drawDialogBox((uint) _dat_8008e7e8_dialogBox_x1, (uint) _dat_8008e844_dialogBox_y1,
							  (uint) _dat_8008e848_dialogBox_x2, (uint) _dat_8008e874_dialogBox_y2, 0);
				if (sVar4 > 0) {
					for (; unaff_s4 < sVar4; unaff_s4++) {
						if ((short)x < 0x14) {
							dialogTextLinePtr = asStack2592 + unaff_s4 * 0x29;
							uVar9 = strlenUTF16(dialogTextLinePtr);
							uVar9 = ((uint)x - ((int)((uint)_dat_800726ec_tfont_field0 * (uVar9 & 0xffff) + 1) >> 1)) + 1;
						} else {
							dialogTextLinePtr = asStack2592 + unaff_s4 * 0x29;
							uVar9 = strlenUTF16(dialogTextLinePtr);
							uVar9 = (uint)x - ((int)((uint)_dat_800726ec_tfont_field0 * (uVar9 & 0xffff) + 1) >> 1);
						}
						_vm->_fontManager->addText(uVar9 * 8,
												   ((uint)_dat_8008e844_dialogBox_y1 + unaff_s4 * (uint)_dat_800726f0_tfont_field2 + 1) * 8,
								dialogTextLinePtr, wideStrLen(dialogTextLinePtr), 0);
//							ProbablyShowUTF16Msg3
//									(dialogTextLinePtr, uVar9 & 0xffff,
//									 (uint)_dat_8008e844_dialogBox_y1 + iVar5 * (uint)_dat_800726f0_tfont_field2 + 1
//									 & 0xffff, (uint)param_4, 0xffffffff);
					}
				}
			}
			if (param_5 == 0) {
				_vm->_isLoadingDialogAudio = false;
				return (uint)returnStatus;
			}
			uVar9 = ((int)((int)(short)unaff_s4 * (uint)1 * (int)sVar3) >> 3) * 0x3c;
			if ((param_7 == 0) && _vm->isFlagSet(ENGINE_FLAG_1000_SUBTITLES_DISABLED)) {
				uVar9 = 0;
			}
			do {
				curDialogTextPtr = puVar18;
				if (!_vm->isFlagSet(ENGINE_FLAG_8000)) goto LAB_80032e18;
				_vm->waitForFrames(1);
				if (_vm->_sound->_dat_8006bb60_sound_related != 0) {
					_vm->_sound->_dat_8006bb60_sound_related = 0;
					curDialogTextPtr = dialogText;
					if (!_vm->isFlagSet(ENGINE_FLAG_1000_SUBTITLES_DISABLED)) {
						returnStatus = 1;
						goto LAB_80032e18;
					}
				}
				if ((uVar9 & 0xffff) != 0) {
					uVar9--;
				}
				uVar10 = _vm->checkForActionButtonRelease() ? 1 : 0; // CheckButtonMapPress_ActionOrCross(0);
				curDialogTextPtr = puVar18;
				if ((uVar10 & 0xffff) != 0) {
					returnStatus = 1;
					goto LAB_80032e18;
				}
				uVar10 = 0; //TODO PressedThisFrameStart(0);
			} while (!Engine::shouldQuit() && (uVar10 & 0xffff) == 0);
			returnStatus = 2;
			curDialogTextPtr = puVar18;
			LAB_80032e18:
			//TODO CheckIfCdShellIsOpen();
			if (!_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_1)) {
				_vm->_fontManager->clearTextDialog((uint) _dat_8008e7e8_dialogBox_x1, (uint) _dat_8008e844_dialogBox_y1,
								(uint) _dat_8008e848_dialogBox_x2, (uint) _dat_8008e874_dialogBox_y2);
			}
		} while (!Engine::shouldQuit() &&
				 !_vm->isUnkFlagSet(ENGINE_UNK1_FLAG_1) &&
				 (((!_vm->isFlagSet(ENGINE_FLAG_1000_SUBTITLES_DISABLED) || (param_7 != 0)) && (*curDialogTextPtr != 0))));
	}
	if (param_5 != 0) {
		if (_vm->isFlagSet(ENGINE_FLAG_8000)) {
			_vm->_sound->resumeMusic();
		}
		if (isFlag8Set) {
			_vm->setFlags(ENGINE_FLAG_8);
		}
	}
	_vm->_isLoadingDialogAudio = false;
	return (uint)returnStatus;
}

uint32 Talk::displayDialogAroundINI(uint32 iniId, uint16 *dialogText, uint32 textIndex) {
	DragonINI *ini = iniId == 0 ? _vm->_dragonINIResource->getFlickerRecord() : _vm->getINI(iniId - 1);

	if ((ini->flags & 1) == 0) {
		Img *local_v1_184 = _vm->_dragonImg->getImg(ini->imgId);
		int x, y;
		if (local_v1_184->field_e == 0) {
			y = (uint)(uint16)local_v1_184->y;
			x = local_v1_184->field_a;
		} else {
			x = local_v1_184->field_a;
			y = (uint)(uint16)local_v1_184->y << 3;
		}
		displayDialogAroundPoint
				(dialogText,
						((x - _vm->_scene->_camera.x) * 0x10000) >> 0x13,
				 ((y - _vm->_scene->_camera.y) * 0x10000) >> 0x13,
				 READ_LE_UINT16(_vm->_dragonOBD->getFromOpt(ini->id) + 6)
						, 1, textIndex);
	} else {
		displayDialogAroundActor
				(ini->actor,
				 READ_LE_UINT16(_vm->_dragonOBD->getFromOpt(ini->id) + 6),
				 dialogText, textIndex);
	}
	return 1;
}

void Talk::loadAndDisplayDialogAroundPoint(uint32 textId, uint16 x, uint16 y, uint16 param_4, int16 param_5) {
	uint16 buffer[1000];
	loadText(textId, buffer, 1000);
	displayDialogAroundPoint(buffer, x, y, param_4, param_5, textId);
}

void Talk::displayDialogAroundPoint(uint16 *dialogText, uint16 x, uint16 y, uint16 param_4, int16 param_5, uint32 textId) {
// TODO
//
//		puVar1 = &_dat_80011a60;
//		puVar2 = local_58;
//		do {
//			puVar8 = puVar2;
//			puVar7 = puVar1;
//			uVar4 = puVar7[1];
//			uVar5 = puVar7[2];
//			uVar6 = puVar7[3];
//			*puVar8 = *puVar7;
//			puVar8[1] = uVar4;
//			puVar8[2] = uVar5;
//			puVar8[3] = uVar6;
//			puVar1 = puVar7 + 4;
//			puVar2 = puVar8 + 4;
//		} while (puVar7 + 4 != (undefined4 *)&_dat_80011a80);
//		uVar4 = puVar7[5];
//		puVar8[4] = __dat_80011a80;
//		puVar8[5] = uVar4;
		_vm->_isLoadingDialogAudio = true;

		// sVar3 = FUN_8001d1ac(0, textId, 0);
		_vm->_sound->playSpeech(textId);

//		if (dialogText == (uint16 *)0x0) {
//			dialogText = (uint16 *)local_58;
//		}
	conversation_related_maybe(dialogText, x, y, param_4, param_5, textId, 0); // sVar3); TODO I think this is audio status
}

void Talk::displayDialogAroundActor(Actor *actor, uint16 param_2, uint16 *dialogText, uint32 textIndex) {
	int16 frameYOffset = actor->_frame ? actor->_frame->yOffset : 0;
	displayDialogAroundPoint
			(dialogText, (uint16)((int)(((uint)actor->_x_pos - _vm->_scene->_camera.x) * 0x10000) >> 0x13),
			 (short)((int)((((uint)actor->_y_pos - (uint)frameYOffset) - (uint)_vm->_scene->_camera.y) * 0x10000) >> 0x13) - 3,
			 param_2, 1, textIndex);
}

void Talk::copyTextToBuffer(uint16 *destBuffer, byte *src, uint32 destBufferLength) {
	for (uint i = 0; i < destBufferLength - 1; i++) {
		destBuffer[i] = READ_LE_UINT16(src);
		src += 2;
		if (destBuffer[i] == 0) {
			return;
		}
	}

	destBuffer[destBufferLength - 1] = 0;
}

uint32 Talk::wideStrLen(uint16 *text) {
	int i = 0;
	while (text[i] != 0) {
		i++;
	}
	return i;
}

void Talk::addTalkDialogEntry(TalkDialogEntry *talkDialogEntry) {
	_dialogEntries.push_back(talkDialogEntry);
}

void callMaybeResetData() {
	//TODO do we need this?
}

bool Talk::talkToActor(ScriptOpCall &scriptOpCall) {
	uint16 numEntries;

	uint16 sequenceId;
	TalkDialogEntry *selectedDialogText;
	uint iniId;
	//short local_990 [5];
	uint16 auStack2438 [195];
	uint16 local_800 [1000];
	Common::Array<TalkDialogEntry *> dialogEntries;

	bool isFlag8Set = _vm->isFlagSet(ENGINE_FLAG_8);
	bool isFlag100Set = _vm->isFlagSet(ENGINE_FLAG_100);

	_vm->clearFlags(ENGINE_FLAG_8);
	Actor *flickerActor = _vm->_dragonINIResource->getFlickerRecord()->actor;

	//TODO clear entries;
	_vm->_scriptOpcodes->loadTalkDialogEntries(scriptOpCall);
	numEntries = _dialogEntries.size();
	if (numEntries == 0) {
		return false;
	}

	for (Common::Array<TalkDialogEntry*>::iterator it = _dialogEntries.begin(); it != _dialogEntries.end(); it++) {
		TalkDialogEntry *entry = (TalkDialogEntry *)malloc(sizeof(TalkDialogEntry));
		memcpy(entry, *it, sizeof(TalkDialogEntry));
		dialogEntries.push_back(entry);
	}

	_vm->setFlags(ENGINE_FLAG_100);
	do {
		_vm->clearAllText();
		int numActiveDialogEntries = 0;
		for (Common::Array<TalkDialogEntry*>::iterator it = dialogEntries.begin(); it != dialogEntries.end(); it++) {
			if (!((*it)->flags & 1)) {
				numActiveDialogEntries++;
			}
		}
		if (numActiveDialogEntries == 0) {
			//TODO logic from LAB_80029bc0 reset cursor
			exitTalkMenu(isFlag8Set, isFlag100Set, dialogEntries);
			return true;
		}

		selectedDialogText = displayTalkDialogMenu(dialogEntries);
		if (selectedDialogText == nullptr) {
			_vm->clearAllText();
			exitTalkMenu(isFlag8Set, isFlag100Set, dialogEntries);
			return true;
		}
		_vm->clearFlags(ENGINE_FLAG_8);
		//strcpy((char *)local_990, selectedDialogText->dialogText);
		UTF16ToUTF16Z(auStack2438, (uint16 *)(selectedDialogText->dialogText + 10));
//		load_string_from_dragon_txt(selectedDialogText->textIndex1, (char *)local_800);
		if (selectedDialogText->hasText) {
			flickerActor->setFlag(ACTOR_FLAG_2000);
			sequenceId = flickerActor->_sequenceID;
//			playSoundFromTxtIndex(selectedDialogText->textIndex);
			if (flickerActor->_direction != -1) {
				flickerActor->updateSequence(flickerActor->_direction + 0x10);
			}
			displayDialogAroundINI(0, auStack2438, selectedDialogText->textIndex);
			flickerActor->updateSequence(sequenceId);
			flickerActor->clearFlag(ACTOR_FLAG_2000);
		}
		if ((selectedDialogText->flags & 2) == 0) {
			selectedDialogText->flags = selectedDialogText->flags | 1;
		}
		_vm->clearAllText();
		if (loadText(selectedDialogText->textIndex1, local_800, 1000)) {
			if (selectedDialogText->field_26c == -1) {
				displayDialogAroundINI(_vm->_cursor->_iniUnderCursor, local_800, selectedDialogText->textIndex1);
			} else {
				iniId = _vm->_cursor->_iniUnderCursor; //dragon_ini_index_under_active_cursor;
				if (selectedDialogText->iniId != 0) {
					iniId = selectedDialogText->iniId;
				}
				Actor *iniActor = _vm->_dragonINIResource->getRecord(iniId - 1)->actor;
				sequenceId = iniActor->_sequenceID;
//				playSoundFromTxtIndex(selectedDialogText->textIndex1);
				iniActor->updateSequence(selectedDialogText->field_26c);
				displayDialogAroundINI(iniId, local_800, selectedDialogText->textIndex1);
				iniActor->updateSequence(sequenceId);
			}
		}
		ScriptOpCall local_1d20(selectedDialogText->scriptCodeStartPtr, selectedDialogText->scriptCodeEndPtr - selectedDialogText->scriptCodeStartPtr);
		_vm->_scriptOpcodes->runScript(local_1d20);
		if (_vm->_scriptOpcodes->_numDialogStackFramesToPop != 0) break;
		local_1d20._code = selectedDialogText->scriptCodeStartPtr;
		local_1d20._codeEnd = selectedDialogText->scriptCodeEndPtr;
		talkToActor(local_1d20);

	} while (_vm->_scriptOpcodes->_numDialogStackFramesToPop == 0);
	_vm->_scriptOpcodes->_numDialogStackFramesToPop--;
//	LAB_80029bc0:
//	actors[0]._x_pos = cursor_x_var;
//	actors[0]._y_pos = cursor_y_var;

	exitTalkMenu(isFlag8Set, isFlag100Set, dialogEntries);
	return true;
}

TalkDialogEntry *Talk::displayTalkDialogMenu(Common::Array<TalkDialogEntry*> dialogEntries) {
	bool bVar1;
	int16 sVar2;
	uint uVar3;
	uint uVar4;
	uint16 y;
	uint16 x;
	uint uVar6;
	uint uVar7;
	uint16 selectedTextLine;
	TalkDialogEntry *talkDialogEntry;
	//uint16 local_390[5];
	uint16 local_386 [195];
	uint16 asStack512 [200];
	uint16 numEntries;
	uint16 totalNumEntries;
	uint16 local_58;
	int16 local_50;
	uint16 *local_40;
	uint local_38;
	bool hasDialogEntries;
	uint16 *_dat_80083104;

	talkDialogEntry = nullptr;

	_vm->_fontManager->clearText();

	selectedTextLine = 0;
	totalNumEntries = 0;
	local_58 = 0;
	numEntries = dialogEntries.size();
	if (numEntries != 0) {
		uVar3 = 0;
		do {
			talkDialogEntry =  dialogEntries[uVar3];
			if ((talkDialogEntry->flags & 1) == 0) {
				totalNumEntries = totalNumEntries + 1;
				talkDialogEntry->yPosMaybe = '\0';
				//strcpy((char *)&local_390, (char *)talkDialogEntry->dialogText);
				UTF16ToUTF16Z(local_386, (uint16 *)(&talkDialogEntry->dialogText[10]));
				_dat_80083104 = local_386;
				if (*local_386 == 0x20) {
					_dat_80083104 = &local_386[1];
				}
				uVar3 = findLastPositionOf5cChar(_dat_80083104);
				sVar2 = truncateDialogText(_dat_80083104, asStack512, uVar3 & 0xffff, 0x20);
				talkDialogEntry->xPosMaybe = (uint8)local_58;
				local_58 = local_58 + sVar2;
				talkDialogEntry->yPosMaybe = talkDialogEntry->yPosMaybe + (char)sVar2;
			}
			selectedTextLine = selectedTextLine + 1;
			uVar3 = (uint)selectedTextLine;
		} while (selectedTextLine < numEntries);
	}
	drawDialogBox(1, (0x17 - (uint) local_58) & 0xffff, 0x26, 0x18, 1);
	selectedTextLine = 0;
	_vm->_cursor->updateSequenceID(3);
	local_50 = -2;
	local_38 = (uint)numEntries;
	_dat_800633f8_talkDialogFlag = 1;
	hasDialogEntries = (numEntries != 0);
	local_40 = asStack512;

	LAB_800317a4:
//		CheckIfCdShellIsOpen();
	_vm->waitForFrames(1);

	if (Engine::shouldQuit()) {
		return nullptr;
	}

	y = 0;
	x = 0;
	if (hasDialogEntries) {
		uVar3 = 0;
		do {
			talkDialogEntry = dialogEntries[uVar3];
			if (((talkDialogEntry->flags & 1) == 0) && (bVar1 = y == selectedTextLine, y = y + 1, bVar1)) break;
			x = x + 1;
			uVar3 = (uint)x;
		} while (x < numEntries);
	}
	assert(talkDialogEntry); // It could be zero if hasDialogEntries == 0. What to do then?
	_vm->_cursor->updateActorPosition(0xf, (((uint16)talkDialogEntry->xPosMaybe + 0x18) - local_58) * 8 + 5);

	if (!_vm->isFlagSet(ENGINE_FLAG_8)) {
		_vm->waitForFrames(1);
		_vm->setFlags(ENGINE_FLAG_8);
	}
	if (local_50 != -2) {
		if (_vm->checkForActionButtonRelease()) {
			_vm->playOrStopSound(0x800a);

			_vm->clearFlags(ENGINE_FLAG_8);
			y = 0;
			_vm->clearAllText();
			if (hasDialogEntries) {
				uVar3 = 0;
				do {
					talkDialogEntry = dialogEntries[uVar3];
					y = y + 1;
					if ((talkDialogEntry->flags & 1) == 0) {
						if (selectedTextLine == 0) {
							_dat_800633f8_talkDialogFlag = 0;
							return talkDialogEntry;
						}
						selectedTextLine--;
					}
					uVar3 = (uint)y;
				} while (y < numEntries);
			}
		}
		if ((selectedTextLine < totalNumEntries - 1) &&
			(_vm->checkForDownKeyRelease() || _vm->checkForWheelDown())) {
			selectedTextLine++;
			_vm->playOrStopSound(0x8009);
		} else if (selectedTextLine > 0 && (_vm->checkForUpKeyRelease() || _vm->checkForWheelUp())) {
			selectedTextLine--;
			_vm->playOrStopSound(0x8009);
		}
	}
	y = 0x18 - local_58;
	local_50 = -1;
	uVar3 = 0;
	if (hasDialogEntries) {
		uVar6 = 0;
		do {
			talkDialogEntry = dialogEntries[uVar6];
			uVar6 = (uint)talkDialogEntry->flags & 1;
			if ((talkDialogEntry->flags & 1) == 0) {
				sVar2 = local_50 + 1;
				local_50 = sVar2;
				UTF16ToUTF16Z(local_386, (uint16 *)(&talkDialogEntry->dialogText[10]));
				_dat_80083104 = local_386;
				if (local_386[0] == 0x20) {
					_dat_80083104 = &local_386[1];
				}
				uVar4 = findLastPositionOf5cChar(_dat_80083104);
				uVar4 = truncateDialogText(_dat_80083104, local_40, uVar4 & 0xffff, 0x20);
				_dat_80083104 = local_40;
				if (sVar2 == selectedTextLine) {
					uVar7 = 0;
					if (uVar6 < (uVar4 & 0xffff)) {
						do {
							x = 5;
							if ((uVar7 & 0xffff) == 0) {
								x = 4;
							}
							//TODO ProbablyShowUTF16Msg(_dat_80083104, x, y, 0, -1);
							_vm->_fontManager->addText(x * 8, y * 8, _dat_80083104, wideStrLen(_dat_80083104), 0);
							sVar2 = *_dat_80083104;
							while (sVar2 != 0) {
								sVar2 = _dat_80083104[1];
								_dat_80083104 = _dat_80083104 + 1;
							}
							uVar7 = uVar7 + 1;
							_dat_80083104 = _dat_80083104 + 1;
							y = y + 1;
						} while ((uVar7 & 0xffff) < (uVar4 & 0xffff));
					}
				} else {
					uVar7 = 0;
					if (uVar6 < (uVar4 & 0xffff)) {
						do {
							x = 5;
							if ((uVar7 & 0xffff) == 0) {
								x = 4;
							}
							//TODO ProbablyShowUTF16Msg2(_dat_80083104, x, (uint)y, 0x401, 0xffffffff);
							_vm->_fontManager->addText(x * 8, y * 8, _dat_80083104, wideStrLen(_dat_80083104), 1);
							sVar2 = *_dat_80083104;
							while (sVar2 != 0) {
								sVar2 = _dat_80083104[1];
								_dat_80083104 = _dat_80083104 + 1;
							}
							uVar7 = uVar7 + 1;
							_dat_80083104 = _dat_80083104 + 1;
							y = y + 1;
						} while ((uVar7 & 0xffff) < (uVar4 & 0xffff));
					}
				}
			}
			uVar3 = uVar3 + 1;
			uVar6 = uVar3 & 0xffff;
		} while ((uVar3 & 0xffff) < local_38);
	}
	goto LAB_800317a4;
}

void Talk::exitTalkMenu(bool isFlag8Set, bool isFlag100Set, Common::Array<TalkDialogEntry*> dialogEntries) {
	_vm->clearFlags(ENGINE_FLAG_8);
	_vm->clearFlags(ENGINE_FLAG_100);

	if (isFlag8Set) {
		_vm->setFlags(ENGINE_FLAG_8);
	}
	if (isFlag100Set) {
		_vm->setFlags(ENGINE_FLAG_100);
	}

	for (Common::Array<TalkDialogEntry*>::iterator it = dialogEntries.begin(); it != dialogEntries.end(); it++) {
		delete *it;
	}
	dialogEntries.clear();
	_vm->_fontManager->clearText();
}

//TODO move to cutscene class
uint Talk::somethingTextAndSpeechAndAnimRelated(Actor *actor, int16 sequenceId1, int16 sequenceId2, uint32 textIndex,
												uint16 param_5) {
	uint16 dialog[2048];
	dialog[0] = 0;

	textIndex = _vm->getDialogTextId(textIndex);

	_vm->_talk->loadText(textIndex, dialog, 2048);

	if (sequenceId1 != -1) {
		actor->updateSequence(sequenceId1);
	}
	displayDialogAroundActor(actor, param_5, dialog, textIndex);
	if (sequenceId2 != -1) {
		actor->updateSequence(sequenceId2);
	}
	return 1; //TODO this should get return value from  displayDialogAroundActor();
}

void Talk::talkFromIni(uint32 iniId, uint32 textIndex) {
	debug(3, "Main actor talk: 0x%04x and text 0x%04x", iniId, textIndex);

	if (textIndex == 0) {
		return;
	}
	Actor *actor = nullptr;
	if (iniId == 0) {
		//TODO playSoundFromTxtIndex(textIndex);
		actor = _vm->_dragonINIResource->getFlickerRecord()->actor;
		if (!_vm->isFlagSet(ENGINE_FLAG_2000000)) {
			if (_vm->getCurrentSceneId() == 0x32) {
				_vm->getINI(0x2b1)->actor->updateSequence(2);
			} else {
				actor->setFlag(ACTOR_FLAG_2000);
				if (actor->_direction != -1) {
					actor->updateSequence(actor->_direction + 0x10);
				}
			}
		} else {
			if (actor->_sequenceID == 5) {
				actor->updateSequence(0x10);
			}
		}
	}
	// TODO sVar1 = findTextToDtSpeechIndex(textIndex);
	uint16 dialog[2048];
	dialog[0] = 0;
	_vm->_talk->loadText(textIndex, dialog, 2048);

//	pcVar2 = (char *)0x0;
//	if (((unkFlags1 & 1) == 0) && (((engine_flags_maybe & 0x1000) == 0 || (sVar1 == -1)))) {
//		pcVar2 = load_string_from_dragon_txt(textIndex, acStack2016);
//	}
	_vm->_talk->displayDialogAroundINI(iniId, dialog, textIndex); //TODO need to pass dialog here (pcVar2). not nullptr
	if (iniId == 0) {
		if (!_vm->isFlagSet(ENGINE_FLAG_2000000)) {
			if (_vm->getCurrentSceneId() != 0x32) {
				actor->setFlag(ACTOR_FLAG_4);
				actor->clearFlag(ACTOR_FLAG_2000);
				_vm->waitForFrames(1);
				return;
			}
			_vm->getINI(0x2b1)->actor->updateSequence(1);

		} else {
			if (actor->_sequenceID != 0x10) {
				return;
			}
			actor->updateSequence(5);
		}
	}
}

void Talk::flickerRandomDefaultResponse() {
	DragonINI *flicker = _vm->_dragonINIResource->getFlickerRecord();
	if (flicker && flicker->actor) {
		flicker->actor->clearFlag(ACTOR_FLAG_10);
		if (_vm->getCurrentSceneId() != 0x2e || !flicker->actor->_actorResource || flicker->actor->_actorResource->_id != 0x91) {
			flicker->actor->setFlag(ACTOR_FLAG_4);
		}
	}
	talkFromIni(0, getDefaultResponseTextIndex());
}

uint32 Talk::getDefaultResponseTextIndex() {
	uint16 rand = _vm->getRand(9);
	return _defaultResponseTbl[(_vm->_cursor->_data_800728b0_cursor_seqID - 1) * 9 + rand];
}

uint32 Talk::extractTextIndex(Common::File *fd, uint16 offset) {
	fd->seek(_vm->defaultResponseOffsetFromDragonEXE() + offset * 4);
	return fd->readUint32LE();
}

void Talk::initDefaultResponseTable() {
	Common::File *fd = new Common::File();
	if (!fd->open("dragon.exe")) {
		error("Failed to open dragon.exe");
	}

	//TODO need to check that other languages follow this pattern
	_defaultResponseTbl[0] = extractTextIndex(fd, 19);
	_defaultResponseTbl[1] = extractTextIndex(fd, 20);
	_defaultResponseTbl[2] = extractTextIndex(fd, 21);
	_defaultResponseTbl[3] = extractTextIndex(fd, 22);
	_defaultResponseTbl[4] = extractTextIndex(fd, 19);
	_defaultResponseTbl[5] = extractTextIndex(fd, 20);
	_defaultResponseTbl[6] = extractTextIndex(fd, 21);
	_defaultResponseTbl[7] = extractTextIndex(fd, 22);
	_defaultResponseTbl[8] = extractTextIndex(fd, 19);

	_defaultResponseTbl[9] = extractTextIndex(fd, 0);
	_defaultResponseTbl[10] = extractTextIndex(fd, 1);
	_defaultResponseTbl[11] = extractTextIndex(fd, 2);
	_defaultResponseTbl[12] = extractTextIndex(fd, 3);
	_defaultResponseTbl[13] = extractTextIndex(fd, 4);
	_defaultResponseTbl[14] = extractTextIndex(fd, 5);
	_defaultResponseTbl[15] = extractTextIndex(fd, 2);
	_defaultResponseTbl[16] = extractTextIndex(fd, 3);
	_defaultResponseTbl[17] = extractTextIndex(fd, 4);

	_defaultResponseTbl[18] = extractTextIndex(fd, 6);
	_defaultResponseTbl[19] = extractTextIndex(fd, 7);
	_defaultResponseTbl[20] = extractTextIndex(fd, 8);
	_defaultResponseTbl[21] = extractTextIndex(fd, 9);
	_defaultResponseTbl[22] = extractTextIndex(fd, 7);
	_defaultResponseTbl[23] = extractTextIndex(fd, 8);
	_defaultResponseTbl[24] = extractTextIndex(fd, 9);
	_defaultResponseTbl[25] = extractTextIndex(fd, 6);
	_defaultResponseTbl[26] = extractTextIndex(fd, 7);

	_defaultResponseTbl[27] = extractTextIndex(fd, 10);
	_defaultResponseTbl[28] = extractTextIndex(fd, 11);
	_defaultResponseTbl[29] = extractTextIndex(fd, 12);
	_defaultResponseTbl[30] = extractTextIndex(fd, 13);
	_defaultResponseTbl[31] = extractTextIndex(fd, 14);
	_defaultResponseTbl[32] = extractTextIndex(fd, 15);
	_defaultResponseTbl[33] = extractTextIndex(fd, 16);
	_defaultResponseTbl[34] = extractTextIndex(fd, 17);
	_defaultResponseTbl[35] = extractTextIndex(fd, 18);

	_defaultResponseTbl[36] = extractTextIndex(fd, 23);
	_defaultResponseTbl[37] = extractTextIndex(fd, 24);
	_defaultResponseTbl[38] = extractTextIndex(fd, 25);
	_defaultResponseTbl[39] = extractTextIndex(fd, 26);
	_defaultResponseTbl[40] = extractTextIndex(fd, 27);
	_defaultResponseTbl[41] = extractTextIndex(fd, 28);
	_defaultResponseTbl[42] = extractTextIndex(fd, 29);
	_defaultResponseTbl[43] = extractTextIndex(fd, 30);
	_defaultResponseTbl[44] = extractTextIndex(fd, 31);

	fd->close();
	delete fd;
}

uint32 Talk::strlenUTF16(uint16 *text) {
	uint32 i = 0;

	for (; text[i] != 0; i++) {
	}
	return i;
}

void Talk::drawDialogBox(uint32 x1, uint32 y1, uint32 x2, uint32 y2, uint16 unk) {
	debug(3, "drawTextDialogBox(%d, %d, %d, %d, %d)", x1, y1, x2, y2, unk);
	_vm->_fontManager->drawTextDialogBox(x1, y1, x2, y2);
}

uint16 *Talk::findCharInU16Str(uint16 *text, uint16 chr) {
	for (int i = 0; text[i] != 0; i++) {
		if (text[i] == chr) {
			return &text[i];
		}
	}

	return nullptr;
}

uint16 *Talk::UTF16ToUTF16Z(uint16 *dest, uint16 *src) {
	uint16 chr;
	uint16 *ptr;

	chr = *src;
	src++;
	ptr = dest + 1;
	*dest = chr;
	while (chr != 0) {
		chr = *src;
		src++;
		*ptr = chr;
		ptr++;
	}
	return dest;
}

uint16 Talk::findLastPositionOf5cChar(uint16 *text) {
	uint16 len = strlenUTF16(text);
	for (int i = len - 1; i >= 0; i--) {
		if (text[i] == 0x5c) {
			return i + 1;
		}
	}
	return len;
}

uint32 Talk::truncateDialogText(uint16 *srcText, uint16 *destText, uint32 srcLength, uint16 destLength) {
	uint16 destCurIndex;
	uint16 uVar1;
	uint16 uVar2;
	uint srcCurIndex;
	uint16 uVar3;
	uint uVar4;
	short chr;
	bool finished;

	finished = false;
	uVar3 = 0;
	srcCurIndex = 0;
	uVar2 = 0;
	destCurIndex = 0;
	uVar4 = 1;
	do {
		if (srcLength <= srcCurIndex) break;
		chr = srcText[srcCurIndex];
		destText[destCurIndex] = chr;
		uVar2 = uVar2 + 1;
		if ((chr == 0x5c) || (chr == 0)) {
			finished = true;
			uVar1 = destCurIndex;
			if (srcText[srcCurIndex + 1] == 0x5c) {
				uVar1 = destCurIndex + 3;
				destText[destCurIndex] = 0x2e;
				destText[destCurIndex + 1] = 0x2e;
				destText[destCurIndex + 2] = 0x2e;
			}
			destCurIndex = uVar1 - 1;
			uVar1 = uVar3;
			uVar3 = uVar1;
		} else {
			if (((((chr != 0x20) && (chr != 0x2e)) && (chr != 0x3f)) &&
				 ((chr != 0x21 && (uVar1 = uVar3, chr != 0x2d)))) ||
				((uVar1 = uVar3, srcText[srcCurIndex + 1] == 0 ||
								 (uVar1 = destCurIndex, srcText[srcCurIndex + 1] != 0x5c)))) {
				uVar3 = uVar1;
			}
		}
		srcCurIndex = srcCurIndex + 1;
		if ((destLength < uVar2) && (!finished)) {
			destText[uVar3] = 0;
			uVar2 = destCurIndex - uVar3;
			uVar4 = uVar4 + 1;
		}
		destCurIndex = destCurIndex + 1;
	} while (!finished);
	destText[destCurIndex] = 0;
	return uVar4 & 0xffff;
}

void Talk::clearDialogEntries() {
	for (Common::Array<TalkDialogEntry*>::iterator it = _dialogEntries.begin(); it != _dialogEntries.end(); it++) {
		delete *it;
	}
	_dialogEntries.clear();
}

void Talk::FUN_8001a7c4_clearDialogBoxMaybe() {
	_vm->_fontManager->clearText();
//	_vm->_fontManager->clearTextDialog((uint) _dat_8008e7e8_dialogBox_x1, (uint) _dat_8008e844_dialogBox_y1,
//					(uint) _dat_8008e848_dialogBox_x2, (uint) _dat_8008e874_dialogBox_y2);
}

void Talk::playDialogAudioDontWait(uint32 textIndex) {
	_vm->_sound->playSpeech(textIndex);
}

} // End of namespace Dragons
