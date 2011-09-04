/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/myst3/myst3.h"
#include "engines/myst3/script.h"
#include "engines/myst3/hotspot.h"
#include "engines/myst3/variables.h"

namespace Myst3 {

Script::Script(Myst3Engine *vm):
	_vm(vm) {
#define OPCODE(op, x) _commands.push_back(Command(op, &Script::x, #x))

	OPCODE(4, nodeCubeInit);
	OPCODE(11, stopWholeScript);
	OPCODE(35, sunspotAdd);
	OPCODE(49, varSetZero);
	OPCODE(50, varSetOne);
	OPCODE(51, varSetTwo);
	OPCODE(52, varSetOneHundred);
	OPCODE(53, varSetValue);
	OPCODE(85, varIncrementMaxTen);
	OPCODE(86, varAddValue);
	OPCODE(87, varArrayAddValue);
	OPCODE(88, varAddVarValue);
	OPCODE(89, varSubValue);
	OPCODE(90, varSubVarValue);
	OPCODE(91, varModValue);
	OPCODE(92, varMultValue);
	OPCODE(93, varMultVarValue);
	OPCODE(94, varDivValue);
	OPCODE(95, varDivVarValue);
	OPCODE(97, varMinValue);
	OPCODE(98, varClipValue);
	OPCODE(103, varRotateValue3);
	OPCODE(104, continueToNextScript);
	OPCODE(105, ifCondition);
	OPCODE(106, ifCond1AndCond2);
	OPCODE(107, ifCond1OrCond2);
	OPCODE(108, ifOneVarSetInRange);
	OPCODE(109, ifVarEqualsValue);
	OPCODE(110, ifVarNotEqualsValue);
	OPCODE(111, ifVar1EqualsVar2);
	OPCODE(112, ifVar1NotEqualsVar2);
	OPCODE(113, ifVarSupEqValue);
	OPCODE(114, ifVarInfEqValue);
	OPCODE(115, ifVarInRange);
	OPCODE(116, ifVarNotInRange);
	OPCODE(117, ifVar1SupEqVar2);
	OPCODE(118, ifVar1SupVar2);
	OPCODE(119, ifVar1InfEqVar2);
	OPCODE(120, ifVarHasAllBitsSet);
	OPCODE(121, ifVarHasNoBitsSet);
	OPCODE(122, ifVarHasSomeBitsSet);
	OPCODE(138, goToNode);
	OPCODE(139, goToRoomNode);
	OPCODE(187, runScriptsFromNode);

#undef OPCODE
}

Script::~Script() {
}

bool Script::run(const Common::Array<Opcode> *script) {
	debugC(kDebugScript, "Script start %p", (void *) script);

	Context c;
	c.result = true;
	c.endScript = false;
	c.script = script;
	c.op = script->begin();

	while (c.op != script->end()) {
		runOp(c, *c.op);

		if (c.endScript || c.op == script->end())
			break;

		c.op++;
	}

	debugC(kDebugScript, "Script stop %p ", (void *) script);

	return c.result;
}

void Script::runOp(Context &c, const Opcode &op) {
	bool ranOpcode = false;

	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].op == op.op) {
			(this->*(_commands[i].proc)) (c, op);
			ranOpcode = true;
			break;
		}

	if (!ranOpcode)
		warning("Trying to run invalid opcode %d", op.op);
}

const Common::String Script::describeCommand(uint16 op) {
	for (uint16 i = 0; i < _commands.size(); i++)
		if (_commands[i].op == op)
			return Common::String::format("%d, %s", op, _commands[i].desc);

	return Common::String::format("%d", op);
}

void Script::nodeCubeInit(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Node cube init %d", cmd.op, cmd.args[0]);

	uint16 nodeId = _vm->_vars->valueOrVarValue(cmd.args[0]);
	_vm->loadNodeCubeFaces(nodeId);
	// TODO: Load rects
}

void Script::stopWholeScript(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Stop whole script", cmd.op);

	c.result = false;
	c.endScript = true;
}

void Script::sunspotAdd(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add sunspot: pitch %d heading %d", cmd.op, cmd.args[0], cmd.args[1]);

	warning("Unimplemented opcode %d", cmd.op);
}

void Script::varSetZero(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 0", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 0);
}

void Script::varSetOne(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 1", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 1);
}

void Script::varSetTwo(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 2", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 2);
}

void Script::varSetOneHundred(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := 100", cmd.op, cmd.args[0]);

	_vm->_vars->set(cmd.args[0], 100);
}

void Script::varSetValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var value %d := %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->_vars->set(cmd.args[0], cmd.args[1]);
}

void Script::varIncrementMaxTen(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Increment var %d max 10", cmd.op, cmd.args[0]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	value++;

	if (value == 10)
		value = 1;

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varAddValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[1]);
	value += cmd.args[0];
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varArrayAddValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add value %d to array base var %d item var %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint32 value = _vm->_vars->get(cmd.args[1] + _vm->_vars->get(cmd.args[2]));
	value += cmd.args[0];
	_vm->_vars->set(cmd.args[1] + _vm->_vars->get(cmd.args[2]), value);
}

