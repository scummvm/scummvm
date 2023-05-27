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

#include "common/debug.h"
#include "common/stream.h"
#include "common/hash-str.h"

#include "vcruise/script.h"
#include "vcruise/textparser.h"


namespace VCruise {

enum ScriptDialect {
	kScriptDialectReah,
	kScriptDialectSchizm,
};

class LogicUnscrambleStream : public Common::ReadStream {
public:
	LogicUnscrambleStream(Common::ReadStream *stream, uint streamSize);

	bool eos() const override;
	uint32 read(void *dataPtr, uint32 dataSize) override;

private:
	byte _cipher[255];
	uint _cipherOffset;
	Common::ReadStream *_stream;
};

LogicUnscrambleStream::LogicUnscrambleStream(Common::ReadStream *stream, uint streamSize) : _stream(stream) {
	int key = 255;
	for (int i = 0; i < 255; i++) {
		int parityBit = ((key ^ (key >> 1) ^ (key >> 6) ^ (key >> 7))) & 1;
		key = (key >> 1) | (parityBit << 7);
		_cipher[254 - i] = key;
	}

	_cipherOffset = 255u - (streamSize % 255u);
}

bool LogicUnscrambleStream::eos() const {
	return _stream->eos();
}

uint32 LogicUnscrambleStream::read(void *dataPtr, uint32 dataSize) {
	uint32 numRead = _stream->read(dataPtr, dataSize);

	byte *decipher = static_cast<byte *>(dataPtr);

	uint cipherOffset = _cipherOffset;

	uint32 remaining = numRead;
	while (remaining) {
		if (cipherOffset == 255)
			cipherOffset = 0;

		(*decipher++) ^= _cipher[cipherOffset++];
		remaining--;
	}

	_cipherOffset = cipherOffset;
	return numRead;
}

Instruction::Instruction() : op(ScriptOps::kInvalid), arg(0) {
}

Instruction::Instruction(ScriptOps::ScriptOp paramOp) : op(paramOp), arg(0) {
}

Instruction::Instruction(ScriptOps::ScriptOp paramOp, int32 paramArg) : op(paramOp), arg(paramArg) {
}

enum ProtoOp {
	kProtoOpScript, // Use script opcode

	kProtoOpNoop,

	kProtoOpJumpToLabel,
	kProtoOpLabel,

	kProtoOpIf,
	kProtoOpElse,
	kProtoOpEndIf,
	kProtoOpSwitch,
	kProtoOpCase,
	kProtoOpEndSwitch,
	kProtoOpDefault,
	kProtoOpBreak,
};

struct ProtoInstruction {
	ProtoInstruction();
	explicit ProtoInstruction(ScriptOps::ScriptOp op);
	ProtoInstruction(ScriptOps::ScriptOp paramOp, int32 paramArg);
	ProtoInstruction(ProtoOp paramProtoOp, ScriptOps::ScriptOp paramOp, int32 paramArg);

	ProtoOp protoOp;
	ScriptOps::ScriptOp op;
	int32 arg;
};

ProtoInstruction::ProtoInstruction() : protoOp(kProtoOpScript), op(ScriptOps::kInvalid), arg(0) {
}

ProtoInstruction::ProtoInstruction(ScriptOps::ScriptOp paramOp) : protoOp(kProtoOpScript), op(paramOp), arg(0) {
}

ProtoInstruction::ProtoInstruction(ScriptOps::ScriptOp paramOp, int32 paramArg) : protoOp(kProtoOpScript), op(paramOp), arg(paramArg) {
}

ProtoInstruction::ProtoInstruction(ProtoOp paramProtoOp, ScriptOps::ScriptOp paramOp, int32 paramArg) : protoOp(paramProtoOp), op(paramOp), arg(paramArg) {
}

struct ProtoScript {
	void reset();

	Common::Array<ProtoInstruction> instrs;
};

void ProtoScript::reset() {
	instrs.clear();
}

struct ScriptNamedInstruction {
	const char *str;
	ProtoOp protoOp;
	ScriptOps::ScriptOp op;
};

class ScriptCompiler {
public:
	ScriptCompiler(TextParser &parser, const Common::String &blamePath, ScriptDialect dialect, uint loadAsRoom, uint fileRoom, IScriptCompilerGlobalState *gs);

	void compileScriptSet(ScriptSet *ss);

private:
	bool parseNumber(const Common::String &token, uint32 &outNumber) const;
	static bool parseDecNumber(const Common::String &token, uint start, uint32 &outNumber);
	static bool parseHexNumber(const Common::String &token, uint start, uint32 &outNumber);
	static bool parseBinNumber(const Common::String &token, uint start, uint32 &outNumber);
	void expectNumber(uint32 &outNumber);

	void compileRoomScriptSet(RoomScriptSet *rss);
	void compileReahScreenScriptSet(ScreenScriptSet *sss);
	void compileSchizmScreenScriptSet(ScreenScriptSet *sss);
	void compileFunction(Script *script);
	bool compileInstructionToken(ProtoScript &script, const Common::String &token);

	void codeGenScript(ProtoScript &protoScript, Script &script);

	uint indexString(const Common::String &str);

	enum NumberParsingMode {
		kNumberParsingDec,
		kNumberParsingHex,
		kNumberParsingBin,
	};

	TextParser &_parser;
	NumberParsingMode _numberParsingMode;
	const Common::String _blamePath;

	ScriptDialect _dialect;
	uint _loadAsRoom;
	uint _fileRoom;

	const char *_scrToken;
	const char *_eroomToken;

	Common::HashMap<Common::String, uint> _stringToIndex;

	IScriptCompilerGlobalState *_gs;
};

class ScriptCompilerGlobalState : public IScriptCompilerGlobalState {
public:
	void define(const Common::String &key, uint roomNumber, int32 value) override;

	bool getDefine(const Common::String &str, uint &outRoomNumber, int32 &outValue) const override;

	uint getFunctionIndex(const Common::String &fnName) override;
	void setFunction(uint fnIndex, const Common::SharedPtr<Script> &fn) override;

	uint getNumFunctions() const override;
	void dumpFunctionNames(Common::Array<Common::String> &fnNames) const override;
	Common::SharedPtr<Script> getFunction(uint fnIndex) const override;

private:
	// Defs are linked to room numbers to deal with some weird variable sharing behavior.  In Reah,
	// variables are bound to room, but in Schizm they are clearly not.  fnInitNewGame in Room01.log
	// initializes a lot of things in various rooms, but some of those IDs also collide with other
	// things.  For example, dwNawigat2SZ is 13, and is read in Room20, but variable ID 13 is also
	// used for dwStartSound47 in Room47 to determine if sounds have started, and it is expected
	// to be initially 0 there.
	struct Def {
		Def();

		int32 _value;
		uint _roomNumber;
	};

	Common::HashMap<Common::String, Def> _defs;

