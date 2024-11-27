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
#include "common/system.h"

#include "sci/sci.h"	// for INCLUDE_OLDGFX
#include "sci/debug.h"	// for g_debug_sleeptime_factor
#include "sci/engine/features.h"
#include "sci/engine/file.h"
#include "sci/engine/guest_additions.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/engine/script.h"
#include "sci/engine/message.h"

namespace Sci {

// Maps half-width single-byte SJIS to full-width double-byte SJIS
// Note: SSCI maps 0x5C (the Yen symbol) to 0x005C, which terminates
// the string with the leading 0x00 byte. We map Yen to 0x818F.
static const uint16 s_halfWidthSJISMap[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x81A8, 0x81A9, 0x81AA, 0x81AB,
	0x8140, 0x8149, 0x818D, 0x8194, 0x8190, 0x8193, 0x8195, 0x818C,
	0x8169, 0x816A, 0x8196, 0x817B, 0x8143, 0x817C, 0x8144, 0x815E,
	0x824F, 0x8250, 0x8251, 0x8252, 0x8253, 0x8254, 0x8255, 0x8256,
	0x8257, 0x8258, 0x8146, 0x8147, 0x8183, 0x8181, 0x8184, 0x8148,
	0x8197, 0x8260, 0x8261, 0x8262, 0x8263, 0x8264, 0x8265, 0x8266,
	0x8267, 0x8268, 0x8269, 0x826A, 0x826B, 0x826C, 0x826D, 0x826E,
	0x826F, 0x8270, 0x8271, 0x8272, 0x8273, 0x8274, 0x8275, 0x8276,
	0x8277, 0x8278, 0x8279, 0x816D, 0x818F /* 0x005C */, 0x816E, 0x814F, 0x8151,
	0x8280, 0x8281, 0x8282, 0x8283, 0x8284, 0x8285, 0x8286, 0x8287,
	0x8288, 0x8289, 0x828A, 0x828B, 0x828C, 0x828D, 0x828E, 0x828F,
	0x8290, 0x8291, 0x8292, 0x8293, 0x8294, 0x8295, 0x8296, 0x8297,
	0x8298, 0x8299, 0x829A, 0x816F, 0x8162, 0x8170, 0x8160, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0x8140, 0x8142, 0x8175, 0x8176, 0x8141, 0x8145, 0x8392, 0x8340,
	0x8342, 0x8344, 0x8346, 0x8348, 0x8383, 0x8385, 0x8387, 0x8362,
	0x815C, 0x8341, 0x8343, 0x8345, 0x8347, 0x8349, 0x834A, 0x834C,
	0x834E, 0x8350, 0x8352, 0x8354, 0x8356, 0x8358, 0x835A, 0x835C,
	0x835E, 0x8360, 0x8363, 0x8365, 0x8367, 0x8369, 0x836A, 0x836B,
	0x836C, 0x836D, 0x836E, 0x8371, 0x8374, 0x8377, 0x837A, 0x837D,
	0x837E, 0x8380, 0x8381, 0x8382, 0x8384, 0x8386, 0x8388, 0x8389,
	0x838A, 0x838B, 0x838C, 0x838D, 0x838F, 0x8393, 0x814A, 0x814B,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

EngineState::EngineState(SegManager *segMan) :
	_segMan(segMan),
	_msgState(nullptr),
	_dirseeker() {

	reset(false);
}

EngineState::~EngineState() {
	delete _msgState;
}

void EngineState::reset(bool isRestoring) {
	if (!isRestoring) {
		_memorySegmentSize = 0;
		_fileHandles.resize(5);
		abortScriptProcessing = kAbortNone;
	} else {
		g_sci->_guestAdditions->reset();
	}

	_delayedRestoreGameId = -1;

	_kq7MacSaveGameId = -1;
	_kq7MacSaveGameDescription.clear();

	executionStackBase = 0;
	_executionStackPosChanged = false;
	stack_base = nullptr;
	stack_top = nullptr;

	r_acc = NULL_REG;
	r_prev = NULL_REG;
	r_rest = 0;

	lastWaitTime = 0;

	gcCountDown = 0;

	_eventCounter = 0;
	_paletteSetIntensityCounter = 0;
	_throttleLastTime = 0;
	_throttleTrigger = false;

	_lastSaveVirtualId = SAVEGAMEID_OFFICIALRANGE_START;
	_lastSaveNewId = 0;

	_chosenQfGImportItem = 0;

	_cursorWorkaroundActive = false;

	scriptStepCounter = 0;
	scriptGCInterval = GC_INTERVAL;
}

void EngineState::speedThrottler(uint32 neededSleep) {
	if (_throttleTrigger) {
		uint32 curTime = g_system->getMillis();
		uint32 duration = curTime - _throttleLastTime;

		if (duration < neededSleep) {
			g_sci->sleep(neededSleep - duration);
			_throttleLastTime = g_system->getMillis();
		} else {
			_throttleLastTime = curTime;
		}
		_throttleTrigger = false;
	}
}

uint16 EngineState::wait(uint16 ticks) {
	uint32 time = g_system->getMillis();

	uint32 ms = ticks * 1000 / 60;
	uint32 duration = time - lastWaitTime;
	if (ms > duration) {
		uint32 sleepTime = ms - duration;
		sleepTime *= g_debug_sleeptime_factor;
		g_sci->sleep(sleepTime);
		time += sleepTime;
	}

	uint16 tickDelta = (uint16)(((long)time - lastWaitTime) * 60 / 1000);
	lastWaitTime = time;
	return tickDelta;
}

void EngineState::sleep(uint16 ticks) {
	ticks *= g_debug_sleeptime_factor;
	g_sci->sleep(ticks * 1000 / 60);
}

void EngineState::initGlobals() {
	Script *script_000 = _segMan->getScript(1);

	if (script_000->getLocalsCount() == 0)
		error("Script 0 has no locals block");

	variablesSegment[VAR_GLOBAL] = script_000->getLocalsSegment();
	variablesBase[VAR_GLOBAL] = variables[VAR_GLOBAL] = script_000->getLocalsBegin();
	variablesMax[VAR_GLOBAL] = script_000->getLocalsCount();

	// The KQ5 CD Windows interpreter set global 400 to tell the scripts that the
	//  platform was Windows. The global determines which cursors the scripts use,
	//  so we only set this if the user has chosen to use Windows cursors.
	if (g_sci->getGameId() == GID_KQ5 && g_sci->isCD()) {
		variables[VAR_GLOBAL][400].setOffset(g_sci->_features->useWindowsCursors());
	}
}

void EngineState::initMessageState() {
	delete _msgState;
	_msgState = new MessageState(_segMan);
}

uint16 EngineState::currentRoomNumber() const {
	return variables[VAR_GLOBAL][kGlobalVarNewRoomNo].toUint16();
}

void EngineState::setRoomNumber(uint16 roomNumber) {
	variables[VAR_GLOBAL][kGlobalVarNewRoomNo] = make_reg(0, roomNumber);
}

void EngineState::shrinkStackToBase() {
	if (_executionStack.size() > 0) {
		uint size = executionStackBase + 1;
		assert(_executionStack.size() >= size);
		Common::List<ExecStack>::iterator iter = _executionStack.begin();
		for (uint i = 0; i < size; ++i)
			++iter;
		_executionStack.erase(iter, _executionStack.end());
	}
}

static kLanguage charToLanguage(const char c) {
	switch (c) {
	case 'F':
		return K_LANG_FRENCH;
	case 'S':
		return K_LANG_SPANISH;
	case 'I':
		return K_LANG_ITALIAN;
	case 'G':
		return K_LANG_GERMAN;
	case 'J':
	case 'j':
		return K_LANG_JAPANESE;
	case 'P':
		return K_LANG_PORTUGUESE;
	default:
		return K_LANG_NONE;
	}
}

Common::String SciEngine::getSciLanguageString(const Common::String &str, kLanguage requestedLanguage, kLanguage *secondaryLanguage, uint16 *languageSplitter) const {
	kLanguage foundLanguage = K_LANG_NONE;
	const byte *textPtr = (const byte *)str.c_str();
	if (secondaryLanguage) {
		*secondaryLanguage = K_LANG_NONE;
	}
	byte curChar = 0;
	byte curChar2 = 0;

	while (1) {
		curChar = *textPtr;
		if (!curChar)
			break;

		if ((curChar == '%') || (curChar == '#')) {
			curChar2 = *(textPtr + 1);
			foundLanguage = charToLanguage(curChar2);

			if (foundLanguage != K_LANG_NONE) {
				// Return language splitter
				if (languageSplitter)
					*languageSplitter = curChar | ( curChar2 << 8 );
				// Return the secondary language found in the string
				if (secondaryLanguage)
					*secondaryLanguage = foundLanguage;
				break;
			}
		}
		textPtr++;
	}

	if (foundLanguage == requestedLanguage) {
		if (curChar2 == 'J') {
			// Japanese including Kanji, displayed with system font
			// Convert half-width characters to full-width equivalents
			Common::String fullWidth;

			textPtr += 2; // skip over language splitter

			while (1) {
				curChar = *textPtr;

				switch (curChar) {
				case 0: // Terminator NUL
					return fullWidth;
				default:
					break;
				}

				textPtr++;

				uint16 mappedChar = s_halfWidthSJISMap[curChar];
				if (mappedChar) {
					fullWidth += mappedChar >> 8;
					fullWidth += mappedChar & 0xFF;
				} else {
					// Copy double-byte character
					curChar2 = *(textPtr++);
					if (!curChar2) {
						error("SJIS character %02X is missing second byte", curChar);
						break;
					}
					fullWidth += curChar;
					fullWidth += curChar2;
				}
			}

		} else {
			return Common::String((const char *)(textPtr + 2));
		}
	}

	if (curChar)
		return Common::String(str.c_str(), (const char *)textPtr - str.c_str());

	return str;
}

kLanguage SciEngine::getSciLanguage() {
	kLanguage lang = K_LANG_ENGLISH;

	if (SELECTOR(printLang) != -1) {
		lang = (kLanguage)readSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(printLang));

		if ((getSciVersion() >= SCI_VERSION_1_1) || (lang == K_LANG_NONE)) {
			// If language is set to none, we use the language from the game detector.
			// SSCI reads this from resource.cfg (early games do not have a language
			// setting in resource.cfg, but instead have the secondary language number
			// hardcoded in the game script).
			// SCI1.1 games always use the language setting from the config file
			// (essentially disabling runtime language switching).
			// Note: only a limited number of multilanguage games have been tested
			// so far, so this information may not be 100% accurate.
			switch (getLanguage()) {
			case Common::FR_FRA:
				lang = K_LANG_FRENCH;
				// WORKAROUND: The French version of LSL1VGA is a fan patch that's based
				// on the official Spanish version, with the Spanish content replaced
				// with French content. The game scripts require printLang to be Spanish
				// in order to use the French text and load the correct views.
				if (g_sci->getGameId() == GID_LSL1) {
					lang = K_LANG_SPANISH;
				}
				break;
			case Common::ES_ESP:
				lang = K_LANG_SPANISH;
				break;
			case Common::IT_ITA:
				lang = K_LANG_ITALIAN;
				break;
			case Common::DE_DEU:
				lang = K_LANG_GERMAN;
				break;
			case Common::JA_JPN:
				lang = K_LANG_JAPANESE;
				break;
			case Common::PT_BRA:
				lang = K_LANG_PORTUGUESE;
				break;
			default:
				lang = K_LANG_ENGLISH;
			}
		}
	}

	return lang;
}

void SciEngine::setSciLanguage(kLanguage lang) {
	if (SELECTOR(printLang) != -1)
		writeSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(printLang), lang);
}

