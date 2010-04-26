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
#include "prisoner/resource.h"
#include "prisoner/scriptops.h"

namespace Prisoner {

byte Script::readByte() {
	return *ip++;
}

int16 Script::readInt16() {
	int16 value = READ_LE_UINT16(ip);
	ip += 2;
	return value;
}

Common::String Script::readString() {
	Common::String value;
	value = (char*)ip;
	ip += value.size() + 1;
	// Strings are aligned on 2-byte-boundaries
	if (value.size() % 2 == 0)
		ip++;
	return value;
}

void PrisonerEngine::clearScriptPrograms() {
	_scriptPrograms[kSceneScriptProgram].resourceCacheSlot = -1;
	_scriptPrograms[kModuleScriptProgram].resourceCacheSlot = -1;
}

void PrisonerEngine::loadScriptProgram(Common::String &pakName, int16 pakSlot, int16 programIndex) {
	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	scriptProgram->resourceCacheSlot = _res->load<ScriptResource>(pakName, pakSlot,
		programIndex == kModuleScriptProgram ? 0 : 10);
	scriptProgram->scriptResource = _res->get<ScriptResource>(scriptProgram->resourceCacheSlot);
	scriptProgram->scriptCount = scriptProgram->scriptResource->getCount() - 1;
}

void PrisonerEngine::clearScriptProgram(int16 programIndex) {
	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	scriptProgram->resourceCacheSlot = -1;
	for (int16 scriptIndex = 0; scriptIndex < kMaxScripts; scriptIndex++) {
		Script *script = &scriptProgram->scripts[scriptIndex];
		script->status = kScriptStatusPaused;
		script->actorIndex = -1;
		script->actorIndex2 = -1;
		script->soundItemIndex = -1;
		script->ip = NULL;
	}
}

void PrisonerEngine::unloadScriptProgram(int16 programIndex) {
	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	if (scriptProgram->resourceCacheSlot != -1) {
		_res->unload(scriptProgram->resourceCacheSlot);
		scriptProgram->resourceCacheSlot = -1;
	}
}

void PrisonerEngine::startScript(int16 programIndex, int16 scriptIndex) {

	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	Script *script = &scriptProgram->scripts[scriptIndex];

	script->ip = scriptProgram->scriptResource->getScript(scriptIndex);
	script->status = kScriptStatusRunCode;
	script->actorIndex2 = -1;
	script->unk1 = -1;
	script->soundItemIndex = -1;
	script->frameIndex = -1;
	script->zoneIndex = -1;
	script->zoneEnterLeaveFlag = 0;
	script->screenTextIndex = -1;
	script->sleepCounter = 0;
	script->unk3 = 0;
	script->syncScriptNumber = -1;
	script->actorIndex = -1;

}

void PrisonerEngine::stopScript(int16 programIndex, int16 scriptIndex) {

	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	Script *script = &scriptProgram->scripts[scriptIndex];

	if (script->status == kScriptStatusText)
		unloadScreenText(script->screenTextIndex);

	script->ip = NULL;
	script->status = kScriptStatusPaused;

	if (script->actorIndex2 != -1) {
		if (_actors[script->actorIndex2].pathResultCount > 0)
			resetActorPathWalk(script->actorIndex2);
		script->actorIndex2 = -1;
	}

	if (script->soundItemIndex != -1) {
		// TODO: stopSoundItem(script->soundItemIndex);
		script->soundItemIndex = -1;
	}

}

void PrisonerEngine::stopScriptProgram(int16 programIndex) {
	debug("PrisonerEngine::stopScriptProgram(%d)", programIndex);
	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	for (int16 scriptIndex = 0; scriptIndex <= scriptProgram->scriptCount; scriptIndex++) {
		stopScript(programIndex, scriptIndex);
	}
	_lipSyncScriptNumber = -1;
}

void PrisonerEngine::runCurrentScript() {

	bool done;
	Actor *actor;

	do {

		done = true;

		//debug(1, "_currScriptIndex = %d; _currScript->status = %d", _currScriptIndex, _currScript->status);

		switch (_currScript->status) {

		case kScriptStatusPaused:
			// Do nothing
			break;

		case kScriptStatusRunCode:
		{
			int16 opcode = _currScript->readInt16();
			//debug(1, "opcode = %d", opcode);
			_scriptOpcodes->execOpcode(_currScript, opcode);
			//debug(5, "------------------------------------------------------");
			if (_currScriptIndex == _leaveSceneScriptIndex &&
				_currScriptProgramIndex == _leaveSceneScriptProgramIndex &&
				_currScript->status == kScriptStatusPaused) {
				stopScriptProgram(_currScriptProgramIndex);
				_scriptContinueFlag = false;
			}
			break;
		}

		case kScriptStatusSleeping:
			debug("_currScript->sleepCounter(1) = %d", _currScript->sleepCounter);
			_currScript->sleepCounter -= _frameTicks;
			debug("_currScript->sleepCounter(2) = %d", _currScript->sleepCounter);
			//debug(1, "kScriptStatusSleeping: sleepCounter = %d", _currScript->sleepCounter);
			if (_currScript->sleepCounter <= 0)
				_currScript->status = kScriptStatusRunCode;
			break;

		case kScriptStatusSync:
		{
			Script *otherScript = &_scriptPrograms[_currScriptProgramIndex].scripts[_currScript->syncScriptNumber];
			if (otherScript->status == kScriptStatusSync && otherScript->syncScriptNumber == _currScriptIndex) {
				otherScript->status = kScriptStatus8;
				_currScript->status = kScriptStatus8;
			}
			break;
		}

		case kScriptStatusAnimation:
			actor = &_actors[_currScript->actorIndex2];
			if (actor->pathResultCount == 0 && actor->status == 0) {
				_currScript->status = kScriptStatusRunCode;
				_currScript->actorIndex2 = -1;
				done = _currScriptIndex != _lipSyncScriptNumber;
			}
			break;

		case kScriptStatusText:
			if (_screenTexts[_currScript->screenTextIndex].used == 0) {
				_currScript->status = kScriptStatusRunCode;
				_currScript->screenTextIndex = -1;
			}
			break;

		case kScriptStatusDialog:
			if (!_dialogRunning) {
				_currScript->status = kScriptStatusRunCode;
				if (_selectedDialogKeywordIndex != -1)
					_currScript->ip += 4;
			}
			break;

		case kScriptStatusWalking:
			actor = &_actors[_currScript->actorIndex2];
			if (actor->pathResultCount == 0) {
				_currScript->status = kScriptStatusRunCode;
				_currScript->actorIndex2 = -1;
			}
			break;

		case kScriptStatus8:
			_currScript->status = kScriptStatusRunCode;
			_currScript->syncScriptNumber = 0;
			break;

		case kScriptStatusActorZone:
			actor = &_actors[_currScript->actorIndex2];
			if (isPointInZone(actor->actorSprite->x, actor->actorSprite->y, _currScript->zoneIndex) ==
				_currScript->zoneEnterLeaveFlag) {
				_currScript->status = kScriptStatusRunCode;
				_currScript->actorIndex2 = -1;
				_currScript->zoneIndex = -1;
				done = false;
			}
			break;

		case kScriptStatus10:
			if (_screenTexts[_currScript->screenTextIndex].used == 0) {
				actor = &_actors[_currScript->actorIndex2];
				setActorAnimation(_currScript->actorIndex2, actor->pakName, actor->pakSlot, actor->frameListIndex, -1);
				_currScript->status = kScriptStatusRunCode;
				_currScript->actorIndex2 = -1;
				_currScript->screenTextIndex = -1;
			}
			break;

		case kScriptStatusSound:
			// TODO
			break;

		case kScriptStatus12:
			actor = &_actors[_currScript->actorIndex2];
			if (actor->pathResultCount == 0 && actor->status == 0) {
				_currScript->status = kScriptStatusRunCode;
				_currScript->actorIndex2 = -1;
			}
			break;

		case kScriptStatus13:
			actor = &_actors[_currScript->actorIndex2];
			if (actor->status == 0 || actor->actorSprite->frameIndex == _currScript->frameIndex) {
				_currScript->status = kScriptStatusRunCode;
				_currScript->actorIndex2 = -1;
			}
			break;

		}

	} while (!done);

}

void PrisonerEngine::runInitScript(int16 programIndex) {
	debug(5, "runInitScript(%d)", programIndex);
	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	_currScript = &scriptProgram->scripts[0];
	_currScriptProgramIndex = programIndex;
	_currScriptIndex = 0;
	while (_currScript->status != kScriptStatusPaused) {
		// TODO: updateMusicItems(); // Probably not
		runCurrentScript();
		if (_needToPlayMux) {
			playMux(_muxFilename);
			_needToPlayMux = false;
		}
	}
}

void PrisonerEngine::runScripts(int16 programIndex) {
	//debug("PrisonerEngine::runScripts(%d)", programIndex);
	ScriptProgram *scriptProgram = &_scriptPrograms[programIndex];
	if (programIndex == _enterSceneScriptProgramIndex && !isEnterSceneScriptFinished()) {
		_currScript = &scriptProgram->scripts[_enterSceneScriptIndex];
		_currScriptProgramIndex = _enterSceneScriptProgramIndex;
		_currScriptIndex = _enterSceneScriptIndex;
		runCurrentScript();
	} else {
		_currScriptProgramIndex = programIndex;
		for (int16 scriptIndex = 1; scriptIndex <= scriptProgram->scriptCount; scriptIndex++) {
			_currScript = &scriptProgram->scripts[scriptIndex];
			_currScriptIndex = scriptIndex;
			if (_currScript->status != kScriptStatusPaused) {
				_scriptContinueFlag = false;
				// TODO: updateMusicItems(); // See above
				do {
					runCurrentScript();
					if (_needToPlayMux)
						return;
				} while (_scriptContinueFlag);
			}
		}
	}
}

void PrisonerEngine::startLocalScript(int16 scriptIndex) {
	startScript(_currScriptProgramIndex, scriptIndex);
}

void PrisonerEngine::stopLocalScript(int16 scriptIndex) {
	debug("stopLocalScript(%d)", scriptIndex);
	stopScript(_currScriptProgramIndex, scriptIndex);
	if (_queuedZoneAction.scriptIndex1 == _currScriptIndex) {
		_queuedZoneAction.used = 0;
		_queuedZoneAction.scriptIndex1 = -1;
		_queuedZoneAction.zoneActionIndex = -1;
		_exitZoneActionFlag = true;
	}
	debug("stopLocalScript(%d) OK", scriptIndex);
}

} // End of namespace Prisoner