	Common::HashMap<Common::String, uint> _functionNameToIndex;
	Common::Array<Common::SharedPtr<Script> > _functions;
};

ScriptCompiler::ScriptCompiler(TextParser &parser, const Common::String &blamePath, ScriptDialect dialect, uint loadAsRoom, uint fileRoom, IScriptCompilerGlobalState *gs)
	: _numberParsingMode(kNumberParsingHex), _parser(parser), _blamePath(blamePath), _dialect(dialect), _loadAsRoom(loadAsRoom), _fileRoom(fileRoom), _gs(gs),
	  _scrToken(nullptr), _eroomToken(nullptr) {
}

bool ScriptCompiler::parseNumber(const Common::String &token, uint32 &outNumber) const {
	if (token.size() == 0)
		return false;

	if (_dialect == kScriptDialectReah) {
		if (token[0] == 'd')
			return parseDecNumber(token, 1, outNumber);

		if (token[0] == '0') {
			switch (_numberParsingMode) {
			case kNumberParsingDec:
				return parseDecNumber(token, 0, outNumber);
			case kNumberParsingHex:
				return parseHexNumber(token, 0, outNumber);
			case kNumberParsingBin:
				return parseBinNumber(token, 0, outNumber);
			default:
				error("Unknown number parsing mode");
				return false;
			}
		}
	} else if (_dialect == kScriptDialectSchizm) {
		if (token.size() >= 2 && token[0] == '0' && token[1] == 'x')
			return parseHexNumber(token, 2, outNumber);
		if (token[token.size() - 1] == 'b')
			return parseBinNumber(token.substr(0, token.size() - 1), 0, outNumber);
		if (token[token.size() - 1] == 'h')
			return parseHexNumber(token.substr(0, token.size() - 1), 0, outNumber);
		return parseDecNumber(token, 0, outNumber);
	}

	return false;
}

bool ScriptCompiler::parseDecNumber(const Common::String &token, uint start, uint32 &outNumber) {
	if (start == token.size())
		return false;

	// We don't use sscanf because sscanf accepts partial results and we want to reject this if any character is mismatched
	uint32 num = 0;
	for (uint i = start; i < token.size(); i++) {
		num *= 10u;

		char c = token[i];
		if (c >= '0' && c <= '9')
			num += static_cast<uint32>(c - '0');
		else
			return false;
	}

	outNumber = num;
	return true;
}

bool ScriptCompiler::parseHexNumber(const Common::String &token, uint start, uint32 &outNumber) {
	if (start == token.size())
		return false;

	// We don't use sscanf because sscanf accepts partial results and we want to reject this if any character is mismatched
	uint32 num = 0;
	for (uint i = start; i < token.size(); i++) {
		num *= 16u;

		char c = token[i];
		if (c >= '0' && c <= '9')
			num += static_cast<uint32>(c - '0');
		else if (c >= 'a' && c <= 'f')
			num += static_cast<uint32>(c - 'a' + 0xa);
		else if (c >= 'A' && c <= 'F')
			num += static_cast<uint32>(c - 'a' + 0xa);
		else
			return false;
	}

	outNumber = num;
	return true;
}

bool ScriptCompiler::parseBinNumber(const Common::String &token, uint start, uint32 &outNumber) {
	if (start == token.size())
		return false;

	uint num = 0;
	for (uint i = start; i < token.size(); i++) {
		char c = token[i];

		num <<= 1;
		if (c == '1')
			num |= 1;
		else if (c != '0')
			return false;
	}

	outNumber = num;
	return true;
}

void ScriptCompiler::expectNumber(uint32 &outNumber) {
	TextParserState state;
	Common::String token;
	if (_parser.parseToken(token, state)) {
		if (!parseNumber(token, outNumber))
			error("Error compiling script at line %i col %i: Expected number but found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), token.c_str());
	} else {
		error("Error compiling script at line %i col %i: Expected number", static_cast<int>(state._lineNum), static_cast<int>(state._col));
	}
}

void ScriptCompiler::compileScriptSet(ScriptSet *ss) {
	Common::SharedPtr<RoomScriptSet> roomScript;

	uint numExistingStrings = ss->strings.size();

	for (uint i = 0; i < numExistingStrings; i++)
		_stringToIndex[ss->strings[i]] = i;

	const char *roomToken = nullptr;

	if (_dialect == kScriptDialectReah) {
		roomToken = "~ROOM";
		_eroomToken = "~EROOM";
		_scrToken = "~SCR";
	} else if (_dialect == kScriptDialectSchizm) {
		roomToken = "~Room";
		_eroomToken = "~ERoom";
		_scrToken = "~Scr";
	} else
		error("Unknown script dialect");

	TextParserState state;
	Common::String token;
	while (_parser.parseToken(token, state)) {
		if (token == roomToken) {
			if (roomScript)
				error("Error compiling script at line %i col %i: Encountered %s without %s", static_cast<int>(state._lineNum), static_cast<int>(state._col), roomToken, _eroomToken);
			roomScript.reset(new RoomScriptSet());

			{
				uint32 roomNumber = 0;

				if (_parser.parseToken(token, state)) {
					if (!parseNumber(token, roomNumber))
						error("Error compiling script at line %i col %i: Expected number but found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), token.c_str());

					ss->roomScripts[roomNumber] = roomScript;
				} else {
					error("Error compiling script at line %i col %i: Expected number", static_cast<int>(state._lineNum), static_cast<int>(state._col));
				}
			}

			compileRoomScriptSet(roomScript.get());
		} else {
			error("Error compiling script at line %i col %i: Expected %s and found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), roomToken, token.c_str());
		}
	}

	for (const Common::HashMap<Common::String, uint>::Node &stiNode : _stringToIndex) {
		if (stiNode._value >= numExistingStrings) {
			if (stiNode._value >= ss->strings.size())
				ss->strings.resize(stiNode._value + 1);

			ss->strings[stiNode._value] = stiNode._key;
		}
	}
}

void ScriptCompiler::compileRoomScriptSet(RoomScriptSet *rss) {
	TextParserState state;
	Common::String token;
	while (_parser.parseToken(token, state)) {
		if (token == _eroomToken) {
			return;
		} else if (token == _scrToken) {
			uint32 screenNumber = 0;
			expectNumber(screenNumber);

			Common::SharedPtr<ScreenScriptSet> sss(new ScreenScriptSet());
			if (_dialect == kScriptDialectReah)
				compileReahScreenScriptSet(sss.get());
			else if (_dialect == kScriptDialectSchizm) {

				if (!_parser.parseToken(token, state))
					error("Error compiling script at line %i col %i: Expected screen name", static_cast<int>(state._lineNum), static_cast<int>(state._col));

				rss->screenNames[token] = screenNumber;

				compileSchizmScreenScriptSet(sss.get());
			}

			// QUIRK: The tower in Reah (Room 06) has two 0cb screens, the second one is bad and must be ignored
			if (rss->screenScripts.find(screenNumber) == rss->screenScripts.end())
				rss->screenScripts[screenNumber] = sss;
		} else if (_dialect == kScriptDialectSchizm && token == "#def") {
			Common::String key;
			Common::String value;
			if (!_parser.parseToken(key, state))
				error("Error compiling script at line %i col %i: Expected key", static_cast<int>(state._lineNum), static_cast<int>(state._col));
			if (!_parser.parseToken(value, state))
				error("Error compiling script at line %i col %i: Expected value", static_cast<int>(state._lineNum), static_cast<int>(state._col));

			bool isNegative = false;
			if (value[0] == '-') {
				isNegative = true;
				value = value.substr(1);
			}

			uint32 number = 0;
			if (!parseNumber(value, number))
				error("Error compiling script at line %i col %i: Expected number", static_cast<int>(state._lineNum), static_cast<int>(state._col));

			int32 signedNumber = static_cast<int32>(number);
			if (isNegative)
				signedNumber = -signedNumber;

			// Based on testing, this should be _loadAsRoom
			_gs->define(key, _loadAsRoom, signedNumber);
		} else if (_dialect == kScriptDialectSchizm && token == "~Fun") {
			Common::String fnName;
			if (!_parser.parseToken(fnName, state))
				error("Error compiling script at line %i col %i: Expected function name", static_cast<int>(state._lineNum), static_cast<int>(state._col));

			Common::SharedPtr<Script> func(new Script());

			compileFunction(func.get());

			uint fnIndex = _gs->getFunctionIndex(fnName);

			if (_gs->getFunction(fnIndex)) {
				// This triggers on fnSoundFountain_Start and fnSoundFountain_Stop in Room30.
				// fnSoundFountain_Start is called in Room31, so might not matter there?  But fnSoundFountain_Stop is called in Room30.
				warning("Function '%s' was defined multiple times", fnName.c_str());
			}

			_gs->setFunction(_gs->getFunctionIndex(fnName), func);
		} else {
			error("Error compiling script at line %i col %i: Expected %s or %s and found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), _eroomToken, _scrToken, token.c_str());
		}
	}

	error("Error compiling script: Room wasn't terminated");
}

void ScriptCompiler::compileReahScreenScriptSet(ScreenScriptSet *sss) {
	TextParserState state;
	Common::String token;

	ProtoScript protoScript;
	Common::SharedPtr<Script> currentScript(new Script());

	sss->entryScript.reset(currentScript);

	while (_parser.parseToken(token, state)) {
		if (token == "~EROOM" || token == "~SCR") {
			_parser.requeue(token, state);

			codeGenScript(protoScript, *currentScript);
			return;
		} else if (token == "~*") {
			uint32 interactionNumber = 0;
			expectNumber(interactionNumber);

			codeGenScript(protoScript, *currentScript);

			currentScript.reset(new Script());
			protoScript.reset();

			sss->interactionScripts[interactionNumber] = currentScript;
		} else if (token == "DEC") {
			_numberParsingMode = kNumberParsingDec;
		} else if (token == "HEX") {
			_numberParsingMode = kNumberParsingHex;
		} else if (token == "BIN") {
			_numberParsingMode = kNumberParsingBin;
		} else if (token == "dubbing") {
			Common::String dubbingName;
			_parser.expectToken(dubbingName, _blamePath);
			protoScript.instrs.push_back(ProtoInstruction(ScriptOps::kDubbing, indexString(dubbingName)));
		} else if (compileInstructionToken(protoScript, token)) {
			// Nothing
		} else {
			error("Error compiling script at line %i col %i: Expected %s or %s or ~* or instruction but found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), _eroomToken, _scrToken, token.c_str());
		}
	}
}

void ScriptCompiler::compileSchizmScreenScriptSet(ScreenScriptSet *sss) {
	TextParserState state;
	Common::String token;

	ProtoScript protoScript;
	Common::SharedPtr<Script> currentScript(new Script());

	sss->entryScript.reset(currentScript);

	while (_parser.parseToken(token, state)) {
		if (token == "~ERoom" || token == "~Scr" || token == "~Fun") {
			_parser.requeue(token, state);

			codeGenScript(protoScript, *currentScript);
			return;
		} else if (token == "~*") {
			uint32 interactionNumber = 0;
			expectNumber(interactionNumber);

			codeGenScript(protoScript, *currentScript);

			currentScript.reset(new Script());
			protoScript.reset();

			sss->interactionScripts[interactionNumber] = currentScript;
		} else if (compileInstructionToken(protoScript, token)) {
			// Nothing
		} else {
			error("Error compiling script at line %i col %i: Expected ~EROOM or ~SCR or ~* or instruction but found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), token.c_str());
		}
	}
}

void ScriptCompiler::compileFunction(Script *script) {
	TextParserState state;
	Common::String token;

	ProtoScript protoScript;

	while (_parser.parseToken(token, state)) {
		if (token == "~ERoom" || token == "~Scr" || token == "~Fun") {
			_parser.requeue(token, state);

			codeGenScript(protoScript, *script);
			return;
		} else if (compileInstructionToken(protoScript, token)) {
			// Nothing
		} else {
			error("Error compiling script at line %i col %i: Expected ~ERoom or ~Scr or ~Fun but found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), token.c_str());
		}
	}
}

static ScriptNamedInstruction g_reahNamedInstructions[] = {
	{"rotate", ProtoOp::kProtoOpScript, ScriptOps::kRotate},
	{"angle", ProtoOp::kProtoOpScript, ScriptOps::kAngle},
	{"angleG@", ProtoOp::kProtoOpScript, ScriptOps::kAngleGGet},
	{"speed", ProtoOp::kProtoOpScript, ScriptOps::kSpeed},
	{"sanimL", ProtoOp::kProtoOpScript, ScriptOps::kSAnimL},
	{"changeL", ProtoOp::kProtoOpScript, ScriptOps::kChangeL},
	{"changeL1", ProtoOp::kProtoOpScript, ScriptOps::kChangeL},	// This seems wrong, but not sure what changeL1 does differently from changeL yet
	{"animF", ProtoOp::kProtoOpScript, ScriptOps::kAnimF},
	{"animG", ProtoOp::kProtoOpScript, ScriptOps::kAnimG},
	{"animN", ProtoOp::kProtoOpScript, ScriptOps::kAnimN},
	{"animR", ProtoOp::kProtoOpScript, ScriptOps::kAnimR},
	{"animS", ProtoOp::kProtoOpScript, ScriptOps::kAnimS},
	{"anim", ProtoOp::kProtoOpScript, ScriptOps::kAnim},
	{"static", ProtoOp::kProtoOpScript, ScriptOps::kStatic},
	{"yes@", ProtoOp::kProtoOpScript, ScriptOps::kVarLoad},
	{"yes!", ProtoOp::kProtoOpScript, ScriptOps::kVarStore},
	{"yesg@", ProtoOp::kProtoOpScript, ScriptOps::kVarGlobalLoad},
	{"yesg!", ProtoOp::kProtoOpScript, ScriptOps::kVarGlobalStore},
	{"setaX+!", ProtoOp::kProtoOpScript, ScriptOps::kVarAddAndStore},
	{"cr?", ProtoOp::kProtoOpScript, ScriptOps::kItemCheck},
	{"cr!", ProtoOp::kProtoOpScript, ScriptOps::kItemRemove},
	{"sr!", ProtoOp::kProtoOpScript, ScriptOps::kItemHighlightSet},
	{"r?", ProtoOp::kProtoOpScript, ScriptOps::kItemHaveSpace},
	{"r!", ProtoOp::kProtoOpScript, ScriptOps::kItemAdd},
	{"clearPocket", ProtoOp::kProtoOpScript, ScriptOps::kItemClear},
	{"cursor!", ProtoOp::kProtoOpScript, ScriptOps::kSetCursor},
	{"room!", ProtoOp::kProtoOpScript, ScriptOps::kSetRoom},
	{"lmb", ProtoOp::kProtoOpScript, ScriptOps::kLMB},
	{"lmb1", ProtoOp::kProtoOpScript, ScriptOps::kLMB1},
	{"volumeDn2", ProtoOp::kProtoOpScript, ScriptOps::kVolumeDn2},
	{"volumeDn3", ProtoOp::kProtoOpScript, ScriptOps::kVolumeDn3},
	{"volumeDn4", ProtoOp::kProtoOpScript, ScriptOps::kVolumeDn4},
	{"volumeUp3", ProtoOp::kProtoOpScript, ScriptOps::kVolumeUp3},
	{"rnd", ProtoOp::kProtoOpScript, ScriptOps::kRandom},
	{"drop", ProtoOp::kProtoOpScript, ScriptOps::kDrop},
	{"dup", ProtoOp::kProtoOpScript, ScriptOps::kDup},
	{"swap", ProtoOp::kProtoOpScript, ScriptOps::kSwap},
	{"say1", ProtoOp::kProtoOpScript, ScriptOps::kSay1},
	{"say2", ProtoOp::kProtoOpScript, ScriptOps::kSay2},
	{"say3", ProtoOp::kProtoOpScript, ScriptOps::kSay3},
	{"say3@", ProtoOp::kProtoOpScript, ScriptOps::kSay3Get},
	{"setTimer", ProtoOp::kProtoOpScript, ScriptOps::kSetTimer},
	{"getTimer", ProtoOp::kProtoOpScript, ScriptOps::kGetTimer},
	{"delay", ProtoOp::kProtoOpScript, ScriptOps::kDelay},
	{"lo!", ProtoOp::kProtoOpScript, ScriptOps::kLoSet},
	{"lo@", ProtoOp::kProtoOpScript, ScriptOps::kLoGet},
	{"hi!", ProtoOp::kProtoOpScript, ScriptOps::kHiSet},
	{"hi@", ProtoOp::kProtoOpScript, ScriptOps::kHiGet},

	{"and", ProtoOp::kProtoOpScript, ScriptOps::kAnd},
	{"or", ProtoOp::kProtoOpScript, ScriptOps::kOr},
	{"+", ProtoOp::kProtoOpScript, ScriptOps::kAdd},
	{"-", ProtoOp::kProtoOpScript, ScriptOps::kSub},
	{"not", ProtoOp::kProtoOpScript, ScriptOps::kNot},
	{"minus", ProtoOp::kProtoOpScript, ScriptOps::kNegate},
	{"=", ProtoOp::kProtoOpScript, ScriptOps::kCmpEq},
	{">", ProtoOp::kProtoOpScript, ScriptOps::kCmpGt},
	{"<", ProtoOp::kProtoOpScript, ScriptOps::kCmpLt},

	{"bit@", ProtoOp::kProtoOpScript, ScriptOps::kBitLoad},
	{"bit0!", ProtoOp::kProtoOpScript, ScriptOps::kBitSet0},
	{"bit1!", ProtoOp::kProtoOpScript, ScriptOps::kBitSet1},

	{"soundS1", ProtoOp::kProtoOpScript, ScriptOps::kSoundS1},
	{"soundS2", ProtoOp::kProtoOpScript, ScriptOps::kSoundS2},
	{"soundS3", ProtoOp::kProtoOpScript, ScriptOps::kSoundS3},
	{"soundL1", ProtoOp::kProtoOpScript, ScriptOps::kSoundL1},
	{"soundL2", ProtoOp::kProtoOpScript, ScriptOps::kSoundL2},
	{"soundL3", ProtoOp::kProtoOpScript, ScriptOps::kSoundL3},
	{"3DsoundS2", ProtoOp::kProtoOpScript, ScriptOps::k3DSoundS2},
	{"3DsoundL2", ProtoOp::kProtoOpScript, ScriptOps::k3DSoundL2},
	{"3DsoundL3", ProtoOp::kProtoOpScript, ScriptOps::k3DSoundL3},
	{"stopaL", ProtoOp::kProtoOpScript, ScriptOps::kStopAL},
	{"range", ProtoOp::kProtoOpScript, ScriptOps::kRange},
	{"addXsound", ProtoOp::kProtoOpScript, ScriptOps::kAddXSound},
	{"clrXsound", ProtoOp::kProtoOpScript, ScriptOps::kClrXSound},
	{"stopSndLA", ProtoOp::kProtoOpScript, ScriptOps::kStopSndLA},
	{"stopSndLO", ProtoOp::kProtoOpScript, ScriptOps::kStopSndLO},

	{"music", ProtoOp::kProtoOpScript, ScriptOps::kMusic},
	{"musicUp", ProtoOp::kProtoOpScript, ScriptOps::kMusicVolRamp},
	{"musicDn", ProtoOp::kProtoOpScript, ScriptOps::kMusicVolRamp},

	{"parm0", ProtoOp::kProtoOpScript, ScriptOps::kParm0},
	{"parm1", ProtoOp::kProtoOpScript, ScriptOps::kParm1},
	{"parm2", ProtoOp::kProtoOpScript, ScriptOps::kParm2},
	{"parm3", ProtoOp::kProtoOpScript, ScriptOps::kParm3},
	{"parmG", ProtoOp::kProtoOpScript, ScriptOps::kParmG},
	{"sparmX", ProtoOp::kProtoOpScript, ScriptOps::kSParmX},
	{"sanimX", ProtoOp::kProtoOpScript, ScriptOps::kSAnimX},

	{"disc1", ProtoOp::kProtoOpScript, ScriptOps::kDisc1},
	{"disc2", ProtoOp::kProtoOpScript, ScriptOps::kDisc2},
	{"disc3", ProtoOp::kProtoOpScript, ScriptOps::kDisc3},

	{"goto", ProtoOp::kProtoOpScript, ScriptOps::kGoto},

	{"#if", ProtoOp::kProtoOpIf, ScriptOps::kInvalid},
	{"#eif", ProtoOp::kProtoOpEndIf, ScriptOps::kInvalid},
	{"#else", ProtoOp::kProtoOpElse, ScriptOps::kInvalid},

	{"#switch:", ProtoOp::kProtoOpSwitch, ScriptOps::kInvalid},
	{"#eswitch", ProtoOp::kProtoOpEndSwitch, ScriptOps::kInvalid},
	{"break", ProtoOp::kProtoOpBreak, ScriptOps::kInvalid},
	{"#default", ProtoOp::kProtoOpDefault, ScriptOps::kInvalid},

	{"esc_on", ProtoOp::kProtoOpScript, ScriptOps::kEscOn},
	{"esc_off", ProtoOp::kProtoOpScript, ScriptOps::kEscOff},
	{"esc_get@", ProtoOp::kProtoOpScript, ScriptOps::kEscGet},
	{"backStart", ProtoOp::kProtoOpScript, ScriptOps::kBackStart},
	{"saveAs", ProtoOp::kProtoOpScript, ScriptOps::kSaveAs},
	{"save0", ProtoOp::kProtoOpNoop, ScriptOps::kSave0},
	{"exit", ProtoOp::kProtoOpScript, ScriptOps::kExit},
	{"allowedSave", ProtoOp::kProtoOpScript, ScriptOps::kAllowSaves},
};



static ScriptNamedInstruction g_schizmNamedInstructions[] = {
	{"StopScore", ProtoOp::kProtoOpScript, ScriptOps::kMusicStop},
	{"PlayScore", ProtoOp::kProtoOpScript, ScriptOps::kMusicPlayScore},
	{"ScoreAlways", ProtoOp::kProtoOpScript, ScriptOps::kScoreAlways},
	{"ScoreNormal", ProtoOp::kProtoOpScript, ScriptOps::kScoreNormal},
	{"SndAddRandom", ProtoOp::kProtoOpScript, ScriptOps::kSndAddRandom},
	{"SndClearRandom", ProtoOp::kProtoOpScript, ScriptOps::kSndClearRandom},
	{"SndPlay", ProtoOp::kProtoOpScript, ScriptOps::kSndPlay},
	{"SndPlayEx", ProtoOp::kProtoOpScript, ScriptOps::kSndPlayEx},
	{"SndPlay3D", ProtoOp::kProtoOpScript, ScriptOps::kSndPlay3D},
	{"SndPlaying", ProtoOp::kProtoOpScript, ScriptOps::kSndPlaying},
	{"SndHalt", ProtoOp::kProtoOpScript, ScriptOps::kSndHalt},
	{"SndWait", ProtoOp::kProtoOpScript, ScriptOps::kSndWait},
	{"SndToBack", ProtoOp::kProtoOpScript, ScriptOps::kSndToBack},
	{"SndStop", ProtoOp::kProtoOpScript, ScriptOps::kSndStop},
	{"SndStopAll", ProtoOp::kProtoOpScript, ScriptOps::kSndStopAll},
	{"VolumeAdd", ProtoOp::kProtoOpScript, ScriptOps::kVolumeAdd},
	{"VolumeChange", ProtoOp::kProtoOpScript, ScriptOps::kVolumeChange},
	{"VolumeDown", ProtoOp::kProtoOpScript, ScriptOps::kVolumeDn2},
	{"esc_on", ProtoOp::kProtoOpScript, ScriptOps::kEscOn},
	{"esc_off", ProtoOp::kProtoOpScript, ScriptOps::kEscOff},
	{"esc_get@", ProtoOp::kProtoOpScript, ScriptOps::kEscGet},
	{"room!", ProtoOp::kProtoOpScript, ScriptOps::kSetRoom},
	{"room@", ProtoOp::kProtoOpScript, ScriptOps::kGetRoom},
	{"lmb", ProtoOp::kProtoOpScript, ScriptOps::kLMB},
	{"lmb1", ProtoOp::kProtoOpScript, ScriptOps::kLMB1},
	{"animVolume", ProtoOp::kProtoOpScript, ScriptOps::kAnimVolume},
	{"animChange", ProtoOp::kProtoOpScript, ScriptOps::kAnimChange},
	{"anim", ProtoOp::kProtoOpScript, ScriptOps::kAnim},			// Dialect difference: Accepts room name
	{"static", ProtoOp::kProtoOpScript, ScriptOps::kStatic},
	{"animF", ProtoOp::kProtoOpScript, ScriptOps::kAnimF},
	{"animG", ProtoOp::kProtoOpScript, ScriptOps::kAnimG},
	{"animN", ProtoOp::kProtoOpScript, ScriptOps::kAnimN},
	{"animR", ProtoOp::kProtoOpScript, ScriptOps::kAnimR},
	{"animS", ProtoOp::kProtoOpScript, ScriptOps::kAnimS},
	{"sanimL", ProtoOp::kProtoOpScript, ScriptOps::kSAnimL},
	{"sparmX", ProtoOp::kProtoOpScript, ScriptOps::kSParmX},
	{"sanimX", ProtoOp::kProtoOpScript, ScriptOps::kSAnimX},
	{"byte@", ProtoOp::kProtoOpScript, ScriptOps::kExtractByte},
	{"byte!", ProtoOp::kProtoOpScript, ScriptOps::kInsertByte},
	{"rotate", ProtoOp::kProtoOpScript, ScriptOps::kRotate},
	{"rotateUpdate", ProtoOp::kProtoOpScript, ScriptOps::kRotateUpdate},
	{"bit@", ProtoOp::kProtoOpScript, ScriptOps::kBitLoad},
	{"bit0!", ProtoOp::kProtoOpScript, ScriptOps::kBitSet0},
	{"bit1!", ProtoOp::kProtoOpScript, ScriptOps::kBitSet1},
	{"speech", ProtoOp::kProtoOpScript, ScriptOps::kSpeech},
	{"speechEx", ProtoOp::kProtoOpScript, ScriptOps::kSpeechEx},
	{"speechTest", ProtoOp::kProtoOpScript, ScriptOps::kSpeechTest},
	{"say", ProtoOp::kProtoOpScript, ScriptOps::kSay},
	{"changeL", ProtoOp::kProtoOpScript, ScriptOps::kChangeL},		// Dialect difference: Accepts room name
	{"range", ProtoOp::kProtoOpScript, ScriptOps::kRange},
	{"sound3DL2", ProtoOp::kProtoOpScript, ScriptOps::k3DSoundL2},	// Dialect difference: Different name
	{"random", ProtoOp::kProtoOpScript, ScriptOps::kRandomInclusive},
	{"heroSetPos", ProtoOp::kProtoOpScript, ScriptOps::kHeroSetPos},
	{"heroGetPos", ProtoOp::kProtoOpScript, ScriptOps::kHeroGetPos},
	{"heroOut", ProtoOp::kProtoOpScript, ScriptOps::kHeroOut},
	{"hero@", ProtoOp::kProtoOpScript, ScriptOps::kHeroGet},
	{"ret", ProtoOp::kProtoOpScript, ScriptOps::kReturn},
	{"setTimer", ProtoOp::kProtoOpScript, ScriptOps::kSetTimer},
	{"getTimer", ProtoOp::kProtoOpScript, ScriptOps::kGetTimer},
	{"delay", ProtoOp::kProtoOpScript, ScriptOps::kDelay},
	{"lo!", ProtoOp::kProtoOpScript, ScriptOps::kLoSet},
	{"lo@", ProtoOp::kProtoOpScript, ScriptOps::kLoGet},
	{"hi!", ProtoOp::kProtoOpScript, ScriptOps::kHiSet},
	{"hi@", ProtoOp::kProtoOpScript, ScriptOps::kHiGet},
	{"angle@", ProtoOp::kProtoOpScript, ScriptOps::kAngleGet},
	{"angleG@", ProtoOp::kProtoOpScript, ScriptOps::kAngleGGet},
	{"cd@", ProtoOp::kProtoOpScript, ScriptOps::kIsCDVersion},
	{"dvd@", ProtoOp::kProtoOpScript, ScriptOps::kIsDVDVersion},
	{"disc", ProtoOp::kProtoOpScript, ScriptOps::kDisc},
	{"save0", ProtoOp::kProtoOpNoop, ScriptOps::kSave0},
	{"hidePanel", ProtoOp::kProtoOpScript, ScriptOps::kHidePanel},
	{"ItemExist@", ProtoOp::kProtoOpScript, ScriptOps::kItemCheck},
	{"ItemSelect!", ProtoOp::kProtoOpScript, ScriptOps::kItemHighlightSetTrue},
	{"ItemPlace@", ProtoOp::kProtoOpScript, ScriptOps::kItemHaveSpace},
	{"ItemPutInto!", ProtoOp::kProtoOpScript, ScriptOps::kItemAdd},
	{"ItemRemove!", ProtoOp::kProtoOpScript, ScriptOps::kItemRemove},
	{"cyfra@", ProtoOp::kProtoOpScript, ScriptOps::kGetDigit},	// Cyfra = digit
	{"puzzleInit", ProtoOp::kProtoOpScript, ScriptOps::kPuzzleInit},
	{"puzzleCanPress", ProtoOp::kProtoOpScript, ScriptOps::kPuzzleCanPress},
	{"puzzleDoMove1", ProtoOp::kProtoOpScript, ScriptOps::kPuzzleDoMove1},
	{"puzzleDoMove2", ProtoOp::kProtoOpScript, ScriptOps::kPuzzleDoMove2},
	{"puzzleDone", ProtoOp::kProtoOpScript, ScriptOps::kPuzzleDone},
	{"puzzleWhoWon", ProtoOp::kProtoOpScript, ScriptOps::kPuzzleWhoWon},
	{"fn", ProtoOp::kProtoOpScript, ScriptOps::kFn},
	{"parm1", ProtoOp::kProtoOpScript, ScriptOps::kParm1},
	{"parm2", ProtoOp::kProtoOpScript, ScriptOps::kParm2},
	{"parm3", ProtoOp::kProtoOpScript, ScriptOps::kParm3},
	{"parmG", ProtoOp::kProtoOpScript, ScriptOps::kParmG},

	{"+", ProtoOp::kProtoOpScript, ScriptOps::kAdd},
	{"-", ProtoOp::kProtoOpScript, ScriptOps::kSub},
	{"*", ProtoOp::kProtoOpScript, ScriptOps::kMul},
	{"/", ProtoOp::kProtoOpScript, ScriptOps::kDiv},
	{"%", ProtoOp::kProtoOpScript, ScriptOps::kMod},
	
	{"&&", ProtoOp::kProtoOpScript, ScriptOps::kAnd},
	{"or", ProtoOp::kProtoOpScript, ScriptOps::kOr},
	{"||", ProtoOp::kProtoOpScript, ScriptOps::kOr},
	{"+", ProtoOp::kProtoOpScript, ScriptOps::kAdd},
	{"-", ProtoOp::kProtoOpScript, ScriptOps::kSub},
	{">", ProtoOp::kProtoOpScript, ScriptOps::kCmpGt},
	{"<", ProtoOp::kProtoOpScript, ScriptOps::kCmpLt},
	{"=", ProtoOp::kProtoOpScript, ScriptOps::kCmpEq},
	{"==", ProtoOp::kProtoOpScript, ScriptOps::kCmpEq},
	{"!=", ProtoOp::kProtoOpScript, ScriptOps::kCmpNE},
	{">=", ProtoOp::kProtoOpScript, ScriptOps::kCmpGE},
	{"<=", ProtoOp::kProtoOpScript, ScriptOps::kCmpLE},

	{"&", ProtoOp::kProtoOpScript, ScriptOps::kBitAnd},
	{"|", ProtoOp::kProtoOpScript, ScriptOps::kBitOr},

	{"#if", ProtoOp::kProtoOpIf, ScriptOps::kInvalid},
	{"#eif", ProtoOp::kProtoOpEndIf, ScriptOps::kInvalid},
	{"#else", ProtoOp::kProtoOpElse, ScriptOps::kInvalid},

	{"#switch:", ProtoOp::kProtoOpSwitch, ScriptOps::kInvalid},
	{"#eswitch", ProtoOp::kProtoOpEndSwitch, ScriptOps::kInvalid},
	{"break", ProtoOp::kProtoOpBreak, ScriptOps::kInvalid},

	{"ret", ProtoOp::kProtoOpScript, ScriptOps::kReturn},

	{"backStart", ProtoOp::kProtoOpScript, ScriptOps::kBackStart},
	{"allowedSave", ProtoOp::kProtoOpScript, ScriptOps::kAllowSaves},
};

bool ScriptCompiler::compileInstructionToken(ProtoScript &script, const Common::String &token) {
	if (_dialect == kScriptDialectSchizm) {
		uint roomNumber = 0;
		int32 value = 0;
		if (_gs->getDefine(token, roomNumber, value)) {
			script.instrs.push_back(ProtoInstruction(ScriptOps::kNumber, value));
			return true;
		}
	}

	if (_dialect == kScriptDialectSchizm && token.hasPrefix("-")) {
		uint32 unumber = 0;
		if (parseNumber(token.substr(1), unumber)) {
			script.instrs.push_back(ProtoInstruction(ScriptOps::kNumber, -static_cast<int32>(unumber)));
			return true;
		}
	}

	uint32 number = 0;
	if (parseNumber(token, number)) {
		script.instrs.push_back(ProtoInstruction(ScriptOps::kNumber, number));
		return true;
	}

	if (token.size() >= 1 && token[0] == ':') {
		if (_dialect == kScriptDialectReah) {
			if (token.size() >= 3 && token[2] == ':') {
				if (token[1] == 'Y') {
					script.instrs.push_back(ProtoInstruction(ScriptOps::kVarName, indexString(token.substr(3))));
					return true;
				} else if (token[1] == 'V') {
					script.instrs.push_back(ProtoInstruction(ScriptOps::kValueName, indexString(token.substr(3))));
					return true;
				} else
					return false;
			}
		}

		script.instrs.push_back(ProtoInstruction(ScriptOps::kAnimName, indexString(token.substr(1))));
		return true;
	}

	if (token.size() >= 2 && token[0] == '_') {
		script.instrs.push_back(ProtoInstruction(ScriptOps::kSoundName, indexString(token.substr(1))));
		return true;
	}

	if (_dialect == kScriptDialectReah) {
		if (token.hasPrefix("CUR_")) {
			script.instrs.push_back(ProtoInstruction(ScriptOps::kCursorName, indexString(token)));
			return true;
		}
	}

	if (token == "#switch") {
		_parser.expect(":", _blamePath);
		script.instrs.push_back(ProtoInstruction(kProtoOpSwitch, ScriptOps::kInvalid, 0));
		return true;
	}

	if (token == "#case") {
		uint32 caseNumber = 0;
		_parser.expect(":", _blamePath);
		expectNumber(caseNumber);

		script.instrs.push_back(ProtoInstruction(kProtoOpCase, ScriptOps::kInvalid, caseNumber));
		return true;
	}

	if (token == "#case:") {
		uint32 caseNumber = 0;
		expectNumber(caseNumber);

		script.instrs.push_back(ProtoInstruction(kProtoOpCase, ScriptOps::kInvalid, caseNumber));
		return true;
	}

	if (_dialect == kScriptDialectReah) {
		for (const ScriptNamedInstruction &namedInstr : g_reahNamedInstructions) {
			if (token == namedInstr.str) {
				script.instrs.push_back(ProtoInstruction(namedInstr.protoOp, namedInstr.op, 0));
				return true;
			}
		}
	} else if (_dialect == kScriptDialectSchizm) {
		if (token.size() >= 2 && token[0] == '\"' && token[token.size() - 1] == '\"') {
			// Seems like these are only used for sounds and music?
			uint fnIndex = indexString(token.substr(1, token.size() - 2));
			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kString, fnIndex));
			return true;
		}

		for (const ScriptNamedInstruction &namedInstr : g_schizmNamedInstructions) {
			if (token == namedInstr.str) {
				script.instrs.push_back(ProtoInstruction(namedInstr.protoOp, namedInstr.op, 0));
				return true;
			}
		}

		if (token.hasPrefix("fn")) {
			uint fnIndex = _gs->getFunctionIndex(token);
			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kCallFunction, fnIndex));
			return true;
		}

		if (token.size() >= 2 && token.hasSuffix("!")) {
			uint roomNumber = 0;
			int32 varNumber = 0;
			if (!_gs->getDefine(token.substr(0, token.size() - 1), roomNumber, varNumber) || varNumber < 0)
				return false;

			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kNumber, (roomNumber << 16) + static_cast<uint>(varNumber)));
			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kVarGlobalStore, 0));
			return true;
		}

		// HACK: Work around bugged variable name in Room02.log
		if (token == "dwFirst\x8c@") {
			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kNumber, 0));
			return true;
		}

		if (token.size() >= 2 && token.hasSuffix("@")) {
			uint roomNumber = 0;
			int32 varNumber = 0;
			if (!_gs->getDefine(token.substr(0, token.size() - 1), roomNumber, varNumber) || varNumber < 0)
				return false;

			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kNumber, (roomNumber << 16) + static_cast<uint>(varNumber)));
			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kVarGlobalLoad, 0));
			return true;
		}

		// Does this look like a screen name?
		bool couldBeScreenName = true;
		for (uint i = 0; i < token.size(); i++) {
			char c = token[i];
			bool isAlphaNumeric = ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
			if (!isAlphaNumeric) {
				couldBeScreenName = false;
				break;
			}
		}

		if (couldBeScreenName) {
			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kScreenName, indexString(token)));
			return true;
		}

		if (token.hasPrefix("cur")) {
			script.instrs.push_back(ProtoInstruction(ScriptOps::kCursorName, indexString(token)));
			script.instrs.push_back(ProtoInstruction(ScriptOps::kSetCursor, 0));
			return true;
		}

		// Disabled since Room02 is a cheat room and so not needed.
