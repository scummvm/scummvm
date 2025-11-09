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

#include "mediastation/mediascript/function.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {
ScriptFunction::ScriptFunction(Chunk &chunk) {
	_contextId = chunk.readTypedUint16();
	// In PROFILE._ST (only present in some titles), the function ID is reported
	// with 19900 added, so function 100 would be reported as 20000. But in
	// bytecode, the zero-based ID is used, so that's what we'll store here.
	_id = chunk.readTypedUint16();
	_code = new CodeChunk(chunk);
}

ScriptFunction::~ScriptFunction() {
	delete _code;
	_code = nullptr;
}

ScriptValue ScriptFunction::execute(Common::Array<ScriptValue> &args) {
	debugC(5, kDebugScript, "\n********** SCRIPT FUNCTION %d **********", _id);
	ScriptValue returnValue = _code->execute(&args);
	debugC(5, kDebugScript, "********** END SCRIPT FUNCTION **********");
	return returnValue;
}

FunctionManager::~FunctionManager() {
	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		delete it->_value;
	}
	_functions.clear();
}

bool FunctionManager::attemptToReadFromStream(Chunk &chunk, uint sectionType) {
	bool handledParam = true;
	switch (sectionType) {
	case 0x31: {
		ScriptFunction *function = new ScriptFunction(chunk);
		_functions.setVal(function->_id, function);
		break;
	}

	default:
		handledParam = false;
	}

	return handledParam;
}

ScriptValue FunctionManager::call(uint functionId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	// The original had a complex function registration system that I deemed too uselessly complex to
	// reimplement. First, we try executing the title-defined function. We try this first because
	// later engine versions used some functions IDs that previously mapped to built-in functions in
	// earlier engine versions. So we will try executing the title-defined function first and only then
	// fall back to the built-in functions.
	ScriptFunction *scriptFunction = _functions.getValOrDefault(functionId);
	if (scriptFunction != nullptr) {
		returnValue = scriptFunction->execute(args);
		return returnValue;
	}

	// If there was no title-defined function, next check for built-in functions.
	switch (functionId) {
	case kRandomFunction:
	case kLegacy_RandomFunction:
		assert(args.size() == 2);
		script_Random(args, returnValue);
		break;

	case kTimeOfDayFunction:
	case kLegacy_TimeOfDayFunction:
		script_TimeOfDay(args, returnValue);
		break;

	case kEffectTransitionFunction:
	case kLegacy_EffectTransitionFunction:
		g_engine->getDisplayManager()->effectTransition(args);
		break;

	case kEffectTransitionOnSyncFunction:
	case kLegacy_EffectTransitionOnSyncFunction:
		g_engine->getDisplayManager()->setTransitionOnSync(args);
		break;

	case kPlatformFunction:
	case kLegacy_PlatformFunction:
		assert(args.empty());
		script_GetPlatform(args, returnValue);
		break;

	case kSquareRootFunction:
	case kLegacy_SquareRootFunction:
		assert(args.size() == 1);
		script_SquareRoot(args, returnValue);
		break;

	case kGetUniqueRandomFunction:
	case kLegacy_GetUniqueRandomFunction:
		assert(args.size() >= 2);
		script_GetUniqueRandom(args, returnValue);
		break;

	case kCurrentRunTimeFunction:
		script_CurrentRunTime(args, returnValue);
		break;

	case kSetGammaCorrectionFunction:
		script_SetGammaCorrection(args, returnValue);
		break;

	case kGetDefaultGammaCorrectionFunction:
		script_GetDefaultGammaCorrection(args, returnValue);
		break;

	case kGetCurrentGammaCorrectionFunction:
		script_GetCurrentGammaCorrection(args, returnValue);
		break;

	case kSetAudioVolumeFunction:
		assert(args.size() == 1);
		script_SetAudioVolume(args, returnValue);
		break;

	case kGetAudioVolumeFunction:
		assert(args.empty());
		script_GetAudioVolume(args, returnValue);
		break;

	case kSystemLanguagePreferenceFunction:
	case kLegacy_SystemLanguagePreferenceFunction:
		script_SystemLanguagePreference(args, returnValue);
		break;

	case kSetRegistryFunction:
		script_SetRegistry(args, returnValue);
		break;

	case kGetRegistryFunction:
		script_GetRegistry(args, returnValue);
		break;

	case kSetProfileFunction:
		script_SetProfile(args, returnValue);
		break;

	case kMazeGenerateFunction:
		script_MazeGenerate(args, returnValue);
		break;

	case kMazeApplyMoveMaskFunction:
		script_MazeApplyMoveMask(args, returnValue);
		break;

	case kMazeSolveFunction:
		script_MazeSolve(args, returnValue);
		break;

	case kBeginTimedIntervalFunction:
		script_BeginTimedInterval(args, returnValue);
		break;

	case kEndTimedIntervalFunction:
		script_EndTimedInterval(args, returnValue);
		break;

	case kDrawingFunction:
		script_Drawing(args, returnValue);
		break;

	case kLegacy_DebugPrintFunction:
		script_DebugPrint(args, returnValue);
		break;

	default:
		// If we got here, that means there was neither a title-defined nor a built-in function
		// for this ID, so we can now declare it unimplemented. This is a warning instead of an error
		// so execution can continue, but if the function is expected to return anything, there will
		// likely be an error about attempting to assign a null value to a variable.
		warning("%s: Unimplemented function 0x%02x", __func__, functionId);
	}

	return returnValue;
}

