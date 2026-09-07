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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FREESCAPE_GAMES_3DCK_H
#define FREESCAPE_GAMES_3DCK_H

#include "freescape/freescape.h"
#include "freescape/language/execution_3dck16.h"

namespace Freescape {

class KitEngine : public FreescapeEngine {
public:
	KitEngine(OSystem *syst, const ADGameDescription *gd);

	void loadAssets() override;
	void initGameState() override;
	void gotoArea(uint16 areaID, int entranceID) override;
	void checkIfStillInArea() override;
	bool checkIfGameEnded() override;
	void borderScreen() override {}
	void drawUI() override;
	bool handleInput(const Common::Event &event) override;
	void updatePlayerMovement(float deltaTime) override;
	void updateTimeVariables() override;
	void updateScripts() override;
	bool executeObjectConditions(GeometricObject *obj, bool shot, bool collided, bool activated) override;
	void executeLocalGlobalConditions(bool shot, bool collided, bool timer) override {}
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override { return false; }
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override { return false; }

private:
	struct ObjectData;
	struct ScriptState : FCLKit16ExecutionState {
		ObjectData *object = nullptr;
		uint16 area = 0;
		byte events = 0;
	};

	struct ConditionData {
		Common::String name;
		FCLInstructionVector condition;
		ScriptState script;
	};

	struct SensorData {
		byte colors[2] = {};
		uint16 interval = 0;
		uint16 range = 0;
		uint16 unknown = 0;
		uint16 directions = 0;
	};

	struct ObjectData {
		uint16 id = 0;
		byte type = 0;
		byte flags = 0;
		uint16 state = 0;
		Math::Vector3d origin, size, initialOrigin;
		Common::Array<uint16> members;
		Common::Array<uint16> extra;
		FCLInstructionVector condition;
		SensorData sensor;
		ScriptState script;
		Common::Array<uint16> animatedObjects;
		uint16 animator = 0;
	};

	struct AreaData {
		Common::HashMap<uint16, ObjectData> objects;
		Common::Array<uint16> objectOrder;
		Common::Array<ConditionData> conditions;
	};

	struct ScriptEntry {
		ScriptState *script;
		bool resume;
		ScriptEntry(ScriptState *s, bool r) : script(s), resume(r) {}
	};

	void loadWorld(Common::SeekableReadStream &file);
	void loadSounds(Common::SeekableReadStream &file);
	void playPendingSound();
	Area *loadArea(Common::SeekableReadStream &file);
	Object *loadObject(Common::SeekableReadStream &file, ObjectData &data);
	Common::Array<ConditionData> loadConditions(Common::SeekableReadStream &file);
	void resetScripts();
	void startScript(ScriptState &script);
	void beginScriptFrame();
	FCLExecutionResult executeCode(ScriptState &script, uint &budget);
	void executeIfThenElse(const FCLInstruction &instruction, ScriptState &script);
	void executeConditional(const FCLInstruction &instruction, ScriptState &script);
	void executeSetVariable(const FCLInstruction &instruction, ScriptState &script);
	void executeIncrementVariable(const FCLInstruction &instruction, ScriptState &script);
	void executeDecrementVariable(const FCLInstruction &instruction, ScriptState &script);
	void executeAndVariable(const FCLInstruction &instruction, ScriptState &script);
	void executeOrVariable(const FCLInstruction &instruction, ScriptState &script);
	void executeNotVariable(const FCLInstruction &instruction, ScriptState &script);
	void executeVariableComparison(const FCLInstruction &instruction, ScriptState &script);
	void executeObjectStatus(const FCLInstruction &instruction);
	bool checkObjectStatus(const FCLInstruction &instruction);
	void executeGetPosition(const FCLInstruction &instruction);
	bool executeCall(const FCLInstruction &instruction, ScriptState &script);
	bool executeGoto(const FCLInstruction &instruction);
	void executeMode(const FCLInstruction &instruction);
	bool executeDelay(const FCLInstruction &instruction);
	void executeLoop(const FCLInstruction &instruction, ScriptState &script);
	bool executeAgain(ScriptState &script, uint32 ip);
	void executeStartAnim(const FCLInstruction &instruction);
	void executeStopAnim(const FCLInstruction &instruction);
	void executeTriggerAnim(const FCLInstruction &instruction);
	void executeInclude(const FCLInstruction &instruction, ScriptState &script);
	void executeRemove(const FCLInstruction &instruction, ScriptState &script);
	bool executeWaitTrigger(ScriptState &script, uint32 ip);
	void executeMove(const FCLInstruction &instruction, ScriptState &script);
	void executeSound(const FCLInstruction &instruction);
	void readSystemVariables();
	void writeSystemVariables();
	void setScriptVariable(byte index, uint32 value);
	void setVariableResult(const FCLInstruction &instruction, ScriptState &script, uint32 value);
	int32 getVariableOrConstant(int32 operand, Token::Type type) const;
	void getObjectReference(const FCLInstruction &instruction, uint16 &area, uint16 &id) const;
	ObjectData *scriptObject(uint16 area, uint16 id);
	ObjectData *scriptAnimator(const FCLInstruction &instruction);
	void collectObjects(uint16 area, uint16 id, Common::Array<uint16> &objects);
	void setObjectStatus(uint16 area, uint16 id, Token::Type operation);
	bool moveAnimation(ScriptState &script, Math::Vector3d movement, bool absolute);
	void updateInteractions();
	void interact(bool shot);
	void printMessage(uint16 indicator, const Common::String &message);
	void updateIndicators();
	uint32 indicatorColor(byte color) const;

	Common::HashMap<uint16, AreaData> _areaData;
	Common::Array<ConditionData> _globalConditions;
	Common::Array<uint16> _indicatorData;
	Common::Array<uint16> _controlData;

	byte _palette[256 * 3];
	uint16 _initialPlayerHeight;
	uint16 _initialCondition = 0, _timerInterval = 0, _activationRange = 0;
	uint32 _kitVariables[256] = {};
	uint32 _changedVariables = 0;
	uint32 _scriptTicks = 0, _timerTicks = 0, _delayUntil = 0;
	int _lastScriptTick = 0;
	bool _timerTriggered = false, _initialScriptPending = false;
	bool _scriptFrameActive = false, _scriptDelayed = false;
	int _pendingSound = -1;
	byte _pendingInteractions = 0, _shootCooldown = 0, _activateCooldown = 0;
	uint _scriptQueueIndex = 0;
	Common::Array<ScriptEntry> _scriptQueue;
	Common::Array<ScriptState *> _suspendedScripts;
	Graphics::ManagedSurface _scriptSurface;
};

} // namespace Freescape

#endif
