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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/puzzles/hivecontrol.h"

#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

const Control puzzleHiveControlIntToControl[] = {
	kControlWingsButton1,
	kControlWingsButton2,
	kControlWingsButton3,
	kControlReset,
	kControlWheelLeft,
	kControlWheelRight,
	kControlButtonRight,
	kControlButtonLeft,
	kControlGlyph1,
	kControlGlyph2,
	kControlGlyph3,
	kControlGlyph4,
	kControlGlyph5,
	kControlGlyph6
};

PuzzleHiveControl::PuzzleHiveControl(AsylumEngine *engine) : Puzzle(engine) {
	_rectIndex = 0;
	_soundVolume = 0;
	_counter = 0;

	_data_457260 = 0;
	_data_457264 = 0;

	_prevLeverPosition = 3;
	_resetFlag = false;
	memset(&_frameIndexes, 	0, sizeof(_frameIndexes));
	reset();
}

PuzzleHiveControl::~PuzzleHiveControl() {
}

void PuzzleHiveControl::saveLoadWithSerializer(Common::Serializer &s) {
	// TODO
	s.skip(6 * 4 * 2);
	debugC(kDebugLevelSavegame, "[PuzzleHiveControl::saveLoadWithSerializer] Not implemented");

	s.syncAsSint32LE(_soundVolume);
}

void PuzzleHiveControl::reset() {
	_leverPosition = 3;
	_leverDelta = 0;
	_currentControl = kControlNone;
	_colorL = _colorR = 0;
	_frameIndexOffset = 0;

	memset(&_glyphFlags, 	false, sizeof(_glyphFlags));
	memset(&_wingsState, 	false, sizeof(_wingsState));

	_frameIndexes[kElementSwirlRim] = 0;
	if (_leverPosition != _prevLeverPosition) {
		_leverDelta = (uint32)abs((double)_leverPosition - (double)_prevLeverPosition) * 16 / 5;
		_currentControl = kControlGlyph4;
	}
}

//////////////////////////////////////////////////////////////////////////
// Event Handling
//////////////////////////////////////////////////////////////////////////
bool PuzzleHiveControl::init(const AsylumEvent &) {
	_controlPoints[kControlWingsButton1] = Common::Point(338, 139);
	_controlPoints[kControlWingsButton2] = Common::Point(376, 151);
	_controlPoints[kControlWingsButton3] = Common::Point(403, 162);
	_controlPoints[kControlReset]        = Common::Point(219,  86);
	_controlPoints[kControlWheelRight]   = Common::Point(204, 263);
	_controlPoints[kControlWheelLeft]    = Common::Point(164, 310);
	_controlPoints[kControlButtonLeft]   = Common::Point(320, 375);
	_controlPoints[kControlButtonRight]  = Common::Point(363, 337);
	_controlPoints[kControlGlyph1]       = Common::Point(102, 201);
	_controlPoints[kControlGlyph2]       = Common::Point(101, 171);
	_controlPoints[kControlGlyph3]       = Common::Point(108, 140);
	_controlPoints[kControlGlyph4]       = Common::Point(126, 111);
	_controlPoints[kControlGlyph5]       = Common::Point(140,  85);
	_controlPoints[kControlGlyph6]       = Common::Point(161,  54);

	_rectIndex = -2;
	_frameIndexes[kElementLever] = (5 - _leverPosition) * (GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementLever]) - 1) / 5;

	getScreen()->setPalette(getWorld()->graphicResourceIds[29]);
	getScreen()->setGammaLevel(getWorld()->graphicResourceIds[29]);

	getCursor()->show();

	getScreen()->setupTransTables(1, getWorld()->graphicResourceIds[70]);
	getScreen()->selectTransTable(0);

	getSound()->playSound(getWorld()->graphicResourceIds[73], true, _soundVolume);
	getSound()->playSound(getWorld()->graphicResourceIds[74], true, Config.ambientVolume);

	return true;
}

