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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/engine/kyra_rpg.h"
#include "kyra/sound/sound.h"

#include "backends/keymapper/keymap.h"
#include "backends/keymapper/action.h"

#include "common/func.h"
#include "common/system.h"

namespace Kyra {

KyraRpgEngine::KyraRpgEngine(OSystem *system, const GameFlags &flags) : KyraEngine_v1(system, flags), _numFlyingObjects(_flags.gameID == GI_LOL ? 8 : 10) {
	_txt = 0;
	_mouseClick = 0;
	_preserveEvents = _buttonListChanged = false;

	_sceneXoffset = 0;
	_sceneShpDim = 5;

	_activeButtons = 0;

	_currentLevel = 0;

	_vcnBlocks = 0;
	_vcfBlocks = 0;
	_vcnTransitionMask = _vcnMaskTbl = 0;
	_vcnShift = 0;
	_vcnColTable = 0;
	_vcnShiftVal = 0;
	_vcnBpp = flags.useHiColorMode ? 2 : 1;
	_vcnSrcBitsPerPixel = (flags.platform == Common::kPlatformAmiga) ? 5 : (_vcnBpp == 2 ? 8 : 4);
	_vcnDrawLine = 0;
	_vmpVisOffs = (flags.platform == Common::kPlatformSegaCD) ? _vmpOffsetsSegaCD : _vmpOffsetsDefault;

	_vmpPtr = 0;
	_blockBrightness = _wllVcnOffset = _wllVcnOffset2 = _wllVcnRmdOffset = 0;
	_blockDrawingBuffer = 0;
	_sceneWindowBuffer = 0;
	_monsterShapes = _monsterPalettes = 0;

	_doorShapes = 0;

	_levelDecorationProperties = 0;
	_levelDecorationData = 0;
	_levelDecorationShapes = 0;
	_decorationCount = 0;
	_mappedDecorationsCount = 0;
	memset(_visibleBlockIndex, 0, sizeof(_visibleBlockIndex));

	_lvlShapeTop = _lvlShapeBottom = _lvlShapeLeftRight = 0;
	_levelBlockProperties = 0;
	_hasTempDataFlags = 0;

	_wllVmpMap = _specialWallTypes = _wllWallFlags = 0;
	_wllShapeMap = 0;

	_sceneDrawVarDown = _sceneDrawVarRight = _sceneDrawVarLeft = _wllProcessFlag = 0;

	_currentBlock = 0;
	_currentDirection = 0;
	_compassDirection = -1;
	_updateFlags = _clickedSpecialFlag = 0;
	_sceneDefaultUpdate = 0;
	_sceneUpdateRequired = false;

	_flyingObjectsPtr = 0;
	_flyingObjectStructSize = sizeof(EoBFlyingObject);

	_clickedShapeXOffs = _clickedShapeYOffs = 0;

	_dscShapeX = 0;
	_dscTileIndex = 0;
	_dscDoorScaleOffs = 0;
	_dscDim1 = 0;
	_dscDim2 = 0;
	_dscBlockMap = 0;
	_dscBlockIndex = 0;
	_dscShapeIndex = 0;
	_dscDimMap = 0;
	_dscDoorShpIndex = 0;
	_dscDoorY2 = 0;
	_dscDoorFrameY1 = 0;
	_dscDoorFrameY2 = 0;
	_dscDoorFrameIndex1 = 0;
	_dscDoorFrameIndex2 = 0;

	_shpDmX1 = _shpDmX2 = 0;

	memset(_openDoorState, 0, sizeof(_openDoorState));
	memset(_dialogueButtonString, 0, 3 * sizeof(const char *));
	_dialogueButtonPosX = 0;
	_dialogueButtonPosY = 0;
	_dialogueNumButtons = _dialogueButtonXoffs = _dialogueButtonYoffs = _dialogueHighlightedButton = 0;
	_currentControlMode = 0;
	_specialSceneFlag = 0;
	_updateCharNum = -1;
	_activeVoiceFileTotalTime = 0;
	_updatePortraitSpeechAnimDuration = _resetPortraitAfterSpeechAnim = _needSceneRestore = 0;
	_fadeText = false;

	memset(_lvlTempData, 0, sizeof(_lvlTempData));

	_dialogueField = false;
	_dialogueFieldAmiga = true;

	_environmentSfx = _environmentSfxVol = _envSfxDistThreshold = 0;
	_monsterStepCounter = _monsterStepMode = 0;

	_buttonFont = Screen::FID_6_FNT;
	if (_flags.use16ColorMode)
		_buttonFont = _flags.gameID == GI_LOL ? Screen::FID_SJIS_TEXTMODE_FNT : Screen::FID_SJIS_FNT;
	else if (_flags.gameID == GI_EOB2 && _flags.platform == Common::kPlatformFMTowns)
		_buttonFont = Screen::FID_8_FNT;
}

KyraRpgEngine::~KyraRpgEngine() {
	delete[] _wllVmpMap;
	delete[] _wllShapeMap;
	delete[] _specialWallTypes;
	delete[] _wllWallFlags;

	delete[] _vmpPtr;
	delete[] _vcnColTable;
	delete[] _vcnBlocks;
	delete[] _vcfBlocks;
	delete[] _vcnTransitionMask;
	delete[] _vcnShift;
	delete[] _blockDrawingBuffer;
	delete[] _sceneWindowBuffer;
	delete _vcnDrawLine;

	delete[] _lvlShapeTop;
	delete[] _lvlShapeBottom;
	delete[] _lvlShapeLeftRight;

	delete[] _doorShapes;

	delete[] _levelDecorationShapes;
	delete[] _levelDecorationData;
	delete[] _levelDecorationProperties;
	delete[] _levelBlockProperties;
}

Common::Error KyraRpgEngine::init() {
	gui_resetButtonList();

	_levelDecorationProperties = new LevelDecorationProperty[100];
	memset(_levelDecorationProperties, 0, 100 * sizeof(LevelDecorationProperty));
	_levelDecorationShapes = new const uint8*[400];
	memset(_levelDecorationShapes, 0, 400 * sizeof(uint8 *));
	_levelBlockProperties = new LevelBlockProperty[1025];
	memset(_levelBlockProperties, 0, 1025 * sizeof(LevelBlockProperty));

	_wllVmpMap = new uint8[256];
	memset(_wllVmpMap, 0, 256);
	_wllShapeMap = new int8[256];
	memset(_wllShapeMap, 0, 256);
	_specialWallTypes = new uint8[256];
	memset(_specialWallTypes, 0, 256);
	_wllWallFlags = new uint8[256];
	memset(_wllWallFlags, 0, 256);

	_blockDrawingBuffer = new uint16[1320];
	memset(_blockDrawingBuffer, 0, 1320 * sizeof(uint16));
	int windowBufferSize = _flags.useHiColorMode ? 42240 : 21120;
	_sceneWindowBuffer = new uint8[windowBufferSize];
	memset(_sceneWindowBuffer, 0, windowBufferSize);

	_lvlShapeTop = new int16[18];
	memset(_lvlShapeTop, 0, 18 * sizeof(int16));
	_lvlShapeBottom = new int16[18];
	memset(_lvlShapeBottom, 0, 18 * sizeof(int16));
	_lvlShapeLeftRight = new int16[36];
	memset(_lvlShapeLeftRight, 0, 36 * sizeof(int16));

	_vcnColTable = new uint8[128];
	for (int i = 0; i < 128; i++)
		_vcnColTable[i] = i & 0x0F;

	if (_vcnBpp == 2)
		_vcnDrawLine = new VcnLineDrawingMethods(new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_fw_hiCol), new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_bw_hiCol),
			new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_fw_trans_hiCol), new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_bw_trans_hiCol));
	else if (_flags.platform == Common::kPlatformAmiga || (_flags.gameID == GI_EOB1 && _flags.use16ColorMode))
		_vcnDrawLine = new VcnLineDrawingMethods(new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_fw_planar), new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_bw_planar),
			new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_fw_trans_planar), new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_bw_trans_planar));
	else
		_vcnDrawLine = new VcnLineDrawingMethods(new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_fw_4bit), new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_bw_4bit),
			new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_fw_trans_4bit), new VcnDrawProc(this, &KyraRpgEngine::vcnDraw_bw_trans_4bit));

	_doorShapes = new uint8*[6];
	memset(_doorShapes, 0, 6 * sizeof(uint8 *));

	initStaticResource();

	_envSfxDistThreshold = ((_flags.gameID == GI_EOB2 && _sound->getSfxType() == Sound::kTowns) || _sound->getSfxType() == Sound::kAdLib || _sound->getSfxType() == Sound::kPCSpkr) ? 15 : (_sound->getSfxType() == Sound::kAmiga ? 4 : 3);

	_dialogueButtonLabelColor1 = guiSettings()->buttons.labelColor1;
	_dialogueButtonLabelColor2 = guiSettings()->buttons.labelColor2;
	_dialogueButtonWidth = guiSettings()->buttons.width;

	return Common::kNoError;
}