#if 0
		// HACK: Work around broken volume variable names in Room02.  Some of these appear to have "par"
		// where it should be "vol" but some are garbage.  Figure this out later.
		if (token.hasPrefix("par")) {
			script.instrs.push_back(ProtoInstruction(kProtoOpScript, ScriptOps::kGarbage, indexString(token)));
			return true;
		}
#endif
	}

	return false;
}

enum CodeGenFlowControlBlockType {
	kFlowControlInvalid,

	kFlowControlIf,
	kFlowControlSwitch,
};

struct CodeGenControlFlowBlock {
	CodeGenControlFlowBlock();

	CodeGenFlowControlBlockType type;
	uint index;
};

CodeGenControlFlowBlock::CodeGenControlFlowBlock() : type(kFlowControlInvalid), index(0) {
}

struct CodeGenSwitchCase {
	CodeGenSwitchCase();

	int32 value;
	uint label;
};

CodeGenSwitchCase::CodeGenSwitchCase() : value(0), label(0) {
}

struct CodeGenSwitch {
	CodeGenSwitch();

	Common::Array<CodeGenSwitchCase> cases;

	uint endLabel;

	uint defaultLabel;
	bool hasDefault;
};

CodeGenSwitch::CodeGenSwitch() : defaultLabel(0), endLabel(0), hasDefault(false) {
}

