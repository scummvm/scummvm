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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

/* ACCIDENCE II		The parser. */

#include "avalanche/avalanche.h"

#include "avalanche/acci2.h"
#include "avalanche/gyro2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/visa2.h"
#include "avalanche/timer.h"
#include "avalanche/animation.h"
#include "avalanche/enid2.h"
#include "avalanche/celer2.h"
#include "avalanche/pingo2.h"
#include "avalanche/sequence2.h"

#include "common/textconsole.h"

#include <cstring>
#include <cmath>

namespace Avalanche {

const Acci::VocabEntry Acci::kVocabulary[kParserWordsNum] = {
	// Verbs: 1-49
	{1, "EXAMINE"},      {1, "READ"},      {1, "XAM"}, // short
	{2, "OPEN"},         {2, "LEAVE"},     {2, "UNLOCK"},
	{3, "PAUSE"},        {47, "TA"}, // Early to avoid Take and Talk.
	{4, "TAKE"},         {4, "GET"},       {4, "PICK"},
	{5, "DROP"},         {6, "INVENTORY"}, {7, "TALK"},
	{7, "SAY"},          {7, "ASK"},
	{8, "GIVE"},         {9, "DRINK"},     {9, "IMBIBE"},
	{9, "DRAIN"},        {10, "LOAD"},     {10, "RESTORE"},
	{11, "SAVE"},        {12, "BRIBE"},    {12, "PAY"},
	{13, "LOOK"},        {14, "BREAK"},    {15, "QUIT"},
	{15, "EXIT"},        {16, "SIT"},      {16, "SLEEP"},
	{17, "STAND"},
	{18, "GO"},          {19, "INFO"},      {20, "UNDRESS"},
	{20, "DOFF"},
	{21, "DRESS"},       {21, "WEAR"},      {21, "DON"},
	{22, "PLAY"},
	{22, "STRUM"},       {23, "RING"},      {24, "HELP"},
	{25, "KENDAL"},      {26, "CAPYBARA"},  {27, "BOSS"},
	{255, "NINET"}, // block for NINETY
	{28, "URINATE"},     {28, "MINGITE"},   {29, "NINETY"},
	{30, "ABRACADABRA"}, {30, "PLUGH"},     {30, "XYZZY"},
	{30, "HOCUS"},       {30, "POCUS"},     {30, "IZZY"},
	{30, "WIZZY"},       {30, "PLOVER"},
	{30, "MELENKURION"}, {30, "ZORTON"},    {30, "BLERBI"},
	{30, "THURB"},       {30, "SNOEZE"},    {30, "SAMOHT"},
	{30, "NOSIDE"},      {30, "PHUGGG"},    {30, "KNERL"},
	{30, "MAGIC"},       {30, "KLAETU"},    {30, "VODEL"},
	{30, "BONESCROLLS"}, {30, "RADOF"},
	{31, "RESTART"},
	{32, "SWALLOW"},     {32, "EAT"},       {33, "LISTEN"},
	{33, "HEAR"},        {34, "BUY"},       {34, "PURCHASE"},
	{34, "ORDER"},       {34, "DEMAND"},
	{35, "ATTACK"},      {35, "HIT"},       {35, "KILL"},
	{35, "PUNCH"},       {35, "KICK"},      {35, "SHOOT"},
	{35, "FIRE"},

	// Passwords: 36
	{36, "TIROS"},     {36, "WORDY"},     {36, "STACK"},
	{36, "SHADOW"},    {36, "OWL"},       {36, "ACORN"},
	{36, "DOMESDAY"},  {36, "FLOPPY"},    {36, "DIODE"},
	{36, "FIELD"},     {36, "COWSLIP"},   {36, "OSBYTE"},
	{36, "OSCLI"},     {36, "TIMBER"},    {36, "ADVAL"},
	{36, "NEUTRON"},   {36, "POSITRON"},  {36, "ELECTRON"},
	{36, "CIRCUIT"},   {36, "AURUM"},     {36, "PETRIFY"},
	{36, "EBBY"},      {36, "CATAPULT"},  {36, "GAMERS"},
	{36, "FUDGE"},     {36, "CANDLE"},    {36, "BEEB"},
	{36, "MICRO"},     {36, "SESAME"},    {36, "LORDSHIP"},
	{37, "DIR"},       {37, "LS"},        {38, "DIE"},
	{39, "SCORE"},
	{40, "PUT"},       {40, "INSERT"},    {41, "KISS"},
	{41, "SNOG"},      {41, "CUDDLE"},    {42, "CLIMB"},
	{42, "CLAMBER"},   {43, "JUMP"},      {44, "HIGHSCORES"},
	{44, "HISCORES"},  {45, "WAKEN"},     {45, "AWAKEN"},
	{46, "HELLO"},     {46, "HI"},        {46, "YO"},
	{47, "THANKS"}, // = 47, "ta", which was defined earlier.

	// Nouns - Objects: 50-100
	{50, "WINE"},       {50, "BOOZE"},    {50, "NASTY"},
	{50, "VINEGAR"},    {51, "MONEYBAG"},
	{51, "BAG"},        {51, "CASH"},     {51, "DOSH"},
	{51, "WALLET"},
	{52, "BODKIN"},     {52, "DAGGER"},   {53, "POTION"},
	{54, "CHASTITY"},   {54, "BELT"},     {55, "BOLT"},
	{55, "ARROW"},      {55, "DART"},
	{56, "CROSSBOW"},   {56, "BOW"},      {57, "LUTE"},
	{58, "PILGRIM"},    {58, "BADGE"},    {59, "MUSHROOMS"},
	{59, "TOADSTOOLS"}, {60, "KEY"},      {61, "BELL"},
	{62, "PRESCRIPT"},  {62, "SCROLL"},   {62, "MESSAGE"},
	{63, "PEN"},        {63, "QUILL"},    {64, "INK"},
	{64, "INKPOT"},     {65, "CLOTHES"},  {66, "HABIT"},
	{66, "DISGUISE"},   {67, "ONION"},
	{99, "PASSWORD"},

	// Objects from Also are placed between 101 and 131.

	// Nouns - People - Male: 150-174
	{150, "AVVY"},       {150, "AVALOT"},    {150, "YOURSELF"},
	{150, "ME"},         {150, "MYSELF"},    {151, "SPLUDWICK"},
	{151, "THOMAS"},     {151, "ALCHEMIST"}, {151, "CHEMIST"},
	{152, "CRAPULUS"},   {152, "SERF"},      {152, "SLAVE"},
	{158, "DU"}, // <<< Put in early for Baron DU Lustie to save confusion with Duck & Duke.
	{152, "CRAPPY"},     {153, "DUCK"},      {153, "DOCTOR"},
	{154, "MALAGAUCHE"},
	{155, "FRIAR"},      {155, "TUCK"},      {156, "ROBIN"},
	{156, "HOOD"},       {157, "CWYTALOT"},  {157, "GUARD"},
	{157, "BRIDGEKEEP"}, {158, "BARON"},     {158, "LUSTIE"},
	{159, "DUKE"},       {159, "GRACE"},     {160, "DOGFOOD"},
	{160, "MINSTREL"},   {161, "TRADER"},    {161, "SHOPKEEPER"},
	{161, "STALLHOLDER"},
	{162, "PILGRIM"},    {162, "IBYTHNETH"}, {163, "ABBOT"},
	{163, "AYLES"},      {164, "PORT"},      {165, "SPURGE"},
	{166, "JACQUES"},    {166, "SLEEPER"},   {166, "RINGER"},

	// Nouns - People - Female: 175-199
	{175, "WIFE"},      {175, "ARKATA"},    {176, "GEDALODAVA"},
	{176, "GEIDA"},     {176, "PRINCESS"},  {178, "WISE"},
	{178, "WITCH"},

	// Pronouns: 200-224
	{200, "HIM"},       {200, "MAN"},       {200, "GUY"},
	{200, "DUDE"},      {200, "CHAP"},      {200, "FELLOW"},
	{201, "HER"},       {201, "GIRL"},      {201, "WOMAN"},
	{202, "IT"},        {202, "THING"},
	{203, "MONK"},      {204, "BARMAN"},    {204, "BARTENDER"},

	// Prepositions: 225-249
	{225, "TO"},        {226, "AT"},        {227, "UP"},
	{228, "INTO"},      {228, "INSIDE"},    {229, "OFF"},
	{230, "UP"},        {231, "DOWN"},      {232, "ON"},

	// Please: 251
	{251, "PLEASE"},

	// About: 252
	{252, "ABOUT"}, {252, "CONCERNING"},

	// Swear words: 253
	/*		  I M P O R T A N T    M E S S A G E

	DO *NOT* READ THE LINES BELOW IF YOU ARE OF A SENSITIVE
	DISPOSITION. THOMAS IS *NOT* RESPONSIBLE FOR THEM.
	GOODNESS KNOWS WHO WROTE THEM.
	READ THEM AT YOUR OWN RISK. BETTER STILL, DON'T READ THEM.
	WHY ARE YOU SNOOPING AROUND IN MY PROGRAM, ANYWAY? */

	{253, "SHIT"},      {28 , "PISS"},    {28 , "PEE"},
	{253, "FART"},      {253, "FUCK"},    {253, "BALLS"},
	{253, "BLAST"},     {253, "BUGGER"},  {253, "KNICKERS"},
	{253, "BLOODY"},    {253, "HELL"},    {253, "DAMN"},
	{253, "SMEG"},
	// ...and other even ruder words. You didn't read them, did you? Good. */

	// Answer-back smart-alec words: 249
	{249, "YES"},       {249, "NO"},        {249, "BECAUSE"},

	// Noise words: 255
	{255, "THE"},       {255, "A"},         {255, "NOW"},
	{255, "SOME"},      {255, "AND"},       {255, "THAT"},
	{255, "POCUS"},     {255, "HIS"},
	{255, "THIS"},      {255, "SENTINEL"} // for "Ken SENT Me"
};

Acci::Acci(AvalancheEngine *vm) {
	_vm = vm;
}

void Acci::init() {
	_vm->_gyro->_weirdWord = false;
}

void Acci::clearWords() {
	for (byte i = 0; i < 11; i++) {
		if (!_realWords[i].empty())
			_realWords[i].clear();
	}
}

byte Acci::wordNum(Common::String word) {
	if (word.empty())
		return 0;

	for (int32 i = kParserWordsNum - 1; i >= 0; i--) {
		if (kVocabulary[i]._word == word)
			return kVocabulary[i]._number;
	}

	// If not found as a whole, we look for it as a substring.
	for (int32 i = kParserWordsNum - 1; i >= 0; i--) {
		if (Common::String(kVocabulary[i]._word.c_str(), word.size()) == word)
			return kVocabulary[i]._number;
	}

	return kPardon;
}

void Acci::replace(Common::String oldChars, byte newChar) {
	int16 pos = _vm->_parser->pos(oldChars, _thats);
	while (pos != -1) {
		if (newChar == 0)
			_thats.deleteChar(pos);
		else {
			for (byte i = pos; i < pos + oldChars.size(); i++)
				_thats.deleteChar(pos);
			_thats.insertChar(newChar, pos);
		}
		pos = _vm->_parser->pos(oldChars, _thats);
	}
}

Common::String Acci::rank() {
	static const RankType kRanks[9] = {
		{0, "Beginner"},     {10, "Novice"},
		{20, "Improving"},   {35, "Not bad"},
		{50, "Passable"},    {65, "Good"},
		{80, "Experienced"}, {108, "The BEST!"},
		{32767, "copyright'93"}
	};

	for (byte i = 0; i < 8; i++) {
		if ((_vm->_gyro->_dna._score >= kRanks[i]._score) && (_vm->_gyro->_dna._score < kRanks[i + 1]._score)) {
			return kRanks[i]._title;
		}
	}
	return "";
}

Common::String Acci::totalTime() {
	// There are 65535 clock ticks in a second, 1092.25 in a minute, and 65535 in an hour.
	const double ticksInOneSec = (double)(65535) / 3600;
	uint16 h, m, s;

	h = _vm->_gyro->_dna._totalTime / ticksInOneSec; // No. of seconds.
	h = floor((float)h);
	m = h % 3600;
	h = h / 3600;
	s = m % 60;
	m = m / 60;

	Common::String result = "You've been playing for ";
	if (h > 0)
		result += _vm->_gyro->intToStr(h) + " hours, ";
	if ((m > 0) || (h != 0))
		result += _vm->_gyro->intToStr(m) + " minutes and ";
	return result + _vm->_gyro->intToStr(s) + " seconds.";
}

void Acci::cheatParse(Common::String codes) {
//	uint16 num;
//	int16 e;
//	char cmd;
//	int16 se, sx, sy;

	warning("STUB: Acci::cheatParse()");
}



void Acci::stripPunctuation(Common::String &word) {
	const char punct[] = "~`!@#$%^&*()_+-={}[]:\"|;'\\,./<>?";

	for (byte i = 0; i < 32; i++) {
		for (;;) {
			int16 pos = _vm->_parser->pos(Common::String(punct[i]), word);
			if (pos == -1)
				break;
			word.deleteChar(pos);
		}
	}
}

void Acci::displayWhat(byte target, bool animate, bool &ambiguous) {
	if (target == kPardon) {
		ambiguous = true;
		if (animate)
			_vm->_scrolls->displayText("Whom?");
		else
			_vm->_scrolls->displayText("What?");
	} else {
		if (animate) {
			Common::String tmpStr = Common::String::format("{ %s }", _vm->_gyro->getName(target).c_str());
			_vm->_scrolls->displayText(tmpStr);
		} else {
			Common::String z = _vm->_gyro->getItem(target);
			if (z != "") {
				Common::String tmpStr = Common::String::format("{ %s }", z.c_str());
				_vm->_scrolls->displayText(tmpStr);
			}
		}
	}
}

bool Acci::doPronouns() {
	bool ambiguous = false;

	for (byte i = 0; i < _thats.size(); i++) {
		byte wordCode = _thats[i];
		switch (wordCode) {
		case 200: {
			displayWhat(_vm->_gyro->_him, true, ambiguous);
			_thats.setChar(_vm->_gyro->_him, i);
			}
			break;
		case 201: {
			displayWhat(_vm->_gyro->_her, true, ambiguous);
			_thats.setChar(_vm->_gyro->_her, i);
			}
			break;
		case 202: {
			displayWhat(_vm->_gyro->_it, false, ambiguous);
			_thats.setChar(_vm->_gyro->_it, i);
			}
			break;
		}
	}

	return ambiguous;
}

void Acci::properNouns() {
	_vm->_parser->_inputText.toLowercase();

	// We set every word's first character to uppercase.
	for (byte i = 1; i < (_vm->_parser->_inputText.size() - 1); i++) {
		if (_vm->_parser->_inputText[i] == ' ')
			_vm->_parser->_inputText.setChar(toupper(_vm->_parser->_inputText[i + 1]), i + 1);
	}

	// And the first character as well.
	_vm->_parser->_inputText.setChar(toupper(_vm->_parser->_inputText[0]), 0);
}

void Acci::sayIt() {
	Common::String x = _vm->_parser->_inputText;
	x.setChar(toupper(x[0]), 0);
	Common::String tmpStr = Common::String::format("%c1%s.%c%c2", Scrolls::kControlRegister, x.c_str(), Scrolls::kControlSpeechBubble, Scrolls::kControlRegister);
	_vm->_scrolls->displayText(tmpStr);
}

void Acci::storeInterrogation(byte interrogation) {
	if (_vm->_parser->_inputText.empty())
		return;

	// Strip _vm->_parser->_inputText:
	while ((_vm->_parser->_inputText[0] == ' ') && (!_vm->_parser->_inputText.empty()))
		_vm->_parser->_inputText.deleteChar(0);
	while ((_vm->_parser->_inputText.lastChar() == ' ') && (!_vm->_parser->_inputText.empty()))
		_vm->_parser->_inputText.deleteLastChar();

	_vm->_timer->loseTimer(Timer::kReasonCardiffsurvey); // If you want to use any other timer, put this into the case statement.

	switch (interrogation) {
	case 1:
		_vm->_parser->_inputText.toLowercase();
		sayIt();
		_vm->_gyro->_dna._favouriteDrink = _vm->_parser->_inputText;
		_vm->_gyro->_dna._cardiffQuestionNum = 2;
		break;
	case 2:
		properNouns();
		sayIt();
		_vm->_gyro->_dna._favouriteSong = _vm->_parser->_inputText;
		_vm->_gyro->_dna._cardiffQuestionNum = 3;
		break;
	case 3:
		properNouns();
		sayIt();
		_vm->_gyro->_dna._worstPlaceOnEarth = _vm->_parser->_inputText;
		_vm->_gyro->_dna._cardiffQuestionNum = 4;
		break;
	case 4:
		_vm->_parser->_inputText.toLowercase();
		sayIt();
		if (!_vm->_gyro->_dna._spareEvening.empty())
			_vm->_gyro->_dna._spareEvening.clear();
		_vm->_gyro->_dna._spareEvening = _vm->_parser->_inputText;
		_vm->_visa->displayScrollChain('z', 5); // His closing statement...
		_vm->_animation->_sprites[1].walkTo(4); // The end of the drawbridge
		_vm->_animation->_sprites[1]._vanishIfStill = true; // Then go away!
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicNothing;
		_vm->_gyro->_dna._cardiffQuestionNum = 5;
		break;
	case 99:
		//store_high(_vm->_parser->_inputText);
		warning("STUB: Acci::store_interrogation()");
		break;
	}

	if (interrogation < 4)
		_vm->_timer->cardiffSurvey();
}



void Acci::parse() {
	// First parsing - word identification
	if (!_thats.empty())
		_thats.clear();

	byte n = 0;
	_polite = false;
	_verb = kPardon;
	_thing = kPardon;
	_thing2 = kPardon;
	_person = kPardon;
	clearWords();


	// A cheat mode attempt.
	if (_vm->_parser->_inputText[0] == '.') {
		cheatParse(_vm->_parser->_inputText);
		_thats = kNothing;
		return;
	}

	// Are we being interrogated right now?
	if (_vm->_gyro->_interrogation > 0) {
		storeInterrogation(_vm->_gyro->_interrogation);
		_vm->_gyro->_weirdWord = true;
		return;
	}

	// Actually process the command.
	Common::String inputText = _vm->_parser->_inputText + ' ';
	Common::String inputTextUpper = inputText;
	inputTextUpper.toUppercase();
	while (!inputTextUpper.empty()) {
		while ((!inputTextUpper.empty()) && (inputTextUpper[0] == ' ')) {
			inputTextUpper.deleteChar(0);
			inputText.deleteChar(0);
		}
		if (inputTextUpper.empty())
			break;

		// Get the following word of the strings.
		byte size = _vm->_parser->pos(Common::String(' '), inputTextUpper) + 1;
		char *subStr = new char[size];
		Common::strlcpy(subStr, inputTextUpper.c_str(), size);
		Common::String thisword = subStr;
		Common::strlcpy(subStr, inputText.c_str(), size);
		_realWords[n] = subStr;
		delete[] subStr;

		stripPunctuation(inputTextUpper);

		bool notfound = true;

		// Check also[] first, which conatins words about the actual room.
		if (!thisword.empty()) {
			for (byte i = 0; i < 31; i++) {
				if ((_vm->_gyro->_also[i][0] != 0) && (_vm->_parser->pos(',' + thisword, *_vm->_gyro->_also[i][0]) > -1)) {
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
		int16 spacePos = _vm->_parser->pos(Common::String(' '), inputTextUpper);
		if (spacePos > -1) {
			for (byte i = 0; i <= spacePos; i++)
				inputTextUpper.deleteChar(0);
		}

		spacePos = _vm->_parser->pos(Common::String(' '), inputText);
		if (spacePos > -1) {
			for (byte i = 0; i <= spacePos; i++)
				inputText.deleteChar(0);
		}
	}

	Common::String unkString;
	int16 pos = _vm->_parser->pos(Common::String('\xFE'), _thats);
	if (pos > -1)
		unkString = _realWords[pos];
	else
		unkString.clear();

	// Replace words' codes that mean the same.
	replace(Common::String('\xFF'), 0); // zap noise words
	replace(Common::String('\xD')  + '\xE2', 1); // "look at" = "examine"
	replace(Common::String('\xD')  + '\xE4', 1); // "look in" = "examine"
	replace(Common::String('\x4')  + '\xE6', 17); // "get up" = "stand"
	replace(Common::String('\x4')  + '\xE7', 17); // "get down" = "stand"... well, why not?
	replace(Common::String('\x12') + '\xE4', 2); // "go in" = "open [door]"
	replace(Common::String('\x1C') + '\xE5', 253); // "P' off" is a swear word
	replace(Common::String('\x4')  + '\x6' , 6); // "Take inventory" (remember Colossal Adventure?)
	replace(Common::String('\x28') + '\xE8', 21); // "put on" = "don"
	replace(Common::String('\x4')  + '\xE5', 20); // "take off" = "doff"

	// Words that could mean more than one _person
	if (_vm->_gyro->_dna._room == r__nottspub)
		replace(Common::String('\xCC'), 164); // Barman = Port
	else
		replace(Common::String('\xCC'), 154); // Barman = Malagauche

	switch (_vm->_gyro->_dna._room) {
	case r__aylesoffice:
		replace(Common::String('\xCB'), 163); // Monk = Ayles
		break;
	case r__musicroom:
		replace(Common::String('\xCB'), 166); // Monk = Jacques
		break;
	default:
		replace(Common::String('\xCB'), 162); // Monk = Ibythneth
	}

	if (doPronouns()) {
		_vm->_gyro->_weirdWord = true;
		_thats = kNothing;
		return;
	}

	// Second parsing.
	if (!_vm->_gyro->_subject.empty())
		_vm->_gyro->_subject.clear();
	_vm->_gyro->_subjectNum = 0; // Find subject of conversation.

	for (int i = 0; (i < 11) && !_realWords[i].empty(); i++) {
		if ((_realWords[i][0] == '\'') || (_realWords[i][0] == '\"')) {
			_vm->_gyro->_subjectNum = (byte)_thats[i];
			_thats.setChar(kMoved, i);
			break;
		}
	}

	if ((_vm->_gyro->_subjectNum == 0) && !_thats.empty()) { // Still not found.
		for (uint16 i = 0; i < _thats.size() - 1; i++) {
			if ((byte)_thats[i] == 252) { // The word is "about", or something similar.
				_vm->_gyro->_subjectNum = (byte)_thats[i + 1];
				_thats.setChar(0, i + 1);
				break;
			}
		}
	}

	if ((_vm->_gyro->_subjectNum == 0) && !_thats.empty()) { // STILL not found! Must be the word after "say".
		for (uint16 i = 0; i < _thats.size() - 1; i++) {
			if (((byte)_thats[i] == 7) && ((byte)_thats[i + 1] != 0) && !((225 <= (byte)_thats[i + 1]) && ((byte)_thats[i + 1] <= 229))) {
				// SAY not followed by a preposition
				_vm->_gyro->_subjectNum = (byte)_thats[i + 1];
				_thats.setChar(0, i + 1);
				break;
			}
		}
	}

	for (int16 i = _thats.size() - 1; i >= 0; i--) { // Reverse order, so first will be used.
		if (((byte)_thats[i] == 253) || ((byte)_thats[i] == 249) || ((1 <= (byte)_thats[i]) && ((byte)_thats[i] <= 49)))
			_verb = (byte)_thats[i];
		else if ((50 <= (byte)_thats[i]) && ((byte)_thats[i] <= 149)) {
			_thing2 = _thing;
			_thing = (byte)_thats[i];
		} else if ((150 <= (byte)_thats[i]) && ((byte)_thats[i] <= 199))
			_person = (byte)_thats[i];
		else if ((byte)_thats[i] == 251)
			_polite = true;
	}

	if ((!unkString.empty()) && (_verb != kVerbCodeExam) && (_verb != kVerbCodeTalk) && (_verb != kVerbCodeSave) && (_verb != kVerbCodeLoad) && (_verb != kVerbCodeDir)) {
		Common::String tmpStr = Common::String::format("Sorry, but I have no idea what \"%s\" means. Can you rephrase it?", unkString.c_str());
		_vm->_scrolls->displayText(tmpStr);
		_vm->_gyro->_weirdWord = true;
	} else
		_vm->_gyro->_weirdWord = false;

	if (_thats.empty())
		_thats = kNothing;

	if (_thing != kPardon)
		_vm->_gyro->_it = _thing;

	if (_person != kPardon) {
		if (_person < 175)
			_vm->_gyro->_him = _person;
		else
			_vm->_gyro->_her = _person;
	}
}

void Acci::examineObject() {
	if (_thing != _vm->_gyro->_thinks)
		_vm->_lucerna->thinkAbout(_thing, Gyro::kThing);
	switch (_thing) {
	case Gyro::kObjectWine :
		switch (_vm->_gyro->_dna._wineState) {// 4 is perfect wine. 0 is not holding the wine.
		case 1:
			_vm->_visa->displayScrollChain('t', 1); // Normal examine wine scroll
			break;
		case 2:
			_vm->_visa->displayScrollChain('d', 6); // Bad wine
			break;
		case 3:
			_vm->_visa->displayScrollChain('d', 7); // Vinegar
			break;
		}
		break;
	case Gyro::kObjectOnion:
		if (_vm->_gyro->_dna._rottenOnion)
			_vm->_visa->displayScrollChain('q', 21); // Yucky onion.
		else
			_vm->_visa->displayScrollChain('t', 18);  // Normal onion scroll
		break;
	default:
		_vm->_visa->displayScrollChain('t', _thing); // <<< Ordinarily
	}
}

bool Acci::isPersonHere() { // Person equivalent of "holding".
	if ((_person == kPardon) || (_person == 0) || (_vm->_gyro->_whereIs[_person - 150] == _vm->_gyro->_dna._room))
		return true;
	else {
		Common::String tmpStr;
		if (_person < 175)
			tmpStr = Common::String::format("H%ce isn't around at the moment.", Scrolls::kControlToBuffer);
		else
			tmpStr = Common::String::format("Sh%ce isn't around at the moment.", Scrolls::kControlToBuffer);
		_vm->_scrolls->displayText(tmpStr);
		return false;
	}
}

void Acci::exampers() {
	if (isPersonHere()) {
		if (_thing != _vm->_gyro->_thinks)
			_vm->_lucerna->thinkAbout(_person, Gyro::kPerson);
		_person -= 149;
		switch (_person) { // Special cases
		case 11:
			if (_vm->_gyro->_dna._wonNim) {
				_vm->_visa->displayScrollChain('Q', 8); // "I'm Not Playing!"
				return;
			}
			break;
		case 99:
			if (_vm->_gyro->_dna._lustieIsAsleep) {
				_vm->_visa->displayScrollChain('Q', 65); // He's asleep. (65! Wow!)
				return;
			}
			break;
		}
		// Otherwise...
		_vm->_visa->displayScrollChain('p', _person);
	}

	// And afterwards...
	if ((_person == 14) && (!_vm->_gyro->_dna._aylesIsAwake))
		_vm->_visa->displayScrollChain('Q', 13);
}

/**
 * Return whether Avvy is holding an object or not
 * @remarks	Originally called 'holding'
 */
bool Acci::isHolding() {
	if ((51 <= _thing) && (_thing <= 99)) // Also.
		return true;

	bool holdingResult = false;

	if (_thing > 100)
		_vm->_scrolls->displayText("Be reasonable!");
	else if (!_vm->_gyro->_dna._objects[_thing - 1])  // Verbs that need "_thing" to be in the inventory.
		_vm->_scrolls->displayText("You're not holding it, Avvy.");
	else
		holdingResult = true;

	return holdingResult;
}

void Acci::openBox(bool isOpening) {
	if ((_vm->_gyro->_dna._room == r__yours) && (_thing == 54)) {
		_vm->_celer->drawBackgroundSprite(-1, -1, 5);

		_vm->_celer->updateBackgroundSprites();
		_vm->_animation->animLink();
		_vm->_graphics->refreshScreen();

		_vm->_system->delayMillis(55);

		if (!isOpening) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			_vm->_celer->updateBackgroundSprites();
			_vm->_animation->animLink();
			_vm->_graphics->refreshScreen();
		}
	}
}

void Acci::examine() {
	// EITHER it's an object OR it's an Also OR it's a _person OR it's something else.
	if ((_person == kPardon) && (_thing != kPardon)) {
		if (isHolding()) {
			// Remember: it's been Slipped! Ie subtract 49.
			if ((1 <= _thing) && (_thing <= 49)) // Standard object
				examineObject();
			else if ((50 <= _thing) && (_thing <= 100)) { // Also _thing
				openBox(true);
				_vm->_scrolls->displayText(*_vm->_gyro->_also[_thing - 50][1]);
				openBox(false);
			}
		}
	} else if (_person != kPardon)
		exampers();
	else
		_vm->_scrolls->displayText("It's just as it looks on the picture.");  // Don't know: guess.
}

void Acci::inventory() {
	byte itemNum = 0;
	_vm->_scrolls->displayText(Common::String("You're carrying ") + Scrolls::kControlToBuffer);

	for (byte i = 0; i < kObjectNum; i++) {
		if (_vm->_gyro->_dna._objects[i]) {
			itemNum++;
			if (itemNum == _vm->_gyro->_dna._carryNum)
				_vm->_scrolls->displayText(Common::String("and ") + Scrolls::kControlToBuffer);
			_vm->_scrolls->displayText(_vm->_gyro->getItem(i + 1) + Scrolls::kControlToBuffer);
			if ((i + 1) == _vm->_gyro->_dna._wearing)
				_vm->_scrolls->displayText(Common::String(", which you're wearing") + Scrolls::kControlToBuffer);
			if (itemNum < _vm->_gyro->_dna._carryNum)
				_vm->_scrolls->displayText(Common::String(", ") + Scrolls::kControlToBuffer);
		}
	}

	if (_vm->_gyro->_dna._wearing == kNothing)
		_vm->_scrolls->displayText(Common::String("...") + Scrolls::kControlNewLine + Scrolls::kControlNewLine + "...and you're stark naked!");
	else
		_vm->_scrolls->displayText(".");
}

void Acci::swallow() {   // Eat something.
	switch (_thing) {
	case Gyro::kObjectWine:
		switch (_vm->_gyro->_dna._wineState) { // 4 is perfect
		case 1:
			if (_vm->_gyro->_dna._teetotal)  {
				_vm->_visa->displayScrollChain('D', 6);
				return;
			}
			_vm->_visa->displayScrollChain('U', 1);
			_vm->_pingo->wobble();
			_vm->_visa->displayScrollChain('U', 2);
			_vm->_gyro->_dna._objects[Gyro::kObjectWine - 1] = false;
			_vm->_lucerna->refreshObjectList();
			drink();
			break;
		case 2:
		case 3:
			_vm->_visa->displayScrollChain('d', 8);
			break; // You can't drink it!
		}
		break;
	case Gyro::kObjectPotion:
		_vm->_gyro->setBackgroundColor(4);
		_vm->_visa->displayScrollChain('U', 3);
		_vm->_lucerna->gameOver();
		_vm->_gyro->setBackgroundColor(0);
		break;
	case Gyro::kObjectInk:
		_vm->_visa->displayScrollChain('U', 4);
		break;
	case Gyro::kObjectChastity:
		_vm->_visa->displayScrollChain('U', 5);
		break;
	case Gyro::kObjectMushroom:
		_vm->_visa->displayScrollChain('U', 6);
		_vm->_lucerna->gameOver();
		break;
	case Gyro::kObjectOnion:
		if (_vm->_gyro->_dna._rottenOnion)
			_vm->_visa->displayScrollChain('U', 11);
		else {
			_vm->_visa->displayScrollChain('U', 8);
			_vm->_gyro->_dna._objects[Gyro::kObjectOnion - 1] = false;
			_vm->_lucerna->refreshObjectList();
		}
		break;
	default:
		if ((_vm->_gyro->_dna._room == r__argentpub) || (_vm->_gyro->_dna._room == r__nottspub))
			_vm->_scrolls->displayText("Try BUYing things before you drink them!");
		else
			_vm->_scrolls->displayText("The taste of it makes you retch!");
	}
}

void Acci::peopleInRoom() {
	byte numPeople = 0; // Number of people in the room.

	for (byte i = 1; i < 29; i++) { // Start at 1 so we don't list Avvy himself!
		if (_vm->_gyro->_whereIs[i] == _vm->_gyro->_dna._room)
			numPeople++;
	}

	if (numPeople == 0) // If nobody's here, we can cut out straight away.
		return;

	byte actPerson = 0; // Actually listed people.
	for (byte i = 1; i < 29; i++) {
		if (_vm->_gyro->_whereIs[i] == _vm->_gyro->_dna._room) {
			actPerson++;
			if (actPerson == 1) // First on the list.
				_vm->_scrolls->displayText(_vm->_gyro->getName(i + 150) + Scrolls::kControlToBuffer);
			else if (actPerson < numPeople) // The middle...
				_vm->_scrolls->displayText(Common::String(", ") + _vm->_gyro->getName(i + 150) + Scrolls::kControlToBuffer);
			else // The end.
				_vm->_scrolls->displayText(Common::String(" and ") + _vm->_gyro->getName(i + 150) + Scrolls::kControlToBuffer);
		}
	}

	if (numPeople == 1)
		_vm->_scrolls->displayText(Common::String(" is") + Scrolls::kControlToBuffer);
	else
		_vm->_scrolls->displayText(Common::String(" are") + Scrolls::kControlToBuffer);

	_vm->_scrolls->displayText(" here."); // End and display it.
}

void Acci::lookAround() {
	_vm->_scrolls->displayText(*_vm->_gyro->_also[0][1]);
	switch (_vm->_gyro->_dna._room) {
	case r__spludwicks:
		if (_vm->_gyro->_dna._avariciusTalk > 0)
			_vm->_visa->displayScrollChain('q', 23);
		else
			peopleInRoom();
		break;
	case r__robins:
		if (_vm->_gyro->_dna._tiedUp)
			_vm->_visa->displayScrollChain('q', 38);
		if (_vm->_gyro->_dna._mushroomGrowing)
			_vm->_visa->displayScrollChain('q', 55);
		break;
	case r__insidecardiffcastle:
		if (!_vm->_gyro->_dna._takenPen)
			_vm->_visa->displayScrollChain('q', 49);
		break;
	case r__lustiesroom:
		if (_vm->_gyro->_dna._lustieIsAsleep)
			_vm->_visa->displayScrollChain('q', 65);
		break;
	case r__catacombs:
		switch (_vm->_gyro->_dna._catacombY * 256 + _vm->_gyro->_dna._catacombX) {
		case 258 :
			_vm->_visa->displayScrollChain('q', 80); // Inside art gallery.
			break;
		case 514 :
			_vm->_visa->displayScrollChain('q', 81); // Outside ditto.
			break;
		case 260 :
			_vm->_visa->displayScrollChain('q', 82); // Outside Geida's room.
			break;
		}
		break;
	default:
		peopleInRoom();
	}
}

void Acci::openDoor() {
	// Special cases.
	switch (_vm->_gyro->_dna._room) {
	case r__yours:
		if (_vm->_animation->inField(2)) {
			// Opening the box.
			_thing = 54; // The box.
			_person = kPardon;
			examine();
			return;
		}
		break;
	case r__spludwicks:
		if (_thing == 61) {
			_vm->_visa->displayScrollChain('q', 85);
			return;
		}
		break;
	}

	if ((!_vm->_gyro->_dna._userMovesAvvy) && (_vm->_gyro->_dna._room != r__lusties))
		return; // No doors can open if you can't move Avvy.

	for (byte fv = 8; fv < 15; fv++) {
		if (_vm->_animation->inField(fv + 1)) {
			fv -= 8;

			switch (_vm->_gyro->_portals[fv]._operation) {
			case Gyro::kMagicExclaim:
				_vm->_animation->_sprites[0].bounce();
				_vm->_visa->displayScrollChain('x', _vm->_gyro->_portals[fv]._data);
				break;
			case Gyro::kMagicTransport:
				_vm->_animation->flipRoom((_vm->_gyro->_portals[fv]._data) >> 8,  // High byte
					                 (_vm->_gyro->_portals[fv]._data) & 0x0F // Low byte
									 );
				break;
			case Gyro::kMagicUnfinished:
				_vm->_animation->_sprites[0].bounce();
				_vm->_scrolls->displayText("Sorry. This place is not available yet!");
				break;
			case Gyro::kMagicSpecial:
				_vm->_animation->callSpecial(_vm->_gyro->_portals[fv]._data);
				break;
			case Gyro::kMagicOpenDoor:
				_vm->_animation->openDoor((_vm->_gyro->_portals[fv]._data) >> 8, (_vm->_gyro->_portals[fv]._data) & 0x0F, fv + 9);
				break;
			}

			return;
		}
	}

	if (_vm->_gyro->_dna._room == r__map)
		_vm->_scrolls->displayText(Common::String("Avvy, you can complete the whole game without ever going "
				"to anywhere other than Argent, Birmingham, Cardiff, Nottingham and Norwich."));
	else
		_vm->_scrolls->displayText("Door? What door?");
}



void Acci::silly() {
	_vm->_scrolls->displayText("Don't be silly!");
}

void Acci::putProc() {
	if (!isHolding())
		return;

	_thing2 -= 49; // Slip the second object.
	char temp = _thing;
	_thing = _thing2;
	if (!isHolding())
		return;
	_thing = temp;

	// Thing is the _thing which you're putting in. _thing2 is where you're putting it.
	switch (_thing2) {
	case Gyro::kObjectWine:
		if (_thing == Gyro::kObjectOnion) {
			if (_vm->_gyro->_dna._rottenOnion)
				_vm->_scrolls->displayText("That's a bit like shutting the stable door after the horse has bolted!");
			else { // Put onion into wine?
				if (_vm->_gyro->_dna._wineState != 3) {
					Common::String tmpStr = Common::String::format("%cOignon au vin%c is a bit too strong for your tastes!", Scrolls::kControlItalic, Scrolls::kControlRoman);
					_vm->_scrolls->displayText(tmpStr);
				} else { // Put onion into vinegar! Yes!
					_vm->_gyro->_dna._onionInVinegar = true;
					_vm->_lucerna->incScore(7);
					_vm->_visa->displayScrollChain('u', 9);
				}
			}
		} else
			silly();
		break;

	case 54:
		if (_vm->_gyro->_dna._room == r__yours) { // Put something into the box.
			if (_vm->_gyro->_dna._boxContent != kNothing)
				_vm->_scrolls->displayText("There's something in the box already, Avvy. Try taking that out first.");
			else {
				switch (_thing) {
				case Gyro::kObjectMoney:
					_vm->_scrolls->displayText("You'd better keep some ready cash on you!");
					break;
				case Gyro::kObjectBell:
					_vm->_scrolls->displayText("That's a silly place to keep a bell.");
					break;
				case Gyro::kObjectBodkin:
					_vm->_scrolls->displayText("But you might need it!");
					break;
				case Gyro::kObjectOnion:
					_vm->_scrolls->displayText("Just give it to Spludwick, Avvy!");
					break;
				default: // Put the object into the box...
					if (_vm->_gyro->_dna._wearing == _thing) {
						Common::String tmpStr = Common::String::format("You'd better take %s off first!", _vm->_gyro->getItem(_thing).c_str());
						_vm->_scrolls->displayText(tmpStr);
					} else {
						openBox(true); // Open box.

						_vm->_gyro->_dna._boxContent = _thing;
						_vm->_gyro->_dna._objects[_thing - 1] = false;
						_vm->_lucerna->refreshObjectList();
						_vm->_scrolls->displayText("OK, it's in the box.");

						openBox(false); // Shut box.
					}
				}
			}
		} else
			silly();
		break;

	default:
		silly();
	}
}

/**
 * Display text when ingredients are not in the right order
 * @remarks	Originally called 'not_in_order'
 */
void Acci::notInOrder() {
	Common::String tmpStr = Common::String::format("Sorry, I need the ingredients in the right order for this potion. " \
		"What I need next is %s%c2%c", _vm->_gyro->getItem(_vm->_gyro->kSpludwicksOrder[_vm->_gyro->_dna._givenToSpludwick]),
		Scrolls::kControlRegister, Scrolls::kControlSpeechBubble);
	_vm->_scrolls->displayText(tmpStr);
}

/**
 * Move Spludwick to cauldron
 * @remarks	Originally called 'go_to_cauldron'
 */
void Acci::goToCauldron() {
	_vm->_animation->_sprites[1]._callEachStepFl = false; // Stops Geida_Procs.
	_vm->_timer->addTimer(1, Timer::kProcSpludwickGoesToCauldron, Timer::kReasonSpludwickWalk);
	_vm->_animation->_sprites[1].walkTo(2);
}

/**
 * Check is it's possible to give something to Spludwick
 * @remarks	Originally called 'give2spludwick'
 */
bool Acci::giveToSpludwick() {
	if (_vm->_gyro->kSpludwicksOrder[_vm->_gyro->_dna._givenToSpludwick] != _thing) {
		notInOrder();
		return false;
	}

	switch (_thing) {
	case Gyro::kObjectOnion:
		_vm->_gyro->_dna._objects[Gyro::kObjectOnion - 1] = false;
		if (_vm->_gyro->_dna._rottenOnion)
			_vm->_visa->displayScrollChain('q', 22);
		else {
			_vm->_gyro->_dna._givenToSpludwick++;
			_vm->_visa->displayScrollChain('q', 20);
			goToCauldron();
			_vm->_lucerna->incScore(3);
		}
		_vm->_lucerna->refreshObjectList();
		break;
	case Gyro::kObjectInk:
		_vm->_gyro->_dna._objects[Gyro::kObjectInk - 1] = false;
		_vm->_lucerna->refreshObjectList();
		_vm->_gyro->_dna._givenToSpludwick++;
		_vm->_visa->displayScrollChain('q', 24);
		goToCauldron();
		_vm->_lucerna->incScore(3);
		break;
	case Gyro::kObjectMushroom:
		_vm->_gyro->_dna._objects[Gyro::kObjectMushroom - 1] = false;
		_vm->_visa->displayScrollChain('q', 25);
		_vm->_lucerna->incScore(5);
		_vm->_gyro->_dna._givenToSpludwick++;
		goToCauldron();
		_vm->_gyro->_dna._objects[Gyro::kObjectPotion - 1] = true;
		_vm->_lucerna->refreshObjectList();
		break;
	default:
		return true;
	}

	return false;
}

void Acci::drink() {
	_vm->_gyro->_dna._alcoholLevel += 1;
	if (_vm->_gyro->_dna._alcoholLevel == 5) {
		_vm->_gyro->_dna._objects[Gyro::kObjectKey - 1] = true; // Get the key.
		_vm->_gyro->_dna._teetotal = true;
		_vm->_gyro->_dna._avvyIsAwake = false;
		_vm->_gyro->_dna._avvyInBed = true;
		_vm->_lucerna->refreshObjectList();
		_vm->_lucerna->dusk();
		_vm->_gyro->hangAroundForAWhile();
		_vm->_animation->flipRoom(1, 1);
		_vm->_gyro->setBackgroundColor(14);
		_vm->_animation->_sprites[0]._visible = false;
	}
}

void Acci::cardiffClimbing() {
	if (_vm->_gyro->_dna._standingOnDais) { // Clamber up.
		_vm->_scrolls->displayText("You climb down, back onto the floor.");
		_vm->_gyro->_dna._standingOnDais = false;
		_vm->_animation->appearPed(1, 3);
	} else { // Clamber down.
		if (_vm->_animation->inField(1)) {
			_vm->_scrolls->displayText("You clamber up onto the dais.");
			_vm->_gyro->_dna._standingOnDais = true;
			_vm->_animation->appearPed(1, 2);
		} else
			_vm->_scrolls->displayText("Get a bit closer, Avvy.");
	}
}

void Acci::already() {
	_vm->_scrolls->displayText("You're already standing!");
}

void Acci::standUp() {
	switch (_vm->_gyro->_dna._room) {
	case r__yours: // Avvy isn't asleep.
		if (_vm->_gyro->_dna._avvyIsAwake && _vm->_gyro->_dna._avvyInBed) {  // But he's in bed.
			if (_vm->_gyro->_dna._teetotal) {
				_vm->_visa->displayScrollChain('d', 12);
				_vm->_gyro->setBackgroundColor(0);
				_vm->_visa->displayScrollChain('d', 14);
			}
			_vm->_animation->_sprites[0]._visible = true;
			_vm->_gyro->_dna._userMovesAvvy = true;
			_vm->_animation->appearPed(1, 2);
			_vm->_gyro->_dna._direction = _vm->_gyro->kDirectionLeft;
			_vm->_celer->drawBackgroundSprite(-1, -1, 4); // Picture of empty pillow.
			_vm->_lucerna->incScore(1);
			_vm->_gyro->_dna._avvyInBed = false;
			_vm->_timer->loseTimer(Timer::kReasonArkataShouts);
		} else
			already();
		break;

	case r__insidecardiffcastle:
		cardiffClimbing();
		break;

	case r__nottspub:
		if (_vm->_gyro->_dna._sittingInPub)  {
			_vm->_celer->drawBackgroundSprite(-1, -1, 4); // Not sitting down.
			_vm->_animation->_sprites[0]._visible = true; // But standing up.
			_vm->_animation->appearPed(1, 4); // And walking away.
			_vm->_gyro->_dna._sittingInPub = false; // Really not sitting down.
			_vm->_gyro->_dna._userMovesAvvy = true; // And ambulant.
		} else
			already();
		break;
	default:
		already();
	}
}

void Acci::getProc(char thing) {
	switch (_vm->_gyro->_dna._room) {
	case r__yours:
		if (_vm->_animation->inField(2)) {
			if (_vm->_gyro->_dna._boxContent == thing) {
				_vm->_celer->drawBackgroundSprite(-1, -1, 5);
				_vm->_scrolls->displayText("OK, I've got it.");
				_vm->_gyro->_dna._objects[thing - 1] = true;
				_vm->_lucerna->refreshObjectList();
				_vm->_gyro->_dna._boxContent = kNothing;
				_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			} else {
				Common::String tmpStr = Common::String::format("I can't see %s in the box.", _vm->_gyro->getItem(thing));
				_vm->_scrolls->displayText(tmpStr);
			}
		} else
			_vm->_visa->displayScrollChain('q', 57);
		break;
	case r__insidecardiffcastle:
		switch (thing) {
		case Gyro::kObjectPen:
			if (_vm->_animation->inField(2)) { // Standing on the dais.
				if (_vm->_gyro->_dna._takenPen)
					_vm->_scrolls->displayText("It's not there, Avvy.");
				else {
					// OK: we're taking the pen, and it's there.
					_vm->_celer->drawBackgroundSprite(-1, -1, 4); // No pen there now.
					_vm->_animation->callSpecial(3); // Zap!
					_vm->_gyro->_dna._takenPen = true;
					_vm->_gyro->_dna._objects[Gyro::kObjectPen - 1] = true;
					_vm->_lucerna->refreshObjectList();
					_vm->_scrolls->displayText("Taken.");
				}
			} else if (_vm->_gyro->_dna._standingOnDais)
				_vm->_visa->displayScrollChain('q', 53);
			else
				_vm->_visa->displayScrollChain('q', 51);
			break;
		case Gyro::kObjectBolt:
			_vm->_visa->displayScrollChain('q', 52);
			break;
		default:
			_vm->_visa->displayScrollChain('q', 57);
		}
		break;
	case r__robins:
		if ((thing == Gyro::kObjectMushroom) & (_vm->_animation->inField(1)) & (_vm->_gyro->_dna._mushroomGrowing)) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_scrolls->displayText("Got it!");
			_vm->_gyro->_dna._mushroomGrowing = false;
			_vm->_gyro->_dna._takenMushroom = true;
			_vm->_gyro->_dna._objects[Gyro::kObjectMushroom - 1] = true;
			_vm->_lucerna->refreshObjectList();
			_vm->_lucerna->incScore(3);
		} else
			_vm->_visa->displayScrollChain('q', 57);
		break;
	default:
		_vm->_visa->displayScrollChain('q', 57);
	}
}

/**
 * Give the lute to Geida
 * @remarks	Originally called 'give_Geida_the_lute'
 */
void Acci::giveGeidaTheLute() {
	if (_vm->_gyro->_dna._room != r__lustiesroom) {
		_vm->_scrolls->displayText(Common::String("Not yet. Try later!") + Scrolls::kControlRegister + '2' + Scrolls::kControlSpeechBubble);
		return;
	}
	_vm->_gyro->_dna._objects[Gyro::kObjectLute - 1] = false;
	_vm->_lucerna->refreshObjectList();
	_vm->_visa->displayScrollChain('q', 64); // She plays it.

	_vm->_timer->addTimer(1, Timer::kProcGiveLuteToGeida, Timer::kReasonGeidaSings);
	_vm->_enid->backToBootstrap(4);
}

void Acci::playHarp() {
	if (_vm->_animation->inField(7))
		_vm->_scrolls->musicalScroll();
	else
		_vm->_scrolls->displayText("Get a bit closer to it, Avvy!");
}

void Acci::winSequence() {
	_vm->_visa->displayScrollChain('q', 78);
	_vm->_sequence->firstShow(7);
	_vm->_sequence->thenShow(8);
	_vm->_sequence->thenShow(9);
	_vm->_sequence->startToClose();
	_vm->_timer->addTimer(30, Timer::kProcWinning, Timer::kReasonWinning);
}

void Acci::personSpeaks() {
	if ((_person == kPardon) || (_person == 0)) {
		if ((_vm->_gyro->_him == kPardon) || (_vm->_gyro->_whereIs[_vm->_gyro->_him - 150] != _vm->_gyro->_dna._room))
			_person = _vm->_gyro->_her;
		else
			_person = _vm->_gyro->_him;
	}

	if (_vm->_gyro->_whereIs[_person - 150] != _vm->_gyro->_dna._room) {
		_vm->_scrolls->displayText(Common::String(Scrolls::kControlRegister) + '1' + Scrolls::kControlToBuffer); // Avvy himself!
		return;
	}

	bool found = false; // The _person we're looking for's code is in _person.

	for (byte i = 0; i < _vm->_animation->kSpriteNumbMax; i++) {
		if (_vm->_animation->_sprites[i]._quick && ((_vm->_animation->_sprites[i]._stat._acciNum + 149) == _person)) {
			_vm->_scrolls->displayText(Common::String(Scrolls::kControlRegister) + byte(i + 49) + Scrolls::kControlToBuffer);
			found = true;
		}
	}

	if (!found) {
		for (byte i = 0; i < 16; i++) {
			if ((_vm->_gyro->kQuasipeds[i]._who == _person) && (_vm->_gyro->kQuasipeds[i]._room == _vm->_gyro->_dna._room))
				_vm->_scrolls->displayText(Common::String(Scrolls::kControlRegister) + byte(i + 65) + Scrolls::kControlToBuffer);
		}
	}
}
void Acci::heyThanks() {
	personSpeaks();
	_vm->_scrolls->displayText(Common::String("Hey, thanks!") + Scrolls::kControlSpeechBubble + "(But now, you've lost it!)");
	_vm->_gyro->_dna._objects[_thing - 1] = false;
}

/**
 * @remarks	Originally called 'do_that'
 */
void Acci::doThat() {
	static const Common::String booze[] = {"Bitter", "GIED", "Whisky", "Cider", "", "", "", "Mead"};
	static const char kWhat[] = "That's not possible!";

	if (_thats == Common::String(kNothing))  {
		if (!_thats.empty())
			_thats.clear();
		return;
	}

	if (_vm->_gyro->_weirdWord)
		return;

	if (_thing < 200)
		_thing -= 49; // "Slip"


	if ((_verb != kVerbCodeLoad) && (_verb != kVerbCodeSave) && (_verb != kVerbCodeQuit) && (_verb != kVerbCodeInfo) && (_verb != kVerbCodeHelp)
	&& (_verb != kVerbCodeLarrypass) && (_verb != kVerbCodePhaon) && (_verb != kVerbCodeBoss) && (_verb != kVerbCodeCheat) && (_verb != kVerbCodeRestart)
	&& (_verb != kVerbCodeDir) && (_verb != kVerbCodeScore) && (_verb != kVerbCodeHiscores) && (_verb != kVerbCodeSmartAlec)) {
		if (!_vm->_gyro->_alive) {
			_vm->_scrolls->displayText(Common::String("You're dead, so don't talk. What are you, a ghost or something? Try restarting, or restoring a saved game!"));
			return;
		}
		if (!_vm->_gyro->_dna._avvyIsAwake  && (_verb != kVerbCodeDie) && (_verb != kVerbCodeExpletive) && (_verb != kVerbCodeWake)) {
			_vm->_scrolls->displayText("Talking in your sleep? Try waking up!");
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
	case kVerbCodePause: // Note that the original game doesn't care about the "O.K." box neither, it accepts clicks from everywhere on the screen to continue. Just like my code.
		_vm->_scrolls->displayText(Common::String("Game paused.") + Scrolls::kControlCenter + Scrolls::kControlNewLine + Scrolls::kControlNewLine
			+ "Press Enter, Esc, or click the mouse on the \"O.K.\" box to continue.");
		break;
	case kVerbCodeGet:
		if (_thing != kPardon) { // Legitimate try to pick something up.
			if (_vm->_gyro->_dna._carryNum >= kCarryLimit)
				_vm->_scrolls->displayText("You can't carry any more!");
			else
				getProc(_thing);
		} else { // Not... ditto.
			if (_person != kPardon)
				_vm->_scrolls->displayText("You can't sweep folk off their feet!");
			else
				_vm->_scrolls->displayText("I assure you, you don't need it.");
		}
		break;
	case kVerbCodeDrop:
		_vm->_scrolls->displayText(Common::String("Two years ago you dropped a florin in the street. Three days ") +
				"later it was gone! So now you never leave ANYTHING lying around. OK?");
		break;
	case kVerbCodeInv:
		inventory();
		break;
	case kVerbCodeTalk:
		if (_person == kPardon) {
			if (_vm->_gyro->_subjectNum == 99) // They typed "say password".
				_vm->_scrolls->displayText(Common::String("Yes, but what ") + Scrolls::kControlItalic + "is" + Scrolls::kControlRoman + " the password?");
			else if (((1 <= _vm->_gyro->_subjectNum) && (_vm->_gyro->_subjectNum <= 49)) || (_vm->_gyro->_subjectNum == 253) || (_vm->_gyro->_subjectNum == 249)) {
				_thats.deleteChar(0);

				for (byte i = 0; i < 10; i++)
					_realWords[i] = _realWords[i + 1];

				_verb = _vm->_gyro->_subjectNum;
				doThat();
				return;
			} else {
				_person = _vm->_gyro->_subjectNum;
				_vm->_gyro->_subjectNum = 0;
				if ((_person == 0) || (_person == kPardon))
					_vm->_scrolls->displayText("Talk to whom?");
				else if (isPersonHere())
					_vm->_visa->talkTo(_person);
			}
		} else if (isPersonHere())
			_vm->_visa->talkTo(_person);
		break;
	case kVerbCodeGive:
		if (isHolding()) {
			if (_person == kPardon)
				_vm->_scrolls->displayText("Give to whom?");
			else if (isPersonHere()) {
				switch (_thing) {
				case Gyro::kObjectMoney :
					_vm->_scrolls->displayText("You can't bring yourself to give away your moneybag.");
					break;
				case Gyro::kObjectBodkin:
				case Gyro::kObjectBell:
				case Gyro::kObjectClothes:
				case Gyro::kObjectHabit :
					_vm->_scrolls->displayText("Don't give it away, it might be useful!");
					break;
				default:
					switch (_person) {
					case Gyro::kPeopleCrapulus:
						if (_thing == Gyro::kObjectWine) {
							_vm->_scrolls->displayText("Crapulus grabs the wine and gulps it down.");
							_vm->_gyro->_dna._objects[Gyro::kObjectWine - 1] = false;
						} else
							heyThanks();
						break;
					case Gyro::kPeopleCwytalot:
						if ((_thing == Gyro::kObjectCrossbow) || (_thing == Gyro::kObjectBolt))
							_vm->_scrolls->displayText(Common::String("You might be able to influence Cwytalot more if you used it!"));
						else
							heyThanks();
						break;
					case Gyro::kPeopleSpludwick:
						if (giveToSpludwick())
							heyThanks();
						break;
					case Gyro::kPeopleIbythneth:
						if (_thing == Gyro::kObjectBadge) {
							_vm->_visa->displayScrollChain('q', 32); // Thanks! Wow!
							_vm->_lucerna->incScore(3);
							_vm->_gyro->_dna._objects[Gyro::kObjectBadge - 1] = false;
							_vm->_gyro->_dna._objects[Gyro::kObjectHabit - 1] = true;
							_vm->_gyro->_dna._givenBadgeToIby = true;
							_vm->_celer->drawBackgroundSprite(-1, -1, 8);
							_vm->_celer->drawBackgroundSprite(-1, -1, 9);
						} else
							heyThanks();
						break;
					case Gyro::kPeopleAyles:
						if (_vm->_gyro->_dna._aylesIsAwake) {
							if (_thing == Gyro::kObjectPen) {
								_vm->_gyro->_dna._objects[Gyro::kObjectPen - 1] = false;
								_vm->_visa->displayScrollChain('q', 54);
								_vm->_gyro->_dna._objects[Gyro::kObjectInk - 1] = true;
								_vm->_gyro->_dna._givenPenToAyles = true;
								_vm->_lucerna->refreshObjectList();
								_vm->_lucerna->incScore(2);
							} else
								heyThanks();
						} else
							_vm->_scrolls->displayText("But he's asleep!");
						break;
					case Gyro::kPeopleGeida:
						switch (_thing) {
						case Gyro::kObjectPotion:
							_vm->_gyro->_dna._objects[Gyro::kObjectPotion - 1] = false;
							_vm->_visa->displayScrollChain('u', 16); // She drinks it.
							_vm->_lucerna->incScore(2);
							_vm->_gyro->_dna._givenPotionToGeida = true;
							_vm->_lucerna->refreshObjectList();
							break;
						case Gyro::kObjectLute:
							giveGeidaTheLute();
							break;
						default:
							heyThanks();
						}
						break;
					case Gyro::kPeopleArkata:
						switch (_thing) {
						case Gyro::kObjectPotion:
							if (_vm->_gyro->_dna._givenPotionToGeida)
								winSequence();
							else
								_vm->_visa->displayScrollChain('q', 77); // That Geida woman!
							break;
						default:
							heyThanks();
						}
						break;
					default:
						heyThanks();
					}
				}
			}
			_vm->_lucerna->refreshObjectList(); // Just in case...
		}
		break;

	case kVerbCodeEat:
	case kVerbCodeDrink:
		if (isHolding())
			swallow();
		break;

	case kVerbCodeLoad:
		break;
	case kVerbCodeSave:
		break;
	// We don't handle these two because we use ScummVM's save/load system.

	case kVerbCodePay:
		_vm->_scrolls->displayText("No money need change hands.");
		break;
	case kVerbCodeLook:
		lookAround();
		break;
	case kVerbCodeBreak:
		_vm->_scrolls->displayText("Vandalism is prohibited within this game!");
		break;
	case kVerbCodeQuit: // quit
		if (_vm->_gyro->kDemo) {
			warning("STUB: Acci::doThat() - case kVerbCodequit");
		//	_vm->_visa->displayScrollChain('pos', 31);
		//	close(demofile);
		//	exit(0); // Change this later!!!
		}
		if (!_polite)
			_vm->_scrolls->displayText("How about a `please\", Avvy?");
		else if (_vm->_scrolls->displayQuestion(Common::String(Scrolls::kControlRegister) + 'C' + Scrolls::kControlIcon + "Do you really want to quit?"))
			_vm->_gyro->_letMeOut = true;
		break;
	case kVerbCodeGo:
		_vm->_scrolls->displayText("Just use the arrow keys to walk there.");
		break;
	case kVerbCodeInfo: {
		_vm->_scrolls->_aboutScroll = true;

		Common::String toDisplay;
		for (byte i = 0; i < 7; i++)
			toDisplay += Scrolls::kControlNewLine;
		toDisplay = toDisplay + "LORD AVALOT D'ARGENT" + Scrolls::kControlCenter + Scrolls::kControlNewLine
			+ "The medi\x91val descendant of" + Scrolls::kControlNewLine
			+ "Denarius Avaricius Sextus" + Scrolls::kControlNewLine + Scrolls::kControlNewLine
			+ "version " + _vm->_gyro->kVersionNum + Scrolls::kControlNewLine + Scrolls::kControlNewLine + "Copyright \xEF "
			+ _vm->_gyro->kCopyright + ", Mark, Mike and Thomas Thurman." + Scrolls::kControlRegister + 'Y' + Scrolls::kControlIcon;
		_vm->_scrolls->displayText(toDisplay);
		_vm->_scrolls->_aboutScroll = false;
		}
		break;
	case kVerbCodeUndress:
		if (_vm->_gyro->_dna._wearing == kNothing)
			_vm->_scrolls->displayText("You're already stark naked!");
		else if (_vm->_gyro->_dna._avvysInTheCupboard) {
			_vm->_scrolls->displayText(Common::String("You take off ") + _vm->_gyro->getItem(_vm->_gyro->_dna._wearing) + '.');
			_vm->_gyro->_dna._wearing = kNothing;
			_vm->_lucerna->refreshObjectList();
		} else
			_vm->_scrolls->displayText("Hadn't you better find somewhere more private, Avvy?");
		break;
	case kVerbCodeWear:
		if (isHolding()) { // Wear something.
			switch (_thing) {
			case Gyro::kObjectChastity:
				// \? are used to avoid that ??! is parsed as a trigraph
				_vm->_scrolls->displayText("Hey, what kind of a weirdo are you\?\?!");
				break;
			case Gyro::kObjectClothes:
			case Gyro::kObjectHabit: { // Change this!
					if (_vm->_gyro->_dna._wearing != kNothing) {
						if (_vm->_gyro->_dna._wearing == _thing)
							_vm->_scrolls->displayText("You're already wearing that.");
						else
							_vm->_scrolls->displayText("You'll be rather warm wearing two sets of clothes!");
						return;
					} else
						_vm->_gyro->_dna._wearing = _thing;

					_vm->_lucerna->refreshObjectList();

					byte i;
					if (_thing == Gyro::kObjectHabit)
						i = 3;
					else
						i = 0;
					if (_vm->_animation->_sprites[0]._id != i) {
						int16 x = _vm->_animation->_sprites[0]._x;
						int16 y = _vm->_animation->_sprites[0]._y;
						_vm->_animation->_sprites[0].remove();
						_vm->_animation->_sprites[0].init(i, true, _vm->_animation);
						_vm->_animation->_sprites[0].appear(x, y, Animation::kDirLeft);
						_vm->_animation->_sprites[0]._visible = false;
					}
				}
				break;
			default:
				_vm->_scrolls->displayText(kWhat);
			}
		}
		break;
	case kVerbCodePlay:
		if (_thing == kPardon) {
			switch (_vm->_gyro->_dna._room) { // They just typed "play"...
			case r__argentpub: { // ...in the pub, => play Nim.
					warning("STUB: Acci::doThat() - case kVerbCodeplay");
					// play_nim();
					// The following parts are copied from play_nim().
					// The player automatically wins the game everytime he wins, until I implement the mini-game.

					if (_vm->_gyro->_dna._wonNim) { // Already won the game.
						_vm->_visa->displayScrollChain('Q', 6);
						return;
					}

					if (!_vm->_gyro->_dna._askedDogfoodAboutNim) {
						_vm->_visa->displayScrollChain('q', 84);
						return;
					}

					_vm->_visa->displayScrollChain('Q', 3);
					_vm->_gyro->_dna._playedNim++;

					// You won - strange!
					_vm->_visa->displayScrollChain('Q', 7); // You won! Give us a lute!
					_vm->_gyro->_dna._objects[Gyro::kObjectLute - 1] = true;
					_vm->_lucerna->refreshObjectList();
					_vm->_gyro->_dna._wonNim = true;
					_vm->_celer->drawBackgroundSprite(-1, -1, 1); // Show the settle with no lute on it.
					_vm->_lucerna->incScore(7); // 7 points for winning!

					if (_vm->_gyro->_dna._playedNim == 1)
						_vm->_lucerna->incScore(3); // 3 points for playing your 1st game.

					// A warning to the player that there should have been a mini-game. TODO: Remove it later!!!
					_vm->_scrolls->displayText(Common::String("P.S.: There should have been the mini-game called \"Nim\", but I haven't implemented it yet: you win and get the lute automatically.")
						+ Scrolls::kControlNewLine + Scrolls::kControlNewLine + "Peter (uruk)");
				}
				break;
			case r__musicroom:
				playHarp();
				break;
			}
		} else if (isHolding()) {
			switch (_thing) {
			case Gyro::kObjectLute :
					_vm->_visa->displayScrollChain('U', 7);

					if (_vm->_gyro->_whereIs[Gyro::kPeopleCwytalot - 150] == _vm->_gyro->_dna._room)
						_vm->_visa->displayScrollChain('U', 10);

					if (_vm->_gyro->_whereIs[Gyro::kPeopleDuLustie - 150] == _vm->_gyro->_dna._room)
						_vm->_visa->displayScrollChain('U', 15);
				break;
			case 52:
				if (_vm->_gyro->_dna._room == r__musicroom)
					playHarp();
				else
					_vm->_scrolls->displayText(kWhat);
				break;
			case 55:
				if (_vm->_gyro->_dna._room == r__argentpub)
					// play_nim();
					warning("STUB: Acci::doThat() - case kVerbCodeplay");
				else
					_vm->_scrolls->displayText(kWhat);
				break;
			default:
				_vm->_scrolls->displayText(kWhat);
			}
		}
		break;
	case kVerbCodeRing:
		if (isHolding()) {
			if (_thing == Gyro::kObjectBell) {
				_vm->_scrolls->displayText("Ding, dong, ding, dong, ding, dong, ding, dong...");
				if ((_vm->_gyro->_dna._bellsAreRinging) & (_vm->_gyro->setFlag('B')))
					// \? are used to avoid that ??! is parsed as a trigraph
					_vm->_scrolls->displayText("(Are you trying to join in, Avvy\?\?!)");
			} else
				_vm->_scrolls->displayText(kWhat);
		}
		break;
	case kVerbCodeHelp:
		// boot_help();
		warning("STUB: Acci::doThat() - case kVerbCodehelp");
		break;
	case kVerbCodeLarrypass:
		_vm->_scrolls->displayText("Wrong game!");
		break;
	case kVerbCodePhaon:
		_vm->_scrolls->displayText("Hello, Phaon!");
		break;
	case kVerbCodeBoss:
		// bosskey();
		warning("STUB: Acci::doThat() - case kVerbCodeboss");
		break;
	case kVerbCodePee:
		if (_vm->_gyro->setFlag('P')) {
			_vm->_scrolls->displayText("Hmm, I don't think anyone will notice...");
			_vm->_timer->addTimer(4, Timer::kProcUrinate, Timer::kReasonGoToToilet);
		} else
			_vm->_scrolls->displayText(Common::String("It would be ") + Scrolls::kControlItalic + "VERY"
			+ Scrolls::kControlRoman + " unwise to do that here, Avvy!");
		break;
	case kVerbCodeCheat:
		_vm->_scrolls->displayText(Common::String(Scrolls::kControlItalic) + "Cheat mode now enabled.");
		_vm->_gyro->_cheat = true;
		break;
	case kVerbCodeMagic:
		if (_vm->_gyro->_dna._avariciusTalk > 0)
			_vm->_visa->displayScrollChain('q', 19);
		else {
			if ((_vm->_gyro->_dna._room == 12) & (_vm->_animation->inField(2))) { // Avaricius appears!
				_vm->_visa->displayScrollChain('q', 17);
				if (_vm->_gyro->_whereIs[1] == 12)
					_vm->_visa->displayScrollChain('q', 18);
				else {
					_vm->_animation->_sprites[1].init(1, false, _vm->_animation); // Avaricius
					_vm->_animation->appearPed(2, 4);
					_vm->_animation->_sprites[1].walkTo(5);
					_vm->_animation->_sprites[1]._callEachStepFl = true;
					_vm->_animation->_sprites[1]._eachStepProc = _vm->_animation->kProcBackAndForth;
					_vm->_gyro->_dna._avariciusTalk = 14;
					_vm->_timer->addTimer(177, Timer::kProcAvariciusTalks, Timer::kReasonAvariciusTalks);
				}
			} else
				_vm->_scrolls->displayText("Nothing appears to happen...");
		}
		break;
	case kVerbCodeSmartAlec:
		_vm->_scrolls->displayText("Listen, smart alec, that was just rhetoric.");
		break;
	case kVerbCodeExpletive:
		switch (_vm->_gyro->_dna._sworeNum) {
		case 0:
			_vm->_scrolls->displayText(Common::String("Avvy! Do you mind? There might be kids playing!")
				+ Scrolls::kControlNewLine + Scrolls::kControlNewLine + "(I shouldn't say it again, if I were you!)");
			break;
		case 1:
			_vm->_scrolls->displayText(Common::String("You hear a distant rumble of thunder. Must you always do things I tell you not to?")
				+ Scrolls::kControlNewLine + Scrolls::kControlNewLine + "Don't do it again!");
			break;
		default:
			_vm->_pingo->zonk();
			_vm->_scrolls->displayText(Common::String("A crack of lightning shoots from the sky, and fries you.")
				+ Scrolls::kControlNewLine + Scrolls::kControlNewLine + "(`Such is the anger of the gods, Avvy!\")");
			_vm->_lucerna->gameOver();
		}
		_vm->_gyro->_dna._sworeNum++;
		break;
	case kVerbCodeListen:
		if ((_vm->_gyro->_dna._bellsAreRinging) & (_vm->_gyro->setFlag('B')))
			_vm->_scrolls->displayText("All other noise is drowned out by the ringing of the bells.");
		else if (_vm->_gyro->_listen.empty())
			_vm->_scrolls->displayText("You can't hear anything much at the moment, Avvy.");
		else
			_vm->_scrolls->displayText(_vm->_gyro->_listen);
		break;
	case kVerbCodeBuy: // What are they trying to buy?
		switch (_vm->_gyro->_dna._room) {
		case r__argentpub:
			if (_vm->_animation->inField(6)) { // We're in a pub, and near the bar.
				switch (_thing) {
				case 51:
				case 53:
				case 54:
				case 58: // Beer, whisky, cider or mead.
					if (_vm->_gyro->_dna._malagauche == 177) { // Already getting us one.
						_vm->_visa->displayScrollChain('D', 15);
						return;
					}

					if (_vm->_gyro->_dna._teetotal)  {
						_vm->_visa->displayScrollChain('D', 6);
						return;
					}

					if (_vm->_gyro->_dna._alcoholLevel == 0)
						_vm->_lucerna->incScore(3);

					_vm->_celer->drawBackgroundSprite(-1, -1, 12);
					_vm->_scrolls->displayText(booze[_thing - 51] + ", please." + Scrolls::kControlRegister + '1' + Scrolls::kControlSpeechBubble);
					_vm->_gyro->_dna._drinking = _thing;

					_vm->_celer->drawBackgroundSprite(-1, -1, 10);
					_vm->_gyro->_dna._malagauche = 177;
					_vm->_timer->addTimer(27, Timer::kProcBuyDrinks, Timer::kReasonDrinks);
					break;
				case 52:
					examine();
					break; // We have a right one here - buy Pepsi??!
				case Gyro::kObjectWine:
					if (_vm->_gyro->_dna._objects[Gyro::kObjectWine - 1])  // We've already got the wine!
						_vm->_visa->displayScrollChain('D', 2); // 1 bottle's shufishent!
					else {
						if (_vm->_gyro->_dna._malagauche == 177) { // Already getting us one.
							_vm->_visa->displayScrollChain('D', 15);
							return;
						}

						if (_vm->_gyro->_dna._carryNum >= kCarryLimit) {
							_vm->_scrolls->displayText("Your hands are full.");
							return;
						}

						_vm->_celer->drawBackgroundSprite(-1, -1, 12);
						_vm->_scrolls->displayText(Common::String("Wine, please.") + Scrolls::kControlRegister + '1' + Scrolls::kControlSpeechBubble);
						if (_vm->_gyro->_dna._alcoholLevel == 0)
							_vm->_lucerna->incScore(3);
						_vm->_celer->drawBackgroundSprite(-1, -1, 10);
						_vm->_gyro->_dna._malagauche = 177;

						_vm->_timer->addTimer(27, Timer::kProcBuyWine, Timer::kReasonDrinks);
					}
					break;
				}
			} else
				_vm->_visa->displayScrollChain('D', 5); // Go to the bar!
			break;

		case r__outsideducks:
			if (_vm->_animation->inField(6)) {
				if (_thing == Gyro::kObjectOnion) {
					if (_vm->_gyro->_dna._objects[Gyro::kObjectOnion - 1])
						_vm->_visa->displayScrollChain('D', 10); // Not planning to juggle with the things!
					else if (_vm->_gyro->_dna._carryNum >= kCarryLimit)
						_vm->_scrolls->displayText("Before you ask, you remember that your hands are full.");
					else {
						if (_vm->_gyro->_dna._boughtOnion)
							_vm->_visa->displayScrollChain('D', 11);
						else {
							_vm->_visa->displayScrollChain('D', 9);
							_vm->_lucerna->incScore(3);
						}
						_vm->_gyro->decreaseMoney(3); // It costs thruppence.
						_vm->_gyro->_dna._objects[Gyro::kObjectOnion - 1] = true;
						_vm->_lucerna->refreshObjectList();
						_vm->_gyro->_dna._boughtOnion = true;
						_vm->_gyro->_dna._rottenOnion = false; // It's OK when it leaves the stall!
						_vm->_gyro->_dna._onionInVinegar = false;
					}
				} else
					_vm->_visa->displayScrollChain('D', 0);
			} else
				_vm->_visa->displayScrollChain('D', 0);
			break;

		case r__nottspub:
			_vm->_visa->displayScrollChain('n', 15); // Can't sell to southerners.
			break;
		default:
			_vm->_visa->displayScrollChain('D', 0); // Can't buy that.
		}
		break;
	case kVerbCodeAttack:
		if ((_vm->_gyro->_dna._room == r__brummieroad) &&
				((_person == 157) || (_thing == Gyro::kObjectCrossbow) || (_thing == Gyro::kObjectBolt))
				&& (_vm->_gyro->_whereIs[7] == _vm->_gyro->_dna._room)) {
			switch (_vm->_gyro->_dna._objects[Gyro::kObjectBolt - 1] + _vm->_gyro->_dna._objects[Gyro::kObjectCrossbow - 1] * 2) {
				// 0 = neither, 1 = only bolt, 2 = only crossbow, 3 = both.
			case 0:
				_vm->_visa->displayScrollChain('Q', 10);
				_vm->_scrolls->displayText("(At the very least, don't use your bare hands!)");
				break;
			case 1:
				_vm->_scrolls->displayText("Attack _vm->_gyro->him with only a crossbow bolt? Are you planning on playing darts?!");
				break;
			case 2:
				_vm->_scrolls->displayText("Come on, Avvy! You're not going to get very far with only a crossbow!");
				break;
			case 3:
				_vm->_visa->displayScrollChain('Q', 11);
				_vm->_gyro->_dna._cwytalotGone = true;
				_vm->_gyro->_dna._objects[Gyro::kObjectBolt - 1] = false;
				_vm->_gyro->_dna._objects[Gyro::kObjectCrossbow - 1] = false;
				_vm->_lucerna->refreshObjectList();
				_vm->_gyro->_magics[11]._operation = Gyro::kMagicNothing;
				_vm->_lucerna->incScore(7);
				_vm->_animation->_sprites[1].walkTo(2);
				_vm->_animation->_sprites[1]._vanishIfStill = true;
				_vm->_animation->_sprites[1]._callEachStepFl = false;
				_vm->_gyro->_whereIs[7] = 177;
				break;
			default:
				_vm->_visa->displayScrollChain('Q', 10); // Please try not to be so violent!
			}
		} else
			_vm->_visa->displayScrollChain('Q', 10);
		break;
	case kVerbCodePasswd:
		if (_vm->_gyro->_dna._room != r__bridge)
			_vm->_visa->displayScrollChain('Q', 12);
		else {
			bool ok = true;
			for (byte i = 0; i < _thats.size(); i++) {
				Common::String temp = _realWords[i];
				temp.toUppercase();
				for (byte j = 0; j < kVocabulary[_vm->_gyro->_dna._passwordNum + kFirstPassword]._word.size(); j++) {
					if (kVocabulary[_vm->_gyro->_dna._passwordNum + kFirstPassword]._word[j] != temp[j])
						ok = false;
				}
			}

			if (ok) {
				if (_vm->_gyro->_dna._drawbridgeOpen != 0)
					_vm->_scrolls->displayText("Contrary to your expectations, the drawbridge fails to close again.");
				else {
					_vm->_lucerna->incScore(4);
					_vm->_scrolls->displayText("The drawbridge opens!");
					_vm->_timer->addTimer(7, Timer::kProcOpenDrawbridge, Timer::kReasonDrawbridgeFalls);
					_vm->_gyro->_dna._drawbridgeOpen = 1;
				}
			} else
				_vm->_visa->displayScrollChain('Q', 12);
		}
		break;
	case kVerbCodeDir:
		_vm->_enid->dir(_realWords[1]);
		break;
	case kVerbCodeDie:
		_vm->_lucerna->gameOver();
		break;
	case kVerbCodeScore: {
		Common::String tmpStr = Common::String::format("Your score is %d,%c%cout of a possible 128.%c%cThis gives you a rank of %s.%c%c%s", _vm->_gyro->_dna._score, Scrolls::kControlCenter, Scrolls::kControlNewLine, Scrolls::kControlNewLine, Scrolls::kControlNewLine, rank().c_str(), Scrolls::kControlNewLine, Scrolls::kControlNewLine, totalTime().c_str());
		_vm->_scrolls->displayText(tmpStr);
		}
		break;
	case kVerbCodePut:
		putProc();
		break;
	case kVerbCodeStand:
		standUp();
		break;
	case kVerbCodeKiss:
		if (_person == kPardon)
			_vm->_scrolls->displayText("Kiss whom?");
		else if (isPersonHere()) {
			switch (_person) {
			case Gyro::kPeopleArkata:
				_vm->_visa->displayScrollChain('U', 12);
				break;
			case Gyro::kPeopleGeida:
				_vm->_visa->displayScrollChain('U', 13);
				break;
			case Gyro::kPeopleWisewoman:
				_vm->_visa->displayScrollChain('U', 14);
				break;
			default:
				_vm->_visa->displayScrollChain('U', 5); // You WHAT?
			}
		} else if ((150 <= _person) && (_person <= 174))
			_vm->_scrolls->displayText("Hey, what kind of a weirdo are you??");

		break;
	case kVerbCodeClimb:
		if (_vm->_gyro->_dna._room == r__insidecardiffcastle)
			cardiffClimbing();
		else // In the wrong room!
			_vm->_scrolls->displayText("Not with your head for heights, Avvy!");
		break;
	case kVerbCodeJump:
		_vm->_timer->addTimer(1, Timer::kProcJump, Timer::kReasonJumping);
		_vm->_gyro->_dna._userMovesAvvy = false;
		break;
	case kVerbCodeHiscores:
		//	show_highs();
		warning("STUB: Acci::doThat() - case kVerbCodehighscores");
		break;
	case kVerbCodeWake:
		if (isPersonHere())
			switch (_person) {
			case kPardon:
			case Gyro::kPeopleAvalot:
			case 0:
				if (!_vm->_gyro->_dna._avvyIsAwake) {
					_vm->_gyro->_dna._avvyIsAwake = true;
					_vm->_lucerna->incScore(1);
					_vm->_gyro->_dna._avvyInBed = true;
					_vm->_celer->drawBackgroundSprite(-1, -1, 3); // Picture of Avvy, awake in bed.
					if (_vm->_gyro->_dna._teetotal)
						_vm->_visa->displayScrollChain('d', 13);
				} else
					_vm->_scrolls->displayText("You're already awake, Avvy!");
				break;
			case Gyro::kPeopleAyles:
				if (!_vm->_gyro->_dna._aylesIsAwake)
					_vm->_scrolls->displayText("You can't seem to wake him by yourself.");
				break;
			case Gyro::kPeopleJacques:
				_vm->_scrolls->displayText(Common::String("Brother Jacques, Brother Jacques, are you asleep?") + Scrolls::kControlRegister + '1' +
					Scrolls::kControlSpeechBubble + "Hmmm... that doesn't seem to do any good...");
				break;
			default:
				_vm->_scrolls->displayText("It's difficult to awaken people who aren't asleep...!");
		}
		break;
	case kVerbCodeSit:
		if (_vm->_gyro->_dna._room == r__nottspub) {
			if (_vm->_gyro->_dna._sittingInPub)
				_vm->_scrolls->displayText("You're already sitting!");
			else {
				_vm->_animation->_sprites[0].walkTo(4); // Move Avvy to the place, and sit him down.
				_vm->_timer->addTimer(1, Timer::kProcAvvySitDown, Timer::kReasonSittingDown);
			}
		} else { // Default doodah.
			_vm->_lucerna->dusk();
			_vm->_gyro->hangAroundForAWhile();
			_vm->_lucerna->dawn();
			_vm->_scrolls->displayText(Common::String("A few hours later...") + Scrolls::kControlParagraph + "nothing much has happened...");
		}
		break;
	case kVerbCodeRestart:
		if (_vm->_scrolls->displayQuestion("Restart game and lose changes?"))  {
			_vm->_lucerna->dusk();
			_vm->_gyro->newGame();
			_vm->_lucerna->dawn();
		}
		break;
	case kPardon:
		_vm->_scrolls->displayText("Hey, a verb would be helpful!");
		break;
	case kVerbCodeHello:
		personSpeaks();
		_vm->_scrolls->displayText(Common::String("Hello.") + Scrolls::kControlSpeechBubble);
		break;
	case kVerbCodeThanks:
		personSpeaks();
		_vm->_scrolls->displayText(Common::String("That's OK.") + Scrolls::kControlSpeechBubble);
		break;
	default:
		_vm->_scrolls->displayText(Common::String(Scrolls::kControlBell) + "Parser bug!");
	}
}

void Acci::verbOpt(byte verb, Common::String &answer, char &ansKey) {
	switch (verb) {
	case kVerbCodeExam:
		answer = "Examine";
		ansKey = 'x';
		break; // The ubiquitous one.
	// kVerbCodegive isn't dealt with by this procedure, but by ddm__with.
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

} // End of namespace Avalanche.
