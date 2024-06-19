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
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/path.h"
#include "prisoner/resourcemgr.h"
#include "prisoner/scriptops.h"

namespace Prisoner {

typedef Common::Functor1Mem<Script*, int16, ScriptOpcodes> OpcodeFunc;
#define Opcode(func) \
	_opcodes.push_back(new Opcode(new OpcodeFunc(this, &ScriptOpcodes::func), #func));
void ScriptOpcodes::setupOpcodes() {

	Opcode(op_returnNegOne);
	Opcode(op_setBackground);
	Opcode(op_returnNegOne);
	Opcode(op_startScript);
	Opcode(op_jumpRel);
	Opcode(op_getGlobalVar);
	Opcode(op_do);
	Opcode(op_false);
	Opcode(op_lt);
	Opcode(op_gt);
	Opcode(op_lte);
	Opcode(op_gte);
	Opcode(op_boolAnd);
	Opcode(op_boolOr);
	Opcode(op_add);
	Opcode(op_sub);
	Opcode(op_mul);
	Opcode(op_execTwo);
	Opcode(op_nop);
	Opcode(op_getModuleVar);
	Opcode(op_loadConstWord);
	Opcode(op_if);
	Opcode(op_sleep);
	Opcode(op_incVar);
	Opcode(op_decVar);
	Opcode(op_setGlobalVar);
	Opcode(op_setModuleVar);
	Opcode(op_setPalette);
	Opcode(op_clearBackground);
	Opcode(op_loadSceneSound);
	Opcode(op_setSoundVolume);
	Opcode(op_stopScript);
	Opcode(op_loadMusic);
	Opcode(op_setMusicVolume);
	Opcode(op_gotoScene);
	Opcode(op_sub_22310);
	Opcode(op_waitForInput);
	Opcode(op_sub_2234D);
	Opcode(op_random);
	Opcode(op_paletteFunc);
	Opcode(op_copyPalette);
	Opcode(op_setScriptContinueFlag);
	Opcode(op_clearScriptContinueFlag);
	Opcode(op_setSomeArrayValue);
	Opcode(op_syncScript);
	Opcode(op_true);
	Opcode(op_getSysVar);
	Opcode(op_addPathNode);
	Opcode(op_addPathEdge);
	Opcode(op_addPathPolygon);
	Opcode(op_actorWalkToPathNode);
	Opcode(op_setActorDirection);
	Opcode(op_addActorScreenText);
	Opcode(op_returnNegOne);
	Opcode(op_setVar);
	Opcode(op_addActorAtPathNode);
	Opcode(op_setMainActor);
	Opcode(op_addActorZone);
	Opcode(op_addZoneAction);
	Opcode(op_addTextZone);
	Opcode(op_addSceneItem);
	Opcode(op_addItemZone);
	Opcode(op_startDialog);
	Opcode(op_registerInventoryItem);
	Opcode(op_addInventoryItemCombination);
	Opcode(op_getSomeArrayValue);
	Opcode(op_loadDialog);
	Opcode(op_addItemToInventory);
	Opcode(op_enableDialogKeyword);
	Opcode(op_disableDialogKeyword);
	Opcode(op_sub_23259);
	Opcode(op_sub_2328A);
	Opcode(op_unloadDialog);
	Opcode(op_getActorX);
	Opcode(op_getActorY);
	Opcode(op_setActorX);
	Opcode(op_setActorY);
	Opcode(op_addActorAtPos);
	Opcode(op_setBackgroundObjects);
	Opcode(op_actorEntersScene);
	Opcode(op_addExitZoneAction);
	Opcode(op_addZoneAction90);
	Opcode(op_removeItemFromInventory);
	Opcode(op_removeActor);
	Opcode(op_clearZoneAction);
	Opcode(op_path207E6);
	Opcode(op_getValue);
	Opcode(op_actorPutAtPos);
	Opcode(op_actorPutAtPathNode);
	Opcode(op_addZone);
	Opcode(op_removeZone);
	Opcode(op_not);
	Opcode(op_actorAssignPathWalker);
	Opcode(op_setUserInput);
	Opcode(op_clearZoneAction);
	Opcode(op_startPaletteTask);
	Opcode(op_playActorAnimation);
	Opcode(op_setActorSpriteFrameListIndexIfIdle);
	Opcode(op_actorAnimation20CC7);
	Opcode(op_resetMessageValues);
	Opcode(op_playSound);
	Opcode(op_unloadSound);
	Opcode(op_togglePathSystem);
	Opcode(op_startMusicE);
	Opcode(op_unloadMusic);
	Opcode(op_stopSound);
	Opcode(op_stopMusic);
	Opcode(op_isSoundPlaying);
	Opcode(op_isMusicPlaying);
	Opcode(op_addActorFrameSound);
	Opcode(op_setZoneActionScript);
	Opcode(op_interactActorMessage);
	Opcode(op_setScriptZoneEnterLeaveFlag);
	Opcode(op_setMouseAddXY);
	Opcode(op_playLoopingSound);
	Opcode(op_removeActorFrameSound);
	Opcode(op_setActorFrameSound);
	Opcode(op_playActorAnimationAtPos);
	Opcode(op_startMusic);
	Opcode(op_setBackgroundCameraLocked);
	Opcode(op_removeInventoryItemCombination);
	Opcode(op_getVar);
	Opcode(op_addLooseScreenText);
	Opcode(op_actorWalkToPoint);
	Opcode(op_resetCurrInventoryItemCursor);
	Opcode(op_interactScreenMessage);
	Opcode(op_nop);
	Opcode(op_nop);
	Opcode(op_addScreenText2);
	Opcode(op_actorText21704);
	Opcode(op_actorAnimation218A1);
	Opcode(op_startModuleScript);
	Opcode(op_death);
	Opcode(op_loadModuleSound);
	Opcode(op_playSoundSync);
	Opcode(op_setActorFontColors);
	Opcode(op_setTextDisplayColor);
	Opcode(op_loadInventoryItemsAnimation);
	Opcode(op_playMux);
	Opcode(op_setCameraFollowsActor);
	Opcode(op_actor21C78);
	Opcode(op_actor21C89);
	Opcode(op_waitForActorAnimation);
	Opcode(op_addActorAltAnimation);
	Opcode(op_unloadActorAltAnimation);
	Opcode(op_sub_21D2D);
	Opcode(op_setActorAltAnimationAtPos);
	Opcode(op_actor21E0D);
	Opcode(op_addZoneAction91);
	Opcode(op_setUpdateDirtyRectsFlag);
	Opcode(op_sub_21FD9);
	Opcode(op_sub_220B8);
	Opcode(op_autoSave);
	Opcode(op_setMainActorValid);

}
#undef Opcode

int16 ScriptOpcodes::execOpcode(Script *script, byte opcode) {
	debug(5, "ScriptOpcodes::execOpcode() %s", _opcodes[opcode]->_name.c_str());
	return (*_opcodes[opcode]->_call)(script);
}

int16 ScriptOpcodes::evaluate(Script *script) {
	int16 opcode = script->readInt16();
	return execOpcode(script, opcode);
}

Common::String ScriptOpcodes::readPakName(Script *script, bool languageSpecific) {
	Common::String value = script->readString();
	// Some PakNames have backslashes or dots in them
	for (uint i = 0; i < value.size(); i++)
		if (value[i] == '\\' || value[i] == '.') {
			value = Common::String(value.c_str(), i);
		}
	value.toUppercase();
	if (languageSpecific)
		_vm->makeLanguageString(value);
	return value;
}

int16 ScriptOpcodes::op_returnNegOne(Script *script) {
	return -1;
}

int16 ScriptOpcodes::op_setBackground(Script *script) {
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(backgroundFlag);
	_vm->setBackground(pakName, pakSlot, backgroundFlag);
	return pakSlot;
}

int16 ScriptOpcodes::op_startScript(Script *script) {
	ARG_EVALUATE(scriptIndex);
	_vm->startLocalScript(scriptIndex);
	return scriptIndex;
}

int16 ScriptOpcodes::op_jumpRel(Script *script) {
	ARG_INT16(offset);
	script->ip += offset * 2;
	return 0;
}

int16 ScriptOpcodes::op_getGlobalVar(Script *script) {
	ARG_INT16(varIndex);
	return _vm->_globalScriptVars[varIndex];
}

int16 ScriptOpcodes::op_do(Script *script) {
	ARG_INT16(opcode);
	execOpcode(script, opcode);
	return 0;
}

int16 ScriptOpcodes::op_false(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 != value2 ? 1 : 0;
}

int16 ScriptOpcodes::op_lt(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 < value2 ? 1 : 0;
}

int16 ScriptOpcodes::op_gt(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 > value2 ? 1 : 0;
}

int16 ScriptOpcodes::op_lte(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 <= value2 ? 1 : 0;
}

int16 ScriptOpcodes::op_gte(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 >= value2 ? 1 : 0;
}

int16 ScriptOpcodes::op_boolAnd(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return (value1 > 0 && value2 > 0) ? 1 : 0;
}

int16 ScriptOpcodes::op_boolOr(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return (value1 > 0 || value2 > 0) ? 1 : 0;
}

int16 ScriptOpcodes::op_add(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 + value2;
}

int16 ScriptOpcodes::op_sub(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 - value2;
}

int16 ScriptOpcodes::op_mul(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 * value2;
}

int16 ScriptOpcodes::op_execTwo(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return 0;
}

int16 ScriptOpcodes::op_getModuleVar(Script *script) {
	ARG_INT16(varIndex);
	return _vm->_moduleScriptVars[varIndex];
}

int16 ScriptOpcodes::op_loadConstWord(Script *script) {
	ARG_INT16(value);
	return value;
}

int16 ScriptOpcodes::op_if(Script *script) {
	ARG_EVALUATE(condition);
	if (condition != 0) {
		// skip jump instruction and offset
		script->ip += 4;
	} else {
		script->ip += 2;
		int16 offset = script->readInt16();
		script->ip += offset * 2;
	}
	return 0;
}

int16 ScriptOpcodes::op_sleep(Script *script) {
	ARG_EVALUATE(sleepCounter);
	script->sleepCounter = sleepCounter;
	script->status = kScriptStatusSleeping;
	return sleepCounter;
}

int16 ScriptOpcodes::op_incVar(Script *script) {
	ARG_INT16(varType);
	ARG_INT16(varIndex);
	int16 result = 0;
	if (varType == 5) {
		result = _vm->getGlobalScriptVar(varIndex) + 1;
		_vm->setGlobalScriptVar(varIndex, result);
	} else if (varType == 19) {
		result = _vm->getModuleScriptVar(varIndex) + 1;
		_vm->setModuleScriptVar(varIndex, result);
	}
	return result;
}

int16 ScriptOpcodes::op_decVar(Script *script) {
	ARG_INT16(varType);
	ARG_INT16(varIndex);
	int16 result = 0;
	if (varType == 5) {
		result = _vm->getGlobalScriptVar(varIndex) - 1;
		_vm->setGlobalScriptVar(varIndex, result);
	} else if (varType == 19) {
		result = _vm->getModuleScriptVar(varIndex) - 1;
		_vm->setModuleScriptVar(varIndex, result);
	}
	return result;
}

int16 ScriptOpcodes::op_setGlobalVar(Script *script) {
	ARG_INT16(varIndex);
	ARG_EVALUATE(value);
	_vm->setGlobalScriptVar(varIndex, value);
	return 0;
}

int16 ScriptOpcodes::op_setModuleVar(Script *script) {
	ARG_INT16(varIndex);
	ARG_EVALUATE(value);
	_vm->setModuleScriptVar(varIndex, value);
	return 0;
}

int16 ScriptOpcodes::op_setPalette(Script *script) {
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	_vm->setPalette(pakName, pakSlot);
	return pakSlot;
}

int16 ScriptOpcodes::op_clearBackground(Script *script) {
	ARG_EVALUATE(backgroundFlag);
	_vm->clearBackground(backgroundFlag);
	return 0;
}

int16 ScriptOpcodes::op_loadSceneSound(Script *script) {
	ARG_PAKNAME(pakName, false);
	ARG_EVALUATE(pakSlot);
	return _vm->loadSound(pakName, pakSlot, false);
}

int16 ScriptOpcodes::op_setSoundVolume(Script *script) {
	ARG_EVALUATE(soundIndex);
	ARG_EVALUATE(volume);
	_vm->setSoundVolume(soundIndex, volume);
	return volume;
}

int16 ScriptOpcodes::op_stopScript(Script *script) {
	ARG_EVALUATE(scriptIndex);
	_vm->stopLocalScript(scriptIndex);
	return scriptIndex;
}

int16 ScriptOpcodes::op_loadMusic(Script *script) {
	ARG_PAKNAME(pakName, false);
	ARG_EVALUATE(pakSlot);
	return _vm->loadMusic(pakName, pakSlot, false);
}

int16 ScriptOpcodes::op_setMusicVolume(Script *script) {
	ARG_EVALUATE(musicIndex);
	ARG_EVALUATE(volume);
	_vm->setMusicVolume(musicIndex, volume);
	return volume;
}

int16 ScriptOpcodes::op_gotoScene(Script *script) {
	ARG_EVALUATE(moduleIndex);
	ARG_EVALUATE(sceneIndex);
	_vm->gotoScene(moduleIndex, sceneIndex);
	_vm->_scriptContinueFlag = false;
	return 0;
}

int16 ScriptOpcodes::op_sub_22310(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_waitForInput(Script *script) {
	if (!_vm->waitForInput())
		script->ip -= 2;
	return 0;
}

int16 ScriptOpcodes::op_sub_2234D(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_random(Script *script) {
	ARG_EVALUATE(maxValue);
	return _vm->_rnd->getRandomNumber(maxValue);
}

int16 ScriptOpcodes::op_paletteFunc(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_copyPalette(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_setScriptContinueFlag(Script *script) {
	_vm->_scriptContinueFlag = true;
	return 0;
}

int16 ScriptOpcodes::op_clearScriptContinueFlag(Script *script) {
	_vm->_scriptContinueFlag = false;
	return 0;
}

int16 ScriptOpcodes::op_setSomeArrayValue(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_syncScript(Script *script) {
	ARG_EVALUATE(syncScriptNumber);
	script->syncScriptNumber = syncScriptNumber;
	script->status = kScriptStatusSync;
	return syncScriptNumber;
}

int16 ScriptOpcodes::op_true(Script *script) {
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	return value1 == value2 ? 1 : 0;
}

int16 ScriptOpcodes::op_getSysVar(Script *script) {
	ARG_INT16(varIndex);
	return _vm->getSysVar(varIndex);
}

int16 ScriptOpcodes::op_addPathNode(Script *script) {
	ARG_INT16(nodeIndex);
	ARG_INT16(x);
	ARG_INT16(y);
	ARG_INT16(scale);
	_vm->_pathSystem->addPathNode(nodeIndex, x, y, scale);
	return nodeIndex;
}

int16 ScriptOpcodes::op_addPathEdge(Script *script) {
	ARG_INT16(edgeIndex);
	ARG_INT16(nodeIndex1);
	ARG_INT16(nodeIndex2);
	ARG_INT16(enabled);
	_vm->_pathSystem->addPathEdge(edgeIndex, nodeIndex1, nodeIndex2, enabled);
	return edgeIndex;
}

int16 ScriptOpcodes::op_addPathPolygon(Script *script) {
	ARG_INT16(polyIndex);
	ARG_INT16(nodeIndex1);
	ARG_INT16(nodeIndex2);
	ARG_INT16(nodeIndex3);
	ARG_INT16(enabled);
	_vm->_pathSystem->addPathPolygon(polyIndex, nodeIndex1, nodeIndex2, nodeIndex3, enabled);
	return polyIndex;
}

int16 ScriptOpcodes::op_actorWalkToPathNode(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(nodeIndex);
	script->status = kScriptStatusWalking;
	script->actorIndex = actorIndex;
	script->actorIndex2 = actorIndex;
	_vm->actorWalkToPathNode(actorIndex, nodeIndex);
	return actorIndex;
}

int16 ScriptOpcodes::op_setActorDirection(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(direction);
	_vm->setActorDirection(actorIndex, direction);
	script->status = kScriptStatusAnimation;
	script->actorIndex2 = actorIndex;
	script->actorIndex = actorIndex;
	return actorIndex;
}

int16 ScriptOpcodes::op_addActorScreenText(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	int16 result = 0;
	if (!_vm->isScreenTextShowing()) {
		int16 resourceCacheSlot = _vm->loadTextResource(pakName, pakSlot);
		script->status = kScriptStatusText;
		script->screenTextIndex = _vm->addActorScreenText(actorIndex, resourceCacheSlot, identifier);
		script->actorIndex = actorIndex;
		result = 1;
	}
	return result;
}

int16 ScriptOpcodes::op_setVar(Script *script) {
	ARG_EVALUATE(varIndex);
	ARG_EVALUATE(value);
	if (varIndex & 0x8000) {
		_vm->_moduleScriptVars[varIndex & 0x7FFF] = value;
	} else {
		_vm->_globalScriptVars[varIndex] = value;
	}
	return 0;
}

int16 ScriptOpcodes::op_addActorAtPathNode(Script *script) {
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(nodeIndex)
	script->actorIndex = _vm->addActor(pakName, pakSlot, frameListIndex, nodeIndex, 0, 0);
	if (_vm->_currScriptIndex > 0)
		_vm->buildActorSpriteDrawQueue();
	return script->actorIndex;
}

int16 ScriptOpcodes::op_setMainActor(Script *script) {
	ARG_EVALUATE(actorIndex);
	script->actorIndex = actorIndex;
	_vm->setMainActor(actorIndex);
	return actorIndex;
}

int16 ScriptOpcodes::op_addActorZone(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(mouseCursor);
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	int16 zoneIndex = _vm->addActorZone(actorIndex, mouseCursor, pakName, pakSlot, identifier);
	script->actorIndex = actorIndex;
	return zoneIndex;
}

int16 ScriptOpcodes::op_addZoneAction(Script *script) {
	ARG_EVALUATE(zoneIndex);
	ARG_EVALUATE(type);
	ARG_EVALUATE(nodeIndex);
	ARG_EVALUATE(scriptIndex);
	int16 zoneActionIndex = -1;
	if (zoneIndex >= 0)
		zoneActionIndex = _vm->addZoneAction(zoneIndex, type, nodeIndex, scriptIndex, kSceneScriptProgram, -1, -1, -1);
	return zoneActionIndex;
}

int16 ScriptOpcodes::op_addTextZone(Script *script) {
	ARG_EVALUATE(x1);
	ARG_EVALUATE(y1);
	ARG_EVALUATE(x2);
	ARG_EVALUATE(y2);
	ARG_EVALUATE(mouseCursor);
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	int16 zoneIndex = _vm->addZone(x1, y1, x2, y2, mouseCursor, &pakName, pakSlot, &identifier);
	return zoneIndex;
}

int16 ScriptOpcodes::op_addSceneItem(Script *script) {
	ARG_EVALUATE(itemIndex);
	ARG_EVALUATE(nodeIndex);
	int16 sceneItemIndex = -1;
	if (_vm->_inventoryItems[itemIndex].status == 0) {
		sceneItemIndex = _vm->addSceneItem(itemIndex, nodeIndex);
		if (_vm->_currScriptIndex > 0)
			_vm->buildActorSpriteDrawQueue();
	}
	return sceneItemIndex;
}

int16 ScriptOpcodes::op_addItemZone(Script *script) {
	ARG_EVALUATE(sceneItemIndex);
	ARG_EVALUATE(mouseCursor);
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	int16 zoneIndex = -1;
	if (sceneItemIndex >= 0) {
		zoneIndex = _vm->addItemZone(sceneItemIndex, mouseCursor, pakName, pakSlot, identifier);
	}
	return zoneIndex;
}

int16 ScriptOpcodes::op_startDialog(Script *script) {
	ARG_EVALUATE(dialogIndex);
	if (!_vm->_dialogFlag) {
		_vm->startDialog(dialogIndex);
		script->status = kScriptStatusDialog;
	} else {
		_vm->_dialogFlag = false;
	}
	return 0;
}

int16 ScriptOpcodes::op_registerInventoryItem(Script *script) {
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(id);
	int16 itemIndex = _vm->registerInventoryItem(pakName, pakSlot, id);
	return itemIndex;
}

int16 ScriptOpcodes::op_addInventoryItemCombination(Script *script) {
	ARG_EVALUATE(inventoryItem1);
	ARG_EVALUATE(inventoryItem2);
	ARG_EVALUATE(scriptIndex);
	int16 combinationIndex = _vm->addInventoryItemCombination(inventoryItem1, inventoryItem2, scriptIndex);
	return combinationIndex;
}

int16 ScriptOpcodes::op_getSomeArrayValue(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_loadDialog(Script *script) {
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	int16 dialogIndex = _vm->loadDialog(pakName, pakSlot);
	return dialogIndex;
}

int16 ScriptOpcodes::op_addItemToInventory(Script *script) {
	ARG_EVALUATE(itemIndex);
	_vm->addItemToInventory(itemIndex);
	return itemIndex;
}

int16 ScriptOpcodes::op_enableDialogKeyword(Script *script) {
	ARG_EVALUATE(dialogIndex);
	ARG_EVALUATE(keywordIndex);
	_vm->enableDialogKeyword(dialogIndex, keywordIndex);
	return keywordIndex;
}

int16 ScriptOpcodes::op_disableDialogKeyword(Script *script) {
	ARG_EVALUATE(dialogIndex);
	ARG_EVALUATE(keywordIndex);
	_vm->disableDialogKeyword(dialogIndex, keywordIndex);
	return keywordIndex;
}

int16 ScriptOpcodes::op_sub_23259(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_sub_2328A(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_unloadDialog(Script *script) {
	ARG_EVALUATE(dialogIndex);
	_vm->unloadDialog(dialogIndex);
	return dialogIndex;
}

int16 ScriptOpcodes::op_getActorX(Script *script) {
	ARG_EVALUATE(actorIndex);
	script->actorIndex = actorIndex;
	return _vm->getActorX(actorIndex);
}

int16 ScriptOpcodes::op_getActorY(Script *script) {
	ARG_EVALUATE(actorIndex);
	script->actorIndex = actorIndex;
	return _vm->getActorY(actorIndex);
}

int16 ScriptOpcodes::op_setActorX(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_setActorY(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_addActorAtPos(Script *script) {
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(x)
	ARG_EVALUATE(y)
	script->actorIndex = _vm->addActor(pakName, pakSlot, frameListIndex, -1, x, y);
	if (_vm->_currScriptIndex > 0)
		_vm->buildActorSpriteDrawQueue();
	return script->actorIndex;
}

int16 ScriptOpcodes::op_setBackgroundObjects(Script *script) {
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	_vm->unloadBackgroundObjects();
	_vm->setBackgroundObjects(pakName, pakSlot);
	return 0;
}

int16 ScriptOpcodes::op_actorEntersScene(Script *script) {
	ARG_EVALUATE(prevModuleIndex);
	ARG_EVALUATE(prevSceneIndex);
	ARG_EVALUATE(nodeIndex);
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(scriptIndex);
	_vm->actorEntersScene(prevModuleIndex, prevSceneIndex, nodeIndex,
		pakName, pakSlot, frameListIndex, scriptIndex);
	return 0;
}

int16 ScriptOpcodes::op_addExitZoneAction(Script *script) {
	ARG_EVALUATE(zoneIndex);
	ARG_EVALUATE(nodeIndex);
	ARG_EVALUATE(scriptIndex);
	ARG_EVALUATE(moduleIndex);
	ARG_EVALUATE(sceneIndex);
	_vm->addZoneAction(zoneIndex, 3, nodeIndex, scriptIndex, kSceneScriptProgram, moduleIndex, sceneIndex, -1);
	return 0;
}

int16 ScriptOpcodes::op_addZoneAction90(Script *script) {
	ARG_EVALUATE(zoneIndex);
	ARG_EVALUATE(itemIndex);
	ARG_EVALUATE(nodeIndex);
	ARG_EVALUATE(scriptIndex);
	int16 zoneActionIndex = -1;
	if (zoneIndex >= 0) {
		zoneActionIndex = _vm->addZoneAction(zoneIndex, 9, nodeIndex, scriptIndex, kSceneScriptProgram, -1, -1, itemIndex);
	}
	return zoneActionIndex;
}

int16 ScriptOpcodes::op_removeItemFromInventory(Script *script) {
	ARG_EVALUATE(itemIndex);
	_vm->removeItemFromInventory(itemIndex);
	return itemIndex;
}

int16 ScriptOpcodes::op_removeActor(Script *script) {
	ARG_EVALUATE(actorIndex);
	_vm->clearActor(actorIndex);
	script->actorIndex = -1;
	return actorIndex;
}

int16 ScriptOpcodes::op_clearZoneAction(Script *script) {
	ARG_EVALUATE(zoneActionIndex);
	_vm->clearZoneAction(zoneActionIndex);
	return zoneActionIndex;
}

int16 ScriptOpcodes::op_path207E6(Script *script) {
	ARG_EVALUATE(type);
	ARG_EVALUATE(index);
	ARG_EVALUATE(enabled);
	_vm->_pathSystem->setPathEdgeOrPolygonEnabled(type, index, enabled);
	return 0;
}

int16 ScriptOpcodes::op_getValue(Script *script) {
	ARG_EVALUATE(type);
	ARG_EVALUATE(index);
	int16 result = 0;
	switch (type) {
	case 0:
		result = _vm->_pathSystem->getEdge(index)->enabled;
		break;
	case 1:
		result = _vm->_pathSystem->getPolygon(index)->enabled;
		break;
	case 2:
		if (_vm->_inventoryItems[index].status == 1)
			result = 1;
		break;
	case 3:
		if (_vm->_scriptPrograms[kSceneScriptProgram].scripts[index].status != kScriptStatusPaused)
			result = 1;
		break;
	default:
		break;
	}
	return result;
}

int16 ScriptOpcodes::op_actorPutAtPos(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(x);
	ARG_EVALUATE(y);
	script->actorIndex = actorIndex;
	_vm->actorPutAtPos(actorIndex, x, y);
	return -1;
}

int16 ScriptOpcodes::op_actorPutAtPathNode(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(nodeIndex);
	script->actorIndex = actorIndex;
	_vm->actorPutAtPathNode(actorIndex, nodeIndex);
	return -1;
}

int16 ScriptOpcodes::op_addZone(Script *script) {
	ARG_EVALUATE(x1);
	ARG_EVALUATE(y1);
	ARG_EVALUATE(x2);
	ARG_EVALUATE(y2);
	int16 zoneIndex = _vm->addZone(x1, y1, x2, y2, -1, NULL, -1, NULL);
	return zoneIndex;
}

int16 ScriptOpcodes::op_removeZone(Script *script) {
	ARG_EVALUATE(zoneIndex);
	_vm->removeZone(zoneIndex);
	return zoneIndex;
}

int16 ScriptOpcodes::op_not(Script *script) {
	ARG_EVALUATE(value);
	return value == 0 ? 1 : 0;
}

int16 ScriptOpcodes::op_actorAssignPathWalker(Script *script) {
	ARG_EVALUATE(actorIndex);
	_vm->actorAssignPathWalker(actorIndex);
	script->actorIndex = actorIndex;
	return actorIndex;
}

int16 ScriptOpcodes::op_setUserInput(Script *script) {
	if (_vm->_screenTextShowing) {
		script->ip -= 2;
		return 0;
	} else {
		ARG_EVALUATE(userInput);
		_vm->setUserInput(userInput != 0);
		return userInput;
	}
}

int16 ScriptOpcodes::op_startPaletteTask(Script *script) {
	ARG_EVALUATE(type);
	ARG_EVALUATE(value1);
	ARG_EVALUATE(value2);
	ARG_EVALUATE(value3);
	_vm->startPaletteTask(type, value1, value2, value3);
	return 0;
}

int16 ScriptOpcodes::op_playActorAnimation(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(nodeIndex);
 	_vm->setActorAnimation(actorIndex, pakName, pakSlot, frameListIndex, nodeIndex);
	script->status = kScriptStatusAnimation;
	script->actorIndex = actorIndex;
	script->actorIndex2 = actorIndex;
	return nodeIndex;
}

int16 ScriptOpcodes::op_setActorSpriteFrameListIndexIfIdle(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(frameListIndex);
	_vm->setActorSpriteFrameListIndexIfIdle(actorIndex, frameListIndex);
	script->actorIndex = actorIndex;
	return 0;
}

int16 ScriptOpcodes::op_actorAnimation20CC7(Script *script) {
	ARG_EVALUATE(actorIndex);
	Actor *actor = &_vm->_actors[actorIndex];
	if (actor->status != 0) {
		actor->status = 1;
		script->status = kScriptStatusAnimation;
		script->actorIndex2 = actorIndex;
	}
	script->actorIndex = -1;
	return 0;
}

int16 ScriptOpcodes::op_resetMessageValues(Script *script) {
	_vm->resetDialogValues();
	return 0;
}

int16 ScriptOpcodes::op_playSound(Script *script) {
	ARG_EVALUATE(soundIndex);
	_vm->playSound(soundIndex);
	return soundIndex;
}

int16 ScriptOpcodes::op_unloadSound(Script *script) {
	ARG_EVALUATE(soundIndex);
	_vm->unloadSound(soundIndex);
	return soundIndex;
}

int16 ScriptOpcodes::op_togglePathSystem(Script *script) {
	ARG_EVALUATE(flag);
	_vm->_pathSystem->togglePathSystem(flag);
	return 0;
}

int16 ScriptOpcodes::op_startMusicE(Script *script) {
	debug("ARGS!"); // TODO
	ARG_EVALUATE(musicIndex);
	_vm->playMusic(musicIndex);
	return 0;
}

int16 ScriptOpcodes::op_unloadMusic(Script *script) {
	ARG_EVALUATE(musicIndex);
	_vm->unloadMusic(musicIndex);
 	// TODO
	return 0;
}

int16 ScriptOpcodes::op_stopSound(Script *script) {
	ARG_EVALUATE(soundIndex);
	_vm->stopSound(soundIndex);
	return soundIndex;
}

int16 ScriptOpcodes::op_stopMusic(Script *script) {
	ARG_EVALUATE(musicIndex);
	_vm->stopMusic(musicIndex);
	debug("ARGS!"); // TODO
	return 0;
}

int16 ScriptOpcodes::op_isSoundPlaying(Script *script) {
	ARG_EVALUATE(soundIndex);
	return _vm->isSoundPlaying(soundIndex) ? 1 : 0;
}

int16 ScriptOpcodes::op_isMusicPlaying(Script *script) {
	ARG_EVALUATE(musicIndex);
	// TODO
	return _vm->isMusicPlaying(musicIndex);
}

int16 ScriptOpcodes::op_addActorFrameSound(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(soundIndex);
	ARG_EVALUATE(volume);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(frameIndex);
	int16 frameSoundIndex = _vm->addActorFrameSound(actorIndex, soundIndex, volume, frameListIndex, frameIndex);
	return frameSoundIndex;
}

int16 ScriptOpcodes::op_setZoneActionScript(Script *script) {
	ARG_EVALUATE(zoneActionIndex);
	ARG_EVALUATE(scriptIndex);
	if (zoneActionIndex >= 0) {
		_vm->setZoneActionScript(zoneActionIndex, scriptIndex);
	}
	return zoneActionIndex;
}

int16 ScriptOpcodes::op_interactActorMessage(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	int16 result = 0;
	if (!_vm->isScreenTextShowing()) {
		int16 outResourceCacheSlot;
		Common::String outIdentifier;
		_vm->getInteractMessage(pakName, pakSlot, identifier, outResourceCacheSlot, outIdentifier);
		script->screenTextIndex = _vm->addActorScreenText(actorIndex, outResourceCacheSlot, outIdentifier);
		script->status = kScriptStatusText;
		result = 1;
	}
	return result;
}

int16 ScriptOpcodes::op_setScriptZoneEnterLeaveFlag(Script *script) {
	ARG_EVALUATE(zoneEnterLeaveFlag);
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(zoneIndex);
	script->status = kScriptStatusActorZone;
	script->actorIndex2 = actorIndex;
	script->zoneIndex = zoneIndex;
	script->zoneEnterLeaveFlag = zoneEnterLeaveFlag == 0;
	return 0;
}

int16 ScriptOpcodes::op_setMouseAddXY(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_playLoopingSound(Script *script) {
	ARG_EVALUATE(soundIndex);
	ARG_EVALUATE(loops);
	_vm->playLoopingSound(soundIndex, loops);
	return soundIndex;
}

int16 ScriptOpcodes::op_removeActorFrameSound(Script *script) {
	ARG_EVALUATE(frameSoundIndex);
	_vm->removeActorFrameSound(frameSoundIndex);
	return 0;
}

int16 ScriptOpcodes::op_setActorFrameSound(Script *script) {
	ARG_EVALUATE(frameSoundIndex);
	ARG_EVALUATE(soundIndex);
	ARG_EVALUATE(volume);
	_vm->setActorFrameSound(frameSoundIndex, soundIndex, volume);
	return 0;
}

int16 ScriptOpcodes::op_playActorAnimationAtPos(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(x);
	ARG_EVALUATE(y);
 	_vm->setActorAnimationAtPos(actorIndex, pakName, pakSlot, frameListIndex, x, y);
	script->status = kScriptStatusAnimation;
	script->actorIndex = actorIndex;
	script->actorIndex2 = actorIndex;
	return frameListIndex;
}

int16 ScriptOpcodes::op_startMusic(Script *script) {
	ARG_EVALUATE(musicIndex);
	ARG_EVALUATE(unk);
 	// TODO
	_vm->playMusic(musicIndex);
	return 0;
}

int16 ScriptOpcodes::op_setBackgroundCameraLocked(Script *script) {
	ARG_EVALUATE(backgroundCameraLocked);
	_vm->setBackgroundCameraLocked(backgroundCameraLocked != 0);
	return 0;
}

int16 ScriptOpcodes::op_removeInventoryItemCombination(Script *script) {
	ARG_EVALUATE(combinationIndex);
	_vm->removeInventoryItemCombination(combinationIndex);
	return 0;
}

int16 ScriptOpcodes::op_getVar(Script *script) {
	ARG_EVALUATE(varIndex);
	int16 value;
	if (varIndex & 0x8000) {
		value = _vm->_moduleScriptVars[varIndex & 0x7FFF];
	} else {
		value = _vm->_globalScriptVars[varIndex];
	}
	return value;
}

int16 ScriptOpcodes::op_addLooseScreenText(Script *script) {
	ARG_EVALUATE(x);
	ARG_EVALUATE(y);
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	int16 result = 0;
	if (!_vm->isScreenTextShowing()) {
		int16 resourceCacheSlot = _vm->loadTextResource(pakName, pakSlot);
		script->status = kScriptStatusText;
		script->screenTextIndex = _vm->addLooseScreenText(x, y, resourceCacheSlot, identifier);
		result = 1;
	}
	return result;
}

int16 ScriptOpcodes::op_actorWalkToPoint(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(x);
	ARG_EVALUATE(y);
	script->status = kScriptStatusWalking;
	script->actorIndex = actorIndex;
	script->actorIndex2 = actorIndex;
	_vm->actorWalkToPoint(actorIndex, x, y);
	return actorIndex;
}

int16 ScriptOpcodes::op_resetCurrInventoryItemCursor(Script *script) {
	_vm->_inventoryItemCursor = -1;
	return 0;
}

int16 ScriptOpcodes::op_interactScreenMessage(Script *script) {
	ARG_EVALUATE(x);
	ARG_EVALUATE(y);
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	int16 result = 0;
	if (!_vm->isScreenTextShowing()) {
		int16 outResourceCacheSlot;
		Common::String outIdentifier;
		_vm->getInteractMessage(pakName, pakSlot, identifier, outResourceCacheSlot, outIdentifier);
		script->screenTextIndex = _vm->addLooseScreenText(x, y, outResourceCacheSlot, outIdentifier);
		script->status = kScriptStatusText;
		result = 1;
	}
	return result;
}

int16 ScriptOpcodes::op_nop(Script *script) {
	return 0;
}

int16 ScriptOpcodes::op_addScreenText2(Script *script) {
	ARG_EVALUATE(lipSyncX);
	ARG_EVALUATE(lipSyncY);
	ARG_PAKNAME_S(animPakName, false);
	ARG_EVALUATE(animPakSlot);
	ARG_EVALUATE(textX);
	ARG_EVALUATE(textY);
	ARG_PAKNAME(textPakName, true);
	ARG_EVALUATE(textPakSlot);
	ARG_STRING(identifier);
	int16 textResourceCacheSlot = _vm->loadTextResource(textPakName, textPakSlot);
	script->status = kScriptStatusText;
	script->screenTextIndex = _vm->addLooseScreenText(textX, textY, textResourceCacheSlot, identifier);
	_vm->startLipSync(animPakName, animPakSlot, _vm->_currScriptIndex, script->actorIndex, lipSyncX, lipSyncY);
	return script->screenTextIndex;
}

int16 ScriptOpcodes::op_actorText21704(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_PAKNAME(textPakName, true);
	ARG_EVALUATE(textPakSlot);
	ARG_STRING(identifier);
	ARG_PAKNAME_S(animPakName, false);
	ARG_EVALUATE(animPakSlot);
	ARG_EVALUATE(firstFrameListIndex);
	ARG_EVALUATE(lastFrameListIndex);
	ARG_EVALUATE(minTicks);
	ARG_EVALUATE(maxTicks);
	int16 result = 0;
	Actor *actor = &_vm->_actors[actorIndex];
	actor->firstFrameListIndex = firstFrameListIndex;
	actor->lastFrameListIndex = lastFrameListIndex;
	actor->minTicks = minTicks;
	actor->maxTicks = maxTicks;
 	if (!_vm->isScreenTextShowing()) {
		int16 textResourceCacheSlot = _vm->loadTextResource(textPakName, textPakSlot);
		script->screenTextIndex = _vm->addActorScreenText(actorIndex, textResourceCacheSlot, identifier);
		_vm->restoreActorAnimation(actorIndex);
		_vm->backupActorAnimation(actorIndex);
		_vm->setActorAnimation(actorIndex, animPakName, animPakSlot, firstFrameListIndex, -1);
		_vm->setActorRandomFrameListIndex(actor);
		actor->ticksFlag = 1;
		actor->ticks = 0;
		script->status = kScriptStatus10;
		script->actorIndex = actorIndex;
		script->actorIndex2 = actorIndex;
		result = 1;
 	}
	return result;
}

int16 ScriptOpcodes::op_actorAnimation218A1(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(firstFrameListIndex);
	ARG_EVALUATE(lastFrameListIndex);
	ARG_EVALUATE(minTicks);
	ARG_EVALUATE(maxTicks);
	_vm->actorAnimation218A1(actorIndex, pakName, pakSlot, firstFrameListIndex,
		lastFrameListIndex, minTicks, maxTicks);
	script->actorIndex = actorIndex;
	return 0;
}

int16 ScriptOpcodes::op_startModuleScript(Script *script) {
	ARG_EVALUATE(scriptIndex);
	_vm->stopScriptProgram(kSceneScriptProgram);
	_vm->startScript(kModuleScriptProgram, scriptIndex);
	_vm->_moduleScriptCalled = true;
	return 0;
}

int16 ScriptOpcodes::op_death(Script *script) {
	_vm->death();
	return 0;
}

int16 ScriptOpcodes::op_loadModuleSound(Script *script) {
	ARG_PAKNAME(pakName, false);
	ARG_EVALUATE(pakSlot);
	return _vm->loadSound(pakName, pakSlot, true);
}

int16 ScriptOpcodes::op_playSoundSync(Script *script) {
	ARG_EVALUATE(soundIndex);
 	_vm->playSound(soundIndex);
 	script->status = kScriptStatusSound;
 	script->soundIndex = soundIndex;
	return soundIndex;
}

int16 ScriptOpcodes::op_setActorFontColors(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(inkColor);
	ARG_EVALUATE(outlineColor);
	_vm->setActorFontColors(actorIndex, inkColor, outlineColor);
	return 0;
}

int16 ScriptOpcodes::op_setTextDisplayColor(Script *script) {
	ARG_EVALUATE(textDisplayNum);
	ARG_EVALUATE(outlineColor);
	ARG_EVALUATE(inkColor);
	_vm->setTextDisplayColor(textDisplayNum, outlineColor, inkColor);
	return 0;
}

int16 ScriptOpcodes::op_loadInventoryItemsAnimation(Script *script) {
 	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	ARG_EVALUATE(slotBaseIndex);
	_vm->loadInventoryItemsAnimation(pakName, pakSlot, slotBaseIndex);
	return 0;
}

int16 ScriptOpcodes::op_playMux(Script *script) {
	ARG_STRING(muxFilename);
	ARG_EVALUATE(muxClearScreenBefore);
	ARG_EVALUATE(muxClearScreenAfter);
	_vm->playMuxSoon(muxFilename, muxClearScreenBefore, muxClearScreenAfter);
	return 0;
}

int16 ScriptOpcodes::op_setCameraFollowsActor(Script *script) {
	ARG_EVALUATE(actorIndex);
 	_vm->setCameraFollowsActor(actorIndex);
	script->actorIndex = actorIndex;
	return 0;
}

int16 ScriptOpcodes::op_actor21C78(Script *script) {
 	_vm->actor21C78();
	return 0;
}

int16 ScriptOpcodes::op_actor21C89(Script *script) {
	_vm->actor21C89();
	return 0;
}

int16 ScriptOpcodes::op_waitForActorAnimation(Script *script) {
	ARG_EVALUATE(actorIndex);
	script->status = kScriptStatusAnimation;
	script->actorIndex = actorIndex;
	script->actorIndex2 = actorIndex;
	return 0;
}

int16 ScriptOpcodes::op_addActorAltAnimation(Script *script) {
	ARG_PAKNAME_S(pakName, false);
	ARG_EVALUATE(pakSlot);
	script->actorIndex = _vm->addActorAltAnimation(pakName, pakSlot);
	return script->actorIndex;
}

int16 ScriptOpcodes::op_unloadActorAltAnimation(Script *script) {
	ARG_EVALUATE(altActorAnimationIndex);
 	_vm->unloadActorAltAnimation(altActorAnimationIndex);
 	script->actorIndex = -1;
	return altActorAnimationIndex;
}

int16 ScriptOpcodes::op_sub_21D2D(Script *script) {
	debug("ARGS!"); // TODO
	return 0;
}

int16 ScriptOpcodes::op_setActorAltAnimationAtPos(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(altAnimationIndex);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(x);
	ARG_EVALUATE(y);
	_vm->setActorAltAnimationAtPos(actorIndex, altAnimationIndex, frameListIndex, x, y);
	script->actorIndex2 = actorIndex;
	script->altAnimationIndex = altAnimationIndex;
	script->actorIndex = actorIndex;
	script->status = kScriptStatus12;
	return 0;
}

int16 ScriptOpcodes::op_actor21E0D(Script *script) {
	ARG_EVALUATE(actorIndex);
	ARG_EVALUATE(frameListIndex);
	ARG_EVALUATE(frameIndex);
	ARG_EVALUATE(scriptIndex);

	Actor *actor = &_vm->_actors[actorIndex];
	ActorSprite *actorSprite = actor->actorSprite;
	Script *otherScript = &_vm->_scriptPrograms[_vm->_currScriptProgramIndex].scripts[scriptIndex];

	_vm->restoreActorAnimation(actorIndex);

	if (frameListIndex == -1) {
		if (actorSprite->frameListIndex >= 8) {
			actorSprite->x += actorSprite->xsub;
			actorSprite->y += actorSprite->ysub;
			actorSprite->xsub = 0;
			actorSprite->ysub = 0;
			_vm->setActorSpriteFrameListIndex(actorSprite, actorSprite->frameListIndex - 8, true);
		}
	} else {
		_vm->setActorSpriteFrameListIndex(actorSprite, frameListIndex, true);
	}

	if (actorIndex == _vm->_mainActorIndex) {
		_vm->_queuedZoneAction.used = 0;
		_vm->_queuedZoneAction.zoneActionIndex = -1;
		_vm->_queuedZoneAction.pathNodeIndex = -1;
	}

	actor->pathResultIndex = 0;
	actor->pathResultCount = 0;
	actor->status = 1;

	script->status = kScriptStatusAnimation;
	script->actorIndex = actorIndex;
	script->actorIndex2 = actorIndex;

	_vm->startScript(_vm->_currScriptProgramIndex, scriptIndex);

	otherScript->actorIndex2 = actorIndex;
	otherScript->frameIndex = frameIndex;
	otherScript->status = kScriptStatus13;

	return actorIndex;
}

int16 ScriptOpcodes::op_addZoneAction91(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_setUpdateDirtyRectsFlag(Script *script) {
	ARG_EVALUATE(updateDirtyRectsFlag);
	_vm->_updateDirtyRectsFlag = updateDirtyRectsFlag != 0;
	return 0;
}

int16 ScriptOpcodes::op_sub_21FD9(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_sub_220B8(Script *script) {
	debug("ARGS!"); // TODO; Unused?
	return 0;
}

int16 ScriptOpcodes::op_autoSave(Script *script) {
	ARG_PAKNAME(pakName, true);
	ARG_EVALUATE(pakSlot);
	ARG_STRING(identifier);
	_vm->requestAutoSave(pakName, pakSlot, identifier);
	return 0;
}

int16 ScriptOpcodes::op_setMainActorValid(Script *script) {
	ARG_EVALUATE(value);
	_vm->_mainActorValid = value != 0;
	script->actorIndex = _vm->_mainActorIndex;
	return 0;
}

} // End of namespace Prisoner