struct CodeGenIf {
	CodeGenIf();

	uint endLabel;

	uint elseLabel;
	bool hasElse;
};

CodeGenIf::CodeGenIf() : endLabel(0), elseLabel(0), hasElse(false) {
}


void ScriptCompiler::codeGenScript(ProtoScript &protoScript, Script &script) {
	Common::Array<ProtoInstruction> instrs;
	Common::Array<CodeGenSwitch> switches;
	Common::Array<CodeGenIf> ifs;
	Common::Array<CodeGenControlFlowBlock> controlFlowStack;

	int32 nextLabel = 0;

	// Pass 1: Collect flow control constructs, make all flow control constructs point to the index of the construct,
	// replace Else, Case, EndIf, EndSwitch, and Default instructions with Label and JumpToLabel.  Clear noops.
	for (const ProtoInstruction &instr : protoScript.instrs) {
		switch (instr.protoOp) {
		case kProtoOpScript:
			instrs.push_back(instr);
			break;
		case kProtoOpBreak: {
			bool found = false;
			for (uint ri = 0; ri < controlFlowStack.size(); ri++) {
				const CodeGenControlFlowBlock &cf = controlFlowStack[controlFlowStack.size() - 1 - ri];
				if (cf.type == kFlowControlSwitch) {
					found = true;
					instrs.push_back(ProtoInstruction(kProtoOpBreak, ScriptOps::kInvalid, cf.index));
					break;
				}
			}

			if (!found)
				error("Error in codegen: break statement outside of a switch case");
		} break;
		case kProtoOpIf: {
			CodeGenControlFlowBlock cf;
			cf.type = kFlowControlIf;
			cf.index = ifs.size();
			controlFlowStack.push_back(cf);

			CodeGenIf ifBlock;
			ifBlock.endLabel = nextLabel++;
			ifs.push_back(ifBlock);

			instrs.push_back(ProtoInstruction(kProtoOpIf, ScriptOps::kInvalid, cf.index));
		} break;
		case kProtoOpElse: {
			if (controlFlowStack.size() == 0)
				error("Error in codegen: #else outside of control flow");

			CodeGenControlFlowBlock &cf = controlFlowStack[controlFlowStack.size() - 1];

			if (cf.type != kFlowControlIf)
				error("Error in codegen: #else inside wrong control block type");

			CodeGenIf &ifBlock = ifs[cf.index];

			if (ifBlock.hasElse)
				error("Error in codegen: #else already set for #if block");

			ifBlock.hasElse = true;
			ifBlock.elseLabel = nextLabel++;

			instrs.push_back(ProtoInstruction(kProtoOpJumpToLabel, ScriptOps::kInvalid, ifBlock.endLabel));
			instrs.push_back(ProtoInstruction(kProtoOpLabel, ScriptOps::kInvalid, ifBlock.elseLabel));
		} break;
		case kProtoOpEndIf: {
			if (controlFlowStack.size() == 0)
				error("Error in codegen: #eif outside of control flow");

			CodeGenControlFlowBlock &cf = controlFlowStack[controlFlowStack.size() - 1];

			if (cf.type != kFlowControlIf)
				error("Error in codegen: #eif inside wrong control block type");

			const CodeGenIf &ifBlock = ifs[cf.index];

			instrs.push_back(ProtoInstruction(kProtoOpLabel, ScriptOps::kInvalid, ifBlock.endLabel));

			controlFlowStack.pop_back();
		} break;
		case kProtoOpSwitch: {
			CodeGenControlFlowBlock cf;
			cf.type = kFlowControlSwitch;
			cf.index = switches.size();
			controlFlowStack.push_back(cf);

			CodeGenSwitch switchBlock;
			switchBlock.endLabel = nextLabel++;
			switches.push_back(switchBlock);

			instrs.push_back(ProtoInstruction(kProtoOpSwitch, ScriptOps::kInvalid, cf.index));
		} break;
		case kProtoOpCase: {
			if (controlFlowStack.size() == 0)
				error("Error in codegen: #case outside of control flow");

			CodeGenControlFlowBlock &cf = controlFlowStack[controlFlowStack.size() - 1];

			if (cf.type != kFlowControlSwitch)
				error("Error in codegen: #case inside wrong control block type");

			CodeGenSwitch &switchBlock = switches[cf.index];

			CodeGenSwitchCase caseDef;
			caseDef.label = nextLabel++;
			caseDef.value = instr.arg;

			switchBlock.cases.push_back(caseDef);

			instrs.push_back(ProtoInstruction(kProtoOpLabel, ScriptOps::kInvalid, caseDef.label));
		} break;
		case kProtoOpDefault: {
			if (controlFlowStack.size() == 0)
				error("Error in codegen: #case outside of control flow");

			CodeGenControlFlowBlock &cf = controlFlowStack[controlFlowStack.size() - 1];

			if (cf.type != kFlowControlSwitch)
				error("Error in codegen: #case inside wrong control block type");

			CodeGenSwitch &switchBlock = switches[cf.index];

			if (switchBlock.hasDefault)
				error("Error in codegen: #switch already has a default");

			switchBlock.hasDefault = true;
			switchBlock.defaultLabel = nextLabel++;

			instrs.push_back(ProtoInstruction(kProtoOpLabel, ScriptOps::kInvalid, switchBlock.defaultLabel));
		} break;
		case kProtoOpEndSwitch: {
			if (controlFlowStack.size() == 0)
				error("Error in codegen: #eswitch outside of control flow");

			CodeGenControlFlowBlock &cf = controlFlowStack[controlFlowStack.size() - 1];

			if (cf.type != kFlowControlSwitch)
				error("Error in codegen: #eswitch inside wrong control block type");

			const CodeGenSwitch &switchBlock = switches[cf.index];

			instrs.push_back(ProtoInstruction(kProtoOpLabel, ScriptOps::kInvalid, switchBlock.endLabel));

			controlFlowStack.pop_back();
		} break;
		case kProtoOpNoop:
			break;
		default:
			error("Internal error: Unhandled proto-op");
			break;
		}
	}

	if (controlFlowStack.size() > 0) {
		if (_dialect == kScriptDialectSchizm) {
			// For some reason line 105 in Room36 and line 342 in Room61 have unterminated conditional blocks,
			// and the comments say that's intentional.

			while (controlFlowStack.size() > 0) {
				const CodeGenControlFlowBlock &cf = controlFlowStack.back();

				switch (controlFlowStack.back().type) {
				case kFlowControlIf: {
						warning("CodeGen encountered unterminated #if statement");
						instrs.push_back(ProtoInstruction(kProtoOpLabel, ScriptOps::kInvalid, ifs[cf.index].endLabel));
						controlFlowStack.pop_back();
					} break;
				case kFlowControlSwitch: {
						warning("CodeGen encountered unterminated #switch statement");
						instrs.push_back(ProtoInstruction(kProtoOpLabel, ScriptOps::kInvalid, switches[cf.index].endLabel));
						controlFlowStack.pop_back();
					} break;
				default:
					error("Unknown control flow type");
				}
			}
		} else {
			error("Error in codegen: Unterminated flow control construct");
		}
	}

	Common::Array<ProtoInstruction> instrs2;

	Common::HashMap<uint, uint> labelToInstr;

	// Pass 2: Unroll If and Switch instructions into CheckValue and JumpToLabel ops, resolve label locations
	for (const ProtoInstruction &instr : instrs) {
		switch (instr.protoOp) {
		case kProtoOpScript:
		case kProtoOpJumpToLabel:
			instrs2.push_back(instr);
			break;
		case kProtoOpIf: {
			const CodeGenIf &ifBlock = ifs[instr.arg];

			instrs2.push_back(ProtoInstruction(ScriptOps::kCheckValue, 0));
			instrs2.push_back(ProtoInstruction(kProtoOpJumpToLabel, ScriptOps::kInvalid, ifBlock.hasElse ? ifBlock.elseLabel : ifBlock.endLabel));
			instrs2.push_back(ProtoInstruction(ScriptOps::kDrop));
		} break;
		case kProtoOpSwitch: {
			const CodeGenSwitch &switchBlock = switches[instr.arg];

			for (const CodeGenSwitchCase &caseDef : switchBlock.cases) {
				instrs2.push_back(ProtoInstruction(ScriptOps::kCheckValue, caseDef.value));
				instrs2.push_back(ProtoInstruction(kProtoOpJumpToLabel, ScriptOps::kInvalid, caseDef.label));
			}

			instrs2.push_back(ProtoInstruction(ScriptOps::kDrop));
			instrs2.push_back(ProtoInstruction(kProtoOpJumpToLabel, ScriptOps::kInvalid, switchBlock.hasDefault ? switchBlock.defaultLabel : switchBlock.endLabel));
		} break;
		case kProtoOpLabel:
			labelToInstr[static_cast<uint>(instr.arg)] = instrs2.size();
			break;
		case kProtoOpBreak: {
			const CodeGenSwitch &switchBlock = switches[instr.arg];

			instrs2.push_back(ProtoInstruction(kProtoOpJumpToLabel, ScriptOps::kInvalid, switchBlock.endLabel));
		} break;
		default:
			error("Internal error: Unhandled proto-op");
			break;
		}
	}

	instrs.clear();

	// Pass 3: Change all JumpToLabel ops to Jump ops and write out final instructions
	script.instrs.reserve(instrs2.size());

	for (const ProtoInstruction &instr : instrs2) {
		switch (instr.protoOp) {
		case kProtoOpScript:
			script.instrs.push_back(Instruction(instr.op, instr.arg));
			break;
		case kProtoOpJumpToLabel: {
			Common::HashMap<uint, uint>::const_iterator it = labelToInstr.find(static_cast<uint>(instr.arg));
			if (it == labelToInstr.end())
				error("Internal error: Unmatched label");

			script.instrs.push_back(Instruction(ScriptOps::kJump, it->_value));
		} break;
		default:
			error("Internal error: Unhandled proto-op");
			break;
		}
	}
}