bool PuzzleHiveControl::mouseLeftDown(const AsylumEvent &) {
	if (_currentControl != kControlNone)
		return true;

	_currentControl = findControl();
	switch (_currentControl) {
	case kControlNone:
		break;

	case kControlWingsButton1:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[81], false, Config.sfxVolume - 10);

		if (_wingsState[1] != _wingsState[2]) {
			if (_wingsState[0])
				--_frameIndexOffset;
			else
				++_frameIndexOffset;
		}

		_frameIndexOffset += _wingsState[0] ? -1 : 1;
		_wingsState[0] = !_wingsState[0];
		_frameIndexes[kElementLensLeft] = _colorL * 8 + _frameIndexOffset;
		_frameIndexes[kElementLensRight] = _colorR * 8 + _frameIndexOffset;
		break;

	case kControlWingsButton2:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[81], false, Config.sfxVolume - 10);

		if (_wingsState[0] != _wingsState[2]) {
			if (_wingsState[1])
				--_frameIndexOffset;
			else
				++_frameIndexOffset;
		}

		_frameIndexOffset += _wingsState[1] ? -2 : 2;
		_wingsState[1] = !_wingsState[1];
		_frameIndexes[kElementLensLeft] = _colorL * 8 + _frameIndexOffset;
		_frameIndexes[kElementLensRight] = _colorR * 8 + _frameIndexOffset;
		break;

	case kControlWingsButton3:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[81], false, Config.sfxVolume - 10);

		if (_wingsState[0] != _wingsState[1]) {
			if (_wingsState[2])
				--_frameIndexOffset;
			else
				++_frameIndexOffset;
		}

		_frameIndexOffset += (_wingsState[2] ? -3 : 3);
		_wingsState[2] = !_wingsState[2];
		_frameIndexes[kElementLensLeft] = _colorL * 8 + _frameIndexOffset;
		_frameIndexes[kElementLensRight] = _colorR * 8 + _frameIndexOffset;
		break;

	case kControlReset:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[78], false, Config.sfxVolume - 10);
		getSound()->playSound(getWorld()->graphicResourceIds[79], false, Config.sfxVolume - 10);
		_resetFlag = true;
		reset();
		break;

	case kControlWheelLeft:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[80], false, Config.sfxVolume - 10);
		_colorL = (_colorL + 1) % 3;
		break;

	case kControlWheelRight:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[80], false, Config.sfxVolume - 10);
		_colorR = (_colorR + 1) % 3;
		break;

	case kControlButtonLeft:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[77], false, Config.sfxVolume - 10);

		if (!_glyphFlags[0][_leverPosition]) {
			_glyphFlags[0][_leverPosition] = puzzleHiveControlHieroglyphs[0][_leverPosition] == _frameIndexes[kElementLensLeft];
			if (_glyphFlags[0][_leverPosition]) {
				getSound()->playSound(getWorld()->graphicResourceIds[83], false, Config.sfxVolume - 10);
				++_frameIndexes[kElementSwirlRim];

				// Check for puzzle completion
				if (_frameIndexes[kElementSwirlRim] == 12) {
					getSound()->stop(getWorld()->graphicResourceIds[73]);
					getSound()->stop(getWorld()->graphicResourceIds[74]);

					getScreen()->clear();
					getScreen()->setupTransTables(3, getWorld()->cellShadeMask1, getWorld()->cellShadeMask2, getWorld()->cellShadeMask3);
					getScreen()->selectTransTable(1);

					_vm->setGameFlag(kGameFlagSolveHiveControl);
					_vm->switchEventHandler(getScene());
				}
			}
		}
		break;

	case kControlButtonRight:
		getCursor()->hide();
		getSound()->playSound(getWorld()->graphicResourceIds[77], false, Config.sfxVolume - 10);
		if (!_glyphFlags[1][_leverPosition]) {
			_glyphFlags[1][_leverPosition] = puzzleHiveControlHieroglyphs[1][_leverPosition] == _frameIndexes[kElementLensRight];
			if (_glyphFlags[1][_leverPosition]) {
				getSound()->playSound(getWorld()->graphicResourceIds[83], false, Config.sfxVolume - 10);
				++_frameIndexes[kElementSwirlRim];

				// Check for puzzle completion
				if (_frameIndexes[kElementSwirlRim] == 12) {
					getSound()->stop(getWorld()->graphicResourceIds[73]);
					getSound()->stop(getWorld()->graphicResourceIds[74]);

					getScreen()->clear();
					getScreen()->setupTransTables(3, getWorld()->cellShadeMask1, getWorld()->cellShadeMask2, getWorld()->cellShadeMask3);
					getScreen()->selectTransTable(1);

					_vm->setGameFlag(kGameFlagSolveHiveControl);
					_vm->switchEventHandler(getScene());
				}
			}
		}
		break;

	case kControlGlyph1:
	case kControlGlyph2:
	case kControlGlyph3:
	case kControlGlyph4:
	case kControlGlyph5:
	case kControlGlyph6:
		_leverPosition = _currentControl - 49;
		_leverDelta = (uint32)abs((double)_leverPosition - (double)_prevLeverPosition) * (GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementLever]) - 1) / 5;
		if (_leverDelta)
			getSound()->playSound(getWorld()->graphicResourceIds[76], false, Config.sfxVolume - 10);
	}

	return true;
}