void FunctionManager::script_GetPlatform(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	Common::Platform platform = g_engine->getPlatform();
	switch (platform) {
	case Common::Platform::kPlatformWindows:
		returnValue.setToParamToken(kPlatformParamTokenWindows);
		break;

	case Common::Platform::kPlatformMacintosh:
		returnValue.setToParamToken(kPlatformParamTokenWindows);
		break;

	default:
		warning("%s: Unknown platform %d", __func__, static_cast<int>(platform));
		returnValue.setToParamToken(kPlatformParamTokenUnknown);
	}
}

void FunctionManager::script_Random(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// This function takes in a range, and then generates a random value within that range.
	ScriptValue bottomArg = args[0];
	ScriptValue topArg = args[1];
	if (bottomArg.getType() != topArg.getType()) {
		error("%s: Both arguments must be of same type", __func__);
	}

	ScriptValueType type = args[0].getType();
	double bottom = 0.0;
	double top = 0.0;
	bool treatAsInteger = false;
	switch (type) {
	case kScriptValueTypeFloat: {
		// For numeric values, treat them as integers (floor values).
		bottom = floor(bottomArg.asFloat());
		top = floor(topArg.asFloat());
		treatAsInteger = true;
		break;
	}

	case kScriptValueTypeBool: {
		// Convert boolean values to numbers.
		bottom = bottomArg.asBool() ? 1.0 : 0.0;
		top = topArg.asBool() ? 1.0 : 0.0;
		treatAsInteger = true;
		break;
	}

	case kScriptValueTypeTime: {
		// Treat time values as capable of having fractional seconds.
		bottom = bottomArg.asTime();
		top = topArg.asTime();
		treatAsInteger = false;
		break;
	}

	default:
		error("%s: Invalid argument type: %s", __func__, scriptValueTypeToStr(type));
	}

	// Ensure proper inclusive ordering of bottom and top.
	if (top < bottom) {
		SWAP(top, bottom);
	}

	// Calculate random value in range.
	double range = top - bottom;
	uint randomValue = g_engine->_randomSource.getRandomNumber(UINT32_MAX);
	double randomFloat = (static_cast<double>(randomValue) * range) / static_cast<double>(UINT32_MAX) + bottom;
	if (treatAsInteger) {
		randomFloat = floor(randomFloat);
	}

	// Set result based on original argument type.
	switch (type) {
	case kScriptValueTypeFloat:
		returnValue.setToFloat(randomFloat);
		break;

	case kScriptValueTypeBool: {
		bool boolResult = (randomFloat != 0.0);
		returnValue.setToBool(boolResult);
		break;
	}

	case kScriptValueTypeTime:
		returnValue.setToTime(randomFloat);
		break;

	default:
		error("%s: Invalid argument type: %s", __func__, scriptValueTypeToStr(type));
	}
}

void FunctionManager::script_TimeOfDay(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: TimeOfDay");
}

