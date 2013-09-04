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
#include "avalanche/timeout2.h"
#include "avalanche/trip6.h"
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

const char Acci::kWhat[] = "That's not possible!";

const Acci::RankType Acci::kRanks[9] = {
	{0, "Beginner"},     {10, "Novice"},
	{20, "Improving"},   {35, "Not bad"},
	{50, "Passable"},    {65, "Good"},
	{80, "Experienced"}, {108, "The BEST!"},
	{32767, "copyright'93"}
};


Acci::Acci(AvalancheEngine *vm) {
	_vm = vm;
}

void Acci::init() {
	_vm->_gyro->weirdword = false;
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
	for (byte i = 0; i < 8; i++) {
		if ((_vm->_gyro->dna.score >= kRanks[i]._score) && (_vm->_gyro->dna.score < kRanks[i + 1]._score)) {
			return kRanks[i]._title;
		}
	}
	return "";
}

Common::String Acci::totalTime() {
	// There are 65535 clock ticks in a second, 1092.25 in a minute, and 65535 in an hour.
	const double ticksInOneSec = (double)(65535) / 3600; 
	uint16 h, m, s;

	h = _vm->_gyro->dna.total_time / ticksInOneSec; // No. of seconds.
	if (h < 0)
		h = ceil((float)h);
	else
		h = floor((float)h);
	m = h % 3600;
	h = h / 3600;
	s = m % 60;
	m = m / 60;

	Common::String result = "You've been playing for ";
	if (h > 0)
		result = result + _vm->_gyro->strf(h) + " hours, ";
	if ((m > 0) || (h != 0))
		result = result + _vm->_gyro->strf(m) + " minutes and ";
	return result + _vm->_gyro->strf(s) + " seconds.";
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
			_vm->_scrolls->display("Whom?");
		else
			_vm->_scrolls->display("What?");
	} else {
		if (animate)
			_vm->_scrolls->display(Common::String("{ ") + _vm->_gyro->getname(target) + " }");
		else {
			Common::String z = _vm->_gyro->get_better(target);
			if (z != "") 
				_vm->_scrolls->display(Common::String("{ ") + z + " }");
		}
	}
}