uint ScriptCompiler::indexString(const Common::String &str) {
	Common::HashMap<Common::String, uint>::const_iterator it = _stringToIndex.find(str);
	if (it == _stringToIndex.end()) {
		uint index = _stringToIndex.size();
		_stringToIndex[str] = index;
		return index;
	}

	return it->_value;
}

ScriptCompilerGlobalState::Def::Def() : _value(0), _roomNumber(0) {
}

void ScriptCompilerGlobalState::define(const Common::String &key, uint roomNumber, int32 value) {
	Def &def = _defs[key];

	def._roomNumber = roomNumber;
	def._value = value;
}

bool ScriptCompilerGlobalState::getDefine(const Common::String &str, uint &outRoomNumber, int32 &outValue) const {
	Common::HashMap<Common::String, Def>::const_iterator it = _defs.find(str);
	if (it == _defs.end())
		return false;

	outRoomNumber = it->_value._roomNumber;
	outValue = it->_value._value;
	return true;
}

uint ScriptCompilerGlobalState::getFunctionIndex(const Common::String &fnName) {
	Common::HashMap<Common::String, uint>::const_iterator it = _functionNameToIndex.find(fnName);

	assert(fnName != "fn");

	if (it == _functionNameToIndex.end()) {
		uint newIndex = _functionNameToIndex.size();
		_functionNameToIndex.setVal(fnName, newIndex);
		_functions.push_back(nullptr);

		return newIndex;
	} else
		return it->_value;
}

