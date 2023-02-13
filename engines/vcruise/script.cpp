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

#include "common/stream.h"
#include "common/hash-str.h"

#include "vcruise/script.h"
#include "vcruise/textparser.h"


namespace VCruise {

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
	Common::Array<ProtoInstruction> instrs;

	void reset();
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
	ScriptCompiler(TextParser &parser, const Common::String &blamePath);

	void compileRoomScriptSet(ScriptSet *ss);

private:
	bool parseNumber(const Common::String &token, uint32 &outNumber) const;
	static bool parseDecNumber(const Common::String &token, uint start, uint32 &outNumber);
	static bool parseHexNumber(const Common::String &token, uint start, uint32 &outNumber);
	void expectNumber(uint32 &outNumber);

	void compileRoomScriptSet(RoomScriptSet *rss);
	void compileScreenScriptSet(ScreenScriptSet *sss);
	bool compileInstructionToken(ProtoScript &script, const Common::String &token);

	void codeGenScript(ProtoScript &protoScript, Script &script);

	uint indexString(const Common::String &str);

	enum NumberParsingMode {
		kNumberParsingDec,
		kNumberParsingHex,
	};

	TextParser &_parser;
	NumberParsingMode _numberParsingMode;
	const Common::String _blamePath;

	Common::HashMap<Common::String, uint> _stringToIndex;
	Common::Array<Common::String> _strings;
};

ScriptCompiler::ScriptCompiler(TextParser &parser, const Common::String &blamePath) : _numberParsingMode(kNumberParsingHex), _parser(parser), _blamePath(blamePath) {
}

bool ScriptCompiler::parseNumber(const Common::String &token, uint32 &outNumber) const {
	if (token.size() == 0)
		return false;

	if (token[0] == 'd')
		return parseDecNumber(token, 1, outNumber);

	if (token[0] == '0') {
		switch (_numberParsingMode) {
		case kNumberParsingDec:
			return parseDecNumber(token, 0, outNumber);
		case kNumberParsingHex:
			return parseHexNumber(token, 0, outNumber);
		default:
			error("Unknown number parsing mode");
			return false;
		}
	}

	return false;
}

bool ScriptCompiler::parseDecNumber(const Common::String &token, uint start, uint32 &outNumber) {
	if (start == token.size())
		return false;

	uint32 num = 0;
	for (uint i = start; i < token.size(); i++) {
		char c = token[i];
		uint32 digit = 0;
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else
			return false;

		num = num * 10u + digit;
	}

	outNumber = num;
	return true;
}