void FunctionManager::script_SquareRoot(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args[0].getType() != kScriptValueTypeFloat) {
		error("%s: Numeric value required", __func__);
	}

	double value = args[0].asFloat();
	if (value < 0.0) {
		error("%s: Argument must be nonnegative", __func__);
	}

	double result = sqrt(value);
	returnValue.setToFloat(result);
}

void FunctionManager::script_GetUniqueRandom(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// Unlike the regular Random which simply returns any random number in a range, GetUniqueRandom allows the caller
	// to specify numbers that should NOT be returned (the third arg and onward), making it useful for generating random
	// values that haven't been used before or avoiding specific unwanted values.
	for (ScriptValue arg : args) {
		if (arg.getType() != kScriptValueTypeFloat) {
			error("%s: All arguments must be numeric", __func__);
		}
	}

	// The original forces that the list of excluded numbers (and the range to choose from)
	// can be at max 100 numbers. With the two args for the range, the max is thus 102.
	const uint MAX_ARGS_SIZE = 102;
	if (args.size() > MAX_ARGS_SIZE) {
		args.resize(MAX_ARGS_SIZE);
	}

	// Ensure that the range is properly constructed.
	double bottom = floor(args[0].asFloat());
	double top = floor(args[1].asFloat());
	if (top < bottom) {
		SWAP(top, bottom);
	}

	// Build list of unused (non-excluded) numbers in the range. For this numeric type,
	// everything is treated as an integer (even though it's stored as a double).
	Common::Array<double> unusedNumbers;
	for (double currentValue = bottom; currentValue < top; currentValue += 1.0) {
		// Check if this value appears in the exclusion list (args 2 onwards).
		bool isExcluded = false;
		for (uint i = 2; i < args.size(); i++) {
			if (args[i].asFloat() == currentValue) {
				isExcluded = true;
				break;
			}
		}

		if (!isExcluded) {
			unusedNumbers.push_back(currentValue);
		}
	}

	if (unusedNumbers.size() > 0) {
		uint randomIndex = g_engine->_randomSource.getRandomNumberRng(0, unusedNumbers.size());
		returnValue.setToFloat(unusedNumbers[randomIndex]);
	} else {
		warning("%s: No unused numbers to choose from", __func__);
	}
}

void FunctionManager::script_CurrentRunTime(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// The current runtime is expected to be returned in seconds.
	const uint MILLISECONDS_IN_ONE_SECOND = 1000;
	double runtimeInSeconds = g_system->getMillis() / MILLISECONDS_IN_ONE_SECOND;
	returnValue.setToFloat(runtimeInSeconds);
}

void FunctionManager::script_SetGammaCorrection(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args.size() != 1 && args.size() != 3) {
		warning("%s: Expected 1 or 3 arguments, got %u", __func__, args.size());
		return;
	}

	double red = 1.0;
	double green = 1.0;
	double blue = 1.0;
	if (args.size() >= 3) {
		if (args[0].getType() != kScriptValueTypeFloat ||
				args[1].getType() != kScriptValueTypeFloat ||
				args[2].getType() != kScriptValueTypeFloat) {
			warning("%s: Expected float arguments", __func__);
			return;
		}

		red = args[0].asFloat();
		green = args[1].asFloat();
		blue = args[2].asFloat();

	} else if (args.size() >= 1) {
		if (args[0].getType() != kScriptValueTypeCollection) {
			warning("%s: Expected collection argument", __func__);
			return;
		}

		Common::SharedPtr<Collection> collection = args[0].asCollection();
		if (collection->size() != 3) {
			warning("%s: Collection must contain exactly 3 elements, got %u", __func__, collection->size());
			return;
		}

		if (collection->operator[](0).getType() != kScriptValueTypeFloat ||
				collection->operator[](1).getType() != kScriptValueTypeFloat ||
				collection->operator[](2).getType() != kScriptValueTypeFloat) {
			warning("%s: Expected float arguments", __func__);
			return;
		}

		red = collection->operator[](0).asFloat();
		green = collection->operator[](1).asFloat();
		blue = collection->operator[](2).asFloat();
	}

	g_engine->getDisplayManager()->setGammaValues(red, green, blue);
}

