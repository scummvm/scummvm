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

#include "neverhood/gamevars.h"

namespace Neverhood {

GameVars::GameVars() {
	addVar(0, 0);
}

GameVars::~GameVars() {
}

uint32 GameVars::getGlobalVar(uint32 nameHash) {
	//debug("GameVars::getGlobalVar(%08X)", nameHash);
	int16 varIndex = findSubVarIndex(0, nameHash);
	return varIndex != -1 ? _vars[varIndex].value : 0;
}

void GameVars::setGlobalVar(uint32 nameHash, uint32 value) {
	//debug("GameVars::setGlobalVar(%08X, %d)", nameHash, value);
	_vars[getSubVarIndex(0, nameHash)].value = value;
}

uint32 GameVars::getSubVar(uint32 nameHash, uint32 subNameHash) {
	//debug("GameVars::getSubVar(%08X, %08X)", nameHash, subNameHash);
	uint32 value = 0;
	int16 varIndex = findSubVarIndex(0, nameHash);
	if (varIndex != -1) {
		int16 subVarIndex = findSubVarIndex(varIndex, subNameHash);
		if (subVarIndex != -1) {
			value = _vars[subVarIndex].value;
		}
	} 
	return value;
}

void GameVars::setSubVar(uint32 nameHash, uint32 subNameHash, uint32 value) {
	//debug("GameVars::setSubVar(%08X, %08X, %d)", nameHash, subNameHash, value);
	int16 varIndex = getSubVarIndex(0, nameHash);
	//debug("  varIndex = %d", varIndex);
	int16 subVarIndex = getSubVarIndex(varIndex, subNameHash);
	//debug("  subVarIndex = %d", subVarIndex);
	_vars[subVarIndex].value = value;
	//_vars[getSubVarIndex(getSubVarIndex(0, nameHash), subNameHash)].value = value;
}

int16 GameVars::addVar(uint32 nameHash, uint32 value) {
	//debug("GameVars::addVar(%08X, %d)", nameHash, value);
	GameVar gameVar;
	gameVar.nameHash = nameHash;
	gameVar.value = value;
	gameVar.firstIndex = -1;
	gameVar.nextIndex = -1;
	_vars.push_back(gameVar);
	return _vars.size() - 1;
}

int16 GameVars::findSubVarIndex(int16 varIndex, uint32 subNameHash) {
	//debug("GameVars::findSubVarIndex(%d, %08X)", varIndex, subNameHash);
	for (int16 nextIndex = _vars[varIndex].firstIndex; nextIndex != -1; nextIndex = _vars[nextIndex].nextIndex) {
		if (_vars[nextIndex].nameHash == subNameHash)
			return nextIndex;
	}
	return -1;
}

int16 GameVars::addSubVar(int16 varIndex, uint32 subNameHash, uint32 value) {
	//debug("GameVars::addSubVar(%d, %08X, %d)", varIndex, subNameHash, value);
	int16 nextIndex = _vars[varIndex].firstIndex;
	int16 subVarIndex;
	if (nextIndex == -1) {
		subVarIndex = addVar(subNameHash, value);
		_vars[varIndex].firstIndex = subVarIndex;
	} else {
		while (_vars[nextIndex].nextIndex != -1)
			nextIndex = _vars[nextIndex].nextIndex;
		subVarIndex = addVar(subNameHash, value);
		_vars[nextIndex].nextIndex = subVarIndex;
	}
	return subVarIndex;
}

int16 GameVars::getSubVarIndex(int16 varIndex, uint32 subNameHash) {
	//debug("GameVars::getSubVarIndex(%d, %08X)", varIndex, subNameHash);
	int16 subVarIndex = findSubVarIndex(varIndex, subNameHash);
	if (subVarIndex == -1) {
		subVarIndex = addSubVar(varIndex, subNameHash, 0);
		debug("need to create: subVarIndex = %d", subVarIndex);
	}
	return subVarIndex;
}

} // End of namespace Neverhood