void Script::varAddVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Add var %d value to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[1]);
	value += _vm->_vars->get(cmd.args[0]);
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varSubValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract value %d to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[1]);
	value -= cmd.args[0];
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varSubVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Substract var %d value to var %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[1]);
	value -= _vm->_vars->get(cmd.args[0]);
	_vm->_vars->set(cmd.args[1], value);
}

void Script::varModValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Apply modulo %d to var %d", cmd.op, cmd.args[1], cmd.args[0]);

	uint32 value = _vm->_vars->get(cmd.args[0]);
	value %= cmd.args[1];
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varMultValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Multiply var %d by value %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[0]);
	value *= cmd.args[1];
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varMultVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Multiply var %d by var %d value", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[0]);
	value *= _vm->_vars->get(cmd.args[1]);
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varDivValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Divide var %d by value %d", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[0]);
	value /= cmd.args[1];
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varDivVarValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Divide var %d by var %d value", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[0]);
	value /= _vm->_vars->get(cmd.args[1]);
	_vm->_vars->set(cmd.args[0], value);
}

void Script::varMinValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Set var %d to min between %d and var value", cmd.op, cmd.args[0], cmd.args[1]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	if (value > cmd.args[1])
		value = cmd.args[1];

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varClipValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Clip var %d value between %d and %d", cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	value = CLIP<uint32>(value, cmd.args[1], cmd.args[2]);

	_vm->_vars->set(cmd.args[0], value);
}

void Script::varRotateValue3(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Var take next value, var %d values %d %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2], cmd.args[3]);

	uint32 value = _vm->_vars->get(cmd.args[0]);

	if (value == cmd.args[1]) {
		value = cmd.args[2];
	} else if (value == cmd.args[2]) {
		value = cmd.args[3];
	} else {
		value = cmd.args[1];
	}

	_vm->_vars->set(cmd.args[0], value);
}

void Script::continueToNextScript(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Return OK", cmd.op);

	c.result = true;
	c.endScript = true;
}

void Script::ifCondition(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If condition %d", cmd.op, cmd.args[0]);

	if (_vm->_vars->evaluate(cmd.args[0]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifCond1AndCond2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If cond %d and cond %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->evaluate(cmd.args[0])
			&& _vm->_vars->evaluate(cmd.args[1]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifCond1OrCond2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If cond %d or cond %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->evaluate(cmd.args[0])
			|| _vm->_vars->evaluate(cmd.args[1]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifOneVarSetInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If one var set int range %d %d",
			cmd.op, cmd.args[0], cmd.args[1]);

	uint16 var = cmd.args[0];
	uint16 end = cmd.args[1];

	if (end > var) {
		c.result = true;
		c.endScript = true;
		return;
	}

	bool result = false;

	do {
		result |= _vm->_vars->get(var);
		var++;
	} while (var <= end);

	if (result)
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarEqualsValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d equals value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) == cmd.args[1])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarNotEqualsValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not equals value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) != cmd.args[1])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVar1EqualsVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d equals var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) == _vm->_vars->get(cmd.args[1]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVar1NotEqualsVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not equals var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) != _vm->_vars->get(cmd.args[1]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarSupEqValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d >= value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) >= cmd.args[1])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarInfEqValue(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d <= value %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) <= cmd.args[1])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d in range %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

    uint32 value = _vm->_vars->get(cmd.args[0]);
    if(value >= cmd.args[1] && value <= cmd.args[2])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarNotInRange(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d not in range %d %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

    uint32 value = _vm->_vars->get(cmd.args[0]);
    if(value < cmd.args[1] && value > cmd.args[2])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVar1SupEqVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d >= var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) >= _vm->_vars->get(cmd.args[1]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVar1SupVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d > var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) > _vm->_vars->get(cmd.args[1]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVar1InfEqVar2(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d <= var %d", cmd.op, cmd.args[0], cmd.args[1]);

	if (_vm->_vars->get(cmd.args[0]) <= _vm->_vars->get(cmd.args[1]))
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarHasAllBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[1]);

	if ((_vm->_vars->get(cmd.args[0]) & cmd.args[1]) == cmd.args[1])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarHasNoBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == 0",
			cmd.op, cmd.args[0], cmd.args[1]);

	if ((_vm->_vars->get(cmd.args[0]) & cmd.args[1]) == 0)
		return;

	c.result = true;
	c.endScript = true;
}

void Script::ifVarHasSomeBitsSet(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: If var %d & val %d == val %d",
			cmd.op, cmd.args[0], cmd.args[1], cmd.args[2]);

	if ((_vm->_vars->get(cmd.args[0]) & cmd.args[1]) == cmd.args[2])
		return;

	c.result = true;
	c.endScript = true;
}

void Script::goToNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to node %d", cmd.op, cmd.args[0]);

	_vm->goToNode(cmd.args[0]);
}

void Script::goToRoomNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Go to room %d, node %d", cmd.op, cmd.args[0], cmd.args[1]);

	_vm->goToNode(cmd.args[1], cmd.args[0]);
}

void Script::runScriptsFromNode(Context &c, const Opcode &cmd) {
	debugC(kDebugScript, "Opcode %d: Run scripts from node %d", cmd.op, cmd.args[0]);

	_vm->runScriptsFromNode(cmd.args[0]);
}

} /* namespace Myst3 */