void FunctionManager::script_GetDefaultGammaCorrection(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args.size() != 0) {
		warning("%s: Expected 0 arguments, got %u", __func__, args.size());
		return;
	}

	double red, green, blue;
	g_engine->getDisplayManager()->getDefaultGammaValues(red, green, blue);

	Common::SharedPtr<Collection> collection = Common::SharedPtr<Collection>(new Collection());
	ScriptValue redValue;
	redValue.setToFloat(red);
	collection->push_back(redValue);

	ScriptValue greenValue;
	greenValue.setToFloat(green);
	collection->push_back(greenValue);

	ScriptValue blueValue;
	blueValue.setToFloat(blue);
	collection->push_back(blueValue);

	returnValue.setToCollection(collection);
}

void FunctionManager::script_GetCurrentGammaCorrection(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args.size() != 0) {
		warning("%s: Expected 0 arguments, got %u", __func__, args.size());
		return;
	}

	double red, green, blue;
	g_engine->getDisplayManager()->getGammaValues(red, green, blue);
	Common::SharedPtr<Collection> collection = Common::SharedPtr<Collection>(new Collection());

	ScriptValue redValue;
	redValue.setToFloat(red);
	collection->push_back(redValue);

	ScriptValue greenValue;
	greenValue.setToFloat(green);
	collection->push_back(greenValue);

	ScriptValue blueValue;
	blueValue.setToFloat(blue);
	collection->push_back(blueValue);

	returnValue.setToCollection(collection);
}

void FunctionManager::script_SetAudioVolume(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	if (args[0].getType() != kScriptValueTypeFloat) {
		warning("%s: Expected float argument", __func__);
		return;
	}

	// Convert from 0.0 - 1.0 to ScummVM's mixer range.
	double volume = args[0].asFloat();
	volume = CLIP(volume, 0.0, 1.0);
	int mixerVolume = static_cast<int>(volume * Audio::Mixer::kMaxMixerVolume);
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, mixerVolume);
}

void FunctionManager::script_GetAudioVolume(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// Convert from ScummVM's mixer range to 0.0 - 1.0.
	int mixerVolume = g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kPlainSoundType);
	double volume = static_cast<double>(mixerVolume) / static_cast<double>(Audio::Mixer::kMaxMixerVolume);
	CLIP(volume, 0.0, 1.0);
	returnValue.setToFloat(volume);
}

void FunctionManager::script_SystemLanguagePreference(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: SystemLanguagePreference");
}

void FunctionManager::script_SetRegistry(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: SetRegistry");
}

void FunctionManager::script_GetRegistry(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: GetRegistry");
}

void FunctionManager::script_SetProfile(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: SetProfile");
}

void FunctionManager::script_DebugPrint(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	// The original reports time in seconds, but milliseconds is fine.
	// The "IMT @ clock ..." format is from the original's debug printing style.
	Common::String output = Common::String::format("IMT @ clock %d", g_system->getMillis());
	for (uint i = 0; i < args.size(); i++) {
		// Append all provided arguments.
		if (i != 0) {
			output += ", ";
		} else {
			output += " ";
		}
		output += args[i].getDebugString();
	}
	debug("%s", output.c_str());
}

void FunctionManager::script_MazeGenerate(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: MazeGenerate");
}

void FunctionManager::script_MazeApplyMoveMask(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: MazeApplyMoveMask");
}

void FunctionManager::script_MazeSolve(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: MazeSolve");
}

void FunctionManager::script_BeginTimedInterval(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: BeginTimedInterval");
}

void FunctionManager::script_EndTimedInterval(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: EndTimedInterval");
}

void FunctionManager::script_Drawing(Common::Array<ScriptValue> &args, ScriptValue &returnValue) {
	warning("STUB: Drawing");
}

void FunctionManager::deleteFunctionsForContext(uint contextId) {
	// Collect function IDs to delete first.
	Common::Array<ScriptFunction *> functionsToDelete;
	for (auto it = _functions.begin(); it != _functions.end(); ++it) {
		ScriptFunction *scriptFunction = it->_value;
		if (scriptFunction->_contextId == contextId) {
			functionsToDelete.push_back(scriptFunction);
		}
	}

	// Now delete them.
	for (ScriptFunction *scriptFunction : functionsToDelete) {
		_functions.erase(scriptFunction->_id);
		delete scriptFunction;
	}
}

} // End of namespace MediaStation
