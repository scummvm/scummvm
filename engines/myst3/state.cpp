/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/myst3/state.h"

#include "common/savefile.h"

namespace Myst3 {

GameState::GameState(Myst3Engine *vm):
	_vm(vm) {

#define VAR(var, x, unk) _descriptions.setVal(var, Description(var, #x, unk));

	VAR(47, ProjectorAngleX, true)
	VAR(48, ProjectorAngleY, true)
	VAR(49, ProjectorAngleZoom, true)
	VAR(50, ProjectorAngleBlur, true)

	VAR(57, DragEnded, true)
	VAR(58, DragLeverSpeed, false)
	VAR(59, DragPositionFound, true)
	VAR(60, DragLeverPositionChanged, true)

	VAR(61, LocationAge, false)
	VAR(62, LocationRoom, false)
	VAR(63, LocationNode, false)
	VAR(64, BookSavedAge, false)
	VAR(65, BookSavedRoom, false)
	VAR(66, BookSavedNode, false)
	VAR(67, MenuSavedAge, false)
	VAR(68, MenuSavedRoom, false)
	VAR(69, MenuSavedNode, false)

	VAR(71, FrameCountdown, true)

	VAR(115, SunspotIntensity, false)
	VAR(116, SunspotColor, false)
	VAR(117, SunspotRadius, false)

	VAR(136, CameraPitch, false)
	VAR(137, CameraHeading, false)
	VAR(140, CameraMinPitch, false)
	VAR(141, CameraMaxPitch, false)

	VAR(142, MovieOverrideStartFrame, true)
	VAR(143, MovieOverrideEndFrame, true)
	VAR(144, MovieVolume1, true)
	VAR(145, MovieVolume2, true)
	VAR(146, MovieUnk146, true)
	VAR(147, MovieUnk147, true)
	VAR(148, MovieUnk148, true)
	VAR(149, MovieConditionBit, true)
	VAR(150, MoviePreloadToMemory, true)
	VAR(151, MovieScriptDriven, true)
	VAR(152, MovieNextFrameSetVar, true)
	VAR(153, MovieNextFrameGetVar, true)
	VAR(154, MovieStartFrameVar, true)
	VAR(155, MovieEndFrameVar, true)
	VAR(156, MovieForce2d, true)
	VAR(157, MovieVolumeVar, true)
	VAR(158, MovieSoundHeading, true)
	VAR(159, MoviePanningStrenght, true)
	VAR(160, MovieSynchronized, true)
	VAR(161, MovieUnk161, true)
	VAR(162, MovieUnk162, true)
	VAR(163, MovieOverrideCondition, true)
	VAR(164, MovieUVar, true)
	VAR(165, MovieVVar, true)
	VAR(166, MovieOverridePosition, true)
	VAR(167, MovieOverridePosU, true)
	VAR(168, MovieOverridePosV, true)
	VAR(169, MovieScale, true)
	VAR(170, MovieUnk170, true)
	VAR(171, MovieUnk171, true)
	VAR(172, MovieUnk172, true)
	VAR(173, MoviePlayingVar, true)

	VAR(178, MovieUseBackground, false)

	VAR(189, LocationNextNode, false)
	VAR(190, LocationNextRoom, false)
	VAR(191, LocationNextAge, false)

	VAR(243, ProjectorX, false)
	VAR(244, ProjectorY, false)
	VAR(245, ProjectorZoom, false)
	VAR(246, ProjectorBlur, false)
	VAR(247, ProjectorAngleXOffset, false)
	VAR(248, ProjectorAngleYOffset, false)
	VAR(249, ProjectorAngleZoomOffset, false)
	VAR(250, ProjectorAngleBlurOffset, false)

	VAR(277, JournalAtrusState, false)
	VAR(279, JournalSaavedroState, false)
	VAR(280, JournalSaavedroClosed, false)
	VAR(281, JournalSaavedroOpen, false)
	VAR(282, JournalSaavedroLastPage, false)
	VAR(283, JournalSaavedroChapter, false)
	VAR(284, JournalSaavedroPageInChapter, false)
	VAR(480, BookStateTomahna, false)
	VAR(481, BookStateReleeshahn, false)

	VAR(1337, MenuEscapePressed, false)
	VAR(1338, MenuNextAction, false)
	VAR(1339, MenuLoadBack, false)
	VAR(1340, MenuSaveBack, false)
	VAR(1341, MenuSaveAction, false)
	VAR(1342, MenuOptionsBack, false)

	VAR(1350, MenuSaveLoadPageLeft, false)
	VAR(1351, MenuSaveLoadPageRight, false)
	VAR(1352, MenuSaveLoadSelectedItem, false)
	VAR(1353, MenuSaveLoadCurrentPage, false)

	VAR(1399, DragLeverLimited, true)
	VAR(1400, DragLeverLimitMin, true)
	VAR(1401, DragLeverLimitMax, true)

#undef VAR

	newGame();
}

GameState::~GameState() {
}

void GameState::syncWithSaveGame(Common::Serializer &s) {
	if (!s.syncVersion(kSaveVersion))
		error("This savegame (v%d) is too recent (max %d) please get a newer version of Residual", s.getVersion(), kSaveVersion);

	s.syncAsUint32LE(_data.gameRunning);
	s.syncAsUint32LE(_data.currentFrame);
	s.syncAsUint32LE(_data.dword_4C2C3C);
	s.syncAsUint32LE(_data.dword_4C2C40);
	s.syncAsUint32LE(_data.dword_4C2C44);
	s.syncAsUint32LE(_data.dword_4C2C48);
	s.syncAsUint32LE(_data.dword_4C2C4C);
	s.syncAsUint32LE(_data.dword_4C2C50);
	s.syncAsUint32LE(_data.dword_4C2C54);
	s.syncAsUint32LE(_data.dword_4C2C58);
	s.syncAsUint32LE(_data.dword_4C2C5C);
	s.syncAsUint32LE(_data.dword_4C2C60);
	s.syncAsUint32LE(_data.currentNodeType);

	// FIXME Syncing IEE754 data is not cross platform
	// Increase the savegame version and save those as integers
	s.syncBytes((byte*) &_data.lookatPitch, sizeof(float));
	s.syncBytes((byte*) &_data.lookatHeading, sizeof(float));
	s.syncBytes((byte*) &_data.lookatFOV, sizeof(float));
	s.syncBytes((byte*) &_data.pitchOffset, sizeof(float));
	s.syncBytes((byte*) &_data.headingOffset, sizeof(float));

	s.syncAsUint32LE(_data.limitCubeCamera);
	s.syncBytes((byte*) &_data.minPitch, sizeof(float));
	s.syncBytes((byte*) &_data.maxPitch, sizeof(float));
	s.syncBytes((byte*) &_data.minHeading, sizeof(float));
	s.syncBytes((byte*) &_data.maxHeading, sizeof(float));
	s.syncAsUint32LE(_data.dword_4C2C90);

	for (uint i = 0; i < 2048; i++)
		s.syncAsSint32LE(_data.vars[i]);

	s.syncAsUint32LE(_data.inventoryCount);

	for (uint i = 0; i < 7; i++)
		s.syncAsUint32LE(_data.inventoryList[i]);

	for (uint i = 0; i < 256; i++)
		s.syncAsByte(_data.zipDestinations[i]);
}

void GameState::newGame() {
	memset(&_data, 0, sizeof(_data));

	_data.version = kSaveVersion;
	_data.gameRunning = true;
	_data.vars[0] = 0;
	_data.vars[1] = 1;
}

bool GameState::load(const Common::String &file) {
	Common::InSaveFile *saveFile = _vm->getSaveFileManager()->openForLoading(file);
	Common::Serializer s = Common::Serializer(saveFile, 0);
	syncWithSaveGame(s);
	delete saveFile;

	return true;
}

bool GameState::save(Common::OutSaveFile *saveFile) {
	Common::Serializer s = Common::Serializer(0, saveFile);
	syncWithSaveGame(s);

	return true;
}

Common::Array<uint16> GameState::getInventory() {
	Common::Array<uint16> items;

	for (uint i = 0; i < _data.inventoryCount; i++)
		items.push_back(_data.inventoryList[i]);

	return items;
}

void GameState::updateInventory(const Common::Array<uint16> &items) {
	for (uint i = 0; i < 7; i++)
		_data.inventoryList[i] = 0;

	for (uint i = 0; i < items.size(); i++)
		_data.inventoryList[i] = items[i];
}

void GameState::checkRange(uint16 var) {
	if (var < 1 || var > 2047)
		error("Variable out of range %d", var);
}

int32 GameState::getVar(uint16 var) {
	checkRange(var);
	return _data.vars[var];
}

void GameState::setVar(uint16 var, int32 value) {
	checkRange(var);

	if (_descriptions.contains(var)) {
		const Description &d = _descriptions.getVal(var);
		if (d.unknown)
			warning("A script is writing to the unimplemented engine-mapped var %d (%s)", var, d.name);
	}

	_data.vars[var] = value;
}

bool GameState::evaluate(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int32 varValue = getVar(var);
	int32 targetValue = (unsignedCond >> 11) - 1;

	if (targetValue >= 0) {
		if (condition >= 0)
			return varValue == targetValue;
		else
			return varValue != targetValue;
	} else {
		if (condition >= 0)
			return varValue != 0;
		else
			return varValue == 0;
	}
}

int32 GameState::valueOrVarValue(int16 value) {
	if (value < 0)
		return getVar(-value);

	return value;
}

int32 GameState::engineGet(uint16 var) {
	if (!_descriptions.contains(var))
		error("The engine is trying to access an undescribed var (%d)", var);

	return _data.vars[var];
}

void GameState::engineSet(uint16 var, int32 value) {
	if (!_descriptions.contains(var))
		error("The engine is trying to access an undescribed var (%d)", var);

	_data.vars[var] = value;
}

const Common::String GameState::describeVar(uint16 var) {
	if (_descriptions.contains(var)) {
		const Description &d = _descriptions.getVal(var);

		return Common::String::format("v%s", d.name);
	} else {
		return Common::String::format("v%d", var);
	}
}

const Common::String GameState::describeCondition(int16 condition) {
	uint16 unsignedCond = abs(condition);
	uint16 var = unsignedCond & 2047;
	int16 value = (unsignedCond >> 11) - 1;

	return Common::String::format("c[%s %s %d]",
			describeVar(var).c_str(),
			(condition >= 0 && value >= 0) || (condition < 0 && value < 0) ? "==" : "!=",
			value >= 0 ? value : 0);
}

void GameState::limitCubeCamera(float minPitch, float maxPitch, float minHeading, float maxHeading) {
	_data.limitCubeCamera = true;
	_data.minPitch = minPitch;
	_data.maxPitch = maxPitch;
	_data.minHeading = minHeading;
	_data.maxHeading = maxHeading;
}

void GameState::updateFrameCounters() {
	_data.currentFrame++;

	int32 frameCountdown = getFrameCountdown();
	if (frameCountdown > 0)
		setFrameCountdown(--frameCountdown);
}

} /* namespace Myst3 */