void ScriptCompilerGlobalState::setFunction(uint fnIndex, const Common::SharedPtr<Script> &fn) {
	_functions[fnIndex] = fn;
}

uint ScriptCompilerGlobalState::getNumFunctions() const {
	return _functionNameToIndex.size();
}

void ScriptCompilerGlobalState::dumpFunctionNames(Common::Array<Common::String> &fnNames) const {
	fnNames.clear();
	fnNames.resize(_functionNameToIndex.size());

	for (const Common::HashMap<Common::String, uint>::Node &node : _functionNameToIndex)
		fnNames[node._value] = node._key;
}

Common::SharedPtr<Script> ScriptCompilerGlobalState::getFunction(uint fnIndex) const {
	return _functions[fnIndex];
}

ScriptSet::ScriptSet() {
}

IScriptCompilerGlobalState::~IScriptCompilerGlobalState() {
}

static void compileLogicFile(ScriptSet &scriptSet, Common::ReadStream &stream, uint streamSize, const Common::String &blamePath, ScriptDialect dialect, uint loadAsRoom, uint fileRoom, IScriptCompilerGlobalState *gs) {
	LogicUnscrambleStream unscrambleStream(&stream, streamSize);
	TextParser parser(&unscrambleStream);

	ScriptCompiler compiler(parser, blamePath, dialect, loadAsRoom, fileRoom, gs);

	compiler.compileScriptSet(&scriptSet);
}

Common::SharedPtr<IScriptCompilerGlobalState> createScriptCompilerGlobalState() {
	return Common::SharedPtr<IScriptCompilerGlobalState>(new ScriptCompilerGlobalState());
}

Common::SharedPtr<ScriptSet> compileReahLogicFile(Common::ReadStream &stream, uint streamSize, const Common::String &blamePath) {
	Common::SharedPtr<ScriptSet> scriptSet(new ScriptSet());

	compileLogicFile(*scriptSet, stream, streamSize, blamePath, kScriptDialectReah, 0, 0, nullptr);
	return scriptSet;
}

void compileSchizmLogicFile(ScriptSet &scriptSet, uint loadAsRoom, uint fileRoom, Common::ReadStream &stream, uint streamSize, const Common::String &blamePath, IScriptCompilerGlobalState *gs) {
	compileLogicFile(scriptSet, stream, streamSize, blamePath, kScriptDialectSchizm, loadAsRoom, fileRoom, gs);
}

bool checkSchizmLogicForDuplicatedRoom(Common::ReadStream &stream, uint streamSize) {
	LogicUnscrambleStream unscrambleStream(&stream, streamSize);
	TextParser parser(&unscrambleStream);

	TextParserState state;
	Common::String token;

	const char *expectedTokenSequence[] = {"~Room", "0xxh", "~ERoom"};

	for (const char *tokenExpected : expectedTokenSequence) {
		if (!parser.parseToken(token, state))
			return false;

		if (token != tokenExpected)
			return false;
	}

	if (parser.parseToken(token, state))
		return false;

	return true;
}

