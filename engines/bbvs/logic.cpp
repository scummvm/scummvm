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

#include "bbvs/bbvs.h"
#include "bbvs/gamemodule.h"

namespace Bbvs {

bool BbvsEngine::evalCondition(Conditions &conditions) {
	bool result = true;
	for (int i = 0; i < 8 && result; ++i) {
		const Condition &condition = conditions.conditions[i];
		switch (condition.cond) {
		case kCondSceneObjectVerb:
			result = _activeItemType == KITSceneObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectVerb:
			result = _activeItemType == kITBgObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondSceneObjectInventory:
			result = _activeItemType == KITSceneObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectInventory:
			result = _activeItemType == kITBgObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondHasInventoryItem:
			result = _inventoryItemStatus[condition.value1] != 0;
			break;
		case kCondHasNotInventoryItem:
			result = _inventoryItemStatus[condition.value1] == 0;
			break;
		case kCondIsGameVar:
			result = _gameVars[condition.value2] != 0;
			break;
		case kCondIsNotGameVar:
			result = _gameVars[condition.value2] == 0;
			break;
		case kCondIsPrevSceneNum:
			result = condition.value2 == _prevSceneNum;
			break;
		case kCondIsCurrTalkObject:
			result = condition.value2 == _currTalkObjectIndex;
			break;
		case kCondIsDialogItem:
			result = _activeItemType == kITDialog &&
				condition.value1 == _activeItemIndex;
			break;
		case kCondIsCameraNum:
			result = condition.value1 == _currCameraNum;
			break;
		case kCondIsNotPrevSceneNum:
			result = condition.value2 != _prevSceneNum;
			break;
		case kCondIsButtheadAtBgObject:
			result = _buttheadObject &&
				_gameModule->getBgObject(condition.value2)->rect.contains(_buttheadObject->x / 65536, _buttheadObject->y / 65536);
			break;
		case kCondIsNotSceneVisited:
			result = _sceneVisited[_currSceneNum] == 0;
			break;
		case kCondIsSceneVisited:
			result = _sceneVisited[_currSceneNum] != 0;
			break;
		case kCondUnused:
		case kCondDialogItem0:
		case kCondIsCameraNumTransition:
			result = false;
			break;
		default:
			break;
		}
	}
	return result;
}

bool BbvsEngine::evalCameraCondition(Conditions &conditions, int value) {
	bool result = true;
	for (int i = 0; i < 8 && result; ++i) {
		const Condition &condition = conditions.conditions[i];
		switch (condition.cond) {
		case kCondHasInventoryItem:
			result =  _inventoryItemStatus[condition.value1] != 0;
			break;
		case kCondHasNotInventoryItem:
			result = _inventoryItemStatus[condition.value1] == 0;
			break;
		case kCondIsGameVar:
			result = _gameVars[condition.value2] != 0;
			break;
		case kCondIsNotGameVar:
			result = _gameVars[condition.value2] == 0;
			break;
		case kCondIsPrevSceneNum:
			result = condition.value2 == _prevSceneNum;
			break;
		case kCondIsNotPrevSceneNum:
			result = condition.value2 != _prevSceneNum;
			break;
		case kCondIsNotSceneVisited:
			result = _sceneVisited[_currSceneNum] == 0;
			break;
		case kCondIsSceneVisited:
			result = _sceneVisited[_currSceneNum] != 0;
			break;
		case kCondIsCameraNumTransition:
			result = condition.value1 == _currCameraNum &&
				condition.value2 == value;
			break;
		case kCondUnused:
		case kCondSceneObjectVerb:
		case kCondBgObjectVerb:
		case kCondSceneObjectInventory:
		case kCondBgObjectInventory:
		case kCondIsCurrTalkObject:
		case kCondIsDialogItem:
		case kCondIsCameraNum:
		case kCondDialogItem0:
		case kCondIsButtheadAtBgObject:
			result = false;
			break;
		default:
			break;
		}
	}
	return result;
}

int BbvsEngine::evalDialogCondition(Conditions &conditions) {
	int result = -1;
	bool success = false;
	for (int i = 0; i < 8; ++i) {
		const Condition &condition = conditions.conditions[i];
		switch (condition.cond) {
		case kCondSceneObjectVerb:
			success = _activeItemType == KITSceneObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectVerb:
			success = _activeItemType == kITBgObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondSceneObjectInventory:
			success = _activeItemType == KITSceneObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectInventory:
			success = _activeItemType == kITBgObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondHasInventoryItem:
			success = _inventoryItemStatus[condition.value1] != 0;
			break;
		case kCondHasNotInventoryItem:
			success = _inventoryItemStatus[condition.value1] == 0;
			break;
		case kCondIsGameVar:
			success = _gameVars[condition.value2] != 0;
			break;
		case kCondIsNotGameVar:
			success = _gameVars[condition.value2] == 0;
			break;
		case kCondIsPrevSceneNum:
			success = condition.value2 == _prevSceneNum;
			break;
		case kCondIsCurrTalkObject:
			success = condition.value2 == _currTalkObjectIndex;
			break;
		case kCondIsDialogItem:
			result = condition.value1;
			break;
		case kCondIsCameraNum:
			success = condition.value1 == _currCameraNum;
			break;
		case kCondIsNotPrevSceneNum:
			success = condition.value2 != _prevSceneNum;
			break;
		case kCondIsButtheadAtBgObject:
			success = _buttheadObject &&
				_gameModule->getBgObject(condition.value2)->rect.contains(_buttheadObject->x / 65536, _buttheadObject->y / 65536);
			break;
		case kCondIsNotSceneVisited:
			success = _sceneVisited[_currSceneNum] == 0;
			break;
		case kCondIsSceneVisited:
			success = _sceneVisited[_currSceneNum] != 0;
			break;
		case kCondDialogItem0:
			return 0;
		case kCondUnused:
		case kCondIsCameraNumTransition:
			success = false;
			break;
		default:
			break;
		}
		if (!success)
			return -1;
	}
	return result;
}

void BbvsEngine::evalActionResults(ActionResults &results) {
	for (int i = 0; i < 8; ++i) {
		const ActionResult &result = results.actionResults[i];
		switch (result.kind) {
		case kActResAddInventoryItem:
			_inventoryItemStatus[result.value1] = 1;
			_currVerbNum = kVerbInvItem;
			_currInventoryItem = result.value1;
			break;
		case kActResRemoveInventoryItem:
			_inventoryItemStatus[result.value1] = 0;
			if (result.value1 == _currInventoryItem)
				_currInventoryItem = -1;
			if (_currVerbNum == kVerbInvItem)
				_currVerbNum = kVerbLook;
			break;
		case kActResSetGameVar:
			_gameVars[result.value2] = 1;
			break;
		case kActResUnsetGameVar:
			_gameVars[result.value2] = 0;
			break;
		case kActResStartDialog:
			_gameState = kGSDialog;
			break;
		case kActResChangeScene:
			_newSceneNum = result.value2;
			break;
		default:
			break;
		}
	}
}

} // End of namespace Bbvs
