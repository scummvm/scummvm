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

#include "sludge/builtin.h"
#include "sludge/function.h"
#include "sludge/loadsave.h"
#include "sludge/newfatal.h"
#include "sludge/people.h"
#include "sludge/sludge.h"
#include "sludge/sound.h"
#include "sludge/speech.h"

namespace Sludge {

int numBIFNames = 0;
Common::String *allBIFNames = NULL;
int numUserFunc = 0;
Common::String *allUserFunc = NULL;

LoadedFunction *saverFunc;
LoadedFunction *allRunningFunctions = NULL;
VariableStack *noStack = NULL;
Variable *globalVars = NULL;

const char *sludgeText[] = { "?????", "RETURN", "BRANCH", "BR_ZERO",
		"SET_GLOBAL", "SET_LOCAL", "LOAD_GLOBAL", "LOAD_LOCAL", "PLUS", "MINUS",
		"MULT", "DIVIDE", "AND", "OR", "EQUALS", "NOT_EQ", "MODULUS",
		"LOAD_VALUE", "LOAD_BUILT", "LOAD_FUNC", "CALLIT", "LOAD_STRING",
		"LOAD_FILE", "LOAD_OBJTYPE", "NOT", "LOAD_NULL", "STACK_PUSH",
		"LESSTHAN", "MORETHAN", "NEGATIVE", "U", "LESS_EQUAL", "MORE_EQUAL",
		"INC_LOCAL", "DEC_LOCAL", "INC_GLOBAL", "DEC_GLOBAL", "INDEXSET",
		"INDEXGET", "INC_INDEX", "DEC_INDEX", "QUICK_PUSH" };

void pauseFunction(LoadedFunction *fun) {
	LoadedFunction **huntAndDestroy = &allRunningFunctions;
	while (*huntAndDestroy) {
		if (fun == *huntAndDestroy) {
			(*huntAndDestroy) = (*huntAndDestroy)->next;
			fun->next = NULL;
		} else {
			huntAndDestroy = &(*huntAndDestroy)->next;
		}
	}
}

void restartFunction(LoadedFunction *fun) {
	fun->next = allRunningFunctions;
	allRunningFunctions = fun;
}

void killSpeechTimers() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel == 0 && thisFunction->isSpeech
				&& thisFunction->timeLeft) {
			thisFunction->timeLeft = 0;
			thisFunction->isSpeech = false;
		}
		thisFunction = thisFunction->next;
	}

	g_sludge->_speechMan->kill();
}

void completeTimers() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel == 0)
			thisFunction->timeLeft = 0;
		thisFunction = thisFunction->next;
	}
}

void finishFunction(LoadedFunction *fun) {
	int a;

	pauseFunction(fun);
	if (fun->stack)
		fatal(ERROR_NON_EMPTY_STACK);
	delete[] fun->compiledLines;
	for (a = 0; a < fun->numLocals; a++)
		fun->localVars[a].unlinkVar();
	delete[] fun->localVars;
	fun->reg.unlinkVar();
	delete fun;
	fun = NULL;
}

void abortFunction(LoadedFunction *fun) {
	int a;

	pauseFunction(fun);
	while (fun->stack)
		trimStack(fun->stack);
	delete []fun->compiledLines;
	for (a = 0; a < fun->numLocals; a++)
		fun->localVars[a].unlinkVar();
	delete []fun->localVars;
	fun->reg.unlinkVar();
	if (fun->calledBy)
		abortFunction(fun->calledBy);
	delete fun;
	fun = NULL;
}

int cancelAFunction(int funcNum, LoadedFunction *myself, bool &killedMyself) {
	int n = 0;
	killedMyself = false;

	LoadedFunction *fun = allRunningFunctions;
	while (fun) {
		if (fun->originalNumber == funcNum) {
			fun->cancelMe = true;
			n++;
			if (fun == myself)
				killedMyself = true;
		}
		fun = fun->next;
	}
	return n;
}

