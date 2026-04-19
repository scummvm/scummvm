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

#include "bolt/carnival/carnival.h"

namespace Bolt {

namespace Carnival {
	
void CarnivalEngine::resolveAllRefsCb() { ((CarnivalEngine *)g_engine)->resolveAllRefs(); }
void CarnivalEngine::swapFredAnimEntryCb() { ((CarnivalEngine *)g_engine)->swapFredAnimEntry(); }
void CarnivalEngine::swapFredAnimDescCb() { ((CarnivalEngine *)g_engine)->swapFredAnimDesc(); }
void CarnivalEngine::swapFredLevelDescCb() { ((CarnivalEngine *)g_engine)->swapFredLevelDesc(); }

void CarnivalEngine::swapGeorgeFrameArrayCb() { ((CarnivalEngine *)g_engine)->swapGeorgeFrameArray(); }
void CarnivalEngine::swapGeorgeHelpEntryCb() { ((CarnivalEngine *)g_engine)->swapGeorgeHelpEntry(); }
void CarnivalEngine::swapGeorgeThresholdsCb() { ((CarnivalEngine *)g_engine)->swapGeorgeThresholds(); }

void CarnivalEngine::swapHuckWordArrayCb() { ((CarnivalEngine *)g_engine)->swapHuckWordArray(); }
void CarnivalEngine::swapHuckWordsCb() { ((CarnivalEngine *)g_engine)->swapHuckWords(); }

void CarnivalEngine::swapScoobyHelpEntryCb() { ((CarnivalEngine *)g_engine)->swapScoobyHelpEntry(); }
void CarnivalEngine::swapScoobyWordArrayCb() { ((CarnivalEngine *)g_engine)->swapScoobyWordArray(); }

void CarnivalEngine::swapTopCatHelpEntryCb() { ((CarnivalEngine *)g_engine)->swapTopCatHelpEntry(); }

void CarnivalEngine::swapYogiAllWordsCb() { ((CarnivalEngine *)g_engine)->swapYogiAllWords(); }
void CarnivalEngine::swapYogiFirstWordCb() { ((CarnivalEngine *)g_engine)->swapYogiFirstWord(); }

BOLTCallback CarnivalEngine::_fredTypeLoadCallbacks[28];
BOLTCallback CarnivalEngine::_fredTypeFreeCallbacks[28];

BOLTCallback CarnivalEngine::_georgeTypeLoadCallbacks[28];
BOLTCallback CarnivalEngine::_georgeTypeFreeCallbacks[28];

BOLTCallback CarnivalEngine::_huckTypeLoadCallbacks[27];
BOLTCallback CarnivalEngine::_huckTypeFreeCallbacks[27];

BOLTCallback CarnivalEngine::_scoobyTypeLoadCallbacks[28];
BOLTCallback CarnivalEngine::_scoobyTypeFreeCallbacks[28];

BOLTCallback CarnivalEngine::_topCatTypeLoadCallbacks[26];
BOLTCallback CarnivalEngine::_topCatTypeFreeCallbacks[26];

BOLTCallback CarnivalEngine::_yogiTypeLoadCallbacks[27];
BOLTCallback CarnivalEngine::_yogiTypeFreeCallbacks[27];

void CarnivalEngine::initCallbacks() {
	// --- BOOTHS ---
	for (int i = 0; i < ARRAYSIZE(_defaultTypeLoadCallbacks); i++) {
		_defaultTypeLoadCallbacks[i] = noOpCb;
	}

	_defaultTypeLoadCallbacks[2] = swapAllWordsCb;
	_defaultTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_defaultTypeLoadCallbacks[10] = swapPicHeaderCb;
	_defaultTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_defaultTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_defaultTypeLoadCallbacks[14] = swapFirstFourWordsCb;

	for (int i = 0; i < ARRAYSIZE(_defaultTypeFreeCallbacks); i++) {
		_defaultTypeFreeCallbacks[i] = noOpCb;
	}

	_defaultTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	for (int i = 0; i < ARRAYSIZE(_defaultMemberLoadCallbacks); i++) {
		_defaultMemberLoadCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(_defaultMemberFreeCallbacks); i++) {
		_defaultMemberFreeCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(_defaultGroupLoadCallbacks); i++) {
		_defaultGroupLoadCallbacks[i] = noOpCb;
	}

	for (int i = 0; i < ARRAYSIZE(_defaultGroupFreeCallbacks); i++) {
		_defaultGroupFreeCallbacks[i] = noOpCb;
	}

	_boothsBoltCallbacks.typeLoadCallbacks = _defaultTypeLoadCallbacks;
	_boothsBoltCallbacks.typeFreeCallbacks = _defaultTypeFreeCallbacks;
	_boothsBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_boothsBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_boothsBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_boothsBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- FRED ---
	for (int i = 0; i < ARRAYSIZE(_fredTypeLoadCallbacks); i++) {
		_fredTypeLoadCallbacks[i] = noOpCb;
	}

	_fredTypeLoadCallbacks[2] = swapAllWordsCb;
	_fredTypeLoadCallbacks[6] = resolveAllRefsCb;
	_fredTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_fredTypeLoadCallbacks[10] = swapPicHeaderCb;
	_fredTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_fredTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_fredTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_fredTypeLoadCallbacks[25] = swapFredAnimEntryCb;
	_fredTypeLoadCallbacks[26] = swapFredAnimDescCb;
	_fredTypeLoadCallbacks[27] = swapFredLevelDescCb;

	for (int i = 0; i < ARRAYSIZE(_fredTypeFreeCallbacks); i++) {
		_fredTypeFreeCallbacks[i] = noOpCb;
	}

	_fredTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_fredBoltCallbacks.typeLoadCallbacks = _fredTypeLoadCallbacks;
	_fredBoltCallbacks.typeFreeCallbacks = _fredTypeFreeCallbacks;
	_fredBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_fredBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_fredBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_fredBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- GEORGE ---
	for (int i = 0; i < ARRAYSIZE(_georgeTypeLoadCallbacks); i++) {
		_georgeTypeLoadCallbacks[i] = noOpCb;
	}

	_georgeTypeLoadCallbacks[2] = swapAllWordsCb;
	_georgeTypeLoadCallbacks[6] = resolveAllRefsCb;
	_georgeTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_georgeTypeLoadCallbacks[10] = swapPicHeaderCb;
	_georgeTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_georgeTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_georgeTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_georgeTypeLoadCallbacks[25] = swapGeorgeFrameArrayCb;
	_georgeTypeLoadCallbacks[26] = swapGeorgeHelpEntryCb;
	_georgeTypeLoadCallbacks[27] = swapGeorgeThresholdsCb;

	for (int i = 0; i < ARRAYSIZE(_georgeTypeFreeCallbacks); i++) {
		_georgeTypeFreeCallbacks[i] = noOpCb;
	}

	_georgeTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_georgeBoltCallbacks.typeLoadCallbacks = _georgeTypeLoadCallbacks;
	_georgeBoltCallbacks.typeFreeCallbacks = _georgeTypeFreeCallbacks;
	_georgeBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_georgeBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_georgeBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_georgeBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- HUCK ---
	for (int i = 0; i < ARRAYSIZE(_huckTypeLoadCallbacks); i++) {
		_huckTypeLoadCallbacks[i] = noOpCb;
	}

	_huckTypeLoadCallbacks[2] = swapAllWordsCb;
	_huckTypeLoadCallbacks[6] = resolveAllRefsCb;
	_huckTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_huckTypeLoadCallbacks[10] = swapPicHeaderCb;
	_huckTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_huckTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_huckTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_huckTypeLoadCallbacks[25] = swapHuckWordArrayCb;
	_huckTypeLoadCallbacks[26] = swapHuckWordsCb;

	for (int i = 0; i < ARRAYSIZE(_huckTypeFreeCallbacks); i++) {
		_huckTypeFreeCallbacks[i] = noOpCb;
	}

	_huckTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_huckBoltCallbacks.typeLoadCallbacks = _huckTypeLoadCallbacks;
	_huckBoltCallbacks.typeFreeCallbacks = _huckTypeFreeCallbacks;
	_huckBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_huckBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_huckBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_huckBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- SCOOBY ---
	for (int i = 0; i < ARRAYSIZE(_scoobyTypeLoadCallbacks); i++) {
		_scoobyTypeLoadCallbacks[i] = noOpCb;
	}

	_scoobyTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_scoobyTypeLoadCallbacks[10] = swapPicHeaderCb;
	_scoobyTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_scoobyTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_scoobyTypeLoadCallbacks[26] = swapScoobyHelpEntryCb;
	_scoobyTypeLoadCallbacks[27] = swapScoobyWordArrayCb;

	for (int i = 0; i < ARRAYSIZE(_scoobyTypeFreeCallbacks); i++) {
		_scoobyTypeFreeCallbacks[i] = noOpCb;
	}

	_scoobyTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_scoobyBoltCallbacks.typeLoadCallbacks = _scoobyTypeLoadCallbacks;
	_scoobyBoltCallbacks.typeFreeCallbacks = _scoobyTypeFreeCallbacks;
	_scoobyBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_scoobyBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_scoobyBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_scoobyBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- TOPCAT ---
	for (int i = 0; i < ARRAYSIZE(_topCatTypeLoadCallbacks); i++) {
		_topCatTypeLoadCallbacks[i] = noOpCb;
	}

	_topCatTypeLoadCallbacks[2] = swapAllWordsCb;
	_topCatTypeLoadCallbacks[4] = swapAllLongsCb;
	_topCatTypeLoadCallbacks[6] = resolveAllRefsCb;
	_topCatTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_topCatTypeLoadCallbacks[10] = swapPicHeaderCb;
	_topCatTypeLoadCallbacks[11] = swapAndResolvePicDescCb;
	_topCatTypeLoadCallbacks[12] = swapFirstTwoWordsCb;
	_topCatTypeLoadCallbacks[25] = swapTopCatHelpEntryCb;

	for (int i = 0; i < ARRAYSIZE(_topCatTypeFreeCallbacks); i++) {
		_topCatTypeFreeCallbacks[i] = noOpCb;
	}

	_topCatTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_topCatBoltCallbacks.typeLoadCallbacks = _topCatTypeLoadCallbacks;
	_topCatBoltCallbacks.typeFreeCallbacks = _topCatTypeFreeCallbacks;
	_topCatBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_topCatBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_topCatBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_topCatBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;

	// --- YOGI ---
	for (int i = 0; i < ARRAYSIZE(_yogiTypeLoadCallbacks); i++) {
		_yogiTypeLoadCallbacks[i] = noOpCb;
	}

	_yogiTypeLoadCallbacks[2] = swapAllWordsCb;
	_yogiTypeLoadCallbacks[8] = swapSpriteHeaderCb;
	_yogiTypeLoadCallbacks[10] = swapPicHeaderCb;
	_yogiTypeLoadCallbacks[14] = swapFirstFourWordsCb;
	_yogiTypeLoadCallbacks[25] = swapYogiFirstWordCb;
	_yogiTypeLoadCallbacks[26] = swapYogiAllWordsCb;

	for (int i = 0; i < ARRAYSIZE(_yogiTypeFreeCallbacks); i++) {
		_yogiTypeFreeCallbacks[i] = noOpCb;
	}

	_yogiTypeFreeCallbacks[8] = freeSpriteCleanUpCb;

	_yogiBoltCallbacks.typeLoadCallbacks = _yogiTypeLoadCallbacks;
	_yogiBoltCallbacks.typeFreeCallbacks = _yogiTypeFreeCallbacks;
	_yogiBoltCallbacks.memberLoadCallbacks = _defaultMemberLoadCallbacks;
	_yogiBoltCallbacks.memberFreeCallbacks = _defaultMemberFreeCallbacks;
	_yogiBoltCallbacks.groupLoadCallbacks = _defaultGroupLoadCallbacks;
	_yogiBoltCallbacks.groupFreeCallbacks = _defaultGroupFreeCallbacks;
}

} // End of namespace Carnival

} // End of namespace Bolt