bool ScriptCompiler::parseHexNumber(const Common::String &token, uint start, uint32 &outNumber) {
	if (start == token.size())
		return false;

	uint32 num = 0;
	for (uint i = start; i < token.size(); i++) {
		char c = token[i];
		uint32 digit = 0;
		if (c >= '0' && c <= '9')
			digit = c - '0';
		else if (c >= 'a' && c <= 'f')
			digit = (c - 'a') + 0xa;
		else
			return false;

		num = num * 16u + digit;
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

void ScriptCompiler::compileRoomScriptSet(ScriptSet *ss) {
	Common::SharedPtr<RoomScriptSet> roomScript;
	uint roomID = 0;

	TextParserState state;
	Common::String token;
	while (_parser.parseToken(token, state)) {
		if (token == "~ROOM") {
			if (roomScript)
				error("Error compiling script at line %i col %i: Encountered ~ROOM without ~EROOM", static_cast<int>(state._lineNum), static_cast<int>(state._col));
			roomScript.reset(new RoomScriptSet());

			uint32 roomNumber = 0;
			expectNumber(roomNumber);

			ss->roomScripts[roomNumber] = roomScript;

			compileRoomScriptSet(roomScript.get());
		} else {
			error("Error compiling script at line %i col %i: Expected ~ROOM and found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), token.c_str());
		}
	}

	ss->strings = Common::move(_strings);
}

void ScriptCompiler::compileRoomScriptSet(RoomScriptSet *rss) {
	TextParserState state;
	Common::String token;
	while (_parser.parseToken(token, state)) {
		if (token == "~EROOM") {
			return;
		} else if (token == "~SCR") {
			uint32 screenNumber = 0;
			expectNumber(screenNumber);

			Common::SharedPtr<ScreenScriptSet> sss(new ScreenScriptSet());
			compileScreenScriptSet(sss.get());

			rss->screenScripts[screenNumber] = sss;
		} else {
			error("Error compiling script at line %i col %i: Expected ~EROOM or ~SCR and found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), token.c_str());
		}
	}

	error("Error compiling script: Room wasn't terminated");
}

void ScriptCompiler::compileScreenScriptSet(ScreenScriptSet *sss) {
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

			sss->interactionScripts[interactionNumber] = currentScript;
		} else if (compileInstructionToken(protoScript, token)) {
			// Nothing
		} else {
			error("Error compiling script at line %i col %i: Expected ~EROOM or ~SCR or ~* or instruction but found '%s'", static_cast<int>(state._lineNum), static_cast<int>(state._col), token.c_str());
		}
	}
}

static ScriptNamedInstruction g_namedInstructions[] = {
	{"rotate", ProtoOp::kProtoOpScript, ScriptOps::kRotate},
	{"angle", ProtoOp::kProtoOpScript, ScriptOps::kAngle},
	{"angleG@", ProtoOp::kProtoOpScript, ScriptOps::kAngleGGet},
	{"speed", ProtoOp::kProtoOpScript, ScriptOps::kSpeed},
	{"sanimL", ProtoOp::kProtoOpScript, ScriptOps::kSAnimL},
	{"changeL", ProtoOp::kProtoOpScript, ScriptOps::kChangeL},
	{"animR", ProtoOp::kProtoOpScript, ScriptOps::kAnimR},
	{"animF", ProtoOp::kProtoOpScript, ScriptOps::kAnimF},
	{"animN", ProtoOp::kProtoOpScript, ScriptOps::kAnimN},
	{"animG", ProtoOp::kProtoOpScript, ScriptOps::kAnimG},
	{"animS", ProtoOp::kProtoOpScript, ScriptOps::kAnimS},
	{"anim", ProtoOp::kProtoOpScript, ScriptOps::kAnim},
	{"static", ProtoOp::kProtoOpScript, ScriptOps::kStatic},
	{"yes@", ProtoOp::kProtoOpScript, ScriptOps::kVarLoad},
	{"yes!", ProtoOp::kProtoOpScript, ScriptOps::kVarStore},
	{"cursor!", ProtoOp::kProtoOpScript, ScriptOps::kSetCursor},
	{"room!", ProtoOp::kProtoOpScript, ScriptOps::kSetRoom},
	{"lmb", ProtoOp::kProtoOpScript, ScriptOps::kLMB},
	{"lmb1", ProtoOp::kProtoOpScript, ScriptOps::kLMB1},
	{"volumeDn4", ProtoOp::kProtoOpScript, ScriptOps::kVolumeDn4},
	{"volumeUp3", ProtoOp::kProtoOpScript, ScriptOps::kVolumeUp3},
	{"rnd", ProtoOp::kProtoOpScript, ScriptOps::kRandom},
	{"drop", ProtoOp::kProtoOpScript, ScriptOps::kDrop},
	{"dup", ProtoOp::kProtoOpScript, ScriptOps::kDup},
	{"say3", ProtoOp::kProtoOpScript, ScriptOps::kSay3},
	{"setTimer", ProtoOp::kProtoOpScript, ScriptOps::kSetTimer},
	{"lo!", ProtoOp::kProtoOpScript, ScriptOps::kLoSet},
	{"lo@", ProtoOp::kProtoOpScript, ScriptOps::kLoGet},
	{"hi!", ProtoOp::kProtoOpScript, ScriptOps::kHiSet},
	{"hi@", ProtoOp::kProtoOpScript, ScriptOps::kHiGet},

	{"and", ProtoOp::kProtoOpScript, ScriptOps::kAnd},
	{"or", ProtoOp::kProtoOpScript, ScriptOps::kOr},
	{"not", ProtoOp::kProtoOpScript, ScriptOps::kNot},
	{"=", ProtoOp::kProtoOpScript, ScriptOps::kCmpEq},

	{"bit@", ProtoOp::kProtoOpScript, ScriptOps::kBitLoad},
	{"bit0!", ProtoOp::kProtoOpScript, ScriptOps::kBitSet0},
	{"bit1!", ProtoOp::kProtoOpScript, ScriptOps::kBitSet1},

	{"soundS1", ProtoOp::kProtoOpScript, ScriptOps::kSoundS1},
	{"soundL2", ProtoOp::kProtoOpScript, ScriptOps::kSoundL2},
	{"music", ProtoOp::kProtoOpScript, ScriptOps::kMusic},
	{"musicUp", ProtoOp::kProtoOpScript, ScriptOps::kMusicUp},
	{"musicDn", ProtoOp::kProtoOpScript, ScriptOps::kMusicDn},

	{"parm1", ProtoOp::kProtoOpScript, ScriptOps::kParm1},
	{"parm2", ProtoOp::kProtoOpScript, ScriptOps::kParm2},
	{"parm3", ProtoOp::kProtoOpScript, ScriptOps::kParm3},
	{"parmG", ProtoOp::kProtoOpScript, ScriptOps::kParmG},

	{"disc1", ProtoOp::kProtoOpScript, ScriptOps::kDisc1},
	{"disc2", ProtoOp::kProtoOpScript, ScriptOps::kDisc2},
	{"disc3", ProtoOp::kProtoOpScript, ScriptOps::kDisc3},

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
};

bool ScriptCompiler::compileInstructionToken(ProtoScript &script, const Common::String &token) {
	uint32 number = 0;
	if (parseNumber(token, number)) {
		script.instrs.push_back(ProtoInstruction(ScriptOps::kNumber, number));
		return true;
	}

	if (token.size() >= 1 && token[0] == ':') {
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

		script.instrs.push_back(ProtoInstruction(ScriptOps::kAnimName, indexString(token.substr(1))));
		return true;
	}

	if (token.size() >= 2 && token[0] == '_') {
		script.instrs.push_back(ProtoInstruction(ScriptOps::kSoundName, indexString(token.substr(1))));
		return true;
	}

	if (token.size() >= 5 && token[0] == 'C' && token[1] == 'U' && token[2] == 'R' && token[3] == '_') {
		script.instrs.push_back(ProtoInstruction(ScriptOps::kCursorName, indexString(token)));
		return true;
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

	for (const ScriptNamedInstruction &namedInstr : g_namedInstructions) {
		if (token == namedInstr.str) {
			script.instrs.push_back(ProtoInstruction(namedInstr.protoOp, namedInstr.op, 0));
			return true;
		}
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
	// replace Else, Case, EndIf, EndSwitch, and Default instructions with Label and JumpToLabel.
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
		default:
			error("Internal error: Unhandled proto-op");
			break;
		}
	}

	if (controlFlowStack.size() > 0)
		error("Error in codegen: Unterminated flow control construct");

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
				instrs2.push_back(ProtoInstruction(kProtoOpJumpToLabel, ScriptOps::kInvalid, caseDef.value));
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
		uint index = _strings.size();
		_stringToIndex[str] = index;
		_strings.push_back(str);
		return index;
	}

	return it->_value;
}

Common::SharedPtr<ScriptSet> compileLogicFile(Common::ReadStream &stream, uint streamSize, const Common::String &blamePath) {
	LogicUnscrambleStream unscrambleStream(&stream, streamSize);
	TextParser parser(&unscrambleStream);

	Common::SharedPtr<ScriptSet> scriptSet(new ScriptSet());

	ScriptCompiler compiler(parser, blamePath);

	compiler.compileRoomScriptSet(scriptSet.get());

	return scriptSet;
}

} // namespace VCruise
