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

#include "mediastation/mediastation.h"
#include "mediastation/mediascript/codechunk.h"
#include "mediastation/datum.h"
#include "mediastation/chunk.h"
#include "mediastation/debugchannels.h"

#include "mediastation/assets/movie.h"
#include "mediastation/assets/path.h"

namespace MediaStation {

CodeChunk::CodeChunk(Common::SeekableReadStream &chunk) : _args(nullptr) {
	uint lengthInBytes = Datum(chunk, DatumType::UINT32_1).u.i;
	debugC(5, kDebugLoading, "CodeChunk::CodeChunk(): Length 0x%x (@0x%llx)", lengthInBytes, static_cast<long long int>(chunk.pos()));
	_bytecode = chunk.readStream(lengthInBytes);
}

Operand CodeChunk::execute(Common::Array<Operand> *args) {
	_args = args;
	Operand returnValue;
	while (_bytecode->pos() < _bytecode->size()) {
		debugC(8, kDebugScript, "-------- Statement --------");
		returnValue = executeNextStatement();
	}

	// Rewind the stream once we're finished, in case we need to execute
	// this code again!
	_bytecode->seek(0);
	// We don't own the args, so we will prevent a potentially out-of-scope
	// variable from being re-accessed.
	_args = nullptr;
	return returnValue;
}

Operand CodeChunk::executeNextStatement() {
	if (_bytecode->eos()) {
		error("CodeChunk::executeNextStatement(): Attempt to read past end of bytecode chunk");
	}

	InstructionType instructionType = InstructionType(Datum(*_bytecode).u.i);
	debugC(8, kDebugScript, " instructionType = %d", (uint)instructionType);
	switch (instructionType) {
	case InstructionType::EMPTY: {
		return Operand();
	}

	case InstructionType::FUNCTION_CALL: {
		Opcode opcode = Opcode(Datum(*_bytecode).u.i);
		debugC(8, kDebugScript, "  *** Opcode %d ***", (uint)opcode);
		switch (opcode) {
		case Opcode::AssignVariable: {
			uint32 id = Datum(*_bytecode).u.i;
			VariableScope scope = VariableScope(Datum(*_bytecode).u.i);
			Operand newValue = executeNextStatement();
			// TODO: Print the new variable value for easier debugging.
			debugC(5, kDebugScript, "SCRIPT: [ %d (scope: %d) ] = [ ? (showing value assigned to var not implemented yet) ]", (uint)scope, id);
			putVariable(id, scope, newValue);
			return Operand();
		}

		case Opcode::CallRoutine: {
			uint functionId = Datum(*_bytecode).u.i;
			uint32 parameterCount = Datum(*_bytecode).u.i;
			Common::Array<Operand> args;
			for (uint i = 0; i < parameterCount; i++) {
				debugC(8, kDebugScript, "   -- Argument %d of %d --", (i + 1), parameterCount);
				Operand arg = executeNextStatement();
				args.push_back(arg);
			}

			// Call the routine.
			debugC(5, kDebugScript, "SCRIPT: [ %d ]( %d args )", functionId, parameterCount);
			Operand returnValue;
			Function *function = g_engine->_functions.getValOrDefault(functionId);
			if (function != nullptr) {
				returnValue = function->execute(args);
			} else {
				returnValue = callBuiltInFunction(functionId, args);
			}
			return returnValue;
		}

		case Opcode::CallMethod: {
			uint32 methodId = Datum(*_bytecode).u.i;
			uint32 parameterCount = Datum(*_bytecode).u.i;
			Operand selfObject = executeNextStatement();
			if (selfObject.getType() != Operand::Type::AssetId) {
				error("CodeChunk::executeNextStatement(): (Opcode::CallMethod) Attempt to call method on operand that is not an asset (type 0x%x)", selfObject.getType());
			}
			Common::Array<Operand> args;
			for (uint i = 0; i < parameterCount; i++) {
				debugC(8, kDebugScript, "   -- Argument %d of %d --", (i + 1), parameterCount);
				Operand arg = executeNextStatement();
				args.push_back(arg);
			}

			// Call the method.
			// TODO: Resolve asset IDs to names in this decompilation so
			// itʻe easier to read.
			debugC(5, kDebugScript, "SCRIPT: @[ %d ].[ %d ]()", selfObject.getAssetId(), methodId);
			// TODO: Where do we get the method from? And can we define
			// our own methods? Or are only the built-in methods
			// supported?
			Operand returnValue = callBuiltInMethod(methodId, selfObject, args);
			return returnValue;
		}

		case Opcode::DeclareVariables: {
			uint32 localVariableCount = Datum(*_bytecode).u.i;
			debugC(5, kDebugScript, "   Declaring %d local variables", localVariableCount);
			_locals.resize(localVariableCount);
			return Operand();
		}

		case Opcode::Subtract: {
			Operand value1 = executeNextStatement();
			Operand value2 = executeNextStatement();

			Operand returnValue = value1 - value2;
			return returnValue;
		}

		default: {
			error("CodeChunk::getNextStatement(): Got unknown opcode 0x%x (%d)", opcode, opcode);
		}
		}
		break;
	}

	case (InstructionType::OPERAND): {
		Operand::Type operandType = Operand::Type(Datum(*_bytecode).u.i);
		debugC(8, kDebugScript, "  *** Operand %d ***", (uint)operandType);
		Operand operand(operandType);
		switch (operandType) {
		// TODO: Add clearer debugging printouts for these.
		case Operand::Type::AssetId: {
			uint32 assetId = Datum(*_bytecode).u.i;
			operand.putAsset(assetId);
			return operand;
		}

		case Operand::Type::Literal1:
		case Operand::Type::Literal2:
		case Operand::Type::DollarSignVariable: {
			int literal = Datum(*_bytecode).u.i;
			operand.putInteger(literal);
			return operand;
		}

		case Operand::Type::Float1:
		case Operand::Type::Float2: {
			double d = Datum(*_bytecode).u.f;
			operand.putDouble(d);
			return operand;
		}

		default: {
			error("CodeChunk::getNextStatement(): Got unknown operand type 0x%d", operandType);
		}
		}
		break;
	}

	case (InstructionType::VARIABLE_REF): {
		// TODO: Add debug printout for this.
		uint32 id = Datum(*_bytecode).u.i;
		VariableScope scope = VariableScope(Datum(*_bytecode).u.i);
		Operand variable = getVariable(id, scope);
		return variable;
	}

	default: {
		error("CodeChunk::getNextStatement(): Got unknown instruction type 0x%x", instructionType);
	}
	}
}

Operand CodeChunk::getVariable(uint32 id, VariableScope scope) {
	switch (scope) {
	case VariableScope::Global: {
		Operand returnValue(Operand::Type::VariableDeclaration);
		Variable *variable = g_engine->_variables.getVal(id);
		returnValue.putVariable(variable);
		return returnValue;
	}

	case VariableScope::Local: {
		uint index = id - 1;
		return _locals.operator[](index);
		break;
	}

	case VariableScope::Parameter: {
		uint32 index = id - 1;
		if (_args == nullptr) {
			error("CodeChunk::getVariable(): Requested a parameter in a code chunk that has no parameters.");
		}
		return _args->operator[](index);
		break;
	}

	default: {
		error("CodeChunk::getVariable(): Got unknown variable scope %d", (uint)scope);
	}
	}
}

void CodeChunk::putVariable(uint32 id, VariableScope scope, Operand value) {
	switch (scope) {
	case VariableScope::Global: {
		Variable *variable = g_engine->_variables.getVal(id);
		if (variable == nullptr) {
			error("CodeChunk::putVariable(): Attempted to assign to a non-existent global variable %d", id);
		}

		switch (value.getType()) {
		case Operand::Type::Literal1:
		case Operand::Type::Literal2: {
			variable->value.i = value.getInteger();
			break;
		}

		case Operand::Type::Float1:
		case Operand::Type::Float2: {
			variable->value.d = value.getDouble();
			break;
		}

		case Operand::Type::String: {
			variable->value.string = value.getString();
			break;
		}

		case Operand::Type::AssetId: {
			variable->value.assetId = value.getAssetId();
			break;
		}

		case Operand::Type::VariableDeclaration: {
			// TODO: Will this cause a memory leak?
			// variable = value.u.variable;
			error("Assigning variable to another variable not supported yet");
			break;
		}

		default: {
			error("CodeChunk::putVariable(): Cannot put operand type 0x%x into variable", (uint)value.getType());
		}
		}
		break;
	}

	case VariableScope::Local: {
		uint index = id - 1;
		_locals[index] = value;
		break;
	}

	case VariableScope::Parameter: {
		error("CodeChunk::putVariable(): Attempted to assign to a parameter");
		break;
	}

	default: {
		error("VariableAssignment::evaluate(): Got unknown variable scope 0x%x", (uint)scope);
	}
	}
}

Operand CodeChunk::callBuiltInFunction(uint32 id, Common::Array<Operand> &args) {
	switch ((BuiltInFunction)id) {
	case BuiltInFunction::effectTransition: {
		switch (args.size()) {
		// TODO: Discover and handle the different ways
		// effectTransition can be called.
		case 1: {
			//uint dollarSignVariable = args[0].getInteger();
			break;
		}

		case 3: {
			//uint dollarSignVariable = args[0].getInteger();
			//double percentComplete = args[1].getDouble();

			// TODO: Verify that this is a palette!
			Asset *asset = args[2].getAsset();
			g_engine->setPalette(asset);
			break;
		}

		default: {
			error("CodeChunk::callBuiltInFunction(): (BuiltInFunction::effectTransition) Got %d args, which is unexpected", args.size());
		}
		}

		warning("CodeChunk::callBuiltInFunction(): effectTransition is not implemented");
		return Operand();
		break;
	}

	default: {
		error("CodeChunk::callBuiltInFunction(): Got unknown built-in function ID %d", id);
	}
	}
}

Operand CodeChunk::callBuiltInMethod(uint32 id, Operand self, Common::Array<Operand> &args) {
	Asset *selfAsset = self.getAsset();
	assert(selfAsset != nullptr);
	Operand returnValue = selfAsset->callMethod((BuiltInMethod)id, args);
	return returnValue;
}

CodeChunk::~CodeChunk() {
	// We don't own the args, so we don't need to delete it.
	_args = nullptr;

	delete _bytecode;
	_bytecode = nullptr;
}

} // End of namespace MediaStation