void KyraRpgEngine::addKeymapAction(Common::Keymap *const keyMap, const char *actionId, const Common::U32String &actionDesc, const Common::Functor0Mem<void, Common::Action>::FuncType setEventProc, const Common::String &mapping1, const Common::String &mapping2) {
	Common::Action *act = new Common::Action(actionId, actionDesc);
	Common::Functor0Mem<void, Common::Action>(act, setEventProc)();
	act->addDefaultInputMapping(mapping1);
	act->addDefaultInputMapping(mapping2);
	keyMap->addAction(act);
}

void KyraRpgEngine::addKeymapAction(Common::Keymap *const keyMap, const char *actionId, const Common::U32String &actionDesc, Common::KeyState eventKeyState, const Common::String &mapping1, const Common::String &mapping2) {
	Common::Action *act = new Common::Action(actionId, actionDesc);
	act->setKeyEvent(eventKeyState);
	act->addDefaultInputMapping(mapping1);
	act->addDefaultInputMapping(mapping2);
	keyMap->addAction(act);
}

bool KyraRpgEngine::posWithinRect(int posX, int posY, int x1, int y1, int x2, int y2) {
	if (posX < x1 || posX > x2 || posY < y1 || posY > y2)
		return false;
	return true;
}

void KyraRpgEngine::drawDialogueButtons() {
	int cp = screen()->setCurPage(0);
	Screen::FontId of = screen()->setFont(_buttonFont);

	for (int i = 0; i < _dialogueNumButtons; i++) {
		int x = _dialogueButtonPosX[i];
		if (_flags.gameID == GI_LOL && _flags.use16ColorMode) {
			gui_drawBox(x, ((_dialogueButtonYoffs + _dialogueButtonPosY[i]) & ~7) - 1, 74, 10, 0xEE, 0xCC, -1);
			screen()->printText(_dialogueButtonString[i], (x + 37 - (screen()->getTextWidth(_dialogueButtonString[i])) / 2) & ~3,
			                    ((_dialogueButtonYoffs + _dialogueButtonPosY[i]) + 2) & ~7, _dialogueHighlightedButton == i ? 0xC1 : 0xE1, 0);
		} else {
			int yOffset = guiSettings()->buttons.txtOffsY;
			if (_flags.gameID == GI_LOL && _flags.lang == Common::JA_JPN && (_dialogueButtonString[i][0] & 0x80))
				yOffset = 0;
			screen()->set16bitShadingLevel(4);
			gui_drawBox(x, (_dialogueButtonYoffs + _dialogueButtonPosY[i]), _dialogueButtonWidth, guiSettings()->buttons.height, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
			screen()->set16bitShadingLevel(0);
			screen()->printText(_dialogueButtonString[i], x + (_dialogueButtonWidth >> 1) - (screen()->getTextWidth(_dialogueButtonString[i])) / 2,
			                    (_dialogueButtonYoffs + _dialogueButtonPosY[i]) + yOffset, _dialogueHighlightedButton == i ? _dialogueButtonLabelColor1 : _dialogueButtonLabelColor2, 0);
		}
	}
	screen()->setFont(of);
	screen()->setCurPage(cp);
}

uint16 KyraRpgEngine::processDialogue() {
	int df = _dialogueHighlightedButton;
	int res = 0;

	for (int i = 0; i < _dialogueNumButtons; i++) {
		int x = _dialogueButtonPosX[i] + _dialogueButtonXoffs;
		int y = ((_flags.gameID == GI_LOL && _flags.use16ColorMode) ? ((_dialogueButtonYoffs + _dialogueButtonPosY[i]) & ~7) - 1 : (_dialogueButtonYoffs + _dialogueButtonPosY[i]));
		Common::Point p = getMousePos();
		if (posWithinRect(p.x, p.y, x, y, x + _dialogueButtonWidth, y + guiSettings()->buttons.height)) {
			_dialogueHighlightedButton = i;
			break;
		}
	}

	if (_dialogueNumButtons == 0) {
		int e = checkInput(0, false) & 0xFF;
		removeInputTop();

		if (e) {
			gui_notifyButtonListChanged();

			if (e == _keyMap[Common::KEYCODE_SPACE] || e == _keyMap[Common::KEYCODE_RETURN]) {
				snd_stopSpeech(true);
			}
		}

		if (snd_updateCharacterSpeech() != 2) {
			res = 1;
			if (!shouldQuit()) {
				removeInputTop();
				gui_notifyButtonListChanged();
			}
		}
	} else {
		int e = checkInput(0, false, 0) & 0xFF;
		removeInputTop();
		if (e)
			gui_notifyButtonListChanged();

		if ((_flags.gameID == GI_LOL && (e == 200 || e == 202)) || (_flags.gameID != GI_LOL && (e == 199 || e == 201))) {
			for (int i = 0; i < _dialogueNumButtons; i++) {
				int x = _dialogueButtonPosX[i];
				int y = ((_flags.gameID == GI_LOL && _flags.use16ColorMode) ? ((_dialogueButtonYoffs + _dialogueButtonPosY[i]) & ~7) - 1 : (_dialogueButtonYoffs + _dialogueButtonPosY[i]));
				Common::Point p = getMousePos();
				if (posWithinRect(p.x, p.y, x, y, x + _dialogueButtonWidth, y + guiSettings()->buttons.height)) {
					_dialogueHighlightedButton = i;
					res = _dialogueHighlightedButton + 1;
					break;
				}
			}
		} else if (e == _keyMap[Common::KEYCODE_SPACE] || e == _keyMap[Common::KEYCODE_RETURN]) {
			snd_stopSpeech(true);
			res = _dialogueHighlightedButton + 1;
		} else if (e == _keyMap[Common::KEYCODE_LEFT] || e == _keyMap[Common::KEYCODE_DOWN]) {
			if (_dialogueNumButtons > 1 && _dialogueHighlightedButton > 0)
				_dialogueHighlightedButton--;
		} else if (e == _keyMap[Common::KEYCODE_RIGHT] || e == _keyMap[Common::KEYCODE_UP]) {
			if (_dialogueNumButtons > 1 && _dialogueHighlightedButton < (_dialogueNumButtons - 1))
				_dialogueHighlightedButton++;
		}
	}

	if (df != _dialogueHighlightedButton)
		drawDialogueButtons();

	screen()->updateScreen();

	if (res == 0)
		return 0;

	stopPortraitSpeechAnim();

	if (game() == GI_LOL) {
		if (!textEnabled() && _currentControlMode) {
			screen()->setScreenDim(5);
			const ScreenDim *d = screen()->getScreenDim(5);
			screen()->fillRect(d->sx, d->sy + d->h - 9, d->sx + d->w - 1, d->sy + d->h - 1, d->unkA);
		} else {
			const ScreenDim *d = screen()->_curDim;
			if (gameFlags().use16ColorMode)
				screen()->fillRect(d->sx, d->sy, d->sx + d->w - 3, d->sy + d->h - 2, d->unkA);
			else
				screen()->fillRect(d->sx, d->sy, d->sx + d->w - 2, d->sy + d->h - 1, d->unkA);
			txt()->clearDim(4);
			txt()->resetDimTextPositions(4);
		}
	}

	return res;
}

void KyraRpgEngine::delayUntil(uint32 time, bool, bool doUpdate, bool isMainLoop) {
	uint32 curTime = _system->getMillis();
	if (time > curTime)
		delay(time - curTime, doUpdate, isMainLoop);
}

int KyraRpgEngine::rollDice(int times, int pips, int inc) {
	if (times <= 0 || pips <= 0)
		return inc;

	int res = 0;
	while (times--)
		res += _rnd.getRandomNumberRng(1, pips);

	return res + inc;
}

bool KyraRpgEngine::snd_processEnvironmentalSoundEffect(int soundId, int block) {
	if (!_sound->sfxEnabled() || shouldQuit())
		return false;

	if (_environmentSfx)
		snd_playSoundEffect(_environmentSfx, _environmentSfxVol);

	int dist = 0;
	if (block) {
		dist = getBlockDistance(_currentBlock, block);
		if (dist > _envSfxDistThreshold) {
			_environmentSfx = 0;
			return false;
		}
	}

	_environmentSfx = soundId;

	if (_flags.gameID == GI_EOB2 && _flags.platform == Common::kPlatformFMTowns)
		_environmentSfxVol = dist ? (16 - dist) * 8 - 1 : 127;
	else if (_flags.platform == Common::kPlatformAmiga)
		_environmentSfxVol = dist ? (soundId != 13 ? dist : (dist >= 4) ? 4 : dist) : 1;
	else if (_flags.platform == Common::kPlatformSegaCD)
		_environmentSfxVol = dist < 3 ? 15 - dist : 11;
	else
		_environmentSfxVol = (15 - ((block || (_flags.gameID == GI_LOL && dist < 2)) ? dist : 0)) << 4;

	return true;
}

void KyraRpgEngine::snd_updateEnvironmentalSfx(int soundId) {
	snd_processEnvironmentalSoundEffect(soundId, _currentBlock);
}

} // End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL
