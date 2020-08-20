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

#include "glk/advsys/vm.h"
#include "common/translation.h"
#include "common/ustr.h"

namespace Glk {
namespace AdvSys {

#define TRUE -1

OpcodeMethod VM::_METHODS[0x34] = {
	&VM::opBRT,
	&VM::opBRF,
	&VM::opBR,
	&VM::opT,
	&VM::opNIL,
	&VM::opPUSH,
	&VM::opNOT,
	&VM::opADD,
	&VM::opSUB,
	&VM::opMUL,
	&VM::opDIV,
	&VM::opREM,
	&VM::opBAND,
	&VM::opBOR,
	&VM::opBNOT,
	&VM::opLT,
	&VM::opEQ,
	&VM::opGT,
	&VM::opLIT,
	&VM::opVAR,
	&VM::opGETP,
	&VM::opSETP,
	&VM::opSET,
	&VM::opPRINT,
	&VM::opTERPRI,
	&VM::opPNUMBER,
	&VM::opFINISH,
	&VM::opCHAIN,
	&VM::opABORT,
	&VM::opEXIT,
	&VM::opRETURN,
	&VM::opCALL,
	&VM::opSVAR,
	&VM::opSSET,
	&VM::opSPLIT,
	&VM::opSNLIT,
	&VM::opYORN,
	&VM::opSAVE,
	&VM::opRESTORE,
	&VM::opARG,
	&VM::opASET,
	&VM::opTMP,
	&VM::opTSET,
	&VM::opTSPACE,
	&VM::opCLASS,
	&VM::opMATCH,
	&VM::opPNOUN,
	&VM::opRESTART,
	&VM::opRAND,
	&VM::opRNDMIZE,
	&VM::opSEND,
	&VM::opVOWEL
};

VM::VM(OSystem *syst, const GlkGameDescription &gameDesc) : GlkInterface(syst, gameDesc), Game(),
		_fp(_stack), _pc(0), _status(IN_PROGRESS), _actor(-1), _action(-1), _dObject(-1),
		_ndObjects(-1), _iObject(-1), _wordPtr(nullptr) {
}

ExecutionResult VM::execute(int offset) {
	// Set the code pointer
	_pc = offset;

	// Clear the stack
	_fp.clear();
	_stack.clear();

	// Iterate through the script
	for (_status = IN_PROGRESS; !shouldQuit() && _status == IN_PROGRESS;)
		executeOpcode();

	return _status;
}

void VM::executeOpcode() {
	// Get next opcode
	uint opcode = readCodeByte();

	if (gDebugLevel > 0) {
		Common::String s;
		for (int idx = (int)_stack.size() - 1; idx >= 0; --idx) s += Common::String::format("  %d", _stack[idx]);
		debugC(kDebugScripts, "%.4x - %.2x - %d%s", _pc - 1, opcode, _stack.size(), s.c_str());
	}

	if (opcode >= OP_BRT && opcode <= OP_VOWEL) {
		(this->*_METHODS[(int)opcode - 1])();
	} else if (opcode >= OP_XVAR && opcode < OP_XSET) {
		_stack.top() = getVariable((int)opcode - OP_XVAR);
	} else if (opcode >= OP_XSET && opcode < OP_XPLIT) {
		setVariable((int)opcode - OP_XSET, _stack.top());
	} else if (opcode >= OP_XPLIT && opcode < OP_XNLIT) {
		_stack.top() = (int)opcode - OP_XPLIT;
	} else if (opcode >= OP_XNLIT && (int)opcode < 256) {
		_stack.top() = OP_XNLIT - opcode;
	} else {
		error("Unknown opcode %x at offset %d", opcode, _pc);
	}
}
void VM::opBRT() {
	_pc = _stack.top() ? readCodeWord() : _pc + 2;
}

void VM::opBRF() {
	_pc = !_stack.top() ? readCodeWord() : _pc + 2;
}

void VM::opBR() {
	_pc = readCodeWord();
}

void VM::opT() {
	_stack.top() = TRUE;
}

void VM::opNIL() {
	_stack.top() = NIL;
}

void VM::opPUSH() {
	_stack.push(NIL);
}

void VM::opNOT() {
	_stack.top() = _stack.top() ? NIL : TRUE;
}

void VM::opADD() {
	int v = _stack.pop();
	_stack.top() += v;
}

void VM::opSUB() {
	int v = _stack.pop();
	_stack.top() -= v;
}

void VM::opMUL() {
	int v = _stack.pop();
	_stack.top() *= v;
}

void VM::opDIV() {
	int v = _stack.pop();
	_stack.top() = (v == 0) ? 0 : _stack.top() / v;
}

void VM::opREM() {
	int v = _stack.pop();
	_stack.top() = (v == 0) ? 0 : _stack.top() % v;
}

void VM::opBAND() {
	int v = _stack.pop();
	_stack.top() &= v;
}

void VM::opBOR() {
	int v = _stack.pop();
	_stack.top() |= v;
}

void VM::opBNOT() {
	_stack.top() = ~_stack.top();
}

void VM::opLT() {
	int v = _stack.pop();
	_stack.top() = (_stack.top() < v) ? TRUE : NIL;
}

void VM::opEQ() {
	int v = _stack.pop();
	_stack.top() = (_stack.top() == v) ? TRUE : NIL;
}

void VM::opGT() {
	int v = _stack.pop();
	_stack.top() = (_stack.top() > v) ? TRUE : NIL;
}

void VM::opLIT() {
	_stack.top() = readCodeWord();
}

void VM::opVAR() {
	_stack.top() = getVariable(readCodeWord());
}

void VM::opGETP() {
	int v = _stack.pop();
	_stack.top() = getObjectProperty(_stack.top(), v);
}

void VM::opSETP() {
	int v3 = _stack.pop();
	int v2 = _stack.pop();
	_stack.top() = setObjectProperty(_stack.top(), v2, v3);
}

void VM::opSET() {
	setVariable(readCodeWord(), _stack.top());
}

void VM::opPRINT() {
	Common::String msg = readString(_stack.top());
	print(msg);
}

void VM::opTERPRI() {
	print("\n");
}

void VM::opPNUMBER() {
	print(_stack.top());
}

void VM::opFINISH() {
	_status = FINISH;
}

void VM::opCHAIN() {
	_status = CHAIN;
}

void VM::opABORT() {
	_status = ABORT;
}

void VM::opEXIT() {
	quitGame();
	_status = ABORT;
}

void VM::opRETURN() {
	if (_fp == 0) {
		_status = CHAIN;
	} else {
		int val = _stack.top();
		_stack.resize(_fp);
		_fp = _stack.pop();
		_pc = _stack.pop();

		int argsSize = _stack.pop();
		_stack.resize(_stack.size() - argsSize);
		_stack.top() = val;
	}
}

void VM::opCALL() {
	int argsSize = readCodeByte();

	_stack.push(argsSize);
	_stack.push(_pc);
	_stack.push(_fp);

	_fp.set();
	_pc = getActionField(_fp[_fp[FP_ARGS_SIZE] + FP_ARGS], A_CODE);
}

void VM::opSVAR() {
	_stack.top() = getVariable(readCodeByte());
}

void VM::opSSET() {
	setVariable(readCodeByte(), _stack.top());
}

void VM::opSPLIT() {
	_stack.top() = readCodeByte();
}

void VM::opSNLIT() {
	_stack.top() = readCodeByte();
}

void VM::opYORN() {
	Common::String line = readLine();
	_stack.top() = !line.empty() && (line[0] == 'Y' || line[0] == 'y') ? TRUE : NIL;
}

void VM::opSAVE() {
	if (saveGame().getCode() != Common::kNoError)
		print(_("Sorry, the savegame couldn't be created"));
}

void VM::opRESTORE() {
	if (loadGame().getCode() != Common::kNoError)
		print(_("Sorry, the savegame couldn't be restored"));
}

void VM::opARG() {
	int argNum = readCodeByte();
	if (argNum >= _fp[FP_ARGS_SIZE])
		error("Invalid argument number");
	_stack.top() = _fp[argNum + FP_ARGS];
}

void VM::opASET() {
	int argNum = readCodeByte();
	if (argNum >= _fp[FP_ARGS_SIZE])
		error("Invalid argument number");
	_fp[argNum + FP_ARGS] = _stack.top();
}

void VM::opTMP() {
	int val = readCodeByte();
	_stack.top() = _fp[-val - 1];
}

void VM::opTSET() {
	int val = readCodeByte();
	_fp[-val - 1] = _stack.top();
}

void VM::opTSPACE() {
	_stack.allocate(readCodeByte());
}

void VM::opCLASS() {
	_stack.top() = getObjectField(_stack.top(), O_CLASS);
}

void VM::opMATCH() {
	int idx = _stack.pop() - 1;
	_stack.top() = match(_stack.top(), _nouns[idx]._noun, _nouns[idx]._adjective) ? TRUE : NIL;
}

void VM::opPNOUN() {
	int noun = _stack.top();
	Common::String str;

	// Add the adjectives
	bool space = false;
	for (const AdjectiveEntry *aPtr = &_adjectiveList[noun - 1]; aPtr->_list; ++aPtr, space = true) {
		if (space)
			str += " ";
		str += _words[aPtr->_word]._text;
	}

	// Add the noun
	if (space)
		str += " ";
	str += _words[_nouns[noun - 1]._num]._text;

	print(str);
}

void VM::opRESTART() {
	restart();
}

void VM::opRAND() {
	_stack.top() = getRandomNumber(_stack.top());
}

void VM::opRNDMIZE() {
	// No implementation
}

void VM::opSEND() {
	int argsSize = readCodeByte();
	_stack.push(argsSize);
	_stack.push(_pc);
	_stack.push(_fp);
	_fp.set();

	int val = _fp[_fp[FP_ARGS_SIZE] + FP_ARGS];
	if (val)
		val = getObjectField(val, O_CLASS);
	else
		val = _fp[_fp[FP_ARGS_SIZE] + FP_ARGS - 1];

	if (val && (val = getObjectProperty(val, _fp[_fp[FP_ARGS_SIZE] + FP_ARGS - 2])) != 0) {
		_pc = getActionField(val, A_CODE);
	} else {
		// Return NIL if there's no action for the given message
		opRETURN();
	}
}

void VM::opVOWEL() {
	// No implementation
}

bool VM::getInput() {
	if (!parseInput())
		return false;

	setVariable(V_ACTOR, _actor);
	setVariable(V_ACTION, _action);
	setVariable(V_DOBJECT, _dObject);
	setVariable(V_NDOBJECTS, _ndObjects);
	setVariable(V_IOBJECT, _iObject);
	return true;
}

bool VM::nextCommand() {
	if (getVariable(V_NDOBJECTS) > 1) {
		setVariable(V_ACTOR, _actor);
		setVariable(V_ACTION, _action);
		setVariable(V_DOBJECT, getVariable(V_DOBJECT) + 1);
		setVariable(V_NDOBJECTS, getVariable(V_NDOBJECTS) - 1);
		setVariable(V_IOBJECT, _iObject);
		return true;
	} else {
		return false;
	}
}

bool VM::parseInput() {
	int noun1 = 0, cnt1 = 0, noun2 = 0, cnt2 = 0;
	int preposition = 0, flags = 0;

	// Initialize the parser result fields
	_actor = _action = _dObject = _iObject = 0;
	_ndObjects = 0;
	_nouns.clear();
	_adjectiveList.clear();
	_adjectiveList.reserve(20);

	// Get the input line
	if (!getLine())
		return false;

	// Check for actor
	WordType wordType = getWordType(*_wordPtr);
	if (wordType == WT_ADJECTIVE || wordType == WT_NOUN) {
		if (!(_actor = getNoun()))
			return false;
		flags |= A_ACTOR;
	}

	// Check for a verb
	if (!getVerb())
		return false;

	// Get direct object, preposition, and/or indirect object
	if (_wordPtr != _words.end()) {
		// Get any direct objects
		noun1 = _adjectiveList.size() + 1;
		for (;;) {
			// Get the next direct object
			if (!getNoun())
				return false;
			++cnt1;

			// Check for more direct objects
			if (_wordPtr == _words.end() || getWordType(*_wordPtr) != WT_CONJUNCTION)
				break;
			++_wordPtr;
		}

		// Get any reposition and indirect object
		if (_wordPtr != _words.end()) {
			// Get the preposition
			if (getWordType(*_wordPtr) == WT_PREPOSITION)
				preposition = *_wordPtr++;

			// Get the indirect object
			noun2 = _adjectiveList.size() + 1;
			for (;;) {
				// Get the indirect object
				if (!getNoun())
					return false;
				++cnt2;

				// Check for more objects
				if (_wordPtr == _words.end() || getWordType(*_wordPtr) != WT_CONJUNCTION)
					break;
				++_wordPtr;
			}
		}

		// Ensure we're at the end of the input line
		if (_wordPtr != _words.end()) {
			parseError();
			return false;
		}
	}

	// Setup resulting properties
	if (preposition) {
		if (cnt2 > 1) {
			parseError();
			return false;
		}

		_dObject = noun1;
		_ndObjects = cnt1;
		_iObject = noun2;
	} else if (noun2) {
		if (cnt1 > 1) {
			parseError();
			return false;
		}

		preposition = findWord("to");
		_dObject = noun2;
		_ndObjects = cnt2;
		_iObject = noun1;
	} else {
		_dObject = noun1;
		_ndObjects = cnt1;
	}

	// Setup the flags for the action lookup
	if (_dObject)
		flags |= A_DOBJECT;
	if (_iObject)
		flags |= A_IOBJECT;

	// Find the action
	if (!(_action = findAction(_verbs, preposition, flags))) {
		parseError();
		return false;
	}

	return true;
}

bool VM::getLine() {
	// Let the user type in an input line
	Common::String line = readLine();
	if (shouldQuit())
		return false;

	skipSpaces(line);
	if (line.empty()) {
		print(_("Speak up! I can't hear you!\n"));
		return false;
	}

	// Get the words of the line
	_words.clear();
	while (!line.empty()) {
		if (!getWord(line))
			return false;
	}

	_wordPtr = _words.begin();
	return true;
}

bool VM::getWord(Common::String &line) {
	// Find the end of the word
	const char *wordP = line.c_str();
	for (; *wordP && !isWhitespace(*wordP); ++wordP) {}

	// Copy out the next word
	InputWord iw;
	iw._text = Common::String(line.c_str(), wordP);
	iw._text.toLowercase();

	// Remove the word from the line
	line = Common::String(wordP);
	skipSpaces(line);

	// Look up the word
	iw._number = findWord(iw._text);

	if (iw._number) {
		_words.push_back(iw);
		return true;
	} else {
		Common::U32String msg = Common::U32String::format(_("I don't know the word \"%s\".\n"), iw._text.c_str());
		print(msg);
		return false;
	}
}

uint VM::getNoun() {
	// Skip over optional article if present
	if (_wordPtr != _words.end() && getWordType(*_wordPtr) == WT_ARTICLE)
		++_wordPtr;

	// Get optional adjectives
	uint alStart = _adjectiveList.size();
	while (_wordPtr != _words.end() && getWordType(*_wordPtr) == WT_ADJECTIVE) {
		AdjectiveEntry ae;
		ae._list = *_wordPtr++;
		ae._word = _wordPtr - _words.begin() - 1;
		_adjectiveList.push_back(ae);
	}
	_adjectiveList.push_back(AdjectiveEntry());
	assert(_adjectiveList.size() <= 20);

	if (_wordPtr == _words.end() || getWordType(*_wordPtr) != WT_NOUN) {
		parseError();
		return NIL;
	}

	// Add a noun entry to the list
	Noun n;
	n._adjective = &_adjectiveList[alStart];
	n._noun = *_wordPtr++;
	n._num = _wordPtr - _words.begin() - 1;
	_nouns.push_back(n);

	return _nouns.size();
}

bool VM::getVerb() {
	_verbs.clear();

	if (_wordPtr == _words.end() || getWordType(*_wordPtr) != WT_VERB) {
		parseError();
		return false;
	}

	_verbs.push_back(*_wordPtr++);

	// Check for a word following the verb
	if (_wordPtr < _words.end()) {
		_verbs.push_back(*_wordPtr);

		if (checkVerb(_verbs)) {
			++_wordPtr;
		} else {
			_verbs.pop_back();
			_verbs.push_back(_words.back());

			if (checkVerb(_verbs)) {
				_words.pop_back();
			} else {
				_verbs.pop_back();

				if (!checkVerb(_verbs)) {
					parseError();
					return false;
				}
			}
		}
	}

	return true;
}

bool VM::match(int obj, int noun, const VM::AdjectiveEntry *adjectives) {
	if (!hasNoun(obj, noun))
		return false;

	for (const VM::AdjectiveEntry *adjPtr = adjectives; adjPtr->_list; ++adjPtr) {
		if (!hasAdjective(obj, adjPtr->_list))
			return false;
	}

	return true;
}

void VM::parseError() {
	print(_("I don't understand.\n"));
}

bool VM::isWhitespace(char c) {
	return c == ' ' || c == ',' || c == '.';
}

bool VM::skipSpaces(Common::String &str) {
	while (!str.empty() && isWhitespace(str[0]))
		str.deleteChar(0);

	return !str.empty();
}

} // End of namespace AdvSys
} // End of namespace Glk
