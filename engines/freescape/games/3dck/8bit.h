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

#ifndef FREESCAPE_GAMES_3DCK_8BIT_H
#define FREESCAPE_GAMES_3DCK_8BIT_H

#include "freescape/freescape.h"
#include "freescape/language/execution_3dck8.h"

namespace Freescape {

class Kit8Engine : public FreescapeEngine {
public:
	Kit8Engine(OSystem *syst, const ADGameDescription *gd);

	void loadAssets() override;
	void initGameState() override;
	void gotoArea(uint16 areaID, int entranceID) override;
	void checkIfStillInArea() override;
	bool checkIfGameEnded() override { return false; }
	void borderScreen() override {}
	void drawUI() override;
	bool handleInput(const Common::Event &event) override;
	void updatePlayerMovement(float deltaTime) override;
	void updateTimeVariables() override;
	void checkSensors() override;
	void updateScripts() override;
	bool executeObjectConditions(GeometricObject *obj, bool shot, bool collided, bool activated) override;
	void executeLocalGlobalConditions(bool shot, bool collided, bool timer) override {}
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override { return false; }
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override { return false; }

private:
	typedef FCLKit8ExecutionState ScriptState;

	struct ConditionData {
		byte id;
		FCLInstructionVector condition;
	};

	struct AreaData {
		byte palette[4];
		Common::Array<byte> globals;
		Common::Array<ConditionData> conditions;
	};

	Common::Array<ConditionData> loadConditions(Common::SeekableReadStream &file);
	Area *loadArea(Common::SeekableReadStream &file);
	GeometricObject *loadGeometricObject(Common::SeekableReadStream &file, const byte header[9]);
	void loadPresentation();
	void applyPalette();
	void setMovementMode(byte mode);
	void readSystemVariables();
	void writeSystemVariables();
	void resetScripts();
	void beginScriptFrame();
	void startScript(ScriptState &script, const FCLInstructionVector &code);
	FCLExecutionResult executeCode(ScriptState &script, uint &budget);
	void executeArithmetic(const FCLInstruction &instruction, ScriptState &script);
	void executeComparison(const FCLInstruction &instruction, ScriptState &script);
	void executeConditional(const FCLInstruction &instruction, ScriptState &script);
	void executeObjectStatus(const FCLInstruction &instruction);
	void executeGoto(const FCLInstruction &instruction);
	void executeMode(const FCLInstruction &instruction);
	void executeCall(const FCLInstruction &instruction, ScriptState &script);
	void executeSound(const FCLInstruction &instruction);
	void executeColour(const FCLInstruction &instruction);
	void getObjectReference(const FCLInstruction &instruction, uint16 &area, uint16 &id) const;
	Object *scriptObject(uint16 area, uint16 id);
	void interact(bool shot);
	void printMessage(byte id, byte x, byte y);
	void printText(const Common::String &text, byte x, byte y, byte color);
	void updateInstruments();

	Common::HashMap<uint16, AreaData> _areaData;
	Common::Array<ConditionData> _globalConditions, _procedures;
	Common::HashMap<byte, Common::String> _kitMessages;
	ScriptState _script;
	const Common::Array<ConditionData> *_activeConditions = nullptr;
	uint _conditionIndex = 0;
	bool _initialScriptPending = true, _scriptFrameActive = false, _globalPhase = false;
	byte _kitVariables[128] = {};
	uint16 _changedVariables = 0;
	byte _currentKey = 255;
	byte _palette[4] = {};
	byte _colorPatterns[15][4] = {};
	byte _instruments[8][6] = {};
	byte _textColor = 7, _movementMode = 1;
	byte _climbHeight = 0, _fallHeight = 0, _walkSpeed = 0, _activationRange = 0;
	byte _shotObject = 0, _hitObject = 0, _activatedObject = 0;
	bool _fallen = false, _crushed = false, _crossVisible = true;
	bool _timerTriggered = false, _pendingTimer = false, _soundWarning = false;
	uint32 _lastTime = 0, _timerTicks = 0, _timerInterval = 0, _delayUntil = 0;
	byte _fontData[96][8] = {};
	bool _hasFont = false;
	Graphics::ManagedSurface _scriptSurface, _overlaySurface, _borderSurface;
};

} // namespace Freescape

#endif
