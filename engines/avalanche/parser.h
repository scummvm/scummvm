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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_PARSER_H
#define AVALANCHE_PARSER_H

#include "common/events.h"
#include "common/scummsys.h"
#include "common/str.h"


namespace Avalanche {
class AvalancheEngine;

class Parser {
public:
	enum VerbCode {
		kVerbCodeExam = 1,     kVerbCodeOpen = 2,    kVerbCodePause = 3,   kVerbCodeGet = 4,       kVerbCodeDrop = 5,
		kVerbCodeInv = 6,      kVerbCodeTalk = 7,    kVerbCodeGive = 8,    kVerbCodeDrink = 9,     kVerbCodeLoad = 10,
		kVerbCodeSave = 11,    kVerbCodePay = 12,    kVerbCodeLook = 13,   kVerbCodeBreak = 14,    kVerbCodeQuit = 15,
		kVerbCodeSit = 16,     kVerbCodeStand = 17,  kVerbCodeGo = 18,     kVerbCodeInfo = 19,     kVerbCodeUndress = 20,
		kVerbCodeWear = 21,    kVerbCodePlay = 22,   kVerbCodeRing = 23,   kVerbCodeHelp = 24,     kVerbCodeLarrypass = 25,
		kVerbCodePhaon = 26,   kVerbCodeBoss = 27,   kVerbCodePee = 28,    kVerbCodeCheat = 29,    kVerbCodeMagic = 30,
		kVerbCodeRestart = 31, kVerbCodeEat = 32,    kVerbCodeListen = 33, kVerbCodeBuy = 34,      kVerbCodeAttack = 35,
		kVerbCodePasswd = 36,  kVerbCodeDir = 37,    kVerbCodeDie = 38,    kVerbCodeScore = 39,    kVerbCodePut = 40,
		kVerbCodeKiss = 41,    kVerbCodeClimb = 42,  kVerbCodeJump = 43,   kVerbCodeHiscores = 44, kVerbCodeWake = 45,
		kVerbCodeHello = 46,   kVerbCodeThanks = 47, kVerbCodeSmartAlec = 249, kVerbCodeExpletive = 253
	};

	static const byte kPardon = 254; // Didn't understand / wasn't given.
	static const int16 kParserWordsNum = 277; // How many words does the parser know?
	static const byte kNothing = 250;
	static const byte kMoved = 0; // This word was moved. (Usually because it was the subject of conversation.)
	static const int16 kFirstPassword = 88; // words[kFirstPassword] should equal "TIROS".

	struct VocabEntry {
		byte _number;
		Common::String _word;

		void init(byte number, Common::String word) {
			_number = number;
			_word = word;
		}
	};

	VocabEntry _vocabulary[kParserWordsNum];

	Common::String _realWords[11];
	byte _verb, _person, _thing;
	bool _polite;
	Common::String _inputText; // Original name: current
	Common::String _inputTextBackup;
	byte _inputTextPos; // Original name: curpos
	bool _quote; // 66 or 99 next?
	byte _leftMargin;
	bool _cursorState;

	Parser(AvalancheEngine *vm);

	void init();
	void parse();
	void doThat();
	void verbOpt(byte verb, Common::String &answer, char &ansKey);
	void drink();

	void handleInputText(const Common::Event &event);
	void handleBackspace();
	void handleReturn();
	void handleFunctionKey(const Common::Event &event);
	void plotText();
	void cursorOn();
	void cursorOff();
	void tryDropdown(); // This asks the parsekey proc in Dropdown if it knows it.
	int16 getPos(const Common::String &crit, const Common::String &src); // Returns the index of the first appearance of crit in src.

private:
	AvalancheEngine *_vm;

	struct RankType {
		uint16 _score;
		Common::String _title;
	};

	Common::String _thats;
	byte _thing2;

	byte wordNum(Common::String word);
	void replace(Common::String oldChars, byte newChar);

	Common::String rank();
	Common::String totalTime();

	void clearWords();
	void cheatParse(Common::String codes);
	void stripPunctuation(Common::String &word); // Strips punctuation from word.
	void displayWhat(byte target, bool animate, bool &ambiguous); // << It's an adjective!
	bool doPronouns();
	void properNouns();
	void lookAround(); // This is called when you say "look".
	void sayIt();  // This makes Avalot say the response.
	void openDoor();
	void storeInterrogation(byte interrogation);
	void examineObject(); // Examine a standard object-thing
	bool isPersonHere();
	void exampers();
	bool isHolding();
	void openBox(bool isOpening);
	void examine();
	void inventory();
	void swallow();
	void peopleInRoom(); // This lists the other people in the room.
	void silly();
	void putProc();  // Called when you call kVerbCodeput.
	void notInOrder();
	void goToCauldron();
	bool giveToSpludwick(); // The result of this fn is whether or not he says "Hey, thanks!".
	void cardiffClimbing();
	void already();
	void standUp(); // Called when you ask Avvy to stand.
	void getProc(char thing);
	void giveGeidaTheLute();
	void playHarp();
	void winSequence();
	Common::String personSpeaks();
	void heyThanks();

	void drawCursor();
	void wipeText();

};

} // End of namespace Avalanche

#endif // AVALANCHE_PARSER_H