void freezeSubs() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->unfreezable) {
			//msgBox ("SLUDGE debugging bollocks!", "Trying to freeze an unfreezable function!");
		} else {
			thisFunction->freezerLevel++;
		}
		thisFunction = thisFunction->next;
	}
}

void unfreezeSubs() {
	LoadedFunction *thisFunction = allRunningFunctions;

	while (thisFunction) {
		if (thisFunction->freezerLevel)
			thisFunction->freezerLevel--;
		thisFunction = thisFunction->next;
	}
}

bool continueFunction(LoadedFunction *fun) {
	bool keepLooping = true;
	bool advanceNow;
	uint param;
	SludgeCommand com;

	if (fun->cancelMe) {
		abortFunction(fun);
		return true;
	}

	while (keepLooping) {
		advanceNow = true;
		debugC(1, kSludgeDebugStackMachine, "Executing command line %i : ", fun->runThisLine);
		param = fun->compiledLines[fun->runThisLine].param;
		com = fun->compiledLines[fun->runThisLine].theCommand;

		if (numBIFNames) {
			setFatalInfo((fun->originalNumber < numUserFunc) ? allUserFunc[fun->originalNumber] : "Unknown user function", (com < numSludgeCommands) ? sludgeText[com] : ERROR_UNKNOWN_MCODE);
		}

		switch (com) {
		case SLU_RETURN:
			if (fun->calledBy) {
				LoadedFunction *returnTo = fun->calledBy;
				if (fun->returnSomething)
					returnTo->reg.copyFrom(fun->reg);
				finishFunction(fun);
				fun = returnTo;
				restartFunction(fun);
			} else {
				finishFunction(fun);
				advanceNow = false;   // So we don't do anything else with "fun"
				keepLooping = false;    // So we drop out of the loop
			}
			break;

		case SLU_CALLIT:
			switch (fun->reg.varType) {
			case SVT_FUNC:
				pauseFunction(fun);
				if (numBIFNames)
					setFatalInfo(
							(fun->originalNumber < numUserFunc) ?
									allUserFunc[fun->originalNumber] :
									"Unknown user function",
							(fun->reg.varData.intValue < numUserFunc) ?
									allUserFunc[fun->reg.varData.intValue] :
									"Unknown user function");

				if (!startNewFunctionNum(fun->reg.varData.intValue, param, fun,
						fun->stack))
					return false;
				fun = allRunningFunctions;
				advanceNow = false;   // So we don't do anything else with "fun"
				break;

			case SVT_BUILT: {
				debugC(1, kSludgeDebugStackMachine, "Built-in init value: %i",
						fun->reg.varData.intValue);
				BuiltReturn br = callBuiltIn(fun->reg.varData.intValue, param,
						fun);

				switch (br) {
				case BR_ERROR:
					return fatal(
							"Unknown error. This shouldn't happen. Please notify the SLUDGE developers.");

				case BR_PAUSE:
					pauseFunction(fun);
					// fall through

				case BR_KEEP_AND_PAUSE:
					keepLooping = false;
					break;

				case BR_ALREADY_GONE:
					keepLooping = false;
					advanceNow = false;
					break;

				case BR_CALLAFUNC: {
					int i = fun->reg.varData.intValue;
					fun->reg.setVariable(SVT_INT, 1);
					pauseFunction(fun);
					if (numBIFNames)
						setFatalInfo(
								(fun->originalNumber < numUserFunc) ?
										allUserFunc[fun->originalNumber] :
										"Unknown user function",
								(i < numUserFunc) ?
										allUserFunc[i] :
										"Unknown user function");
					if (!startNewFunctionNum(i, 0, fun, noStack, false))
						return false;
					fun = allRunningFunctions;
					advanceNow = false; // So we don't do anything else with "fun"
				}
					break;

				default:
					break;
				}
			}
				break;

			default:
				return fatal(ERROR_CALL_NONFUNCTION);
			}
			break;

			// These all grab things and shove 'em into the register

		case SLU_LOAD_NULL:
			fun->reg.setVariable(SVT_NULL, 0);
			break;

		case SLU_LOAD_FILE:
			fun->reg.setVariable(SVT_FILE, param);
			break;

		case SLU_LOAD_VALUE:
			fun->reg.setVariable(SVT_INT, param);
			break;

		case SLU_LOAD_LOCAL:
			if (!fun->reg.copyFrom(fun->localVars[param]))
				return false;
			break;

		case SLU_AND:
			fun->reg.setVariable(SVT_INT,
					fun->reg.getBoolean() && fun->stack->thisVar.getBoolean());
			trimStack(fun->stack);
			break;

		case SLU_OR:
			fun->reg.setVariable(SVT_INT,
					fun->reg.getBoolean() || fun->stack->thisVar.getBoolean());
			trimStack(fun->stack);
			break;

		case SLU_LOAD_FUNC:
			fun->reg.setVariable(SVT_FUNC, param);
			break;

		case SLU_LOAD_BUILT:
			fun->reg.setVariable(SVT_BUILT, param);
			break;

		case SLU_LOAD_OBJTYPE:
			fun->reg.setVariable(SVT_OBJTYPE, param);
			break;

		case SLU_UNREG:
			break;

		case SLU_LOAD_STRING:
			if (!fun->reg.loadStringToVar(param)) {
				return false;
			}
			break;

		case SLU_INDEXGET:
		case SLU_INCREMENT_INDEX:
		case SLU_DECREMENT_INDEX:
			switch (fun->stack->thisVar.varType) {
			case SVT_NULL:
				if (com == SLU_INDEXGET) {
					fun->reg.setVariable(SVT_NULL, 0);
					trimStack(fun->stack);
				} else {
					return fatal(ERROR_INCDEC_UNKNOWN);
				}
				break;

			case SVT_FASTARRAY:
			case SVT_STACK:
				if (fun->stack->thisVar.varData.theStack->first == NULL) {
					return fatal(ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (!fun->reg.getValueType(ii, SVT_INT))
						return false;
					Variable *grab =
							(fun->stack->thisVar.varType == SVT_FASTARRAY) ?
									fun->stack->thisVar.varData.fastArray->fastArrayGetByIndex(ii) :
									fun->stack->thisVar.varData.theStack->first->stackGetByIndex(ii);

					trimStack(fun->stack);

					if (!grab) {
						fun->reg.setVariable(SVT_NULL, 0);
					} else {
						int kk;
						switch (com) {
						case SLU_INCREMENT_INDEX:
							if (!grab->getValueType(kk, SVT_INT))
								return false;
							fun->reg.setVariable(SVT_INT, kk);
							grab->varData.intValue = kk + 1;
							break;

						case SLU_DECREMENT_INDEX:
							if (!grab->getValueType(kk, SVT_INT))
								return false;
							fun->reg.setVariable(SVT_INT, kk);
							grab->varData.intValue = kk - 1;
							break;

						default:
							if (!fun->reg.copyFrom(*grab))
								return false;
						}
					}
				}
				break;

			default:
				return fatal(ERROR_INDEX_NONSTACK);
			}
			break;

		case SLU_INDEXSET:
			switch (fun->stack->thisVar.varType) {
			case SVT_STACK:
				if (fun->stack->thisVar.varData.theStack->first == NULL) {
					return fatal(ERROR_INDEX_EMPTY);
				} else {
					int ii;
					if (!fun->reg.getValueType(ii, SVT_INT))
						return false;
					if (!fun->stack->thisVar.varData.theStack->first->stackSetByIndex(ii, fun->stack->next->thisVar)) {
						return false;
					}
					trimStack(fun->stack);
					trimStack(fun->stack);
				}
				break;

			case SVT_FASTARRAY: {
				int ii;
				if (!fun->reg.getValueType(ii, SVT_INT))
					return false;
				Variable *v = fun->stack->thisVar.varData.fastArray->fastArrayGetByIndex(ii);
				if (v == NULL)
					return fatal("Not within bounds of fast array.");
				if (!v->copyFrom(fun->stack->next->thisVar))
					return false;
				trimStack(fun->stack);
				trimStack(fun->stack);
			}
				break;

			default:
				return fatal(ERROR_INDEX_NONSTACK);
			}
			break;

			// What can we do with the register? Well, we can copy it into a local
			// variable, a global or onto the stack...

		case SLU_INCREMENT_LOCAL: {
			int ii;
			if (!fun->localVars[param].getValueType(ii, SVT_INT))
				return false;
			fun->reg.setVariable(SVT_INT, ii);
			fun->localVars[param].setVariable(SVT_INT, ii + 1);
		}
			break;

		case SLU_INCREMENT_GLOBAL: {
			int ii;
			if (!globalVars[param].getValueType(ii, SVT_INT))
				return false;
			fun->reg.setVariable(SVT_INT, ii);
			globalVars[param].setVariable(SVT_INT, ii + 1);
		}
			break;

		case SLU_DECREMENT_LOCAL: {
			int ii;
			if (!fun->localVars[param].getValueType(ii, SVT_INT))
				return false;
			fun->reg.setVariable(SVT_INT, ii);
			fun->localVars[param].setVariable(SVT_INT, ii - 1);
		}
			break;

		case SLU_DECREMENT_GLOBAL: {
			int ii;
			if (!globalVars[param].getValueType(ii, SVT_INT))
				return false;
			fun->reg.setVariable(SVT_INT, ii);
			globalVars[param].setVariable(SVT_INT, ii - 1);
		}
			break;

		case SLU_SET_LOCAL:
			if (!fun->localVars[param].copyFrom(fun->reg))
				return false;
			break;

		case SLU_SET_GLOBAL:
			if (!globalVars[param].copyFrom(fun->reg))
				return false;
			break;

		case SLU_LOAD_GLOBAL:
			if (!fun->reg.copyFrom(globalVars[param]))
				return false;
			break;

		case SLU_STACK_PUSH:
			if (!addVarToStack(fun->reg, fun->stack))
				return false;
			break;

		case SLU_QUICK_PUSH:
			if (!addVarToStackQuick(fun->reg, fun->stack))
				return false;
			break;

		case SLU_NOT:
			fun->reg.setVariable(SVT_INT, !fun->reg.getBoolean());
			break;

		case SLU_BR_ZERO:
			if (!fun->reg.getBoolean()) {
				advanceNow = false;
				fun->runThisLine = param;
			}
			break;

		case SLU_BRANCH:
			advanceNow = false;
			fun->runThisLine = param;
			break;

		case SLU_NEGATIVE: {
			int i;
			if (!fun->reg.getValueType(i, SVT_INT))
				return false;
			fun->reg.setVariable(SVT_INT, -i);
		}
			break;

			// All these things rely on there being somet' on the stack

		case SLU_MULT:
		case SLU_PLUS:
		case SLU_MINUS:
		case SLU_MODULUS:
		case SLU_DIVIDE:
		case SLU_EQUALS:
		case SLU_NOT_EQ:
		case SLU_LESSTHAN:
		case SLU_MORETHAN:
		case SLU_LESS_EQUAL:
		case SLU_MORE_EQUAL:
			if (fun->stack) {
				int firstValue, secondValue;

				switch (com) {
				case SLU_PLUS:
					fun->reg.addVariablesInSecond(fun->stack->thisVar);
					trimStack(fun->stack);
					break;

				case SLU_EQUALS:
					fun->reg.compareVariablesInSecond(fun->stack->thisVar);
					trimStack(fun->stack);
					break;

				case SLU_NOT_EQ:
					fun->reg.compareVariablesInSecond(fun->stack->thisVar);
					trimStack(fun->stack);
					fun->reg.varData.intValue = !fun->reg.varData.intValue;
					break;

				default:
					if (!fun->stack->thisVar.getValueType(firstValue, SVT_INT))
						return false;
					if (!fun->reg.getValueType(secondValue, SVT_INT))
						return false;
					trimStack(fun->stack);

					switch (com) {
					case SLU_MULT:
						fun->reg.setVariable(SVT_INT,
								firstValue * secondValue);
						break;

					case SLU_MINUS:
						fun->reg.setVariable(SVT_INT,
								firstValue - secondValue);
						break;

					case SLU_MODULUS:
						fun->reg.setVariable(SVT_INT,
								firstValue % secondValue);
						break;

					case SLU_DIVIDE:
						fun->reg.setVariable(SVT_INT,
								firstValue / secondValue);
						break;

					case SLU_LESSTHAN:
						fun->reg.setVariable(SVT_INT,
								firstValue < secondValue);
						break;

					case SLU_MORETHAN:
						fun->reg.setVariable(SVT_INT,
								firstValue > secondValue);
						break;

					case SLU_LESS_EQUAL:
						fun->reg.setVariable(SVT_INT,
								firstValue <= secondValue);
						break;

					case SLU_MORE_EQUAL:
						fun->reg.setVariable(SVT_INT,
								firstValue >= secondValue);
						break;

					default:
						break;
					}
				}
			} else {
				return fatal(ERROR_NOSTACK);
			}
			break;

		default:
			return fatal(ERROR_UNKNOWN_CODE);
		}

		if (advanceNow)
			fun->runThisLine++;

	}
	return true;
}

void killAllFunctions() {
	while (allRunningFunctions)
		finishFunction(allRunningFunctions);
}

bool loadFunctionCode(LoadedFunction *newFunc) {
	uint numLines, numLinesRead;

	if (!g_sludge->_resMan->openSubSlice(newFunc->originalNumber))
		return false;

	debugC(3, kSludgeDebugDataLoad, "Load function code");

	Common::SeekableReadStream *readStream = g_sludge->_resMan->getData();
	newFunc->unfreezable = readStream->readByte();
	numLines = readStream->readUint16BE();
	debugC(3, kSludgeDebugDataLoad, "numLines: %i", numLines);
	newFunc->numArgs = readStream->readUint16BE();
	debugC(3, kSludgeDebugDataLoad, "numArgs: %i", newFunc->numArgs);
	newFunc->numLocals = readStream->readUint16BE();
	debugC(3, kSludgeDebugDataLoad, "numLocals: %i", newFunc->numLocals);
	newFunc->compiledLines = new LineOfCode[numLines];
	if (!checkNew(newFunc->compiledLines))
		return false;

	for (numLinesRead = 0; numLinesRead < numLines; numLinesRead++) {
		newFunc->compiledLines[numLinesRead].theCommand = (SludgeCommand)readStream->readByte();
		newFunc->compiledLines[numLinesRead].param = readStream->readUint16BE();
		debugC(3, kSludgeDebugDataLoad, "command line %i: %i", numLinesRead,
				newFunc->compiledLines[numLinesRead].theCommand);
	}
	g_sludge->_resMan->finishAccess();

	// Now we need to reserve memory for the local variables
	newFunc->localVars = new Variable[newFunc->numLocals];
	if (!checkNew(newFunc->localVars))
		return false;

	return true;
}

int startNewFunctionNum(uint funcNum, uint numParamsExpected,
		LoadedFunction *calledBy, VariableStack *&vStack, bool returnSommet) {
	LoadedFunction *newFunc = new LoadedFunction;
	checkNew(newFunc);
	newFunc->originalNumber = funcNum;

	loadFunctionCode(newFunc);

	if (newFunc->numArgs != (int)numParamsExpected)
		return fatal("Wrong number of parameters!");
	if (newFunc->numArgs > newFunc->numLocals)
		return fatal("More arguments than local Variable space!");

	// Now, lets copy the parameters from the calling function's stack...

	while (numParamsExpected) {
		numParamsExpected--;
		if (vStack == NULL)
			return fatal(
					"Corrupted file!The stack's empty and there were still parameters expected");
		newFunc->localVars[numParamsExpected].copyFrom(vStack->thisVar);
		trimStack(vStack);
	}

	newFunc->cancelMe = false;
	newFunc->timeLeft = 0;
	newFunc->returnSomething = returnSommet;
	newFunc->calledBy = calledBy;
	newFunc->stack = NULL;
	newFunc->freezerLevel = 0;
	newFunc->runThisLine = 0;
	newFunc->isSpeech = 0;

	restartFunction(newFunc);
	return 1;
}

bool runAllFunctions() {

	LoadedFunction *thisFunction = allRunningFunctions;
	LoadedFunction *nextFunction;

	while (thisFunction) {
		nextFunction = thisFunction->next;

		if (!thisFunction->freezerLevel) {
			if (thisFunction->timeLeft) {
				if (thisFunction->timeLeft < 0) {
					if (!g_sludge->_soundMan->stillPlayingSound(
							g_sludge->_speechMan->getLastSpeechSound())) {
						thisFunction->timeLeft = 0;
					}
				} else if (!--(thisFunction->timeLeft)) {
				}
			} else {
				if (thisFunction->isSpeech) {
					thisFunction->isSpeech = false;
					g_sludge->_speechMan->kill();
				}
				if (!continueFunction(thisFunction))
					return false;
			}
		}

		thisFunction = nextFunction;
	}

	return true;
}

void saveFunction(LoadedFunction *fun, Common::WriteStream *stream) {
	int a;
	stream->writeUint16BE(fun->originalNumber);
	if (fun->calledBy) {
		stream->writeByte(1);
		saveFunction(fun->calledBy, stream);
	} else {
		stream->writeByte(0);
	}
	stream->writeUint32LE(fun->timeLeft);
	stream->writeUint16BE(fun->runThisLine);
	stream->writeByte(fun->cancelMe);
	stream->writeByte(fun->returnSomething);
	stream->writeByte(fun->isSpeech);
	fun->reg.save(stream);

	if (fun->freezerLevel) {
		fatal(ERROR_GAME_SAVE_FROZEN);
	}
	saveStack(fun->stack, stream);
	for (a = 0; a < fun->numLocals; a++) {
		fun->localVars[a].save(stream);
	}
}

LoadedFunction *loadFunction(Common::SeekableReadStream *stream) {
	int a;

	// Reserve memory...

	LoadedFunction *buildFunc = new LoadedFunction;
	if (!checkNew(buildFunc))
		return NULL;

	// See what it was called by and load if we need to...

	buildFunc->originalNumber = stream->readUint16BE();
	buildFunc->calledBy = NULL;
	if (stream->readByte()) {
		buildFunc->calledBy = loadFunction(stream);
		if (!buildFunc->calledBy) {
			delete buildFunc;

			return NULL;
		}
	}

	buildFunc->timeLeft = stream->readUint32LE();
	buildFunc->runThisLine = stream->readUint16BE();
	buildFunc->freezerLevel = 0;
	buildFunc->cancelMe = stream->readByte();
	buildFunc->returnSomething = stream->readByte();
	buildFunc->isSpeech = stream->readByte();
	buildFunc->reg.load(stream);
	loadFunctionCode(buildFunc);

	buildFunc->stack = loadStack(stream, NULL);

	for (a = 0; a < buildFunc->numLocals; a++) {
		buildFunc->localVars[a].load(stream);
	}

	return buildFunc;
}

} // End of namespace Sludge