bool PuzzleHiveControl::mouseRightDown(const AsylumEvent &) {
	reset();

	getSound()->stop(getWorld()->graphicResourceIds[73]);
	getSound()->stop(getWorld()->graphicResourceIds[74]);

	getScreen()->clear();
	getScreen()->setupTransTables(3, getWorld()->cellShadeMask1, getWorld()->cellShadeMask2, getWorld()->cellShadeMask3);
	getScreen()->selectTransTable(1);

	_vm->switchEventHandler(getScene());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void PuzzleHiveControl::updateCursor() {
	int32 index = findControl();

	if (_rectIndex == index)
		return;

	_rectIndex = index;
	if (index == -1)
		getCursor()->set(getWorld()->graphicResourceIds[30], -1, kCursorAnimationNone);
	else
		getCursor()->set(getWorld()->graphicResourceIds[30], -1);
}

Control PuzzleHiveControl::findControl() {
	for (uint32 i = 0; i < ARRAYSIZE(puzzleHiveControlIntToControl); ++i)
		if (hitTest1(puzzleHiveControlIntToControl[i], getCursor()->position(), _controlPoints[puzzleHiveControlIntToControl[i]]))
			return puzzleHiveControlIntToControl[i];

	return kControlNone;
}

void PuzzleHiveControl::updateScreen() {
	getScreen()->clearGraphicsInQueue();
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[28], 0, Common::Point(0, 0), kDrawFlagNone, 0, 3);

	switch (_currentControl) {
	case kControlNone:
		break;
	case kControlGlyph1:
	case kControlGlyph2:
	case kControlGlyph3:
	case kControlGlyph4:
	case kControlGlyph5:
	case kControlGlyph6:
		if (_leverDelta) {
			if (_leverPosition > _prevLeverPosition)
				--_frameIndexes[kElementLever];
			else
				++_frameIndexes[kElementLever];
			--_leverDelta;
			if (_leverDelta == 0) {
				_prevLeverPosition = _leverPosition;
				_currentControl = kControlNone;
			}
		} else
			_currentControl = kControlNone;
		break;
	default:
		_frameIndexes[_currentControl] = (_frameIndexes[_currentControl] + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[_currentControl]);
		if (_frameIndexes[_currentControl] == 0) {
			getCursor()->show();
			if (_currentControl == kControlWheelLeft || _currentControl == kControlWheelRight) {
				getSound()->playSound(getWorld()->graphicResourceIds[75], false, Config.sfxVolume - 10);
				if (_currentControl == kControlWheelLeft)
					_frameIndexes[kElementLensLeft] = (_frameIndexes[kElementLensLeft] + 8) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementLensLeft]);
				else
					_frameIndexes[kElementLensRight] = (_frameIndexes[kElementLensRight] + 8) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementLensRight]);
			}
			_currentControl = kControlNone;
		}
	}

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementSwirl], _frameIndexes[kElementSwirl], Common::Point(486, 291), kDrawFlagNone, 0, 2);
	_frameIndexes[kElementSwirl] = (_frameIndexes[kElementSwirl] + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementSwirl]);

	if (_resetFlag) {
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementResetDynamic], _frameIndexes[kElementResetDynamic], Common::Point(211, 77), kDrawFlagNone, 0, 2);
		_frameIndexes[kElementResetDynamic] = (_frameIndexes[kElementResetDynamic] + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementResetDynamic]);
		if (_frameIndexes[kElementResetDynamic] == 0) {
			_resetFlag = false;
			getCursor()->show();
			if (!(_wingsState[0] || _wingsState[1] || _wingsState[2])) {
				_frameIndexes[kElementLensLeft] = 0;
				_frameIndexes[kElementLensRight] = 0;
			}
		}
	} else {
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementFlyHead],	_frameIndexes[kElementFlyHead], Common::Point(258, 86), kDrawFlagNone, 0, 2);
		getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementResetStatic], _frameIndexes[kElementResetStatic], Common::Point(232, 77), kDrawFlagNone, 0, 2);
		_frameIndexes[kElementFlyHead] = (_frameIndexes[kElementFlyHead] + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementFlyHead]);
	}

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlWingsButton1],	_frameIndexes[kControlWingsButton1],	_controlPoints[kControlWingsButton1],	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlWingsButton2],	_frameIndexes[kControlWingsButton2],	_controlPoints[kControlWingsButton2],	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlWingsButton3],	_frameIndexes[kControlWingsButton3],	_controlPoints[kControlWingsButton3],	kDrawFlagNone, 0, 2);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementLever],		_frameIndexes[kElementLever],		Common::Point(9,    40),		kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlReset],		_frameIndexes[kControlReset],		_controlPoints[kControlReset],		kDrawFlagNone, 0, 2);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlWheelRight],	_frameIndexes[kControlWheelRight],	_controlPoints[kControlWheelRight],	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlWheelLeft],	_frameIndexes[kControlWheelLeft],	_controlPoints[kControlWheelLeft],	kDrawFlagNone, 0, 2);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlButtonLeft],	_frameIndexes[kControlButtonLeft],	_controlPoints[kControlButtonLeft],	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlButtonRight],	_frameIndexes[kControlButtonRight],	_controlPoints[kControlButtonRight],	kDrawFlagNone, 0, 2);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementWingLeft1],	_frameIndexes[kElementWingLeft1],	Common::Point(326, 162),		kDrawFlagNone, 1, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementWingRight1],	_frameIndexes[kElementWingRight1],	Common::Point(374,  86),		kDrawFlagNone, 1, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementWingLeft2],	_frameIndexes[kElementWingLeft2],	Common::Point(275, 186),		kDrawFlagNone, 1, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementWingRight2],	_frameIndexes[kElementWingRight2],	Common::Point(419,  59),		kDrawFlagNone, 1, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementWingLeft3],	_frameIndexes[kElementWingLeft3],	Common::Point(386, 196),		kDrawFlagNone, 1, 1);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementWingRight3],	_frameIndexes[kElementWingRight3],	Common::Point(433, 111),		kDrawFlagNone, 1, 1);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlGlyph1],		_frameIndexes[kControlGlyph1],		_controlPoints[kControlGlyph1], 	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlGlyph2],		_frameIndexes[kControlGlyph2],		_controlPoints[kControlGlyph2], 	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlGlyph3],		_frameIndexes[kControlGlyph3],		_controlPoints[kControlGlyph3], 	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlGlyph4],		_frameIndexes[kControlGlyph4],		_controlPoints[kControlGlyph4], 	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlGlyph5],		_frameIndexes[kControlGlyph5],		_controlPoints[kControlGlyph5], 	kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kControlGlyph6],		_frameIndexes[kControlGlyph6],		_controlPoints[kControlGlyph6], 	kDrawFlagNone, 0, 2);

	bool reseted = false;
	for (uint32 i = 0; i < 3; ++i) {
		if (_wingsState[i]) {
			if (_frameIndexes[kElementWingLeft1 + 2*i] != GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementWingLeft1 + 2*i]) - 1)
				_frameIndexes[kElementWingLeft1 + 2*i] = (_frameIndexes[kElementWingLeft1 + 2*i] + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementWingLeft1 + 2*i]);
			if (_frameIndexes[kElementWingRight1 + 2*i] != GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementWingRight1 + 2*i]) - 1)
				_frameIndexes[kElementWingRight1 + 2*i] = (_frameIndexes[kElementWingRight1 + 2*i] + 1) % GraphicResource::getFrameCount(_vm, getWorld()->graphicResourceIds[kElementWingRight1 + 2*i]);
		} else {
			if (_frameIndexes[kElementWingLeft1 + 2*i] > 0) {
				--_frameIndexes[kElementWingLeft1 + 2*i];
				if (_resetFlag && _frameIndexes[kElementWingLeft1 + 2*i] == 0 && !reseted) {
					_frameIndexes[kElementLensLeft] = 0;
					reseted = true;
				}
			}
			if (_frameIndexes[kElementWingRight1 + 2*i ] > 0) {
				--_frameIndexes[kElementWingRight1 + 2*i];
				if (_resetFlag && _frameIndexes[kElementWingRight1 + 2*i] == 0 && !reseted) {
					_frameIndexes[kElementLensRight] = 0;
					reseted = true;
				}
			}
		}
	}

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementLensLeft], _frameIndexes[kElementLensLeft], Common::Point(305, 216), kDrawFlagNone, 0, 2);
	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementLensRight], _frameIndexes[kElementLensRight], Common::Point(411, 65), kDrawFlagNone, 0, 2);

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementIndicator], _frameIndexes[kElementSwirlRim], Common::Point(158, 148), kDrawFlagNone, 0, 2);

	for (uint32 i = 0; i < 6; ++i) {
		if (_glyphFlags[0][i])
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[58 + i], 0, Common::Point(104, 58), kDrawFlagNone, 0, 1);
		if (_glyphFlags[1][i])
			getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[64 + i], 0, Common::Point(133, 70), kDrawFlagNone, 0, 1);
	}

	getScreen()->addGraphicToQueue(getWorld()->graphicResourceIds[kElementSwirlRim], _frameIndexes[kElementSwirlRim], Common::Point(458, 278), kDrawFlagNone, 0, 2);

	if (!_data_457260 && !_data_457264)
		playSound();

	if (_counter) {
		if (_counter < 30 || getSound()->isPlaying(getWorld()->graphicResourceIds[83])) {
			++_counter;
		} else {
			AsylumEvent evt;
			mouseRightDown(evt);
			getCursor()->show();
		}
	}
}

void PuzzleHiveControl::playSound() {
	// TODO
}

bool PuzzleHiveControl::hitTest1(Control control, const Common::Point &point, const Common::Point &location) {
	if (control == kControlNone)
		error("[PuzzleHiveControl::hitTest1] Invalid control");

	GraphicResource resource(_vm);
	resource.load(getWorld()->graphicResourceIds[control]);
	GraphicFrame *frame = resource.getFrame(0);
	Common::Point point1(point.x - location.x, point.y - location.y);

	if (!frame->getRect().contains(point1)) {
		return false;
	} else {
		point1.x -= frame->x;
		point1.y -= frame->y;
		return *((byte *)frame->surface.getPixels() + point1.x + frame->surface.pitch * point1.y) != 0;
	}
}

} // End of namespace Asylum