static bool opArgIsStringIndex(ScriptOps::ScriptOp op) {
	switch (op) {
		case ScriptOps::kDubbing:
		case ScriptOps::kVarName:
		case ScriptOps::kValueName:
		case ScriptOps::kAnimName:
		case ScriptOps::kSoundName:
		case ScriptOps::kCursorName:
		case ScriptOps::kString:
		case ScriptOps::kScreenName:
		case ScriptOps::kGarbage:
			return true;
		default:
			return false;
	}
}

void optimizeScriptSet(ScriptSet &scriptSet) {
	Common::HashMap<uint, uint> functionIndexToUsedFunction;
	Common::HashMap<uint, uint> stringIndexToUsedString;

	Common::Array<Script *> scriptCheckQueue;

	for (const RoomScriptSetMap_t::Node &rsNode : scriptSet.roomScripts) {
		for (const ScreenScriptSetMap_t::Node &ssNode : rsNode._value->screenScripts) {
			if (ssNode._value->entryScript)
				scriptCheckQueue.push_back(ssNode._value->entryScript.get());

			for (const ScriptMap_t::Node &isNode : ssNode._value->interactionScripts)
				scriptCheckQueue.push_back(isNode._value.get());
		}
	}

	// scriptCheckQueue.size() may grow during the loop
	for (uint i = 0; i < scriptCheckQueue.size(); i++) {
		Script *script = scriptCheckQueue[i];

		for (Instruction &instr : script->instrs) {
			if (instr.op == ScriptOps::kCallFunction) {
				uint funcID = instr.arg;

				Common::HashMap<uint, uint>::const_iterator funcIDIt = functionIndexToUsedFunction.find(funcID);

				if (funcIDIt == functionIndexToUsedFunction.end()) {
					uint newIndex = functionIndexToUsedFunction.size();
					functionIndexToUsedFunction[funcID] = newIndex;

					scriptCheckQueue.push_back(scriptSet.functions[funcID].get());
					instr.arg = newIndex;
				} else
					instr.arg = funcIDIt->_value;
			} else if (opArgIsStringIndex(instr.op)) {
				uint strID = instr.arg;

				Common::HashMap<uint, uint>::const_iterator strIndexIt = stringIndexToUsedString.find(strID);

				if (strIndexIt == stringIndexToUsedString.end()) {
					uint newIndex = stringIndexToUsedString.size();
					stringIndexToUsedString[strID] = newIndex;

					instr.arg = newIndex;
				} else
					instr.arg = strIndexIt->_value;
			}
		}
	}

	debug(1, "Optimize result: Fns: %u -> %u  Strs: %u -> %u", static_cast<uint>(scriptSet.functions.size()), static_cast<uint>(functionIndexToUsedFunction.size()), static_cast<uint>(scriptSet.strings.size()), static_cast<uint>(stringIndexToUsedString.size()));

	Common::Array<Common::SharedPtr<Script> > functions;
	Common::Array<Common::String> functionNames;

	functions.resize(functionIndexToUsedFunction.size());

	if (scriptSet.functionNames.size())
		functionNames.resize(functionIndexToUsedFunction.size());

	for (const Common::HashMap<uint, uint>::Node &fnRemapNode : functionIndexToUsedFunction) {
		functions[fnRemapNode._value] = scriptSet.functions[fnRemapNode._key];
		if (functionNames.size())
			functionNames[fnRemapNode._value] = scriptSet.functionNames[fnRemapNode._key];
	}

	Common::Array<Common::String> strings;
	strings.resize(stringIndexToUsedString.size());

	for (const Common::HashMap<uint, uint>::Node &strRemapNode : stringIndexToUsedString)
		strings[strRemapNode._value] = scriptSet.strings[strRemapNode._key];

	scriptSet.functions = Common::move(functions);
	scriptSet.functionNames = Common::move(functionNames);
	scriptSet.strings = Common::move(strings);
}


} // namespace VCruise