void SciEngine::setSciLanguage() {
	setSciLanguage(getSciLanguage());
}

Common::String SciEngine::strSplitLanguage(const char *str, uint16 *languageSplitter, const char *sep) {
	kLanguage activeLanguage = getSciLanguage();
	kLanguage subtitleLanguage = K_LANG_NONE;

	if (SELECTOR(subtitleLang) != -1)
		subtitleLanguage = (kLanguage)readSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(subtitleLang));

	kLanguage foundLanguage;
	Common::String retval = getSciLanguageString(str, activeLanguage, &foundLanguage, languageSplitter);

	// Don't add subtitle when separator is not set, subtitle language is not set, or
	// string contains only one language
	if ((sep == nullptr) || (subtitleLanguage == K_LANG_NONE) || (foundLanguage == K_LANG_NONE))
		return retval;

	// Add subtitle, unless the subtitle language doesn't match the languages in the string
	if ((subtitleLanguage == K_LANG_ENGLISH) || (subtitleLanguage == foundLanguage)) {
		retval += sep;
		retval += getSciLanguageString(str, subtitleLanguage);
	}

	return retval;
}

void SciEngine::checkVocabularySwitch() {
	uint16 parserLanguage = 1;
	if (SELECTOR(parseLang) != -1)
		parserLanguage = readSelectorValue(_gamestate->_segMan, _gameObjectAddress, SELECTOR(parseLang));

	if (parserLanguage != _vocabularyLanguage) {
		delete _vocabulary;
		_vocabulary = new Vocabulary(_resMan, parserLanguage > 1 ? true : false);
		_vocabulary->reset();
		_vocabularyLanguage = parserLanguage;
	}
}