bool Acci::doPronouns() {
	bool ambiguous = false;

	for (byte i = 0; i < _thats.size(); i++) {
		byte wordCode = _thats[i];
		switch (wordCode) {
		case 200: {
			displayWhat(_vm->_gyro->him, true, ambiguous);
			_thats.setChar(_vm->_gyro->him, i);
			}
			break;
		case 201: {
			displayWhat(_vm->_gyro->her, true, ambiguous);
			_thats.setChar(_vm->_gyro->her, i);
			}
			break;
		case 202: {
			displayWhat(_vm->_gyro->it, false, ambiguous);
			_thats.setChar(_vm->_gyro->it, i);
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
	_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + '1' + x 
		+ '.' + _vm->_scrolls->kControlSpeechBubble + _vm->_scrolls->kControlRegister + '2');
}

void Acci::storeInterrogation(byte interrogation) {
	if (_vm->_parser->_inputText.empty())
		return;

	// Strip _vm->_parser->_inputText:
	while ((_vm->_parser->_inputText[0] == ' ') && (!_vm->_parser->_inputText.empty()))
		_vm->_parser->_inputText.deleteChar(0);
	while ((_vm->_parser->_inputText.lastChar() == ' ') && (!_vm->_parser->_inputText.empty()))
		_vm->_parser->_inputText.deleteLastChar();
	
	_vm->_timeout->lose_timer(_vm->_timeout->reason_cardiffsurvey); // If you want to use any other timer, put this into the case statement.

	switch (interrogation) {
	case 1:
		_vm->_parser->_inputText.toLowercase();
		sayIt();
		_vm->_gyro->dna.like2drink = _vm->_parser->_inputText;
		_vm->_gyro->dna.cardiff_things = 2;
		break;
	case 2:
		properNouns();
		sayIt();
		_vm->_gyro->dna.favourite_song = _vm->_parser->_inputText;
		_vm->_gyro->dna.cardiff_things = 3;
		break;
	case 3:
		properNouns();
		sayIt();
		_vm->_gyro->dna.worst_place_on_earth = _vm->_parser->_inputText;
		_vm->_gyro->dna.cardiff_things = 4;
		break;
	case 4:
		_vm->_parser->_inputText.toLowercase();
		sayIt();
		if (!_vm->_gyro->dna.spare_evening.empty())
			_vm->_gyro->dna.spare_evening.clear();
		_vm->_gyro->dna.spare_evening = _vm->_parser->_inputText;
		_vm->_visa->dixi('z', 5); /* His closing statement... */
		_vm->_trip->tr[1].walkto(4); /* The end of the drawbridge */
		_vm->_trip->tr[1].vanishifstill = true; /* Then go away! */
		_vm->_gyro->magics[1].op = _vm->_gyro->nix;
		_vm->_gyro->dna.cardiff_things = 5;
		break;
	case 99:
		//store_high(_vm->_parser->_inputText);
		warning("STUB: Acci::store_interrogation()");
		break;
	}

	if (interrogation < 4)
		_vm->_timeout->cardiff_survey();
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
	if (_vm->_gyro->interrogation > 0) {
		storeInterrogation(_vm->_gyro->interrogation);
		_vm->_gyro->weirdword = true;
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
				if ((_vm->_gyro->also[i][0] != 0) && (_vm->_parser->pos(',' + thisword, *_vm->_gyro->also[i][0]) > -1)) {
					_thats = _thats + Common::String(99 + i);
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

	if (_vm->_parser->pos(Common::String('\xFE'), _thats) > -1) 
		_unknown = _realWords[_vm->_parser->pos(Common::String('\xFE'), _thats)];
	else {
		if (!_unknown.empty())
			_unknown.clear();
	}
	// Replace words' codes that mean the same.
	replace(Common::String('\xFF'), 0); /* zap noise words */
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
	if (_vm->_gyro->dna.room == r__nottspub)
		replace(Common::String('\xCC'), 164); // Barman = Port
	else
		replace(Common::String('\xCC'), 154); // Barman = Malagauche

	switch (_vm->_gyro->dna.room) {
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
		_vm->_gyro->weirdword = true;
		_thats = kNothing;
		return;
	}

	// Second parsing.
	if (!_vm->_gyro->subject.empty())
		_vm->_gyro->subject.clear();
	_vm->_gyro->subjnumber = 0; // Find subject of conversation.
	
	byte i = 0;
	while ((i < 11) && !_realWords[i].empty()) {
		if ((_realWords[i][0] == '\'') || (_realWords[i][0] == '\"')) {
			_vm->_gyro->subjnumber = (byte)_thats[i];
			_thats.setChar(kMoved, i);
			break;
		}
		i++;
	}

	if ((_vm->_gyro->subjnumber == 0) && !_thats.empty()) { // Still not found.
		for (i = 0; i < _thats.size() - 1; i++) {
			if ((byte)_thats[i] == 252) { // The word is "about", or something similar.
				_vm->_gyro->subjnumber = (byte)_thats[i + 1];
				_thats.setChar(0, i + 1);
				break;
			}
		}
	}

	if ((_vm->_gyro->subjnumber == 0) && !_thats.empty()) { // STILL not found! Must be the word after "say".
		for (i = 0; i < _thats.size() - 1; i++) {
			if (((byte)_thats[i] == 7) && ((byte)_thats[i + 1] != 0) && !((225 <= (byte)_thats[i + 1]) && ((byte)_thats[i + 1] <= 229))) {
				// SAY not followed by a preposition
				_vm->_gyro->subjnumber = (byte)_thats[i + 1];
				_thats.setChar(0, i + 1);
				break;
			}
		}
	}

	for (int8 i = _thats.size() - 1; i >= 0; i--) { // Reverse order, so first will be used.
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

	if ((!_unknown.empty()) && (_verb != kVerbCodeExam) && (_verb != kVerbCodeTalk) && (_verb != kVerbCodeSave) && (_verb != kVerbCodeLoad) && (_verb != kVerbCodeDir)) {
			_vm->_scrolls->display(Common::String("Sorry, but I have no idea kWhat \"") + _unknown + "\" means. Can you rephrase it?");
			_vm->_gyro->weirdword = true;
	} else
		_vm->_gyro->weirdword = false;

	if (_thats.empty())
		_thats = kNothing;

	if (_thing != kPardon)
		_vm->_gyro->it = _thing;

	if (_person != kPardon) {
		if (_person < 175)
			_vm->_gyro->him = _person;
		else
			_vm->_gyro->her = _person;
	}
}

void Acci::examineObject() {   
	if (_thing != _vm->_gyro->thinks)
		_vm->_lucerna->thinkabout(_thing, _vm->_gyro->a_thing);
	switch (_thing) {
	case Gyro::wine :
		switch (_vm->_gyro->dna.winestate) {// 4 is perfect wine. 0 is not holding the wine.
		case 1:
			_vm->_visa->dixi('t', 1); // Normal examine wine scroll
			break; 
		case 2:
			_vm->_visa->dixi('d', 6); // Bad wine
			break;
		case 3:
			_vm->_visa->dixi('d', 7); // Vinegar
			break;
		}
		break;
	case Gyro::onion:
		if (_vm->_gyro->dna.rotten_onion)
			_vm->_visa->dixi('q', 21); // Yucky onion.
		else
			_vm->_visa->dixi('t', 18);  // Normal onion scroll
		break;       
	default:
		_vm->_visa->dixi('t', _thing); // <<< Ordinarily
	}
}

bool Acci::isPersonHere() { // Person equivalent of "holding".
	if ((_person == kPardon) || (_person == 0) || (_vm->_gyro->whereis[_person - 150] == _vm->_gyro->dna.room))
		return true;
	else {
		if (_person < 175)
			_vm->_scrolls->display(Common::String('H') + _vm->_scrolls->kControlToBuffer);
		else
			_vm->_scrolls->display(Common::String("Sh") + _vm->_scrolls->kControlToBuffer);
		_vm->_scrolls->display("e isn't around at the moment.");
		return false;
	}
}

void Acci::exampers() {
	if (isPersonHere()) {
		if (_thing != _vm->_gyro->thinks)
			_vm->_lucerna->thinkabout(_person, _vm->_gyro->a_person);
		_person -= 149;
		switch (_person) { /* Special cases */
		case 11:
			if (_vm->_gyro->dna.wonnim) {
				_vm->_visa->dixi('Q', 8); // "I'm Not Playing!"
				return;
			}
			break;
		case 99:
			if (_vm->_gyro->dna.lustie_is_asleep) {
				_vm->_visa->dixi('Q', 65); // He's asleep. (65! Wow!)
				return;
			}
			break;
		}
		// Otherwise...
		_vm->_visa->dixi('p', _person);
	} 
	
	// And afterwards...
	if ((_person == 14) && (!_vm->_gyro->dna.ayles_is_awake))
		_vm->_visa->dixi('Q', 13);
}

bool Acci::holding() {
	if ((51 <= _thing) && (_thing <= 99)) // Also.
		return true;

	bool holdingResult = false;

	if (_thing > 100)
		_vm->_scrolls->display("Be reasonable!");
	else if (!_vm->_gyro->dna.obj[_thing - 1])  // Verbs that need "_thing" to be in the inventory.
		_vm->_scrolls->display("You're not holding it, Avvy.");
	else 
		holdingResult = true;

	return holdingResult;
}

void Acci::openBox(bool isOpening) {
	if ((_vm->_gyro->dna.room == r__yours) && (_thing == 54)) {
		_vm->_celer->drawBackgroundSprite(-1, -1, 5);

		_vm->_celer->updateBackgroundSprites();
		_vm->_trip->trippancy_link();
		_vm->_graphics->refreshScreen();

		_vm->_system->delayMillis(55);

		if (!isOpening) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			_vm->_celer->updateBackgroundSprites();
			_vm->_trip->trippancy_link();
			_vm->_graphics->refreshScreen();
		}
	}
}

void Acci::examine() {
	// EITHER it's an object OR it's an Also OR it's a _person OR it's something else.
	if ((_person == kPardon) && (_thing != kPardon)) {
		if (holding()) {
			// Remember: it's been Slipped! Ie subtract 49.
			if ((1 <= _thing) && (_thing <= 49)) // Standard object
				examineObject();
			else if ((50 <= _thing) && (_thing <= 100)) { // Also _thing
				openBox(true);
				_vm->_scrolls->display(*_vm->_gyro->also[_thing - 50][1]);
				openBox(false);
			}
		}
	} else if (_person != kPardon)
		exampers();
	else
		_vm->_scrolls->display("It's just as it looks on the picture.");  // Don't know: guess.
}

void Acci::inventory() {
	byte itemNum = 0;
	_vm->_scrolls->display(Common::String("You're carrying ") + _vm->_scrolls->kControlToBuffer);

	for (byte i = 0; i < numobjs; i++) {
		if (_vm->_gyro->dna.obj[i]) {
			itemNum++;
			if (itemNum == _vm->_gyro->dna.carrying)
				_vm->_scrolls->display(Common::String("and ") + _vm->_scrolls->kControlToBuffer);
			_vm->_scrolls->display(_vm->_gyro->get_better(i + 1) + _vm->_scrolls->kControlToBuffer);
			if ((i + 1) == _vm->_gyro->dna.wearing)
				_vm->_scrolls->display(Common::String(", which you're wearing") + _vm->_scrolls->kControlToBuffer);
			if (itemNum < _vm->_gyro->dna.carrying)
				_vm->_scrolls->display(Common::String(", ") + _vm->_scrolls->kControlToBuffer);
		}
	}

	if (_vm->_gyro->dna.wearing == kNothing)
		_vm->_scrolls->display(Common::String("...") + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "...and you're stark naked!");
	else
		_vm->_scrolls->display(".");
}

void Acci::swallow() {   /* Eat something. */
	switch (_thing) {
	case Gyro::wine:
		switch (_vm->_gyro->dna.winestate) { /* 4 is perfect */
		case 1:
			if (_vm->_gyro->dna.teetotal)  {
				_vm->_visa->dixi('D', 6);
				return;
			}
			_vm->_visa->dixi('U', 1);
			_vm->_pingo->wobble();
			_vm->_visa->dixi('U', 2);
			_vm->_gyro->dna.obj[_vm->_gyro->wine - 1] = false;
			_vm->_lucerna->objectlist();
			drink();
			break;
		case 2:
		case 3:
			_vm->_visa->dixi('d', 8);
			break; /* You can't drink it! */
		}
		break;
	case Gyro::potion:
		_vm->_gyro->background(4);
		_vm->_visa->dixi('U', 3);
		_vm->_lucerna->gameover();
		_vm->_gyro->background(0);
		break;
	case Gyro::ink:
		_vm->_visa->dixi('U', 4);
		break;
	case Gyro::chastity:
		_vm->_visa->dixi('U', 5);
		break;
	case Gyro::mushroom:
		_vm->_visa->dixi('U', 6);
		_vm->_lucerna->gameover();
		break;
	case Gyro::onion:
		if (_vm->_gyro->dna.rotten_onion)
			_vm->_visa->dixi('U', 11);
		else {
			_vm->_visa->dixi('U', 8);
			_vm->_gyro->dna.obj[_vm->_gyro->onion - 1] = false;
			_vm->_lucerna->objectlist();
		}
		break;
	default:
		if ((_vm->_gyro->dna.room == r__argentpub) || (_vm->_gyro->dna.room == r__nottspub))
			_vm->_scrolls->display("Try BUYing things before you drink them!");
		else
			_vm->_scrolls->display("The taste of it makes you retch!");
	}
}

void Acci::peopleInRoom() {
	byte numPeople = 0; // Number of people in the room.
	
	for (byte i = 1; i < 29; i++) { // Start at 1 so we don't list Avvy himself!
		if (_vm->_gyro->whereis[i] == _vm->_gyro->dna.room)
			numPeople++;
	}

	if (numPeople == 0) // If nobody's here, we can cut out straight away.
		return;

	byte actPerson = 0; // Actually listed people.
	for (byte i = 1; i < 29; i++) {
		if (_vm->_gyro->whereis[i] == _vm->_gyro->dna.room) {
			actPerson++;
			if (actPerson == 1) // First on the list.
				_vm->_scrolls->display(_vm->_gyro->getname(i + 150) + _vm->_scrolls->kControlToBuffer);
			else if (actPerson < numPeople) // The middle...
				_vm->_scrolls->display(Common::String(", ") + _vm->_gyro->getname(i + 150) + _vm->_scrolls->kControlToBuffer);
			else // The end.
				_vm->_scrolls->display(Common::String(" and ") + _vm->_gyro->getname(i + 150) + _vm->_scrolls->kControlToBuffer);
		}
	}

	if (numPeople == 1)
		_vm->_scrolls->display(Common::String(" is") + _vm->_scrolls->kControlToBuffer);
	else
		_vm->_scrolls->display(Common::String(" are") + _vm->_scrolls->kControlToBuffer);

	_vm->_scrolls->display(" here."); // End and display it.
}

void Acci::lookAround() {
	_vm->_scrolls->display(*_vm->_gyro->also[0][1]);
	switch (_vm->_gyro->dna.room) {
	case r__spludwicks:
		if (_vm->_gyro->dna.avaricius_talk > 0)
			_vm->_visa->dixi('q', 23);
		else
			peopleInRoom();
		break;
	case r__robins:
		if (_vm->_gyro->dna.tied_up)
			_vm->_visa->dixi('q', 38);
		if (_vm->_gyro->dna.mushroom_growing)
			_vm->_visa->dixi('q', 55);
		break;
	case r__insidecardiffcastle:
		if (!_vm->_gyro->dna.taken_pen)
			_vm->_visa->dixi('q', 49);
		break;
	case r__lustiesroom:
		if (_vm->_gyro->dna.lustie_is_asleep)
			_vm->_visa->dixi('q', 65);
		break;
	case r__catacombs:
		switch (_vm->_gyro->dna.cat_y * 256 + _vm->_gyro->dna.cat_x) {
		case 258 :
			_vm->_visa->dixi('q', 80); // Inside art gallery.
			break; 
		case 514 :
			_vm->_visa->dixi('q', 81); // Outside ditto.
			break;
		case 260 :
			_vm->_visa->dixi('q', 82); // Outside Geida's room.
			break; 
		}
		break;
	default:
		peopleInRoom();
	}
}

void Acci::openDoor() { 
	// Special cases.
	switch (_vm->_gyro->dna.room) {   
	case r__yours:
		if (_vm->_trip->infield(2)) {
			// Opening the box.
			_thing = 54; // The box.
			_person = kPardon;
			examine();
			return;
		}
		break;
	case r__spludwicks:
		if (_thing == 61) {
			_vm->_visa->dixi('q', 85);
			return;
		}
		break;
	}

	if ((!_vm->_gyro->dna.user_moves_avvy) && (_vm->_gyro->dna.room != r__lusties))
		return; // No doors can open if you can't move Avvy.

	for (byte fv = 8; fv < 15; fv++) {
		if (_vm->_trip->infield(fv + 1)) {
			fv -= 8;

			switch (_vm->_gyro->portals[fv].op) {
			case Gyro::exclaim:
				_vm->_trip->tr[0].bounce();
				_vm->_visa->dixi('x', _vm->_gyro->portals[fv].data);
				break;
			case Gyro::transport:
				_vm->_trip->fliproom((_vm->_gyro->portals[fv].data) >> 8 /*High byte*/, (_vm->_gyro->portals[fv].data) & 0x0F /*Low byte*/);
				break;
			case Gyro::unfinished:
				_vm->_trip->tr[0].bounce();
				_vm->_scrolls->display("Sorry. This place is not available yet!");
				break;
			case Gyro::special:
				_vm->_trip->call_special(_vm->_gyro->portals[fv].data);
				break;
			case Gyro::mopendoor:
				_vm->_trip->open_the_door((_vm->_gyro->portals[fv].data) >> 8, (_vm->_gyro->portals[fv].data) & 0x0F, fv + 9);
				break;
			}
			
			return;
		}
	}

	if (_vm->_gyro->dna.room == r__map)
		_vm->_scrolls->display(Common::String("Avvy, you can complete the whole game without ever going "
				"to anywhere other than Argent, Birmingham, Cardiff, Nottingham and Norwich."));
	else
		_vm->_scrolls->display("Door? What door?");
}



void Acci::silly() {
	_vm->_scrolls->display("Don't be silly!");
}

void Acci::putProc() {
	if (!holding())
		return;

	_thing2 -= 49; // Slip the second object.
	char temp = _thing;
	_thing = _thing2;
	if (!holding())
		return;
	_thing = temp;

	// Thing is the _thing which you're putting in. _thing2 is where you're putting it.
	switch (_thing2) {
	case Gyro::wine:
		if (_thing == _vm->_gyro->onion) {
			if (_vm->_gyro->dna.rotten_onion)
				_vm->_scrolls->display("That's a bit like shutting the stable door after the horse has bolted!");
			else { // Put onion into wine?
				if (_vm->_gyro->dna.winestate != 3)
					_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlItalic) + "Oignon au vin"
					+ _vm->_scrolls->kControlRoman + " is a bit too strong for your tastes!");
				else { // Put onion into vinegar! Yes!
					_vm->_gyro->dna.onion_in_vinegar = true;
					_vm->_lucerna->points(7);
					_vm->_visa->dixi('u', 9);
				}
			}
		} else
			silly();
		break;

	case 54:
		if (_vm->_gyro->dna.room == r__yours) { // Put something into the box.
			if (_vm->_gyro->dna.box_contents != kNothing)
				_vm->_scrolls->display("There's something in the box already, Avvy. Try taking that out first.");
			else {
				switch (_thing) {
				case Gyro::money:
					_vm->_scrolls->display("You'd better keep some ready cash on you!");
					break;
				case Gyro::bell:
					_vm->_scrolls->display("That's a silly place to keep a bell.");
					break;
				case Gyro::bodkin:
					_vm->_scrolls->display("But you might need it!");
					break;
				case Gyro::onion:
					_vm->_scrolls->display("Just give it to Spludwick, Avvy!");
					break;
				default: // Put the object into the box...
					if (_vm->_gyro->dna.wearing == _thing)
						_vm->_scrolls->display(Common::String("You'd better take ") + _vm->_gyro->get_better(_thing) + " off first!");
					else {
						openBox(true); // Open box.

						_vm->_gyro->dna.box_contents = _thing;
						_vm->_gyro->dna.obj[_thing - 1] = false;
						_vm->_lucerna->objectlist();
						_vm->_scrolls->display("OK, it's in the box.");

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



void Acci::notInOrder() {
	_vm->_scrolls->display(Common::String("Sorry, I need the ingredients in the right order for this potion. What I need next is ")
			+ _vm->_gyro->get_better(_vm->_gyro->spludwick_order[_vm->_gyro->dna.given2spludwick])
			+ _vm->_scrolls->kControlRegister + 2 + _vm->_scrolls->kControlSpeechBubble);
}

void Acci::goToCauldron() {
	_vm->_trip->tr[1].call_eachstep = false; // Stops Geida_Procs.
	_vm->_timeout->set_up_timer(1, _vm->_timeout->procspludwick_goes_to_cauldron, _vm->_timeout->reason_spludwalk);
	_vm->_trip->tr[1].walkto(2);
}

/**
 * Check is it's possible to give something to Spludwick
 * @remarks	Originally called 'give2spludwick'
 */
bool Acci::giveToSpludwick() { 
	if (_vm->_gyro->spludwick_order[_vm->_gyro->dna.given2spludwick] != _thing) {
		notInOrder();
		return false;
	}

	switch (_thing) {
	case Gyro::onion:
		_vm->_gyro->dna.obj[_vm->_gyro->onion - 1] = false;
		if (_vm->_gyro->dna.rotten_onion)
			_vm->_visa->dixi('q', 22);
		else {
			_vm->_gyro->dna.given2spludwick++;
			_vm->_visa->dixi('q', 20);
			goToCauldron();
			_vm->_lucerna->points(3);
		}
		_vm->_lucerna->objectlist();
		break;
	case Gyro::ink:
		_vm->_gyro->dna.obj[_vm->_gyro->ink - 1] = false;
		_vm->_lucerna->objectlist();
		_vm->_gyro->dna.given2spludwick++;
		_vm->_visa->dixi('q', 24);
		goToCauldron();
		_vm->_lucerna->points(3);
		break;
	case Gyro::mushroom:
		_vm->_gyro->dna.obj[_vm->_gyro->mushroom - 1] = false;
		_vm->_visa->dixi('q', 25);
		_vm->_lucerna->points(5);
		_vm->_gyro->dna.given2spludwick++;
		goToCauldron();
		_vm->_gyro->dna.obj[_vm->_gyro->potion - 1] = true;
		_vm->_lucerna->objectlist();
		break;
	default:
		return true;
	}

	return false;
}

void Acci::drink() {
	_vm->_gyro->dna.alcohol += 1;
	if (_vm->_gyro->dna.alcohol == 5) {
		_vm->_gyro->dna.obj[_vm->_gyro->key - 1] = true; // Get the key.
		_vm->_gyro->dna.teetotal = true;
		_vm->_gyro->dna.avvy_is_awake = false;
		_vm->_gyro->dna.avvy_in_bed = true;
		_vm->_lucerna->objectlist();
		_vm->_lucerna->dusk();
		_vm->_gyro->hang_around_for_a_while();
		_vm->_trip->fliproom(1, 1);
		_vm->_gyro->background(14);
		_vm->_trip->new_game_for_trippancy(); // Not really.
	}
}

void Acci::cardiffClimbing() {
	if (_vm->_gyro->dna.standing_on_dais) { // Clamber up.
		_vm->_scrolls->display("You climb down, back onto the floor.");
		_vm->_gyro->dna.standing_on_dais = false;
		_vm->_trip->apped(1, 3);
	} else { // Clamber down.
		if (_vm->_trip->infield(1)) {
			_vm->_scrolls->display("You clamber up onto the dais.");
			_vm->_gyro->dna.standing_on_dais = true;
			_vm->_trip->apped(1, 2);
		} else
			_vm->_scrolls->display("Get a bit closer, Avvy.");
	}
}

void Acci::already() {
	_vm->_scrolls->display("You're already standing!");
}

void Acci::standUp() {
	switch (_vm->_gyro->dna.room) {
	case r__yours: // Avvy isn't asleep.
		if (_vm->_gyro->dna.avvy_is_awake && _vm->_gyro->dna.avvy_in_bed) {  // But he's in bed.
			if (_vm->_gyro->dna.teetotal) {
				_vm->_visa->dixi('d', 12);
				_vm->_gyro->background(0);
				_vm->_visa->dixi('d', 14);
			}
			_vm->_trip->tr[0].visible = true;
			_vm->_gyro->dna.user_moves_avvy = true;
			_vm->_trip->apped(1, 2);
			_vm->_gyro->dna.rw = _vm->_gyro->left;
			_vm->_celer->drawBackgroundSprite(-1, -1, 4); // Picture of empty pillow.
			_vm->_lucerna->points(1);
			_vm->_gyro->dna.avvy_in_bed = false;
			_vm->_timeout->lose_timer(_vm->_timeout->reason_arkata_shouts);
		} else
			already();
		break;

	case r__insidecardiffcastle:
		cardiffClimbing();
		break;

	case r__nottspub:
		if (_vm->_gyro->dna.sitting_in_pub)  {
			_vm->_celer->drawBackgroundSprite(-1, -1, 4); // Not sitting down.
			_vm->_trip->tr[0].visible = true; // But standing up.
			_vm->_trip->apped(1, 4); // And walking away.
			_vm->_gyro->dna.sitting_in_pub = false; // Really not sitting down.
			_vm->_gyro->dna.user_moves_avvy = true; // And ambulant.
		} else
			already();
		break;
	default:
		already();
	}
}



void Acci::getProc(char thing) {
	switch (_vm->_gyro->dna.room) {
	case r__yours:
		if (_vm->_trip->infield(2)) {
			if (_vm->_gyro->dna.box_contents == thing) {
				_vm->_celer->drawBackgroundSprite(-1, -1, 5);
				_vm->_scrolls->display("OK, I've got it.");
				_vm->_gyro->dna.obj[thing - 1] = true;
				_vm->_lucerna->objectlist();
				_vm->_gyro->dna.box_contents = kNothing;
				_vm->_celer->drawBackgroundSprite(-1, -1, 6);
			} else
				_vm->_scrolls->display(Common::String("I can't see ") + _vm->_gyro->get_better(thing) + " in the box.");
		} else
			_vm->_visa->dixi('q', 57);
		break;
	case r__insidecardiffcastle:
		switch (thing) {
		case Gyro::pen:
			if (_vm->_trip->infield(2)) { // Standing on the dais.
				if (_vm->_gyro->dna.taken_pen)
					_vm->_scrolls->display("It's not there, Avvy.");
				else {
					// OK: we're taking the pen, and it's there.
					_vm->_celer->drawBackgroundSprite(-1, -1, 4); // No pen there now.
					_vm->_trip->call_special(3); // Zap!
					_vm->_gyro->dna.taken_pen = true;
					_vm->_gyro->dna.obj[_vm->_gyro->pen - 1] = true;
					_vm->_lucerna->objectlist();
					_vm->_scrolls->display("Taken.");
				}
			} else if (_vm->_gyro->dna.standing_on_dais)
				_vm->_visa->dixi('q', 53);
			else
				_vm->_visa->dixi('q', 51);
			break;
		case Gyro::bolt:
			_vm->_visa->dixi('q', 52);
			break;
		default:
			_vm->_visa->dixi('q', 57);
		}
		break;
	case r__robins:
		if ((thing == _vm->_gyro->mushroom) & (_vm->_trip->infield(1)) & (_vm->_gyro->dna.mushroom_growing)) {
			_vm->_celer->drawBackgroundSprite(-1, -1, 3);
			_vm->_scrolls->display("Got it!");
			_vm->_gyro->dna.mushroom_growing = false;
			_vm->_gyro->dna.taken_mushroom = true;
			_vm->_gyro->dna.obj[_vm->_gyro->mushroom - 1] = true;
			_vm->_lucerna->objectlist();
			_vm->_lucerna->points(3);
		} else
			_vm->_visa->dixi('q', 57);
		break;
	default:
		_vm->_visa->dixi('q', 57);
	}
}

/**
 * Give the lute to Geida
 * @remarks	Originally called 'give_Geida_the_lute'
 */
void Acci::giveGeidaTheLute() {
	if (_vm->_gyro->dna.room != r__lustiesroom) {
		_vm->_scrolls->display(Common::String("Not yet. Try later!") + _vm->_scrolls->kControlRegister + '2' + _vm->_scrolls->kControlSpeechBubble);
		return;
	}
	_vm->_gyro->dna.obj[_vm->_gyro->lute - 1] = false;
	_vm->_lucerna->objectlist();
	_vm->_visa->dixi('q', 64); // She plays it.

	_vm->_timeout->set_up_timer(1, _vm->_timeout->procgive_lute_to_geida, _vm->_timeout->reason_geida_sings);
	_vm->_enid->back_to_bootstrap(4);
}

void Acci::playHarp() {
	if (_vm->_trip->infield(7))
		_vm->_scrolls->musical_scroll();
	else
		_vm->_scrolls->display("Get a bit closer to it, Avvy!");
}

void Acci::winSequence() {
	_vm->_visa->dixi('q', 78);
	_vm->_sequence->first_show(7);
	_vm->_sequence->then_show(8);
	_vm->_sequence->then_show(9);
	_vm->_sequence->start_to_close();
	_vm->_timeout->set_up_timer(30, _vm->_timeout->procwinning, _vm->_timeout->reason_winning);
}

void Acci::personSpeaks() {
	if ((_person == kPardon) || (_person == 0)) {
		if ((_vm->_gyro->him == kPardon) || (_vm->_gyro->whereis[_vm->_gyro->him - 150] != _vm->_gyro->dna.room))
			_person = _vm->_gyro->her;
		else
			_person = _vm->_gyro->him;
	}

	if (_vm->_gyro->whereis[_person - 150] != _vm->_gyro->dna.room) {
		_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + '1' + _vm->_scrolls->kControlToBuffer); // Avvy himself!
		return;
	}

	bool found = false; // The _person we're looking for's code is in _person.

	for (byte i = 0; i < _vm->_trip->numtr; i++) {
		if (_vm->_trip->tr[i].quick && ((_vm->_trip->tr[i].a.accinum + 149) == _person)) {
			_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + byte(i + 49) + _vm->_scrolls->kControlToBuffer);
			found = true;
		}
	}

	if (!found) {
		for (byte i = 0; i < 16; i++) {
			if ((_vm->_gyro->quasipeds[i].who == _person) && (_vm->_gyro->quasipeds[i].room == _vm->_gyro->dna.room))
				_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + byte(i + 65) + _vm->_scrolls->kControlToBuffer);
		}
	}
}
void Acci::heyThanks() {
	personSpeaks();
	_vm->_scrolls->display(Common::String("Hey, thanks!") + _vm->_scrolls->kControlSpeechBubble + "(But now, you've lost it!)");
	_vm->_gyro->dna.obj[_thing - 1] = false;
}

/**
 * @remarks	Originally called 'do_that'
 */
void Acci::doThat() {
	const Common::String booze[] = {"Bitter", "GIED", "Whisky", "Cider", "", "", "", "Mead"};

	if (_thats == Common::String(kNothing))  {
		if (!_thats.empty())
			_thats.clear();
		return;
	}

	if (_vm->_gyro->weirdword)
		return;

	if (_thing < 200)
		_thing -= 49; // "Slip"


	if ((_verb != kVerbCodeLoad) && (_verb != kVerbCodeSave) && (_verb != kVerbCodeQuit) && (_verb != kVerbCodeInfo) && (_verb != kVerbCodeHelp)
	&& (_verb != kVerbCodeLarrypass) && (_verb != kVerbCodePhaon) && (_verb != kVerbCodeBoss) && (_verb != kVerbCodeCheat) && (_verb != kVerbCodeRestart) && (_verb != kVerbCodeDir)
	&& (_verb != kVerbCodeScore) && (_verb != kVerbCodeHiscores) && (_verb != kVerbCodeSmartAlec)) {
		if (!_vm->_gyro->alive) {
			_vm->_scrolls->display(Common::String("You're dead, so don't talk. What are you, a ghost or something? Try restarting, or restoring a saved game!"));
			return;
		}
		if (!_vm->_gyro->dna.avvy_is_awake  && (_verb != kVerbCodeDie) && (_verb != kVerbCodeExpletive) && (_verb != kVerbCodeWake)) {
			_vm->_scrolls->display("Talking in your sleep? Try waking up!");
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
		_vm->_scrolls->display(Common::String("Game paused.") + _vm->_scrolls->kControlCenter + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine
			+ "Press Enter, Esc, or click the mouse on the \"O.K.\" box to continue.");
		break;
	case kVerbCodeGet:
		if (_thing != kPardon) { // Legitimate try to pick something up.
			if (_vm->_gyro->dna.carrying >= maxobjs)
				_vm->_scrolls->display("You can't carry any more!");
			else
				getProc(_thing);
		} else { // Not... ditto.
			if (_person != kPardon)
				_vm->_scrolls->display("You can't sweep folk off their feet!");
			else
				_vm->_scrolls->display("I assure you, you don't need it.");
		}
		break;
	case kVerbCodeDrop:
		_vm->_scrolls->display(Common::String("Two years ago you dropped a florin in the street. Three days ") +
				"later it was gone! So now you never leave ANYTHING lying around. OK?");
		break;
	case kVerbCodeInv:
		inventory();
		break;
	case kVerbCodeTalk:
		if (_person == kPardon) {
			if (_vm->_gyro->subjnumber == 99) // They typed "say password".
				_vm->_scrolls->display(Common::String("Yes, but what ") + _vm->_scrolls->kControlItalic + "is" + _vm->_scrolls->kControlRoman + " the password?");
			else if (((1 <= _vm->_gyro->subjnumber) && (_vm->_gyro->subjnumber <= 49)) || (_vm->_gyro->subjnumber == 253) || (_vm->_gyro->subjnumber == 249)) {
				_thats.deleteChar(0);

				for (byte i = 0; i < 10; i++)
					_realWords[i] = _realWords[i + 1];

				_verb = _vm->_gyro->subjnumber;
				doThat();
				return;
			} else {
				_person = _vm->_gyro->subjnumber;
				_vm->_gyro->subjnumber = 0;
				if ((_person == 0) || (_person == kPardon))
					_vm->_scrolls->display("Talk to whom?");
				else if (isPersonHere())
					_vm->_visa->talkto(_person);
			}
		} else if (_person == kPardon)
			_vm->_scrolls->display("Talk to whom?");
		else if (isPersonHere())
			_vm->_visa->talkto(_person);
		break;
	case kVerbCodeGive:
		if (holding()) {
			if (_person == kPardon)
				_vm->_scrolls->display("Give to whom?");
			else if (isPersonHere()) {
				switch (_thing) {
				case Gyro::money :
					_vm->_scrolls->display("You can't bring yourself to give away your moneybag.");
					break;
				case Gyro::bodkin:
				case Gyro::bell:
				case Gyro::clothes:
				case Gyro::habit :
					_vm->_scrolls->display("Don't give it away, it might be useful!");
					break;
				default:
					switch (_person) {
					case Gyro::pcrapulus:
						if (_thing == _vm->_gyro->wine) {
							_vm->_scrolls->display("Crapulus grabs the wine and gulps it down.");
							_vm->_gyro->dna.obj[_vm->_gyro->wine - 1] = false;
						} else
							heyThanks();
						break;
					case Gyro::pcwytalot:
						if ((_thing == _vm->_gyro->crossbow) || (_thing == _vm->_gyro->bolt))
							_vm->_scrolls->display(Common::String("You might be able to influence Cwytalot more if you used it!"));
						else
							heyThanks();
						break;
					case Gyro::pspludwick:
						if (giveToSpludwick())
							heyThanks();
						break;
					case Gyro::pibythneth:
						if (_thing == _vm->_gyro->badge) {
							_vm->_visa->dixi('q', 32); // Thanks! Wow!
							_vm->_lucerna->points(3);
							_vm->_gyro->dna.obj[_vm->_gyro->badge - 1] = false;
							_vm->_gyro->dna.obj[_vm->_gyro->habit - 1] = true;
							_vm->_gyro->dna.givenbadgetoiby = true;
							_vm->_celer->drawBackgroundSprite(-1, -1, 8);
							_vm->_celer->drawBackgroundSprite(-1, -1, 9);
						} else
							heyThanks();
						break;
					case Gyro::payles:
						if (_vm->_gyro->dna.ayles_is_awake) {
							if (_thing == _vm->_gyro->pen) {
								_vm->_gyro->dna.obj[_vm->_gyro->pen - 1] = false;
								_vm->_visa->dixi('q', 54);
								_vm->_gyro->dna.obj[_vm->_gyro->ink - 1] = true;
								_vm->_gyro->dna.given_pen_to_ayles = true;
								_vm->_lucerna->objectlist();
								_vm->_lucerna->points(2);
							} else
								heyThanks();
						} else
							_vm->_scrolls->display("But he's asleep!");
						break;
					case Gyro::pgeida:
						switch (_thing) {
						case Gyro::potion:
							_vm->_gyro->dna.obj[_vm->_gyro->potion - 1] = false;
							_vm->_visa->dixi('u', 16); // She drinks it.
							_vm->_lucerna->points(2);
							_vm->_gyro->dna.geida_given_potion = true;
							_vm->_lucerna->objectlist();
							break;
						case Gyro::lute:
							giveGeidaTheLute();
							break;
						default:
							heyThanks();
						}
						break;
					case Gyro::parkata:
						switch (_thing) {
						case Gyro::potion:
							if (_vm->_gyro->dna.geida_given_potion)
								winSequence();
							else
								_vm->_visa->dixi('q', 77); // That Geida woman!
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
			_vm->_lucerna->objectlist(); // Just in case...
		}
		break;

	case kVerbCodeEat:
	case kVerbCodeDrink:
		if (holding())
			swallow();
		break;

	case kVerbCodeLoad:
		break;
	case kVerbCodeSave:
		break;
	// We don't handle these two because we use ScummVM's save/load system.

	case kVerbCodePay:
		_vm->_scrolls->display("No money need change hands.");
		break;
	case kVerbCodeLook:
		lookAround();
		break;
	case kVerbCodeBreak:
		_vm->_scrolls->display("Vandalism is prohibited within this game!");
		break;
	case kVerbCodeQuit: /* quit */
		if (_vm->_gyro->demo) {
			warning("STUB: Acci::doThat() - case kVerbCodequit");
		//	_vm->_visa->dixi('pos', 31);
		//	close(demofile);
		//	exit(0); /* Change this later!!! */
		}
		if (!_polite)
			_vm->_scrolls->display("How about a `please\", Avvy?");
		else if (_vm->_scrolls->ask(Common::String(_vm->_scrolls->kControlRegister) + 'C' + _vm->_scrolls->kControlIcon + "Do you really want to quit?"))
			_vm->_gyro->lmo = true;
		break;
	case kVerbCodeGo:
		_vm->_scrolls->display("Just use the arrow keys to walk there.");
		break;
	case kVerbCodeInfo: {
		_vm->_scrolls->aboutscroll = true;

		Common::String toDisplay;
		for (byte i = 0; i < 7; i++)
			toDisplay += _vm->_scrolls->kControlNewLine;
		toDisplay = toDisplay + "LORD AVALOT D'ARGENT" + _vm->_scrolls->kControlCenter + _vm->_scrolls->kControlNewLine
			+ "The medi‘val descendant of" + _vm->_scrolls->kControlNewLine
			+ "Denarius Avaricius Sextus" + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine
			+ "version " + _vm->_gyro->vernum + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "Copyright ï "
			+ _vm->_gyro->copyright + ", Mark, Mike and Thomas Thurman." + _vm->_scrolls->kControlRegister + 'Y' + _vm->_scrolls->kControlIcon;
		_vm->_scrolls->display(toDisplay);
		_vm->_scrolls->aboutscroll = false;
		}
		break;
	case kVerbCodeUndress:
		if (_vm->_gyro->dna.wearing == kNothing)
			_vm->_scrolls->display("You're already stark naked!");
		else if (_vm->_gyro->dna.avvys_in_the_cupboard) {
			_vm->_scrolls->display(Common::String("You take off ") + _vm->_gyro->get_better(_vm->_gyro->dna.wearing) + '.');
			_vm->_gyro->dna.wearing = kNothing;
			_vm->_lucerna->objectlist();
		} else
			_vm->_scrolls->display("Hadn't you better find somewhere more private, Avvy?");
		break;
	case kVerbCodeWear:
		if (holding()) { // Wear something.
			switch (_thing) {
			case Gyro::chastity:
				// \? are used to avoid that ??! is parsed as a trigraph
				_vm->_scrolls->display("Hey, kWhat kind of a weirdo are you\?\?!");
				break;
			case Gyro::clothes:
			case Gyro::habit: { /* Change this! */
					if (_vm->_gyro->dna.wearing != kNothing) {
						if (_vm->_gyro->dna.wearing == _thing)
							_vm->_scrolls->display("You're already wearing that.");
						else
							_vm->_scrolls->display("You'll be rather warm wearing two sets of clothes!");
						return;
					} else
						_vm->_gyro->dna.wearing = _thing;

					_vm->_lucerna->objectlist();

					byte i;
					if (_thing == _vm->_gyro->habit)
						i = 3;
					else
						i = 0;
					if (_vm->_trip->tr[0].whichsprite != i) {
						int16 x = _vm->_trip->tr[0].x;
						int16 y = _vm->_trip->tr[0].y;
						_vm->_trip->tr[0].done();
						_vm->_trip->tr[0].init(i, true, _vm->_trip);
						_vm->_trip->tr[0].appear(x, y, _vm->_trip->left);
						_vm->_trip->tr[0].visible = false;
					}
				}
				break;
			default:
				_vm->_scrolls->display(kWhat);
			}
		}
		break;
	case kVerbCodePlay:
		if (_thing == kPardon) {
			switch (_vm->_gyro->dna.room) { // They just typed "play"...
			case r__argentpub: { // ...in the pub, => play Nim.
					warning("STUB: Acci::doThat() - case kVerbCodeplay");
					// play_nim();
					// The following parts are copied from play_nim().
					// The player automatically wins the game everytime he wins, until I implement the mini-game.

					if (_vm->_gyro->dna.wonnim) { // Already won the game.
						_vm->_visa->dixi('Q', 6);
						return;
					}

					if (!_vm->_gyro->dna.asked_dogfood_about_nim) {
						_vm->_visa->dixi('q', 84);
						return;
					}

					_vm->_visa->dixi('Q', 3);
					_vm->_gyro->dna.playednim++;

					// You won - strange!
					_vm->_visa->dixi('Q', 7); // You won! Give us a lute!
					_vm->_gyro->dna.obj[_vm->_gyro->lute - 1] = true;
					_vm->_lucerna->objectlist();
					_vm->_gyro->dna.wonnim = true;
					_vm->_celer->drawBackgroundSprite(-1, -1, 1); // Show the settle with no lute on it.
					_vm->_lucerna->points(7); // 7 points for winning!

					if (_vm->_gyro->dna.playednim == 1)
						_vm->_lucerna->points(3); // 3 points for playing your 1st game.

					// A warning to the player that there should have been a mini-game. TODO: Remove it later!!!
					_vm->_scrolls->display(Common::String("P.S.: There should have been the mini-game called \"Nim\", but I haven't implemented it yet: you win and get the lute automatically.")
						+ _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "Peter (uruk)");
				}
				break; 
			case r__musicroom:
				playHarp();
				break;
			}
		} else if (holding()) {
			switch (_thing) {
			case Gyro::lute :
					_vm->_visa->dixi('U', 7);

					if (_vm->_gyro->whereis[_vm->_gyro->pcwytalot - 150] == _vm->_gyro->dna.room)
						_vm->_visa->dixi('U', 10);

					if (_vm->_gyro->whereis[_vm->_gyro->pdulustie - 150] == _vm->_gyro->dna.room)
						_vm->_visa->dixi('U', 15);
				break;
			case 52:
				if (_vm->_gyro->dna.room == r__musicroom)
					playHarp();
				else
					_vm->_scrolls->display(kWhat);
				break;
			case 55:
				if (_vm->_gyro->dna.room == r__argentpub)
					// play_nim();
					warning("STUB: Acci::doThat() - case kVerbCodeplay");
				else
					_vm->_scrolls->display(kWhat);
				break;
			default:
				_vm->_scrolls->display(kWhat);
			}
		}
		break;
	case kVerbCodeRing:
		if (holding()) {
			if (_thing == _vm->_gyro->bell) {
				_vm->_scrolls->display("Ding, dong, ding, dong, ding, dong, ding, dong...");
				if ((_vm->_gyro->dna.ringing_bells) & (_vm->_gyro->flagset('B')))
					// \? are used to avoid that ??! is parsed as a trigraph
					_vm->_scrolls->display("(Are you trying to join in, Avvy\?\?!)");
			} else
				_vm->_scrolls->display(kWhat);
		}
		break;
	case kVerbCodeHelp:
		// boot_help();
		warning("STUB: Acci::doThat() - case kVerbCodehelp");
		break;
	case kVerbCodeLarrypass:
		_vm->_scrolls->display("Wrong game!");
		break;
	case kVerbCodePhaon:
		_vm->_scrolls->display("Hello, Phaon!");
		break;
	case kVerbCodeBoss:
		// bosskey();
		warning("STUB: Acci::doThat() - case kVerbCodeboss");
		break;
	case kVerbCodePee:
		if (_vm->_gyro->flagset('P')) {
			_vm->_scrolls->display("Hmm, I don't think anyone will notice...");
			_vm->_timeout->set_up_timer(4, _vm->_timeout->procurinate, _vm->_timeout->reason_gototoilet);
		} else
			_vm->_scrolls->display(Common::String("It would be ") + _vm->_scrolls->kControlItalic + "VERY"
			+ _vm->_scrolls->kControlRoman + " unwise to do that here, Avvy!");
		break;
	case kVerbCodeCheat:
		_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlItalic) + "Cheat mode now enabled.");
		_vm->_gyro->cheat = true;
		break;
	case kVerbCodeMagic:
		if (_vm->_gyro->dna.avaricius_talk > 0)
			_vm->_visa->dixi('q', 19);
		else {
			if ((_vm->_gyro->dna.room == 12) & (_vm->_trip->infield(2))) { // Avaricius appears!
				_vm->_visa->dixi('q', 17);
				if (_vm->_gyro->whereis[1] == 12)
					_vm->_visa->dixi('q', 18);
				else {
					_vm->_trip->tr[1].init(1, false, _vm->_trip); // Avaricius
					_vm->_trip->apped(2, 4);
					_vm->_trip->tr[1].walkto(5);
					_vm->_trip->tr[1].call_eachstep = true;
					_vm->_trip->tr[1].eachstep = _vm->_trip->procback_and_forth;
					_vm->_gyro->dna.avaricius_talk = 14;
					_vm->_timeout->set_up_timer(177, _vm->_timeout->procavaricius_talks, _vm->_timeout->reason_avariciustalks);
				}
			} else
				_vm->_scrolls->display("Nothing appears to happen...");
		}
		break;
	case kVerbCodeSmartAlec:
		_vm->_scrolls->display("Listen, smart alec, that was just rhetoric.");
		break;
	case kVerbCodeExpletive:
		switch (_vm->_gyro->dna.swore) {
		case 0:
			_vm->_scrolls->display(Common::String("Avvy! Do you mind? There might be kids playing!")
				+ _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "(I shouldn't say it again, if I were you!)");
			break;
		case 1:
			_vm->_scrolls->display(Common::String("You hear a distant rumble of thunder. Must you always do things I tell you not to?")
				+ _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "Don't do it again!");
			break;
		default:
			_vm->_pingo->zonk();
			_vm->_scrolls->display(Common::String("A crack of lightning shoots from the sky, and fries you.")
				+ _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "(`Such is the anger of the gods, Avvy!\")");
			_vm->_lucerna->gameover();
		}
		_vm->_gyro->dna.swore++;
		break;
	case kVerbCodeListen:
		if ((_vm->_gyro->dna.ringing_bells) & (_vm->_gyro->flagset('B')))
			_vm->_scrolls->display("All other noise is drowned out by the ringing of the bells.");
		else if (_vm->_gyro->listen.empty())
			_vm->_scrolls->display("You can't hear anything much at the moment, Avvy.");
		else
			_vm->_scrolls->display(_vm->_gyro->listen);
		break;
	case kVerbCodeBuy: // What are they trying to buy?
		switch (_vm->_gyro->dna.room) {
		case r__argentpub:
			if (_vm->_trip->infield(6)) { // We're in a pub, and near the bar.
				switch (_thing) {
				case 51:
				case 53:
				case 54:
				case 58: // Beer, whisky, cider or mead.
					if (_vm->_gyro->dna.malagauche == 177) { // Already getting us one.
						_vm->_visa->dixi('D', 15);
						return;
					}

					if (_vm->_gyro->dna.teetotal)  {
						_vm->_visa->dixi('D', 6);
						return;
					}

					if (_vm->_gyro->dna.alcohol == 0)
						_vm->_lucerna->points(3);

					_vm->_celer->drawBackgroundSprite(-1, -1, 12);
					_vm->_scrolls->display(booze[_thing - 51] + ", please." + _vm->_scrolls->kControlRegister + '1' + _vm->_scrolls->kControlSpeechBubble);
					_vm->_gyro->dna.drinking = _thing;

					_vm->_celer->drawBackgroundSprite(-1, -1, 10);
					_vm->_gyro->dna.malagauche = 177;
					_vm->_timeout->set_up_timer(27, _vm->_timeout->procbuydrinks, _vm->_timeout->reason_drinks);
					break;
				case 52:
					examine();
					break; // We have a right one here - buy Pepsi??!
				case Gyro::wine:
					if (_vm->_gyro->dna.obj[_vm->_gyro->wine - 1])  // We've already got the wine!
						_vm->_visa->dixi('D', 2); // 1 bottle's shufishent!
					else {
						if (_vm->_gyro->dna.malagauche == 177) { // Already getting us one.
							_vm->_visa->dixi('D', 15);
							return;
						}

						if (_vm->_gyro->dna.carrying >= maxobjs) {
							_vm->_scrolls->display("Your hands are full.");
							return;
						}

						_vm->_celer->drawBackgroundSprite(-1, -1, 12);
						_vm->_scrolls->display(Common::String("Wine, please.") + _vm->_scrolls->kControlRegister + '1' + _vm->_scrolls->kControlSpeechBubble);
						if (_vm->_gyro->dna.alcohol == 0)
							_vm->_lucerna->points(3);
						_vm->_celer->drawBackgroundSprite(-1, -1, 10);
						_vm->_gyro->dna.malagauche = 177;

						_vm->_timeout->set_up_timer(27, _vm->_timeout->procbuywine, _vm->_timeout->reason_drinks);
					}
					break;
				}
			} else
				_vm->_visa->dixi('D', 5); // Go to the bar!
			break;                

		case r__outsideducks:
			if (_vm->_trip->infield(6)) {
				if (_thing == _vm->_gyro->onion) {
					if (_vm->_gyro->dna.obj[_vm->_gyro->onion - 1])
						_vm->_visa->dixi('D', 10); // Not planning to juggle with the things!
					else if (_vm->_gyro->dna.carrying >= maxobjs)
						_vm->_scrolls->display("Before you ask, you remember that your hands are full.");
					else {
						if (_vm->_gyro->dna.bought_onion)
							_vm->_visa->dixi('D', 11);
						else {
							_vm->_visa->dixi('D', 9);
							_vm->_lucerna->points(3);
						}
						_vm->_gyro->pennycheck(3); // It costs thruppence.
						_vm->_gyro->dna.obj[_vm->_gyro->onion - 1] = true;
						_vm->_lucerna->objectlist();
						_vm->_gyro->dna.bought_onion = true;
						_vm->_gyro->dna.rotten_onion = false; // It's OK when it leaves the stall!
						_vm->_gyro->dna.onion_in_vinegar = false;
					}
				} else
					_vm->_visa->dixi('D', 0);
			} else
				_vm->_visa->dixi('D', 0);
			break;

		case r__nottspub:
			_vm->_visa->dixi('n', 15); // Can't sell to southerners.
			break; 
		default:
			_vm->_visa->dixi('D', 0); // Can't buy that.
		}
		break;
	case kVerbCodeAttack:
		if ((_vm->_gyro->dna.room == r__brummieroad) &&
				((_person == 157) || (_thing == _vm->_gyro->crossbow) || (_thing == _vm->_gyro->bolt))
				&& (_vm->_gyro->whereis[7] == _vm->_gyro->dna.room)) {
			switch (_vm->_gyro->dna.obj[_vm->_gyro->bolt - 1] + _vm->_gyro->dna.obj[_vm->_gyro->crossbow - 1] * 2) {
				// 0 = neither, 1 = only bolt, 2 = only crossbow, 3 = both.
			case 0:
				_vm->_visa->dixi('Q', 10);
				_vm->_scrolls->display("(At the very least, don't use your bare hands!)");
				break;
			case 1:
				_vm->_scrolls->display("Attack _vm->_gyro->him with only a crossbow bolt? Are you planning on playing darts?!");
				break;
			case 2:
				_vm->_scrolls->display("Come on, Avvy! You're not going to get very far with only a crossbow!");
				break;
			case 3:
				_vm->_visa->dixi('Q', 11);
				_vm->_gyro->dna.cwytalot_gone = true;
				_vm->_gyro->dna.obj[_vm->_gyro->bolt - 1] = false;
				_vm->_gyro->dna.obj[_vm->_gyro->crossbow - 1] = false;
				_vm->_lucerna->objectlist();
				_vm->_gyro->magics[11].op = _vm->_gyro->nix;
				_vm->_lucerna->points(7);
				_vm->_trip->tr[1].walkto(2);
				_vm->_trip->tr[1].vanishifstill = true;
				_vm->_trip->tr[1].call_eachstep = false;
				_vm->_gyro->whereis[7] = 177;
				break;
			default:
				_vm->_visa->dixi('Q', 10); /* Please try not to be so violent! */
			}
		} else
			_vm->_visa->dixi('Q', 10);
		break;
	case kVerbCodePasswd:
		if (_vm->_gyro->dna.room != r__bridge)
			_vm->_visa->dixi('Q', 12);
		else {
			bool ok = true;
			for (byte i = 0; i < _thats.size(); i++) {
				Common::String temp = _realWords[i];
				temp.toUppercase();
				for (byte j = 0; j < kVocabulary[_vm->_gyro->dna.pass_num + kFirstPassword]._word.size(); j++) {
					if (kVocabulary[_vm->_gyro->dna.pass_num + kFirstPassword]._word[j] != temp[j])
						ok = false;
				}
			}

			if (ok) {
				if (_vm->_gyro->dna.drawbridge_open != 0)
					_vm->_scrolls->display("Contrary to your expectations, the drawbridge fails to close again.");
				else {
					_vm->_lucerna->points(4);
					_vm->_scrolls->display("The drawbridge opens!");
					_vm->_timeout->set_up_timer(7, _vm->_timeout->procopen_drawbridge, _vm->_timeout->reason_drawbridgefalls);
					_vm->_gyro->dna.drawbridge_open = 1;
				}
			} else
				_vm->_visa->dixi('Q', 12);
		}
		break;
	case kVerbCodeDir:
		_vm->_enid->dir(_realWords[1]);
		break;
	case kVerbCodeDie:
		_vm->_lucerna->gameover();
		break;
	case kVerbCodeScore:
		_vm->_scrolls->display(Common::String("Your score is ") + _vm->_gyro->strf(_vm->_gyro->dna.score) + ',' + _vm->_scrolls->kControlCenter
			+ _vm->_scrolls->kControlNewLine + "out of a possible 128." + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine
			+ "This gives you a rank of " + rank() + '.' + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + totalTime());
		break;
	case kVerbCodePut:
		putProc();
		break;
	case kVerbCodeStand:
		standUp();
		break;
	case kVerbCodeKiss:
		if (_person == kPardon)
			_vm->_scrolls->display("Kiss whom?");
		else if (isPersonHere()) {
			switch (_person) {
			case Gyro::parkata:
				_vm->_visa->dixi('U', 12);
				break;
			case Gyro::pgeida:
				_vm->_visa->dixi('U', 13);
				break;
			case Gyro::pwisewoman:
				_vm->_visa->dixi('U', 14);
				break;
			default:
				_vm->_visa->dixi('U', 5); // You WHAT?
			}
		} else if ((150 <= _person) && (_person <= 174))
			_vm->_scrolls->display("Hey, kWhat kind of a weirdo are you??");

		break;
	case kVerbCodeClimb:
		if (_vm->_gyro->dna.room == r__insidecardiffcastle)
			cardiffClimbing();
		else // In the wrong room!
			_vm->_scrolls->display("Not with your head for heights, Avvy!");
		break;
	case kVerbCodeJump:
		_vm->_timeout->set_up_timer(1, _vm->_timeout->procjump, _vm->_timeout->reason_jumping);
		_vm->_gyro->dna.user_moves_avvy = false;
		break;
	case kVerbCodeHiscores:
		//	show_highs();
		warning("STUB: Acci::doThat() - case kVerbCodehighscores");
		break;
	case kVerbCodeWake:
		if (isPersonHere())
			switch (_person) {
			case kPardon:
			case Gyro::pavalot:
			case 0:
				if (!_vm->_gyro->dna.avvy_is_awake) {
					_vm->_gyro->dna.avvy_is_awake = true;
					_vm->_lucerna->points(1);
					_vm->_gyro->dna.avvy_in_bed = true;
					_vm->_celer->drawBackgroundSprite(-1, -1, 3); /* Picture of Avvy, awake in bed. */
					if (_vm->_gyro->dna.teetotal)
						_vm->_visa->dixi('d', 13);
				} else
					_vm->_scrolls->display("You're already awake, Avvy!");
				break;
			case Gyro::payles:
				if (!_vm->_gyro->dna.ayles_is_awake)
					_vm->_scrolls->display("You can't seem to wake him by yourself.");
				break;
			case Gyro::pjacques:
				_vm->_scrolls->display(Common::String("Brother Jacques, Brother Jacques, are you asleep?") + _vm->_scrolls->kControlRegister + '1' + 
					_vm->_scrolls->kControlSpeechBubble + "Hmmm... that doesn't seem to do any good...");
				break;
			default:
				_vm->_scrolls->display("It's difficult to awaken people who aren't asleep...!");
		}
		break;
	case kVerbCodeSit:
		if (_vm->_gyro->dna.room == r__nottspub) {
			if (_vm->_gyro->dna.sitting_in_pub)
				_vm->_scrolls->display("You're already sitting!");
			else {
				_vm->_trip->tr[0].walkto(4); // Move Avvy to the place, and sit him down.
				_vm->_timeout->set_up_timer(1, _vm->_timeout->procavvy_sit_down, _vm->_timeout->reason_sitting_down);
			}
		} else { // Default doodah.
			_vm->_lucerna->dusk();
			_vm->_gyro->hang_around_for_a_while();
			_vm->_lucerna->dawn();
			_vm->_scrolls->display(Common::String("A few hours later...") + _vm->_scrolls->kControlParagraph + "nothing much has happened...");
		}
		break;
	case kVerbCodeRestart:
		if (_vm->_scrolls->ask("Restart game and lose changes?"))  {
			_vm->_lucerna->dusk();
			_vm->_gyro->newgame();
			_vm->_lucerna->dawn();
		}
		break;
	case kPardon:
		_vm->_scrolls->display("Hey, a _verb would be helpful!");
		break;
	case kVerbCodeHello:
		personSpeaks();
		_vm->_scrolls->display(Common::String("Hello.") + _vm->_scrolls->kControlSpeechBubble);
		break;
	case kVerbCodeThanks:
		personSpeaks();
		_vm->_scrolls->display(Common::String("That's OK.") + _vm->_scrolls->kControlSpeechBubble);
		break;
	default:
		_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlBell) + "Parser bug!");
	}
}

void Acci::verbOpt(byte verb, Common::String &answer, char &ansKey) {
	switch (verb) {
	case kVerbCodeExam:
		answer = "Examine";
		ansKey = 'x';
		break; // The ubiqutous one.
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
