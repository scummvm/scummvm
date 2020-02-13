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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#include "avalanche/avalanche.h"
#include "avalanche/parser.h"
#include "avalanche/nim.h"

#include "gui/saveload.h"
#include "common/system.h"
#include "common/translation.h"

namespace Avalanche {

const char *Parser::kCopyright = "1995";
const char *Parser::kVersionNum = "1.30";

Parser::Parser(AvalancheEngine *vm) {
	_vm = vm;

	_verb = kVerbCodePardon;
	_thing = kPardon;
	_person = kPeopleNone;
	_polite = false;
	_inputTextPos = 0;
	_quote = false;
	_cursorState = false;
	_weirdWord = false;
	_wearing = kNothing;
	_thing2 = 0;
	_sworeNum = 0;
	_alcoholLevel = 0;

	_boughtOnion = false;
}

void Parser::init() {
	if (!_inputText.empty())
		_inputText.clear();
	_inputTextPos = 0;

	_weirdWord = false;

	// Initailaze the vocabulary.
	// Verbs: 1-49
	_vocabulary[0].init(1, "EXAMINE");
	_vocabulary[1].init(1, "READ");
	_vocabulary[2].init(1, "XAM");
	_vocabulary[3].init(2, "OPEN");
	_vocabulary[4].init(2, "LEAVE");
	_vocabulary[5].init(2, "UNLOCK");
	_vocabulary[6].init(3, "PAUSE");
	_vocabulary[7].init(47, "TA"); // Early to avoid Take and Talk.
	_vocabulary[8].init(4, "TAKE");
	_vocabulary[9].init(4, "GET");
	_vocabulary[10].init(4, "PICK");
	_vocabulary[11].init(5, "DROP");
	_vocabulary[12].init(6, "INVENTORY");
	_vocabulary[13].init(7, "TALK");
	_vocabulary[14].init(7, "SAY");
	_vocabulary[15].init(7, "ASK");
	_vocabulary[16].init(8, "GIVE");
	_vocabulary[17].init(9, "DRINK");
	_vocabulary[18].init(9, "IMBIBE");
	_vocabulary[19].init(9, "DRAIN");
	_vocabulary[20].init(10, "LOAD");
	_vocabulary[21].init(10, "RESTORE");
	_vocabulary[22].init(11, "SAVE");
	_vocabulary[23].init(12, "BRIBE");
	_vocabulary[24].init(12, "PAY");
	_vocabulary[25].init(13, "LOOK");
	_vocabulary[26].init(14, "BREAK");
	_vocabulary[27].init(15, "QUIT");
	_vocabulary[28].init(15, "EXIT");
	_vocabulary[29].init(16, "SIT");
	_vocabulary[30].init(16, "SLEEP");
	_vocabulary[31].init(17, "STAND");

	_vocabulary[32].init(18, "GO");
	_vocabulary[33].init(19, "INFO");
	_vocabulary[34].init(20, "UNDRESS");
	_vocabulary[35].init(20, "DOFF");
	_vocabulary[36].init(21, "DRESS");
	_vocabulary[37].init(21, "WEAR");
	_vocabulary[38].init(21, "DON");
	_vocabulary[39].init(22, "PLAY");
	_vocabulary[40].init(22, "STRUM");
	_vocabulary[41].init(23, "RING");
	_vocabulary[42].init(24, "HELP");
	_vocabulary[43].init(25, "KENDAL");
	_vocabulary[44].init(26, "CAPYBARA");
	_vocabulary[45].init(27, "BOSS");
	_vocabulary[46].init(255, "NINET"); // block for NINETY
	_vocabulary[47].init(28, "URINATE");
	_vocabulary[48].init(28, "MINGITE");
	_vocabulary[49].init(29, "NINETY");
	_vocabulary[50].init(30, "ABRACADABRA");
	_vocabulary[51].init(30, "PLUGH");
	_vocabulary[52].init(30, "XYZZY");
	_vocabulary[53].init(30, "HOCUS");
	_vocabulary[54].init(30, "POCUS");
	_vocabulary[55].init(30, "IZZY");
	_vocabulary[56].init(30, "WIZZY");
	_vocabulary[57].init(30, "PLOVER");
	_vocabulary[58].init(30, "MELENKURION");
	_vocabulary[59].init(30, "ZORTON");
	_vocabulary[60].init(30, "BLERBI");
	_vocabulary[61].init(30, "THURB");
	_vocabulary[62].init(30, "SNOEZE");
	_vocabulary[63].init(30, "SAMOHT");
	_vocabulary[64].init(30, "NOSIDE");
	_vocabulary[65].init(30, "PHUGGG");
	_vocabulary[66].init(30, "KNERL");
	_vocabulary[67].init(30, "MAGIC");
	_vocabulary[68].init(30, "KLAETU");
	_vocabulary[69].init(30, "VODEL");
	_vocabulary[70].init(30, "BONESCROLLS");
	_vocabulary[71].init(30, "RADOF");

	_vocabulary[72].init(31, "RESTART");
	_vocabulary[73].init(32, "SWALLOW");
	_vocabulary[74].init(32, "EAT");
	_vocabulary[75].init(33, "LISTEN");
	_vocabulary[76].init(33, "HEAR");
	_vocabulary[77].init(34, "BUY");
	_vocabulary[78].init(34, "PURCHASE");
	_vocabulary[79].init(34, "ORDER");
	_vocabulary[80].init(34, "DEMAND");
	_vocabulary[81].init(35, "ATTACK");
	_vocabulary[82].init(35, "HIT");
	_vocabulary[83].init(35, "KILL");
	_vocabulary[84].init(35, "PUNCH");
	_vocabulary[85].init(35, "KICK");
	_vocabulary[86].init(35, "SHOOT");
	_vocabulary[87].init(35, "FIRE");

	// Passwords: 36
	_vocabulary[88].init(36, "TIROS");
	_vocabulary[89].init(36, "WORDY");
	_vocabulary[90].init(36, "STACK");
	_vocabulary[91].init(36, "SHADOW");
	_vocabulary[92].init(36, "OWL");
	_vocabulary[93].init(36, "ACORN");
	_vocabulary[94].init(36, "DOMESDAY");
	_vocabulary[95].init(36, "FLOPPY");
	_vocabulary[96].init(36, "DIODE");
	_vocabulary[97].init(36, "FIELD");
	_vocabulary[98].init(36, "COWSLIP");
	_vocabulary[99].init(36, "OSBYTE");
	_vocabulary[100].init(36, "OSCLI");
	_vocabulary[101].init(36, "TIMBER");
	_vocabulary[102].init(36, "ADVAL");
	_vocabulary[103].init(36, "NEUTRON");
	_vocabulary[104].init(36, "POSITRON");
	_vocabulary[105].init(36, "ELECTRON");
	_vocabulary[106].init(36, "CIRCUIT");
	_vocabulary[107].init(36, "AURUM");
	_vocabulary[108].init(36, "PETRIFY");
	_vocabulary[109].init(36, "EBBY");
	_vocabulary[110].init(36, "CATAPULT");
	_vocabulary[111].init(36, "GAMERS");
	_vocabulary[112].init(36, "FUDGE");
	_vocabulary[113].init(36, "CANDLE");
	_vocabulary[114].init(36, "BEEB");
	_vocabulary[115].init(36, "MICRO");
	_vocabulary[116].init(36, "SESAME");
	_vocabulary[117].init(36, "LORDSHIP");

	_vocabulary[118].init(37, "DIR");
	_vocabulary[119].init(37, "LS");
	_vocabulary[120].init(38, "DIE");
	_vocabulary[121].init(39, "SCORE");
	_vocabulary[122].init(40, "PUT");
	_vocabulary[123].init(40, "INSERT");
	_vocabulary[124].init(41, "KISS");
	_vocabulary[125].init(41, "SNOG");
	_vocabulary[126].init(41, "CUDDLE");
	_vocabulary[127].init(42, "CLIMB");
	_vocabulary[128].init(42, "CLAMBER");
	_vocabulary[129].init(43, "JUMP");
	_vocabulary[130].init(44, "HIGHSCORES");
	_vocabulary[131].init(44, "HISCORES");
	_vocabulary[132].init(45, "WAKEN");
	_vocabulary[133].init(45, "AWAKEN");
	_vocabulary[134].init(46, "HELLO");
	_vocabulary[135].init(46, "HI");
	_vocabulary[136].init(46, "YO");
	_vocabulary[137].init(47, "THANKS"); // = 47, "ta", which was defined earlier.

	// Nouns - Objects: 50-100
	_vocabulary[138].init(50, "WINE");
	_vocabulary[139].init(50, "BOOZE");
	_vocabulary[140].init(50, "NASTY");
	_vocabulary[141].init(50, "VINEGAR");
	_vocabulary[142].init(51, "MONEYBAG");
	_vocabulary[143].init(51, "BAG");
	_vocabulary[144].init(51, "CASH");
	_vocabulary[145].init(51, "DOSH");
	_vocabulary[146].init(51, "WALLET");
	_vocabulary[147].init(52, "BODKIN");
	_vocabulary[148].init(52, "DAGGER");
	_vocabulary[149].init(53, "POTION");
	_vocabulary[150].init(54, "CHASTITY");
	_vocabulary[151].init(54, "BELT");
	_vocabulary[152].init(55, "BOLT");
	_vocabulary[153].init(55, "ARROW");
	_vocabulary[154].init(55, "DART");
	_vocabulary[155].init(56, "CROSSBOW");
	_vocabulary[156].init(56, "BOW");
	_vocabulary[157].init(57, "LUTE");
	_vocabulary[158].init(58, "PILGRIM");
	_vocabulary[159].init(58, "BADGE");
	_vocabulary[160].init(59, "MUSHROOMS");
	_vocabulary[161].init(59, "TOADSTOOLS");
	_vocabulary[162].init(60, "KEY");
	_vocabulary[163].init(61, "BELL");
	_vocabulary[164].init(62, "PRESCRIPT");
	_vocabulary[165].init(62, "SCROLL");
	_vocabulary[166].init(62, "MESSAGE");
	_vocabulary[167].init(63, "PEN");
	_vocabulary[168].init(63, "QUILL");
	_vocabulary[169].init(64, "INK");
	_vocabulary[170].init(64, "INKPOT");
	_vocabulary[171].init(65, "CLOTHES");
	_vocabulary[172].init(66, "HABIT");
	_vocabulary[173].init(66, "DISGUISE");
	_vocabulary[174].init(67, "ONION");

	_vocabulary[175].init(99, "PASSWORD");

	// Objects from Also are placed between 101 and 131.
	// Nouns - People - Male: 150-174
	_vocabulary[176].init(150, "AVVY");
	_vocabulary[177].init(150, "AVALOT");
	_vocabulary[178].init(150, "YOURSELF");
	_vocabulary[179].init(150, "ME");
	_vocabulary[180].init(150, "MYSELF");
	_vocabulary[181].init(151, "SPLUDWICK");
	_vocabulary[182].init(151, "THOMAS");
	_vocabulary[183].init(151, "ALCHEMIST");
	_vocabulary[184].init(151, "CHEMIST");
	_vocabulary[185].init(152, "CRAPULUS");
	_vocabulary[186].init(152, "SERF");
	_vocabulary[187].init(152, "SLAVE");
	_vocabulary[188].init(158, "DU"); // Put in early for Baron DU Lustie to save confusion with Duck & Duke.
	_vocabulary[189].init(152, "CRAPPY");
	_vocabulary[190].init(153, "DUCK");
	_vocabulary[191].init(153, "DOCTOR");
	_vocabulary[192].init(154, "MALAGAUCHE");
	_vocabulary[193].init(155, "FRIAR");
	_vocabulary[194].init(155, "TUCK");
	_vocabulary[195].init(156, "ROBIN");
	_vocabulary[196].init(156, "HOOD");
	_vocabulary[197].init(157, "CWYTALOT");
	_vocabulary[198].init(157, "GUARD");
	_vocabulary[199].init(157, "BRIDGEKEEP");
	_vocabulary[200].init(158, "BARON");
	_vocabulary[201].init(158, "LUSTIE");
	_vocabulary[202].init(159, "DUKE");
	_vocabulary[203].init(159, "GRACE");
	_vocabulary[204].init(160, "DOGFOOD");
	_vocabulary[205].init(160, "MINSTREL");
	_vocabulary[206].init(161, "TRADER");
	_vocabulary[207].init(161, "SHOPKEEPER");
	_vocabulary[208].init(161, "STALLHOLDER");
	_vocabulary[209].init(162, "PILGRIM");
	_vocabulary[210].init(162, "IBYTHNETH");
	_vocabulary[211].init(163, "ABBOT");
	_vocabulary[212].init(163, "AYLES");
	_vocabulary[213].init(164, "PORT");
	_vocabulary[214].init(165, "SPURGE");
	_vocabulary[215].init(166, "JACQUES");
	_vocabulary[216].init(166, "SLEEPER");
	_vocabulary[217].init(166, "RINGER");

	// Nouns - People - Female: 175-199
	_vocabulary[218].init(175, "WIFE");
	_vocabulary[219].init(175, "ARKATA");
	_vocabulary[220].init(176, "GEDALODAVA");
	_vocabulary[221].init(176, "GEIDA");
	_vocabulary[222].init(176, "PRINCESS");
	_vocabulary[223].init(178, "WISE");
	_vocabulary[224].init(178, "WITCH");

	// Pronouns: 200-224
	_vocabulary[225].init(200, "HIM");
	_vocabulary[226].init(200, "MAN");
	_vocabulary[227].init(200, "GUY");
	_vocabulary[228].init(200, "DUDE");
	_vocabulary[229].init(200, "CHAP");
	_vocabulary[230].init(200, "FELLOW");
	_vocabulary[231].init(201, "HER");
	_vocabulary[232].init(201, "GIRL");
	_vocabulary[233].init(201, "WOMAN");
	_vocabulary[234].init(202, "IT");
	_vocabulary[235].init(202, "THING");
	_vocabulary[236].init(203, "MONK");
	_vocabulary[237].init(204, "BARMAN");
	_vocabulary[238].init(204, "BARTENDER");

	// Prepositions: 225-249
	_vocabulary[239].init(225, "TO");
	_vocabulary[240].init(226, "AT");
	_vocabulary[241].init(227, "UP");
	_vocabulary[242].init(228, "INTO");
	_vocabulary[243].init(228, "INSIDE");
	_vocabulary[244].init(229, "OFF");
	_vocabulary[245].init(230, "UP");
	_vocabulary[246].init(231, "DOWN");
	_vocabulary[247].init(232, "ON");

	// Please: 251
	_vocabulary[248].init(251, "PLEASE");

	// About: 252
	_vocabulary[249].init(252, "ABOUT");
	_vocabulary[250].init(252, "CONCERNING");

	// Swear words: 253
	/*		  I M P O R T A N T    M E S S A G E

	DO *NOT* READ THE LINES BELOW IF YOU ARE OF A SENSITIVE
	DISPOSITION. THOMAS IS *NOT* RESPONSIBLE FOR THEM.
	GOODNESS KNOWS WHO WROTE THEM.
	READ THEM AT YOUR OWN RISK. BETTER STILL, DON'T READ THEM.
	WHY ARE YOU SNOOPING AROUND IN MY PROGRAM, ANYWAY? */
	_vocabulary[251].init(253, "SHIT");
	_vocabulary[252].init(28 , "PISS");
	_vocabulary[253].init(28 , "PEE");
	_vocabulary[254].init(253, "FART");
	_vocabulary[255].init(253, "FUCK");
	_vocabulary[256].init(253, "BALLS");
	_vocabulary[257].init(253, "BLAST");
	_vocabulary[258].init(253, "BUGGER");
	_vocabulary[259].init(253, "KNICKERS");
	_vocabulary[260].init(253, "BLOODY");
	_vocabulary[261].init(253, "HELL");
	_vocabulary[262].init(253, "DAMN");
	_vocabulary[263].init(253, "SMEG");
	// ...and other even ruder words. You didn't read them, did you? Good.

	// Answer-back smart-alec words: 249
	_vocabulary[264].init(249, "YES");
	_vocabulary[265].init(249, "NO");
	_vocabulary[266].init(249, "BECAUSE");

	// Noise words: 255
	_vocabulary[267].init(255, "THE");
	_vocabulary[268].init(255, "A");
	_vocabulary[269].init(255, "NOW");
	_vocabulary[270].init(255, "SOME");
	_vocabulary[271].init(255, "AND");
	_vocabulary[272].init(255, "THAT");
	_vocabulary[273].init(255, "POCUS");
	_vocabulary[274].init(255, "HIS");
	_vocabulary[275].init(255, "THIS");
	_vocabulary[276].init(255, "SENTINEL"); // for "Ken SENT Me"
}

void Parser::handleInputText(const Common::Event &event) {
	byte inChar = event.kbd.ascii;
	warning("STUB: Parser::handleInputText()");
//	if (_vm->_dropdown->_activeMenuItem._activeNow) {
//		_vm->_dropdown->parseKey(inChar, _vm->_enhanced->extd);
//	} else {
		if (_inputText.size() < 76) {
			if ((inChar == '"') || (inChar == '`')) {
				if (_quote)
					inChar = '`';
				else
					inChar = '"';
				_quote = !_quote;  // quote - unquote
			}

			_inputText.insertChar(inChar, _inputTextPos);
			_inputTextPos++;
			plotText();
		} else
			_vm->_sound->blip();
//	}
}

void Parser::handleBackspace() {
	if (_vm->_dropdown->_activeMenuItem._activeNow)
		return;

	if (_inputTextPos > 0) {
		_inputTextPos--;
		if ((_inputText[_inputTextPos] == '"') || (_inputText[_inputTextPos] == '`'))
			_quote = !_quote;
		_inputText.deleteChar(_inputTextPos);
		plotText();
	} else
		_vm->_sound->blip();
}

void Parser::handleReturn() {
	if (_vm->_dropdown->_activeMenuItem._activeNow)
		tryDropdown();
	else if (!_inputText.empty()) {
		_inputTextBackup = _inputText;
		parse();
		doThat();
		_inputText.clear();
		wipeText();
	}
}

void Parser::handleFunctionKey(const Common::Event &event) {
	switch (event.kbd.keycode) {
	case Common::KEYCODE_F1:
		_vm->callVerb(kVerbCodeHelp);
		break;
	case Common::KEYCODE_F2:
		if (event.kbd.flags & Common::KBD_CTRL) {
			clearWords();
			_vm->callVerb(kVerbCodeSave);
		} else
			_vm->_sound->toggleSound();
		break;
	case Common::KEYCODE_F3:
		if (event.kbd.flags & Common::KBD_CTRL) {
			clearWords();
			_vm->callVerb(kVerbCodeLoad);
		} else if (_inputText.size() < _inputTextBackup.size()) {
			_inputText = _inputText + &(_inputTextBackup.c_str()[_inputText.size()]);
			_inputTextPos = _inputText.size();
			plotText();
		}
		break;
	case Common::KEYCODE_F4:
		if (event.kbd.flags & Common::KBD_ALT)
			_vm->callVerb(kVerbCodeQuit);
		else
			_vm->callVerb(kVerbCodeRestart);
		break;
	case Common::KEYCODE_F5: {
		_person = kPeoplePardon;
		_thing = kPardon;
		Common::String f5does = _vm->f5Does();
		VerbCode verb = (VerbCode)(byte)f5does[0];
		_vm->callVerb(verb);
		}
		break;
	case Common::KEYCODE_F6:
		_vm->callVerb(kVerbCodePause);
		break;
	case Common::KEYCODE_F7:
		if (event.kbd.flags & Common::KBD_CTRL)
			_vm->_graphics->refreshScreen();
		else
			_vm->callVerb(kVerbCodeOpen);
		break;
	case Common::KEYCODE_F8:
		_vm->callVerb(kVerbCodeLook);
		break;
	case Common::KEYCODE_F9:
		_vm->callVerb(kVerbCodeScore);
		break;
	case Common::KEYCODE_F10:
		if (event.kbd.flags & Common::KBD_SHIFT)
			_vm->callVerb(kVerbCodeInfo);
		else
			_vm->callVerb(kVerbCodeQuit);
		break;
	case Common::KEYCODE_F11:
		clearWords();
		_vm->callVerb(kVerbCodeSave);
		break;
	case Common::KEYCODE_F12:
		clearWords();
		_vm->callVerb(kVerbCodeLoad);
		break;
	default:
		break;
	}
}

void Parser::plotText() {
	CursorMan.showMouse(false);
	cursorOff();

	_vm->_graphics->clearTextBar();
	_vm->_graphics->drawNormalText(_inputText, _vm->_font, 8, 24, 161, kColorWhite);

	cursorOn();
	CursorMan.showMouse(true);
}

void Parser::cursorOn() {
	if (_cursorState == true)
		return;
	_vm->_graphics->drawCursor(_inputTextPos);
	_cursorState = true;
}

void Parser::cursorOff() {
	if (_cursorState == false)
		return;
	_vm->_graphics->drawCursor(_inputTextPos);
	_cursorState = false;
}

/**
 * Asks the parsekey proc in Dropdown if it knows it.
 */
void Parser::tryDropdown() {
    // TODO: Implement at the same time with Dropdown's keyboard handling.
	warning("STUB: Parser::tryDropdown()");
}

/**
 * Returns the index of the first appearance of crit in src.
 */
int16 Parser::getPos(const Common::String &crit, const Common::String &src) {
	if (src.contains(crit))
		return strstr(src.c_str(),crit.c_str()) - src.c_str();
	else
		return -1;
}

void Parser::wipeText() {
	CursorMan.showMouse(false);
	cursorOff();

	_vm->_graphics->clearTextBar();

	_quote = true;
	_inputTextPos = 0;

	cursorOn();
	CursorMan.showMouse(true);
}

void Parser::clearWords() {
	for (int i = 0; i < 11; i++) {
		if (!_realWords[i].empty())
			_realWords[i].clear();
	}
}

byte Parser::wordNum(Common::String word) {
	if (word.empty())
		return 0;

	for (int32 i = kParserWordsNum - 1; i >= 0; i--) {
		if (_vocabulary[i]._word == word)
			return _vocabulary[i]._number;
	}

	// If not found as a whole, we look for it as a substring.
	for (int32 i = kParserWordsNum - 1; i >= 0; i--) {
		if (Common::String(_vocabulary[i]._word.c_str(), word.size()) == word)
			return _vocabulary[i]._number;
	}

	return kPardon;
}

void Parser::replace(Common::String oldChars, byte newChar) {
	int16 pos = getPos(oldChars, _thats);
	while (pos != -1) {
		if (newChar == 0)
			_thats.deleteChar(pos);
		else {
			for (uint i = pos; i < pos + oldChars.size(); i++)
				_thats.deleteChar(pos);
			_thats.insertChar(newChar, pos);
		}
		pos = getPos(oldChars, _thats);
	}
}

Common::String Parser::rank() {
	static const RankType ranks[9] = {
		{0, "Beginner"},     {10, "Novice"},
		{20, "Improving"},   {35, "Not bad"},
		{50, "Passable"},    {65, "Good"},
		{80, "Experienced"}, {108, "The BEST!"},
		{32767, "copyright'93"}
	};

	for (int i = 0; i < 8; i++) {
		if ((_vm->_score >= ranks[i]._score) && (_vm->_score < ranks[i + 1]._score))
			return Common::String(ranks[i]._title);
	}
	return "";
}

Common::String Parser::totalTime() {
	uint16 h, m, s;

	uint32 curTime = _vm->getTimeInSeconds() - _vm->_startTime;
	if (_vm->_isLoaded)
		curTime += _vm->_totalTime;

	h = (uint16)(curTime / 3600);
	s = (uint16)(curTime  % 3600);
	m = s / 60;
	s = s % 60;

	Common::String result = "You've been playing for ";
	if (h > 0)
		result += Common::String::format("%d hours, ", h);
	if ((m > 0) || (h != 0))
		result += Common::String::format("%d minutes and ", m);
	return result + Common::String::format("%d seconds", s);
}

void Parser::cheatParse(Common::String codes) {
	warning("STUB: Parser::cheatParse()");
}

/**
 * Strips punctuation from word.
 */
void Parser::stripPunctuation(Common::String &word) {
	const char punct[] = "~`!@#$%^&*()_+-={}[]:\"|;'\\,./<>?";

	for (int i = 0; i < 32; i++) {
		for (;;) {
			int16 pos = getPos(Common::String(punct[i]), word);
			if (pos == -1)
				break;
			word.deleteChar(pos);
		}
	}
}

void Parser::displayWhat(byte target, bool animate, bool &ambiguous) {
	if (target == kPardon) {
		ambiguous = true;
		if (animate)
			_vm->_dialogs->displayText("Whom?");
		else
			_vm->_dialogs->displayText("What?");
	} else {
		if (animate) {
			Common::String tmpStr = Common::String::format("{ %s }", _vm->getName((People)target).c_str());
			_vm->_dialogs->displayText(tmpStr);
		} else {
			Common::String z = _vm->getItem(target);
			if (z != "") {
				Common::String tmpStr = Common::String::format("{ %s }", z.c_str());
				_vm->_dialogs->displayText(tmpStr);
			}
		}
	}
}

bool Parser::doPronouns() {
	bool ambiguous = false;

	for (uint i = 0; i < _thats.size(); i++) {
		byte wordCode = _thats[i];
		switch (wordCode) {
		case 200:
			displayWhat(_vm->_him, true, ambiguous);
			_thats.setChar(_vm->_him, i);
			break;
		case 201:
			displayWhat(_vm->_her, true, ambiguous);
			_thats.setChar(_vm->_her, i);
			break;
		case 202:
			displayWhat(_vm->_it, false, ambiguous);
			_thats.setChar(_vm->_it, i);
			break;
		default:
			break;
		}
	}

	return ambiguous;
}

void Parser::properNouns() {
	_inputText.toLowercase();

	// We set every word's first character to uppercase.
	for (uint i = 1; i < (_inputText.size() - 1); i++) {
		if (_inputText[i] == ' ')
			_inputText.setChar(toupper(_inputText[i + 1]), i + 1);
	}

	// And the first character as well.
	_inputText.setChar(toupper(_inputText[0]), 0);
}

void Parser::storeInterrogation(byte interrogation) {
	if (_inputText.empty())
		return;

	// Strip _inputText:
	while ((_inputText[0] == ' ') && (!_inputText.empty()))
		_inputText.deleteChar(0);
	while ((_inputText.lastChar() == ' ') && (!_inputText.empty()))
		_inputText.deleteLastChar();

	_vm->_timer->loseTimer(Timer::kReasonCardiffsurvey); // If you want to use any other timer, put this into the case statement.

	switch (interrogation) {
	case 1:
		_inputText.toLowercase();
		_vm->_dialogs->sayIt(_inputText);
		_vm->_favoriteDrink = _inputText;
		_vm->_cardiffQuestionNum = 2;
		break;
	case 2:
		properNouns();
		_vm->_dialogs->sayIt(_inputText);
		_vm->_favoriteSong = _inputText;
		_vm->_cardiffQuestionNum = 3;
		break;
	case 3:
		properNouns();
		_vm->_dialogs->sayIt(_inputText);
		_vm->_worstPlaceOnEarth = _inputText;
		_vm->_cardiffQuestionNum = 4;
		break;
	case 4:
		_inputText.toLowercase();
		_vm->_dialogs->sayIt(_inputText);
		if (!_vm->_spareEvening.empty())
			_vm->_spareEvening.clear();
		_vm->_spareEvening = _inputText;
		_vm->_dialogs->displayScrollChain('Z', 5); // His closing statement...
		_vm->_animation->_sprites[1]->walkTo(3); // The end of the drawbridge
		_vm->_animation->_sprites[1]->_vanishIfStill = true; // Then go away!
		_vm->_magics[1]._operation = kMagicNothing;
		_vm->_cardiffQuestionNum = 5;
		break;
	case 99:
		//store_high(_inputText);
		warning("STUB: Parser::store_interrogation()");
		break;
	default:
		break;
	}

	if (interrogation < 4)
		_vm->_timer->cardiffSurvey();
}

void Parser::parse() {
	// First parsing - word identification
	if (!_thats.empty())
		_thats.clear();

	_polite = false;
	_verb = kVerbCodePardon;
	_thing = kPardon;
	_thing2 = kPardon;
	_person = kPeoplePardon;
	clearWords();

	// A cheat mode attempt.
	if (_inputText[0] == '.') {
		cheatParse(_inputText);
		_thats = kNothing;
		return;
	}

	// Are we being interrogated right now?
	if (_vm->_interrogation > 0) {
		storeInterrogation(_vm->_interrogation);
		_weirdWord = true;
		return;
	}

	// Actually process the command.
	Common::String inputText = _inputText + ' ';
	Common::String inputTextUpper = inputText;
	byte n = 0;
	inputTextUpper.toUppercase();
	while (!inputTextUpper.empty()) {
		while ((!inputTextUpper.empty()) && (inputTextUpper[0] == ' ')) {
			inputTextUpper.deleteChar(0);
			inputText.deleteChar(0);
		}
		if (inputTextUpper.empty())
			break;

		// Get the following word of the strings.
		byte size = getPos(Common::String(' '), inputTextUpper) + 1;
		char *subStr = new char[size];
		Common::strlcpy(subStr, inputTextUpper.c_str(), size);
		Common::String thisword = subStr;
		Common::strlcpy(subStr, inputText.c_str(), size);
		_realWords[n] = subStr;
		delete[] subStr;

		stripPunctuation(inputTextUpper);

		bool notfound = true;

		// Check also[] first, which contains words about the actual room.
		if (!thisword.empty()) {
			for (int i = 0; i < 31; i++) {
				if ((_vm->_also[i][0]) && (getPos(',' + thisword, *_vm->_also[i][0]) > -1)) {
					_thats += Common::String(99 + i);
					notfound = false;
				}
			}
		}

		// Check Accis's own table (words[]) for "global" commands.
		if (notfound) {
			byte answer = wordNum(thisword);
			if (answer == kPardon) {
				notfound = true;
				_thats = _thats + kPardon;
			} else
				_thats = _thats + answer;
			n++;
		}

		// Delete words we already processed.
		int16 spacePos = getPos(Common::String(' '), inputTextUpper);
		if (spacePos > -1) {
			for (int i = 0; i <= spacePos; i++)
				inputTextUpper.deleteChar(0);
		}

		spacePos = getPos(Common::String(' '), inputText);
		if (spacePos > -1) {
			for (int i = 0; i <= spacePos; i++)
				inputText.deleteChar(0);
		}
	}

	Common::String unkString;
	int16 pos = getPos(Common::String('\xFE'), _thats);
	if (pos > -1)
		unkString = _realWords[pos];
	else
		unkString.clear();

	// Replace words' codes that mean the same.
	replace(Common::String("\xFF"),     0); // zap noise words
	replace(Common::String("\xD\xE2"),  1); // "look at" = "examine"
	replace(Common::String("\xD\xE4"),  1); // "look in" = "examine"
	replace(Common::String("\x4\xE6"),  17); // "get up" = "stand"
	replace(Common::String("\x4\xE7"),  17); // "get down" = "stand"... well, why not?
	replace(Common::String("\x12\xE4"), 2); // "go in" = "open [door]"
	replace(Common::String("\x1C\xE5"), 253); // "P' off" is a swear word
	replace(Common::String("\x4\x6"),   6); // "Take inventory" (remember Colossal Adventure?)
	replace(Common::String("\x28\xE8"), 21); // "put on" = "don"
	replace(Common::String("\x4\xE5"),  20); // "take off" = "doff"

	// Words that could mean more than one _person
	if (_vm->_room == kRoomNottsPub)
		replace(Common::String('\xCC'), 164); // Barman = Port
	else
		replace(Common::String('\xCC'), 154); // Barman = Malagauche

	switch (_vm->_room) {
	case kRoomAylesOffice:
		replace(Common::String('\xCB'), 163); // Monk = Ayles
		break;
	case kRoomMusicRoom:
		replace(Common::String('\xCB'), 166); // Monk = Jacques
		break;
	default:
		replace(Common::String('\xCB'), 162); // Monk = Ibythneth
	}

	if (doPronouns()) {
		_weirdWord = true;
		_thats = kNothing;
		return;
	}

	// Second parsing.
	_vm->_subjectNum = 0; // Find subject of conversation.

	for (int i = 0; (i < 11) && !_realWords[i].empty(); i++) {
		if ((_realWords[i][0] == '\'') || (_realWords[i][0] == '\"')) {
			_vm->_subjectNum = (byte)_thats[i];
			_thats.setChar(kMoved, i);
			break;
		}
	}

	if ((_vm->_subjectNum == 0) && !_thats.empty()) { // Still not found.
		for (uint16 i = 0; i < _thats.size() - 1; i++) {
			if ((byte)_thats[i] == 252) { // The word is "about", or something similar.
				_vm->_subjectNum = (byte)_thats[i + 1];
				_thats.setChar(0, i + 1);
				break;
			}
		}
	}

	if ((_vm->_subjectNum == 0) && !_thats.empty()) { // STILL not found! Must be the word after "say".
		for (uint16 i = 0; i < _thats.size() - 1; i++) {
			if (((byte)_thats[i] == 7) && ((byte)_thats[i + 1] != 0) && !((225 <= (byte)_thats[i + 1]) && ((byte)_thats[i + 1] <= 229))) {
				// SAY not followed by a preposition
				_vm->_subjectNum = (byte)_thats[i + 1];
				_thats.setChar(0, i + 1);
				break;
			}
		}
	}

	for (int16 i = _thats.size() - 1; i >= 0; i--) { // Reverse order, so first will be used.
		byte curChar = (byte)_thats[i];
		if ((curChar == 253) || (curChar == 249) || ((1 <= curChar) && (curChar <= 49)))
			_verb = (VerbCode)curChar;
		else if ((50 <= curChar) && (curChar <= 149)) {
			_thing2 = _thing;
			_thing = curChar;
		} else if ((150 <= curChar) && (curChar <= 199))
			_person = (People)curChar;
		else if (curChar == 251)
			_polite = true;
	}

	if ((!unkString.empty()) && (_verb != kVerbCodeExam) && (_verb != kVerbCodeTalk) &&
		(_verb != kVerbCodeSave) && (_verb != kVerbCodeLoad) && (_verb != kVerbCodeDir)) {
		Common::String tmpStr = Common::String::format("Sorry, but I have no idea what \"%s\" means. Can you rephrase it?", unkString.c_str());
		_vm->_dialogs->displayText(tmpStr);
		_weirdWord = true;
	} else
		_weirdWord = false;

	if (_thats.empty())
		_thats = kNothing;

	if (_thing != kPardon)
		_vm->_it = _thing;

	if (_person != kPardon) {
		if (_person < kPeopleArkata)
			_vm->_him = _person;
		else
			_vm->_her = _person;
	}
}

/**
 * Examine a standard object-thing
 */
void Parser::examineObject() {
	if (_thing != _vm->_thinks)
		_vm->thinkAbout(_thing, AvalancheEngine::kThing);
	switch (_thing) {
	case kObjectWine :
		// 4 is perfect wine. 0 is not holding the wine.
		switch (_vm->_wineState) {
		case 1:
			// Normal examine wine scroll
			_vm->_dialogs->displayScrollChain('T', 1);
			break;
		case 2:
			// Bad wine
			_vm->_dialogs->displayScrollChain('D', 6);
			break;
		case 3:
			// Vinegar
			_vm->_dialogs->displayScrollChain('D', 7);
			break;
		default:
			break;
		}
		break;
	case kObjectOnion:
		if (_vm->_rottenOnion) {
			// Yucky onion
			_vm->_dialogs->displayScrollChain('Q', 21);
		} else {
			// Normal onion
			_vm->_dialogs->displayScrollChain('T', 18);
		}
		break;
	default:
		// Ordinarily
		_vm->_dialogs->displayScrollChain('T', _thing);
		break;
	}
}

bool Parser::isPersonHere() {
	// Person equivalent of "isHolding".
	if ((_person == kPeoplePardon) || (_person == kPeopleNone) || (_vm->getRoom(_person) == _vm->_room))
		return true;
	else {
		Common::String tmpStr;
		if (_person < kPeopleArkata)
			tmpStr = "He isn't around at the moment.";
		else
			tmpStr = "She isn't around at the moment.";
		_vm->_dialogs->displayText(tmpStr);
		return false;
	}
}

void Parser::exampers() {
	if (isPersonHere()) {
		if (_thing != _vm->_thinks)
			_vm->thinkAbout(_person, AvalancheEngine::kPerson);

		byte newPerson = _person - 149;

		if ((_person == kPeopleDogfood) && _vm->_wonNim)
			// "I'm Not Playing!"
			_vm->_dialogs->displayScrollChain('Q', 8);
		else if ((_person == kPeopleDuLustie) && _vm->_lustieIsAsleep)
			// He's asleep.
			_vm->_dialogs->displayScrollChain('Q', 65);
		else
			_vm->_dialogs->displayScrollChain('P', newPerson);

		if ((_person == kPeopleAyles) && !_vm->_aylesIsAwake)
			_vm->_dialogs->displayScrollChain('Q', 13);

		// CHECKME: Present in the original, but it doesn't make sense.
		// _person = newPerson;
	}
}

/**
 * Return whether Avvy is holding an object or not
 * @remarks	Originally called 'holding'
 */
bool Parser::isHolding() {
	// Also object
	if ((51 <= _thing) && (_thing <= 99))
		return true;
	if (_thing == 0)
		return false;

	bool holdingResult = false;

	if (_thing >= 100)
		_vm->_dialogs->displayText("Be reasonable!");
	else if (_thing <= kObjectNum) {
		if (!_vm->_objects[_thing - 1])
			// Verbs that need "_thing" to be in the inventory.
			_vm->_dialogs->displayText("You're not holding it, Avvy.");
		else
			holdingResult = true;
	} else
		holdingResult = true;

	return holdingResult;
}

void Parser::openBox(bool isOpening) {
	if ((_vm->_room == kRoomYours) && (_thing == 54)) {
		_vm->_background->draw(-1, -1, 4);

		_vm->_background->update();
		_vm->_animation->animLink();
		_vm->_graphics->refreshScreen();

		_vm->_system->delayMillis(55);

		if (!isOpening) {
			_vm->_background->draw(-1, -1, 5);
			_vm->_background->update();
			_vm->_animation->animLink();
			_vm->_graphics->refreshScreen();
		}
	}
}

void Parser::examine() {
	// EITHER it's an object OR it's an Also OR it's a _person OR it's something else.
	if ((_person == kPeoplePardon) && (_thing != kPardon)) {
		if (isHolding()) {
			// Remember: it's been slipped! Ie subtract 49.
			if ((1 <= _thing) && (_thing <= 49))
				// Standard object
				examineObject();
			else if ((50 <= _thing) && (_thing <= 100)) {
				// Also _thing
				int id = _thing - 50;
				assert(id < 31);
				openBox(true);
				_vm->_dialogs->displayText(*_vm->_also[id][1]);
				openBox(false);
			}
		}
	} else if (_person != kPardon)
		exampers();
	else
		// Don't know: guess.
		_vm->_dialogs->displayText("It's just as it looks on the picture.");
}

void Parser::inventory() {
	byte itemNum = 0;
	Common::String tmpStr = Common::String("You're carrying ");

	for (int i = 0; i < kObjectNum; i++) {
		if (_vm->_objects[i]) {
			itemNum++;
			if (itemNum == _vm->_carryNum)
				tmpStr += "and ";

			tmpStr += _vm->getItem(i + 1);

			if ((i + 1) == _wearing)
				tmpStr += ", which you're wearing";

			if (itemNum < _vm->_carryNum)
				tmpStr += ", ";
		}
	}

	if (_wearing == kNothing)
		tmpStr += Common::String::format("...%c%c...and you're stark naked!", kControlNewLine, kControlNewLine);
	else
		tmpStr += '.';

	_vm->_dialogs->displayText(tmpStr);
}

/**
 * Eat something.
 */
void Parser::swallow() {
	switch (_thing) {
	case kObjectWine:
		// _wineState == 4 for perfect wine
		switch (_vm->_wineState) {
		case 1:
			if (_vm->_teetotal)  {
				_vm->_dialogs->displayScrollChain('D', 6);
				return;
			}
			_vm->_dialogs->displayScrollChain('U', 1);
			_vm->_animation->wobble();
			_vm->_dialogs->displayScrollChain('U', 2);
			_vm->_objects[kObjectWine - 1] = false;
			_vm->refreshObjectList();
			drink();
			break;
		case 2:
		case 3:
			// You can't drink it!
			_vm->_dialogs->displayScrollChain('D', 8);
			break;
		default:
			break;
		}
		break;
	case kObjectPotion:
		_vm->_graphics->setBackgroundColor(kColorRed);
		_vm->_dialogs->displayScrollChain('U', 3);
		_vm->gameOver();
		_vm->_graphics->setBackgroundColor(kColorBlack);
		break;
	case kObjectInk:
		_vm->_dialogs->displayScrollChain('U', 4);
		break;
	case kObjectChastity:
		_vm->_dialogs->displayScrollChain('U', 5);
		break;
	case kObjectMushroom:
		_vm->_dialogs->displayScrollChain('U', 6);
		_vm->gameOver();
		break;
	case kObjectOnion:
		if (_vm->_rottenOnion)
			_vm->_dialogs->displayScrollChain('U', 11);
		else {
			_vm->_dialogs->displayScrollChain('U', 8);
			_vm->_objects[kObjectOnion - 1] = false;
			_vm->refreshObjectList();
		}
		break;
	default:
		if ((_vm->_room == kRoomArgentPub) || (_vm->_room == kRoomNottsPub))
			_vm->_dialogs->displayText("Try BUYing things before you drink them!");
		else
			_vm->_dialogs->displayText("The taste of it makes you retch!");
	}
}

/**
 * this lists the other people in the room.
 */
void Parser::peopleInRoom() {
	// First compute the number of people in the room.
	byte numPeople = 0;
	for (int i = 151; i < 179; i++) { // Start at 1 so we don't list Avvy himself!
		if (_vm->getRoom((People)i) == _vm->_room)
			numPeople++;
	}

	// If nobody's here, we can cut out straight away.
	if (numPeople == 0)
		return;

	Common::String tmpStr;
	byte actPerson = 0;
	for (int i = 151; i < 179; i++) {
		if (_vm->getRoom((People)i) == _vm->_room) {
			actPerson++;
			if (actPerson == 1)
				// Display first name on the list.
				tmpStr = _vm->getName((People)i);
			else if (actPerson < numPeople)
				// Display one the names in the middle of the list
				tmpStr += ", " + _vm->getName((People)i);
			else
				// Display the last name of the list
				tmpStr += " and " + _vm->getName((People)i);
		}
	}

	if (numPeople == 1)
		tmpStr += " is";
	else
		tmpStr += " are";

	_vm->_dialogs->displayText(tmpStr + " here.");
}

/**
 * This is called when you say "look".
 */
void Parser::lookAround() {
	_vm->_dialogs->displayText(*_vm->_also[0][1]);
	switch (_vm->_room) {
	case kRoomSpludwicks:
		if (_vm->_avariciusTalk > 0)
			_vm->_dialogs->displayScrollChain('Q', 23);
		else
			peopleInRoom();
		break;
	case kRoomRobins:
		if (_vm->_tiedUp)
			_vm->_dialogs->displayScrollChain('Q', 38);
		if (_vm->_mushroomGrowing)
			_vm->_dialogs->displayScrollChain('Q', 55);
		break;
	case kRoomInsideCardiffCastle:
		if (!_vm->_takenPen)
			_vm->_dialogs->displayScrollChain('Q', 49);
		break;
	case kRoomLustiesRoom:
		if (_vm->_lustieIsAsleep)
			_vm->_dialogs->displayScrollChain('Q', 65);
		break;
	case kRoomCatacombs:
		switch (_vm->_catacombY * 256 + _vm->_catacombX) {
		case 258 :
			// Inside art gallery.
			_vm->_dialogs->displayScrollChain('Q', 80);
			break;
		case 514 :
			// Outside ditto.
			_vm->_dialogs->displayScrollChain('Q', 81);
			break;
		case 260 :
			// Outside Geida's room.
			_vm->_dialogs->displayScrollChain('Q', 82);
			break;
		default:
			break;
		}
		break;
	default:
		peopleInRoom();
	}
}

void Parser::openDoor() {
	// Special cases.
	switch (_vm->_room) {
	case kRoomYours:
		if (_vm->_animation->inField(1)) {
			// Opening the box.
			_thing = 54; // The box.
			_person = kPeoplePardon;
			examine();
			return;
		}
		break;
	case kRoomSpludwicks:
		if (_thing == 61) {
			_vm->_dialogs->displayScrollChain('Q', 85);
			return;
		}
		break;
	default:
		break;
	}

	if ((!_vm->_userMovesAvvy) && (_vm->_room != kRoomLusties))
		// No doors can open if you can't move Avvy.
		return;

	for (int i = 0; i < 7; i++) {
		if (_vm->_animation->inField(i + 8)) {
			MagicType *portal = &_vm->_portals[i];
			switch (portal->_operation) {
			case kMagicExclaim:
				_vm->_animation->_sprites[0]->bounce();
				_vm->_dialogs->displayScrollChain('X', portal->_data);
				break;
			case kMagicTransport:
				_vm->flipRoom((Room)((portal->_data) >> 8), portal->_data & 0x0F);
				break;
			case kMagicUnfinished:
				_vm->_animation->_sprites[0]->bounce();
				_vm->_dialogs->displayText("Sorry. This place is not available yet!");
				break;
			case kMagicSpecial:
				_vm->_animation->callSpecial(portal->_data);
				break;
			case kMagicOpenDoor:
				_vm->openDoor((Room)(portal->_data >> 8), portal->_data & 0x0F, i + 9);
				break;
			case kMagicBounce: // Not valid for portals.
			case kMagicNothing:
			default:
				break;
			}

			return;
		}
	}

	if (_vm->_room == kRoomMap)
		_vm->_dialogs->displayText("Avvy, you can complete the whole game without ever going " \
				"to anywhere other than Argent, Birmingham, Cardiff, Nottingham and Norwich.");
	else
		_vm->_dialogs->displayText("Door? What door?");
}

/**
 * Called when you call kVerbCodeput.
 */
void Parser::putProc() {
	if (!isHolding())
		return;

	// Slip the second object.
	_thing2 -= 49;
	char temp = _thing;
	_thing = _thing2;
	if (!isHolding())
		return;
	_thing = temp;

	// Thing is the _thing which you're putting in. _thing2 is where you're putting it.
	switch (_thing2) {
	case kObjectWine:
		if (_thing == kObjectOnion) {
			if (_vm->_rottenOnion)
				_vm->_dialogs->displayText("That's a bit like shutting the stable door after the horse has bolted!");
			else {
				// Put onion into wine?
				if (_vm->_wineState != 3) {
					Common::String tmpStr = Common::String::format("%cOignon au vin%c is a bit too strong for your tastes!",
						kControlItalic, kControlRoman);
					_vm->_dialogs->displayText(tmpStr);
				} else {
					// Put onion into vinegar! Yes!
					_vm->_onionInVinegar = true;
					_vm->incScore(7);
					_vm->_dialogs->displayScrollChain('U', 9);
				}
			}
		} else {
			_vm->_dialogs->saySilly();
		}
		break;

	case 54:
		if (_vm->_room == kRoomYours) {
			// Put something into the box.
			if (_vm->_boxContent != kNothing)
				_vm->_dialogs->displayText("There's something in the box already, Avvy. Try taking that out first.");
			else {
				switch (_thing) {
				case kObjectMoney:
					_vm->_dialogs->displayText("You'd better keep some ready cash on you!");
					break;
				case kObjectBell:
					_vm->_dialogs->displayText("That's a silly place to keep a bell.");
					break;
				case kObjectBodkin:
					_vm->_dialogs->displayText("But you might need it!");
					break;
				case kObjectOnion:
					_vm->_dialogs->displayText("Just give it to Spludwick, Avvy!");
					break;
				default:
					// Put the object into the box...
					if (_wearing == _thing) {
						Common::String tmpStr = Common::String::format("You'd better take %s off first!", _vm->getItem(_thing).c_str());
						_vm->_dialogs->displayText(tmpStr);
					} else {
						// Open box.
						openBox(true);

						_vm->_boxContent = _thing;
						_vm->_objects[_thing - 1] = false;
						_vm->refreshObjectList();
						_vm->_dialogs->displayText("OK, it's in the box.");

						// Shut box.
						openBox(false);
					}
				}
			}
		} else {
			_vm->_dialogs->saySilly();
		}
		break;

	default:
		_vm->_dialogs->saySilly();
		break;
	}
}

/**
 * Display text when ingredients are not in the right order
 * @remarks	Originally called 'not_in_order'
 */
void Parser::notInOrder() {
	Common::String itemStr = _vm->getItem(_vm->kSpludwicksOrder[_vm->_givenToSpludwick]);
	Common::String tmpStr = Common::String::format("Sorry, I need the ingredients in the right order for this potion. " \
		"What I need next is %s%c2%c", itemStr.c_str(), kControlRegister, kControlSpeechBubble);
	_vm->_dialogs->displayText(tmpStr);
}

/**
 * Move Spludwick to cauldron
 * @remarks	Originally called 'go_to_cauldron'
 */
void Parser::goToCauldron() {
	// Stops Geida_Procs.
	_vm->_animation->_sprites[1]->_callEachStepFl = false;
	_vm->_timer->addTimer(1, Timer::kProcSpludwickGoesToCauldron, Timer::kReasonSpludwickWalk);
	_vm->_animation->_sprites[1]->walkTo(1);
}

/**
 * Check is it's possible to give something to Spludwick
 * The result of this function is whether or not he says "Hey, thanks!".
 * @remarks	Originally called 'give2spludwick'
 */
bool Parser::giveToSpludwick() {
	if (_vm->kSpludwicksOrder[_vm->_givenToSpludwick] != _thing) {
		notInOrder();
		return false;
	}

	switch (_thing) {
	case kObjectOnion:
		_vm->_objects[kObjectOnion - 1] = false;
		if (_vm->_rottenOnion)
			_vm->_dialogs->displayScrollChain('Q', 22);
		else {
			_vm->_givenToSpludwick++;
			_vm->_dialogs->displayScrollChain('Q', 20);
			goToCauldron();
			_vm->incScore(3);
		}
		_vm->refreshObjectList();
		break;
	case kObjectInk:
		_vm->_objects[kObjectInk - 1] = false;
		_vm->refreshObjectList();
		_vm->_givenToSpludwick++;
		_vm->_dialogs->displayScrollChain('Q', 24);
		goToCauldron();
		_vm->incScore(3);
		break;
	case kObjectMushroom:
		_vm->_objects[kObjectMushroom - 1] = false;
		_vm->_dialogs->displayScrollChain('Q', 25);
		_vm->incScore(5);
		_vm->_givenToSpludwick++;
		goToCauldron();
		_vm->_objects[kObjectPotion - 1] = true;
		_vm->refreshObjectList();
		break;
	default:
		return true;
	}
	return false;
}

void Parser::drink() {
	_alcoholLevel++;
	if (_alcoholLevel == 5) {
		// Get the key.
		_vm->_objects[kObjectKey - 1] = true;
		_vm->_teetotal = true;
		_vm->_avvyIsAwake = false;
		_vm->_avvyInBed = true;
		_vm->refreshObjectList();
		_vm->fadeOut();
		_vm->flipRoom(kRoomYours, 1);
		_vm->_graphics->setBackgroundColor(kColorYellow);
		_vm->_animation->_sprites[0]->_visible = false;
	}
}

void Parser::cardiffClimbing() {
	if (_vm->_standingOnDais) {
		// Clamber up.
		_vm->_dialogs->displayText("You climb down, back onto the floor.");
		_vm->_standingOnDais = false;
		_vm->_animation->appearPed(0, 2);
	} else if (_vm->_animation->inField(0)) {
		// Clamber down
		_vm->_dialogs->displayText("You clamber up onto the dais.");
		_vm->_standingOnDais = true;
		_vm->_animation->appearPed(0, 1);
	} else
			_vm->_dialogs->displayText("Get a bit closer, Avvy.");
}

void Parser::already() {
	_vm->_dialogs->displayText("You're already standing!");
}

/**
 * Called when you ask Avvy to stand.
 */
void Parser::standUp() {
	switch (_vm->_room) {
	case kRoomYours:
		// Avvy isn't asleep.
		if (_vm->_avvyIsAwake && _vm->_avvyInBed) {
			// But he's in bed.
			if (_vm->_teetotal) {
				_vm->_dialogs->displayScrollChain('D', 12);
				_vm->_graphics->setBackgroundColor(kColorBlack);
				_vm->_dialogs->displayScrollChain('D', 14);
			}
			_vm->_animation->_sprites[0]->_visible = true;
			_vm->_userMovesAvvy = true;
			_vm->_animation->appearPed(0, 1);
			_vm->_animation->setDirection(kDirLeft);
			// Display a picture of empty pillow in the background.
			_vm->_background->draw(-1, -1, 3);
			_vm->incScore(1);
			_vm->_avvyInBed = false;
			_vm->_timer->loseTimer(Timer::kReasonArkataShouts);
		} else
			already();
		break;

	case kRoomInsideCardiffCastle:
		cardiffClimbing();
		break;

	case kRoomNottsPub:
		if (_vm->_sittingInPub)  {
			// Not sitting down.
			_vm->_background->draw(-1, -1, 3);
			// But standing up.
			_vm->_animation->_sprites[0]->_visible = true;
			// And walking away.
			_vm->_animation->appearPed(0, 3);
			// Really not sitting down.
			_vm->_sittingInPub = false;
			// And ambulant.
			_vm->_userMovesAvvy = true;
		} else
			already();
		break;
	default:
		already();
	}
}

void Parser::getProc(char thing) {
	switch (_vm->_room) {
	case kRoomYours:
		if (_vm->_animation->inField(1)) {
			if (_vm->_boxContent == thing) {
				_vm->_background->draw(-1, -1, 4);
				_vm->_dialogs->displayText("OK, I've got it.");
				_vm->_objects[thing - 1] = true;
				_vm->refreshObjectList();
				_vm->_boxContent = kNothing;
				_vm->_background->draw(-1, -1, 5);
			} else {
				Common::String tmpStr = Common::String::format("I can't see %s in the box.", _vm->getItem(thing).c_str());
				_vm->_dialogs->displayText(tmpStr);
			}
		} else
			_vm->_dialogs->displayScrollChain('Q', 57);
		break;
	case kRoomInsideCardiffCastle:
		switch (thing) {
		case kObjectPen:
			if (_vm->_animation->inField(1)) {
				// Standing on the dais.
				if (_vm->_takenPen)
					_vm->_dialogs->displayText("It's not there, Avvy.");
				else {
					// OK: we're taking the pen, and it's there.
					// No pen there now.
					_vm->_background->draw(-1, -1, 3);
					// Zap!
					_vm->_animation->callSpecial(3);
					_vm->_takenPen = true;
					_vm->_objects[kObjectPen - 1] = true;
					_vm->refreshObjectList();
					_vm->_dialogs->displayText("Taken.");
				}
			} else if (_vm->_standingOnDais)
				_vm->_dialogs->displayScrollChain('Q', 53);
			else
				_vm->_dialogs->displayScrollChain('Q', 51);
			break;
		case kObjectBolt:
			_vm->_dialogs->displayScrollChain('Q', 52);
			break;
		default:
			_vm->_dialogs->displayScrollChain('Q', 57);
		}
		break;
	case kRoomRobins:
		if ((thing == kObjectMushroom) & (_vm->_animation->inField(0)) & (_vm->_mushroomGrowing)) {
			_vm->_background->draw(-1, -1, 2);
			_vm->_dialogs->displayText("Got it!");
			_vm->_mushroomGrowing = false;
			_vm->_takenMushroom = true;
			_vm->_objects[kObjectMushroom - 1] = true;
			_vm->refreshObjectList();
			_vm->incScore(3);
		} else
			_vm->_dialogs->displayScrollChain('Q', 57);
		break;
	default:
		_vm->_dialogs->displayScrollChain('Q', 57);
	}
}

/**
 * Give the lute to Geida
 * @remarks	Originally called 'give_Geida_the_lute'
 */
void Parser::giveGeidaTheLute() {
	if (_vm->_room != kRoomLustiesRoom) {
		Common::String tmpStr = Common::String::format("Not yet. Try later!%c2%c", kControlRegister, kControlSpeechBubble);
		_vm->_dialogs->displayText(tmpStr);
		return;
	}
	_vm->_objects[kObjectLute - 1] = false;
	_vm->refreshObjectList();
	// She plays it.
	_vm->_dialogs->displayScrollChain('Q', 64);

	_vm->_timer->addTimer(1, Timer::kProcGiveLuteToGeida, Timer::kReasonGeidaSings);
	//_vm->_enid->backToBootstrap(4); TODO: Replace it with proper ScummVM-friendly function(s)!  Do not remove until then!
}

void Parser::playHarp() {
	if (_vm->_animation->inField(6))
		_vm->_dialogs->displayMusicalScroll();
	else
		_vm->_dialogs->displayText("Get a bit closer to it, Avvy!");
}

void Parser::winSequence() {
	_vm->_dialogs->displayScrollChain('Q', 78);
	_vm->_sequence->startWinSeq();
	_vm->_timer->addTimer(30, Timer::kProcWinning, Timer::kReasonWinning);
}

/**
 * @remarks	Originally called 'do_that'
 */
void Parser::doThat() {
	static const char booze[8][8] = {"Bitter", "GIED", "Whisky", "Cider", "", "", "", "Mead"};
	static const char kWhat[] = "That's not possible!";

	if (_thats == Common::String(kNothing))  {
		if (!_thats.empty())
			_thats.clear();
		return;
	}

	if (_weirdWord)
		return;

	if (_thing < 200)
		// "Slip" object
		_thing -= 49;

	if (_vm->_tiedUp) {
		_vm->_dialogs->displayText("You better stay quiet now!");
		return;
	}

	if ((_verb != kVerbCodeLoad) && (_verb != kVerbCodeSave) && (_verb != kVerbCodeQuit) && (_verb != kVerbCodeInfo) && (_verb != kVerbCodeHelp)
	&& (_verb != kVerbCodeLarrypass) && (_verb != kVerbCodePhaon) && (_verb != kVerbCodeBoss) && (_verb != kVerbCodeCheat) && (_verb != kVerbCodeRestart)
	&& (_verb != kVerbCodeDir) && (_verb != kVerbCodeScore) && (_verb != kVerbCodeHiscores) && (_verb != kVerbCodeSmartAlec)) {
		if (!_vm->_alive) {
			_vm->_dialogs->displayText("You're dead, so don't talk. What are you, a ghost or something? " \
				"Try restarting, or restoring a saved game!");
			return;
		}
		if (!_vm->_avvyIsAwake && (_verb != kVerbCodeWake)) {
			_vm->_dialogs->displayText("Talking in your sleep? Try waking up!");
			return;
		}
	}

	switch (_verb) {
	case kVerbCodeExam:
		examine();
		break;
	case kVerbCodeOpen:
		openDoor();
		break;
	case kVerbCodePause: {
		// Note that the original game doesn't care about the "O.K." box neither, it accepts
		// clicks from everywhere on the screen to continue. Just like my code.
		Common::String tmpStr = Common::String::format("Game paused.%c%c%cPress Enter, Esc, or click the mouse on the `O.K.\" " \
			"box to continue.", kControlCenter, kControlNewLine, kControlNewLine);
		_vm->_dialogs->displayText(tmpStr);
		}
		break;
	case kVerbCodeGet:
		if (_thing != kPardon) {
			// Legitimate try to pick something up.
			if (_vm->_carryNum >= kCarryLimit)
				_vm->_dialogs->displayText("You can't carry any more!");
			else
				getProc(_thing);
		} else {
			// Not... ditto.
			if (_person != kPeoplePardon)
				_vm->_dialogs->displayText("You can't sweep folk off their feet!");
			else
				_vm->_dialogs->displayText("I assure you, you don't need it.");
		}
		break;
	case kVerbCodeDrop:
		_vm->_dialogs->displayText("Two years ago you dropped a florin in the street. Three days " \
				"later it was gone! So now you never leave ANYTHING lying around. OK?");
		break;
	case kVerbCodeInv:
		inventory();
		break;
	case kVerbCodeTalk:
		if (_person == kPeoplePardon) {
			if (_vm->_subjectNum == 99) {
				// They typed "say password".
				Common::String tmpStr = Common::String::format("Yes, but what %cis%c the password?", kControlItalic, kControlRoman);
				_vm->_dialogs->displayText(tmpStr);
			} else if (((1 <= _vm->_subjectNum) && (_vm->_subjectNum <= 49)) || (_vm->_subjectNum == 253) || (_vm->_subjectNum == 249)) {
				_thats.deleteChar(0);

				for (int i = 0; i < 10; i++)
					_realWords[i] = _realWords[i + 1];

				_verb = (VerbCode)_vm->_subjectNum;
				doThat();
				return;
			} else {
				_person = (People)_vm->_subjectNum;
				_vm->_subjectNum = 0;
				if ((_person == kPeopleNone) || (_person == kPeoplePardon))
					_vm->_dialogs->displayText("Talk to whom?");
				else if (isPersonHere())
					_vm->_dialogs->talkTo(_person);
			}
		} else if (isPersonHere())
			_vm->_dialogs->talkTo(_person);
		break;
	case kVerbCodeGive:
		if (isHolding()) {
			if (_person == kPeoplePardon)
				_vm->_dialogs->displayText("Give to whom?");
			else if (isPersonHere()) {
				switch (_thing) {
				case kObjectMoney :
					_vm->_dialogs->displayText("You can't bring yourself to give away your moneybag.");
					break;
				case kObjectBodkin:
				case kObjectBell:
				case kObjectClothes:
				case kObjectHabit :
					_vm->_dialogs->displayText("Don't give it away, it might be useful!");
					break;
				default:
					switch (_person) {
					case kPeopleCrapulus:
						if (_thing == kObjectWine) {
							_vm->_dialogs->displayText("Crapulus grabs the wine and gulps it down.");
							_vm->_objects[kObjectWine - 1] = false;
						} else
							_vm->_dialogs->sayThanks(_thing - 1);
						break;
					case kPeopleCwytalot:
						if ((_thing == kObjectCrossbow) || (_thing == kObjectBolt))
							_vm->_dialogs->displayText("You might be able to influence Cwytalot more if you used it!");
						else
							_vm->_dialogs->sayThanks(_thing - 1);
						break;
					case kPeopleSpludwick:
						if (giveToSpludwick())
							_vm->_dialogs->sayThanks(_thing - 1);
						break;
					case kPeopleIbythneth:
						if (_thing == kObjectBadge) {
							_vm->_dialogs->displayScrollChain('Q', 32); // Thanks! Wow!
							_vm->incScore(3);
							_vm->_objects[kObjectBadge - 1] = false;
							_vm->_objects[kObjectHabit - 1] = true;
							_vm->_givenBadgeToIby = true;
							_vm->_background->draw(-1, -1, 7);
							_vm->_background->draw(-1, -1, 8);
						} else
							_vm->_dialogs->sayThanks(_thing - 1);
						break;
					case kPeopleAyles:
						if (_vm->_aylesIsAwake) {
							if (_thing == kObjectPen) {
								_vm->_objects[kObjectPen - 1] = false;
								_vm->_dialogs->displayScrollChain('Q', 54);
								_vm->_objects[kObjectInk - 1] = true;
								_vm->_givenPenToAyles = true;
								_vm->refreshObjectList();
								_vm->incScore(2);
							} else
								_vm->_dialogs->sayThanks(_thing - 1);
						} else
							_vm->_dialogs->displayText("But he's asleep!");
						break;
					case kPeopleGeida:
						switch (_thing) {
						case kObjectPotion:
							_vm->_objects[kObjectPotion - 1] = false;
							// She drinks it.
							_vm->_dialogs->displayScrollChain('U', 16);
							_vm->incScore(2);
							_vm->_givenPotionToGeida = true;
							_vm->refreshObjectList();
							break;
						case kObjectLute:
							giveGeidaTheLute();
							break;
						default:
							_vm->_dialogs->sayThanks(_thing - 1);
						}
						break;
					case kPeopleArkata:
						switch (_thing) {
						case kObjectPotion:
							if (_vm->_givenPotionToGeida)
								winSequence();
							else
								// That Geida woman!
								_vm->_dialogs->displayScrollChain('Q', 77);
							break;
						default:
							_vm->_dialogs->sayThanks(_thing - 1);
						}
						break;
					default:
						_vm->_dialogs->sayThanks(_thing - 1);
					}
				}
			}
			// Just in case...
			_vm->refreshObjectList();
		}
		break;

	case kVerbCodeEat:
	case kVerbCodeDrink:
		if (isHolding())
			swallow();
		break;

	case kVerbCodeLoad: {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		int16 savegameId = dialog->runModalWithCurrentTarget();
		delete dialog;

		if (savegameId < 0)
			// dialog aborted, nothing to load
			return;

		_vm->loadGame(savegameId);
		}
		break;
	case kVerbCodeSave: {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		int16 savegameId = dialog->runModalWithCurrentTarget();
		Common::String savegameDescription = dialog->getResultString();
		delete dialog;

		if (savegameId < 0)
			// dialog aborted, nothing to save
			return;

		_vm->saveGame(savegameId, savegameDescription);
		}
		break;
	case kVerbCodePay:
		_vm->_dialogs->displayText("No money need change hands.");
		break;
	case kVerbCodeLook:
		lookAround();
		break;
	case kVerbCodeBreak:
		_vm->_dialogs->displayText("Vandalism is prohibited within this game!");
		break;
	case kVerbCodeQuit:
		if (!_polite)
			_vm->_dialogs->displayText("How about a `please\", Avvy?");
		else {
			Common::String tmpStr = Common::String::format("%cC%cDo you really want to quit?", kControlRegister, kControlIcon);
			if (_vm->_dialogs->displayQuestion(tmpStr))
				_vm->_letMeOut = true;
		}
		break;
	case kVerbCodeGo:
		_vm->_dialogs->displayText("Just use the arrow keys to walk there.");
		break;
	case kVerbCodeInfo: {
		_vm->_dialogs->_aboutBox = true;

		Common::String toDisplay;
		for (int i = 0; i < 7; i++)
			toDisplay += kControlNewLine;
		toDisplay = toDisplay + "LORD AVALOT D'ARGENT" + kControlCenter + kControlNewLine
			+ "The medi\x91val descendant of" + kControlNewLine
			+ "Denarius Avaricius Sextus" + kControlNewLine + kControlNewLine
			+ "version " + kVersionNum + kControlNewLine + kControlNewLine + "Copyright \xEF "
			+ kCopyright + ", Mark, Mike and Thomas Thurman." + kControlRegister + 'Y' + kControlIcon;
		_vm->_dialogs->displayText(toDisplay);
		_vm->_dialogs->_aboutBox = false;
		}
		break;
	case kVerbCodeUndress:
		if (_wearing == kNothing)
			_vm->_dialogs->displayText("You're already stark naked!");
		else if (_vm->_avvysInTheCupboard) {
			Common::String tmpStr = Common::String::format("You take off %s.", _vm->getItem(_wearing).c_str());
			_vm->_dialogs->displayText(tmpStr);
			_wearing = kNothing;
			_vm->refreshObjectList();
		} else
			_vm->_dialogs->displayText("Hadn't you better find somewhere more private, Avvy?");
		break;
	case kVerbCodeWear:
		if (isHolding()) { // Wear something.
			switch (_thing) {
			case kObjectChastity:
				// \? are used to avoid that ??! is parsed as a trigraph
				_vm->_dialogs->displayText("Hey, what kind of a weirdo are you\?\?!");
				break;
			case kObjectClothes:
			case kObjectHabit: {
				// Change clothes!
				if (_wearing != kNothing) {
					if (_wearing == _thing)
						_vm->_dialogs->displayText("You're already wearing that.");
					else
						_vm->_dialogs->displayText("You'll be rather warm wearing two sets of clothes!");
					return;
				} else
					_wearing = _thing;

				_vm->refreshObjectList();

				byte i;
				if (_thing == kObjectHabit)
					i = 3;
				else
					i = 0;

				_vm->_animation->setAvvyClothes(i);
				}
				break;
			default:
				_vm->_dialogs->displayText(kWhat);
			}
		}
		break;
	case kVerbCodePlay:
		if (_thing == kPardon) {
			// They just typed "play"...
			switch (_vm->_room) {
			case kRoomArgentPub:
				_vm->_nim->playNim(); // ...in the pub, => play Nim.
				break;
			case kRoomMusicRoom:
				playHarp();
				break;
			default:
				break;
			}
		} else if (isHolding()) {
			switch (_thing) {
			case kObjectLute :
					_vm->_dialogs->displayScrollChain('U', 7);

					if (_vm->getRoom(kPeopleCwytalot) == _vm->_room)
						_vm->_dialogs->displayScrollChain('U', 10);

					if (_vm->getRoom(kPeopleDuLustie) == _vm->_room)
						_vm->_dialogs->displayScrollChain('U', 15);
				break;
			case 52:
				if (_vm->_room == kRoomMusicRoom)
					playHarp();
				else
					_vm->_dialogs->displayText(kWhat);
				break;
			case 55:
				if (_vm->_room == kRoomArgentPub)
					_vm->_nim->playNim();
				else
					_vm->_dialogs->displayText(kWhat);
				break;
			default:
				_vm->_dialogs->displayText(kWhat);
			}
		}
		break;
	case kVerbCodeRing:
		if (isHolding()) {
			if (_thing == kObjectBell) {
				_vm->_dialogs->displayText("Ding, dong, ding, dong, ding, dong, ding, dong...");
				if ((_vm->_bellsAreRinging) & (_vm->getFlag('B')))
					// '\?' are used to avoid that '??!' is parsed as a trigraph
					_vm->_dialogs->displayText("(Are you trying to join in, Avvy\?\?!)");
			} else
				_vm->_dialogs->displayText(kWhat);
		}
		break;
	case kVerbCodeHelp:
		_vm->_help->run();
		break;
	case kVerbCodeLarrypass:
		_vm->_dialogs->displayText("Wrong game!");
		break;
	case kVerbCodePhaon:
		_vm->_dialogs->displayText("Hello, Phaon!");
		break;
	case kVerbCodeBoss:
		bossKey();
		break;
	case kVerbCodePee:
		if (_vm->getFlag('P')) {
			_vm->_dialogs->displayText("Hmm, I don't think anyone will notice...");
			_vm->_timer->addTimer(4, Timer::kProcUrinate, Timer::kReasonGoToToilet);
		} else {
			Common::String tmpStr = Common::String::format("It would be %cVERY%c unwise to do that here, Avvy!", kControlItalic, kControlRoman);
			_vm->_dialogs->displayText(tmpStr);
		}
		break;
	case kVerbCodeCheat: {
		Common::String tmpStr = Common::String::format("%cCheat mode now enabled.", kControlItalic);
		_vm->_dialogs->displayText(tmpStr);
		_vm->_cheat = true;
		}
		break;
	case kVerbCodeMagic:
		if (_vm->_avariciusTalk > 0)
			_vm->_dialogs->displayScrollChain('Q', 19);
		else {
			if ((_vm->_room == kRoomSpludwicks) & (_vm->_animation->inField(1))) {
				// Avaricius appears!
				_vm->_dialogs->displayScrollChain('Q', 17);
				if (_vm->getRoom(kPeopleSpludwick) == kRoomSpludwicks)
					_vm->_dialogs->displayScrollChain('Q', 18);
				else {
					Avalanche::AnimationType *spr = _vm->_animation->_sprites[1];
					// Avaricius
					spr->init(1, false);
					_vm->_animation->appearPed(1, 3);
					spr->walkTo(4);
					spr->_callEachStepFl = true;
					spr->_eachStepProc = Animation::kProcBackAndForth;
					_vm->_avariciusTalk = 14;
					_vm->_timer->addTimer(177, Timer::kProcAvariciusTalks, Timer::kReasonAvariciusTalks);
				}
			} else
				_vm->_dialogs->displayText("Nothing appears to happen...");
		}
		break;
	case kVerbCodeSmartAlec:
		_vm->_dialogs->displayText("Listen, smart alec, that was just rhetoric.");
		break;
	case kVerbCodeExpletive:
		switch (_sworeNum) {
		case 0: {
			Common::String tmpStr = Common::String::format("Avvy! Do you mind? There might be kids playing!%c%c" \
				"(I shouldn't say it again, if I were you!)", kControlNewLine, kControlNewLine);
			_vm->_dialogs->displayText(tmpStr);
			}
			break;
		case 1: {
			Common::String tmpStr = Common::String::format("You hear a distant rumble of thunder. Must you always" \
				"do things I tell you not to?%c%cDon't do it again!", kControlNewLine, kControlNewLine);
			_vm->_dialogs->displayText(tmpStr);
			}
			break;
		default: {
			_vm->_animation->thunder();
			Common::String tmpStr = Common::String::format("A crack of lightning shoots from the sky, and fries you." \
				"%c%c(`Such is the anger of the gods, Avvy!\")", kControlNewLine, kControlNewLine);
			_vm->_dialogs->displayText(tmpStr);
			_vm->gameOver();
			}
		}
		_sworeNum++;
		break;
	case kVerbCodeListen:
		if ((_vm->_bellsAreRinging) & (_vm->getFlag('B')))
			_vm->_dialogs->displayText("All other noise is drowned out by the ringing of the bells.");
		else if (_vm->_listen.empty())
			_vm->_dialogs->displayText("You can't hear anything much at the moment, Avvy.");
		else
			_vm->_dialogs->displayText(_vm->_listen);
		break;
	case kVerbCodeBuy:
		// What are they trying to buy?
		switch (_vm->_room) {
		case kRoomArgentPub:
			// We're in a pub, and near the bar.
			if (_vm->_animation->inField(5)) {
				switch (_thing) {
				case 51:
				case 53:
				case 54:
				case 58:
					// Beer, whisky, cider or mead.
					if (_vm->_malagauche == 177) {
						// Already getting us one.
						_vm->_dialogs->displayScrollChain('D', 15);
						return;
					}

					if (_vm->_teetotal)  {
						_vm->_dialogs->displayScrollChain('D', 6);
						return;
					}

					if (_alcoholLevel == 0)
						_vm->incScore(3);

					_vm->_background->draw(-1, -1, 11);
					_vm->_dialogs->displayText(Common::String(booze[_thing - 51]) + ", please." + kControlRegister + '1' + kControlSpeechBubble);
					_vm->_drinking = _thing;

					_vm->_background->draw(-1, -1, 9);
					_vm->_malagauche = 177;
					_vm->_timer->addTimer(27, Timer::kProcBuyDrinks, Timer::kReasonDrinks);
					break;
				case 52:
					examine();
					break; // We have a right one here - buy Pepsi??!
				case kObjectWine:
					if (_vm->_objects[kObjectWine - 1])
						// We've already got the wine!
						// 1 bottle's shufishent!
						_vm->_dialogs->displayScrollChain('D', 2);
					else {
						if (_vm->_malagauche == 177) {
							// Already getting us one.
							_vm->_dialogs->displayScrollChain('D', 15);
							return;
						}

						if (_vm->_carryNum >= kCarryLimit) {
							_vm->_dialogs->displayText("Your hands are full.");
							return;
						}

						_vm->_background->draw(-1, -1, 11);
						Common::String tmpStr = Common::String::format("Wine, please.%c1%c", kControlRegister, kControlSpeechBubble);
						_vm->_dialogs->displayText(tmpStr);
						if (_alcoholLevel == 0)
							_vm->incScore(3);
						_vm->_background->draw(-1, -1, 9);
						_vm->_malagauche = 177;

						_vm->_timer->addTimer(27, Timer::kProcBuyWine, Timer::kReasonDrinks);
					}
					break;
				default:
					break;
				}
			} else
				// Go to the bar!
				_vm->_dialogs->displayScrollChain('D', 5);
			break;

		case kRoomOutsideDucks:
			if (_vm->_animation->inField(5)) {
				if (_thing == kObjectOnion) {
					if (_vm->_objects[kObjectOnion - 1])
						// Not planning to juggle with the things!
						_vm->_dialogs->displayScrollChain('D', 10);
					else if (_vm->_carryNum >= kCarryLimit)
						_vm->_dialogs->displayText("Before you ask, you remember that your hands are full.");
					else {
						if (_boughtOnion)
							_vm->_dialogs->displayScrollChain('D', 11);
						else {
							_vm->_dialogs->displayScrollChain('D', 9);
							_vm->incScore(3);
						}
						// It costs thruppence.
						_vm->decreaseMoney(3);
						_vm->_objects[kObjectOnion - 1] = true;
						_vm->refreshObjectList();
						_boughtOnion = true;
						// It's OK when it leaves the stall!
						_vm->_rottenOnion = false;
						_vm->_onionInVinegar = false;
					}
				} else
					_vm->_dialogs->displayScrollChain('D', 0);
			} else
				_vm->_dialogs->displayScrollChain('D', 0);
			break;

		case kRoomNottsPub:
			// Can't sell to southerners.
			_vm->_dialogs->displayScrollChain('N', 15);
			break;
		default:
			// Can't buy that.
			_vm->_dialogs->displayScrollChain('D', 0);
		}
		break;
	case kVerbCodeAttack:
		if ((_vm->_room == kRoomBrummieRoad) &&
			((_person == kPeopleCwytalot) || (_thing == kObjectCrossbow) || (_thing == kObjectBolt)) &&
			(_vm->getRoom(kPeopleCwytalot) == _vm->_room)) {
			switch (_vm->_objects[kObjectBolt - 1] + _vm->_objects[kObjectCrossbow - 1] * 2) {
				// 0 = neither, 1 = only bolt, 2 = only crossbow, 3 = both.
			case 0:
				_vm->_dialogs->displayScrollChain('Q', 10);
				_vm->_dialogs->displayText("(At the very least, don't use your bare hands!)");
				break;
			case 1:
				_vm->_dialogs->displayText("Attack _vm->him with only a crossbow bolt? Are you planning on playing darts?!");
				break;
			case 2:
				_vm->_dialogs->displayText("Come on, Avvy! You're not going to get very far with only a crossbow!");
				break;
			case 3:
				_vm->_dialogs->displayScrollChain('Q', 11);
				_vm->_cwytalotGone = true;
				_vm->_objects[kObjectBolt - 1] = false;
				_vm->_objects[kObjectCrossbow - 1] = false;
				_vm->refreshObjectList();
				_vm->_magics[11]._operation = kMagicNothing;
				_vm->incScore(7);
				_vm->_animation->_sprites[1]->walkTo(1);
				_vm->_animation->_sprites[1]->_vanishIfStill = true;
				_vm->_animation->_sprites[1]->_callEachStepFl = false;
				_vm->setRoom(kPeopleCwytalot, kRoomDummy);
				break;
			default:
				// Please try not to be so violent!
				_vm->_dialogs->displayScrollChain('Q', 10);
			}
		} else
			_vm->_dialogs->displayScrollChain('Q', 10);
		break;
	case kVerbCodePasswd:
		if (_vm->_room != kRoomBridge)
			_vm->_dialogs->displayScrollChain('Q', 12);
		else {
			bool ok = true;
			for (uint i = 0; i < _thats.size(); i++) {
				Common::String temp = _realWords[i];
				temp.toUppercase();
				int pwdId = _vm->_passwordNum + kFirstPassword;
				for (uint j = 0; j < _vocabulary[pwdId]._word.size(); j++) {
					if (_vocabulary[pwdId]._word[j] != temp[j])
						ok = false;
				}
			}

			if (ok) {
				if (_vm->_drawbridgeOpen != 0)
					_vm->_dialogs->displayText("Contrary to your expectations, the drawbridge fails to close again.");
				else {
					_vm->incScore(4);
					_vm->_dialogs->displayText("The drawbridge opens!");
					_vm->_timer->addTimer(7, Timer::kProcOpenDrawbridge, Timer::kReasonDrawbridgeFalls);
					_vm->_drawbridgeOpen = 1;
				}
			} else
				_vm->_dialogs->displayScrollChain('Q', 12);
		}
		break;
	case kVerbCodeDie:
		_vm->gameOver();
		break;
	case kVerbCodeScore: {
		Common::String tmpStr = Common::String::format("Your score is %d,%c%cout of a possible 128.%c%c " \
			"This gives you a rank of %s.%c%c%s", _vm->_score, kControlCenter, kControlNewLine, kControlNewLine,
			kControlNewLine, rank().c_str(), kControlNewLine, kControlNewLine, totalTime().c_str());
		_vm->_dialogs->displayText(tmpStr);
		}
		break;
	case kVerbCodePut:
		putProc();
		break;
	case kVerbCodeStand:
		standUp();
		break;
	case kVerbCodeKiss:
		if (_person == kPeoplePardon)
			_vm->_dialogs->displayText("Kiss whom?");
		else if (isPersonHere()) {
			switch (_person) {
			case kPeopleArkata:
				_vm->_dialogs->displayScrollChain('U', 12);
				break;
			case kPeopleGeida:
				_vm->_dialogs->displayScrollChain('U', 13);
				break;
			case kPeopleWisewoman:
				_vm->_dialogs->displayScrollChain('U', 14);
				break;
			default:
				// You WHAT?
				_vm->_dialogs->displayScrollChain('U', 5);
			}
		} else if ((kPeopleAvalot <= _person) && (_person < kPeopleArkata))
			_vm->_dialogs->displayText("Hey, what kind of a weirdo are you??");

		break;
	case kVerbCodeClimb:
		if (_vm->_room == kRoomInsideCardiffCastle)
			cardiffClimbing();
		else
			// In the wrong room!
			_vm->_dialogs->displayText("Not with your head for heights, Avvy!");
		break;
	case kVerbCodeJump:
		_vm->_timer->addTimer(1, Timer::kProcJump, Timer::kReasonJumping);
		_vm->_userMovesAvvy = false;
		break;
	case kVerbCodeHiscores:
		//	show_highs();
		warning("STUB: Parser::doThat() - case kVerbCodehighscores");
		break;
	case kVerbCodeWake:
		if (isPersonHere())
			switch (_person) {
			case kPeoplePardon:
			case kPeopleAvalot:
			case 0:
				if (!_vm->_avvyIsAwake) {
					_vm->_avvyIsAwake = true;
					_vm->incScore(1);
					_vm->_avvyInBed = true;
					// Picture of Avvy, awake in bed.
					_vm->_background->draw(-1, -1, 2);
					if (_vm->_teetotal)
						_vm->_dialogs->displayScrollChain('D', 13);
				} else
					_vm->_dialogs->displayText("You're already awake, Avvy!");
				break;
			case kPeopleAyles:
				if (!_vm->_aylesIsAwake)
					_vm->_dialogs->displayText("You can't seem to wake him by yourself.");
				break;
			case kPeopleJacques: {
				Common::String tmpStr = Common::String::format("Brother Jacques, Brother Jacques, are you asleep?%c1%c" \
					"Hmmm... that doesn't seem to do any good...", kControlRegister, kControlSpeechBubble);
				_vm->_dialogs->displayText(tmpStr);
				}
				break;
			default:
				_vm->_dialogs->displayText("It's difficult to awaken people who aren't asleep...!");
		}
		break;
	case kVerbCodeSit:
		if (_vm->_room == kRoomNottsPub) {
			if (_vm->_sittingInPub)
				_vm->_dialogs->displayText("You're already sitting!");
			else {
				// Move Avvy to the place, and sit him down.
				_vm->_animation->_sprites[0]->walkTo(3);
				_vm->_timer->addTimer(1, Timer::kProcAvvySitDown, Timer::kReasonSittingDown);
			}
		} else {
			// Default doodah.
			_vm->fadeOut();
			_vm->fadeIn();
			Common::String tmpStr = Common::String::format("A few hours later...%cnothing much has happened...", kControlParagraph);
			_vm->_dialogs->displayText(tmpStr);
		}
		break;
	case kVerbCodeRestart:
		if (_vm->_dialogs->displayQuestion("Restart game and lose changes?"))  {
			_vm->fadeOut();
			_vm->newGame();
			_vm->fadeIn();
		}
		break;
	case kVerbCodePardon:
		_vm->_dialogs->displayText("Hey, a verb would be helpful!");
		break;
	case kVerbCodeHello:
		_vm->_dialogs->sayHello();
		break;
	case kVerbCodeThanks:
		_vm->_dialogs->sayOK();
		break;
	default:
		Common::String tmpStr = Common::String::format("%cUnhandled verb: %d", kControlBell, _verb);
		_vm->_dialogs->displayText(tmpStr);
	}
}

void Parser::bossKey() {
	_vm->_graphics->saveScreen();
	_vm->_graphics->blackOutScreen();
	_vm->_graphics->loadMouse(kCurUpArrow);
	_vm->loadBackground(98);
	_vm->_graphics->drawNormalText("Graph/Histo/Draw/Sample: \"JANJUN93.GRA\": (W3-AB3)", _vm->_font, 8, 120, 169, kColorDarkgray);
	_vm->_graphics->drawNormalText("Press any key or click the mouse to return.", _vm->_font, 8, 144, 182, kColorDarkgray);
	_vm->_graphics->refreshScreen();
	Common::Event event;
	_vm->getEvent(event);
	while ((!_vm->shouldQuit()) && (event.type != Common::EVENT_KEYDOWN) && (event.type != Common::EVENT_LBUTTONDOWN)){
		_vm->getEvent(event);
		_vm->_graphics->refreshScreen();
	}
	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
	_vm->loadBackground(_vm->_room);
}

void Parser::verbOpt(byte verb, Common::String &answer, char &ansKey) {
	// kVerbCodegive isn't dealt with by this procedure, but by ddm__with.
	switch (verb) {
	case kVerbCodeExam:
		answer = "Examine";
		ansKey = 'x';
		break;
	case kVerbCodeDrink:
		answer = "Drink";
		ansKey = 'D';
		break;
	case kVerbCodeWear:
		answer = "Wear";
		ansKey = 'W';
		break;
	case kVerbCodeRing:
		answer = "Ring";
		ansKey = 'R';
		break; // Only the bell!
	case kVerbCodePlay:
		answer = "Play";
		ansKey = 'P';
		break;
	case kVerbCodeEat:
		answer = "Eat";
		ansKey = 'E';
		break;
	default:
		answer = "? Unknown!"; // Bug!
		ansKey = '?';
	}
}

void Parser::doVerb(VerbCode id) {
	_weirdWord = false;
	_polite = true;
	_verb = id;
	doThat();
}

void Parser::resetVariables() {
	_wearing = kNothing;
	_sworeNum = 0;
	_alcoholLevel = 0;
	_boughtOnion = false;
}

void Parser::synchronize(Common::Serializer &sz) {
	sz.syncAsByte(_wearing);
	sz.syncAsByte(_sworeNum);
	sz.syncAsByte(_alcoholLevel);
	if (sz.isLoading() && sz.getVersion() < 2) {
		int dummy;
		sz.syncAsByte(dummy);
	}
	sz.syncAsByte(_boughtOnion);
}

} // End of namespace Avalanche