SciCallOrigin EngineState::getCurrentCallOrigin() const {
	// IMPORTANT: This method must always return values that match *exactly* the
	// values in the workaround tables in workarounds.cpp, or workarounds will
	// be broken

	Common::String curObjectName = _segMan->getObjectName(xs->sendp);
	Common::String curMethodName;
	const Script *localScript = _segMan->getScriptIfLoaded(xs->local_segment);
	if (localScript == nullptr) {
		error("current script not found at: %04x", xs->local_segment);
	}
	int curScriptNr = localScript->getScriptNumber();

	Selector debugSelector = xs->debugSelector;
	int debugExportId = xs->debugExportId;

	if (xs->debugLocalCallOffset != -1) {
		// if lastcall was actually a local call search back for a real call
		Common::List<ExecStack>::const_iterator callIterator = _executionStack.end();
		while (callIterator != _executionStack.begin()) {
			callIterator--;
			const ExecStack &loopCall = *callIterator;
			if (loopCall.debugSelector != -1 || loopCall.debugExportId != -1) {
				debugSelector = loopCall.debugSelector;
				debugExportId = loopCall.debugExportId;
				break;
			}
		}
	}

	if (xs->type == EXEC_STACK_TYPE_CALL) {
		if (debugSelector != -1) {
			curMethodName = g_sci->getKernel()->getSelectorName(debugSelector);
		} else if (debugExportId != -1) {
			curObjectName = "";
			curMethodName = Common::String::format("export %d", debugExportId);
		}
	}

	SciCallOrigin reply;
	reply.objectName = curObjectName;
	reply.methodName = curMethodName;
	reply.scriptNr = curScriptNr;
	reply.localCallOffset = xs->debugLocalCallOffset;
	reply.roomNr = currentRoomNumber();
	return reply;
}

bool EngineState::callInStack(const reg_t object, const Selector selector) const {
	Common::List<ExecStack>::const_iterator it;
	for (it = _executionStack.begin(); it != _executionStack.end(); ++it) {
		const ExecStack &call = *it;
		if (call.sendp == object && call.debugSelector == selector) {
			return true;
		}
	}

	return false;
}

Common::String EngineState::getGameVersionFromGlobal() const {
	// The version global was originally 28 but then became 27.
	// When it was 28, 27 was a volume level, so differentiate by type.
	reg_t versionRef = variables[VAR_GLOBAL][kGlobalVarVersionNew];
	if (versionRef.isNumber()) {
		versionRef = variables[VAR_GLOBAL][kGlobalVarVersionOld];
	}
#ifdef ENABLE_SCI32
	// LSL7 and Phant2 store the version string as an object instead of a reference
	if (_segMan->isObject(versionRef)) {
		versionRef = readSelector(_segMan, versionRef, SELECTOR(data));
	}
#endif
	if (versionRef.isPointer()) {
		return _segMan->getString(versionRef);
	}
	return Common::String();
}

} // End of namespace Sci
