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


/*#include "NimUnit.h"*/
/*#include "Highs.h"*/
/*#include "Helper.h"*/

namespace Avalanche {

const Acci::vocab Acci::words[nowords] = {
		/* Verbs, 1-49 */
	{1, "EXAMINE"},   {1, "READ"},      {1, "XAM"},                  /* short */
	{2, "OPEN"},      {2, "LEAVE"},     {2, "UNLOCK"},
	{3, "PAUSE"},     {47, "TA"},            /* Early to avoid Take and Talk. */
	{4, "TAKE"},      {4, "GET"},       {4, "PICK"},
	{5, "DROP"},      {6, "INVENTORY"}, {7, "TALK"},
	{7, "SAY"},       {7, "ASK"},
	{8, "GIVE"},      {9, "DRINK"},     {9, "IMBIBE"},
	{9, "DRAIN"},     {10, "LOAD"},      {10, "RESTORE"},
	{11, "SAVE"},      {12, "BRIBE"},     {12, "PAY"},
	{13, "LOOK"},      {14, "BREAK"},     {15, "QUIT"},
	{15, "EXIT"},      {16, "SIT"},       {16, "SLEEP"},
	{17, "STAND"},

	{18, "GO"},        {19, "INFO"},      {20, "UNDRESS"},
	{20, "DOFF"},
	{21, "DRESS"},     {21, "WEAR"},      {21, "DON"},
	{22, "PLAY"},
	{22, "STRUM"},     {23, "RING"},      {24, "HELP"},
	{25, "KENDAL"},    {26, "CAPYBARA"},  {27, "BOSS"},
	{255, "NINET"},    /* block for NINETY */
	{28, "URINATE"},   {28, "MINGITE"},   {29, "NINETY"},
	{30, "ABRACADABRA"}, {30, "PLUGH"},     {30, "XYZZY"},
	{30, "HOCUS"},     {30, "POCUS"},     {30, "IZZY"},
	{30, "WIZZY"},     {30, "PLOVER"},
	{30, "MELENKURION"}, {30, "ZORTON"},    {30, "BLERBI"},
	{30, "THURB"},     {30, "SNOEZE"},    {30, "SAMOHT"},
	{30, "NOSIDE"},    {30, "PHUGGG"},    {30, "KNERL"},
	{30, "MAGIC"},     {30, "KLAETU"},    {30, "VODEL"},
	{30, "BONESCROLLS"}, {30, "RADOF"},

	{31, "RESTART"},
	{32, "SWALLOW"},   {32, "EAT"},       {33, "LISTEN"},
	{33, "HEAR"},      {34, "BUY"},       {34, "PURCHASE"},
	{34, "ORDER"},     {34, "DEMAND"},
	{35, "ATTACK"},    {35, "HIT"},       {35, "KILL"},
	{35, "PUNCH"},     {35, "KICK"},      {35, "SHOOT"},
	{35, "FIRE"},

	/* Passwords, 36: */

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
	{47, "THANKS"},       /* = 47, "ta", which was defined earlier. */


	/* Nouns - Objects: 50-100. */

	{50, "WINE"},      {50, "BOOZE"},    {50, "NASTY"},
	{50, "VINEGAR"},   {51, "MONEYBAG"},
	{51, "BAG"},       {51, "CASH"},     {51, "DOSH"},
	{51, "WALLET"},
	{52, "BODKIN"},    {52, "DAGGER"},   {53, "POTION"},
	{54, "CHASTITY"},  {54, "BELT"},     {55, "BOLT"},
	{55, "ARROW"},     {55, "DART"},
	{56, "CROSSBOW"},  {56, "BOW"},      {57, "LUTE"},
	{58, "PILGRIM"},   {58, "BADGE"},    {59, "MUSHROOMS"},
	{59, "TOADSTOOLS"}, {60, "KEY"},      {61, "BELL"},
	{62, "PRESCRIPT"}, {62, "SCROLL"},   {62, "MESSAGE"},
	{63, "PEN"},       {63, "QUILL"},    {64, "INK"},
	{64, "INKPOT"},    {65, "CLOTHES"},  {66, "HABIT"},
	{66, "DISGUISE"},  {67, "ONION"},

	{99, "PASSWORD"},

	/* Objects from Also are placed between 101 and 131. */

	/* Nouns - People - Male, 150-174 */
	{150, "AVVY"},      {150, "AVALOT"},    {150, "YOURSELF"},
	{150, "ME"},        {150, "MYSELF"},    {151, "SPLUDWICK"},
	{151, "THOMAS"},    {151, "ALCHEMIST"}, {151, "CHEMIST"},
	{152, "CRAPULUS"},  {152, "SERF"},      {152, "SLAVE"},
	{158, "DU"},      /* <<< Put in early for Baron DU Lustie to save confusion with Duck & Duke.*/
	{152, "CRAPPY"},    {153, "DUCK"},      {153, "DOCTOR"},
	{154, "MALAGAUCHE"},
	{155, "FRIAR"},     {155, "TUCK"},      {156, "ROBIN"},
	{156, "HOOD"},      {157, "CWYTALOT"},  {157, "GUARD"},
	{157, "BRIDGEKEEP"}, {158, "BARON"},     {158, "LUSTIE"},
	{159, "DUKE"},      {159, "GRACE"},     {160, "DOGFOOD"},
	{160, "MINSTREL"},  {161, "TRADER"},    {161, "SHOPKEEPER"},
	{161, "STALLHOLDER"},
	{162, "PILGRIM"},   {162, "IBYTHNETH"}, {163, "ABBOT"},
	{163, "AYLES"},     {164, "PORT"},      {165, "SPURGE"},
	{166, "JACQUES"},   {166, "SLEEPER"},   {166, "RINGER"},

	/* Nouns- People - Female: 175-199 */
	{175, "WIFE"},      {175, "ARKATA"},    {176, "GEDALODAVA"},
	{176, "GEIDA"},     {176, "PRINCESS"},  {178, "WISE"},
	{178, "WITCH"},

	/* Pronouns, 200-224 */
	{200, "HIM"},       {200, "MAN"},       {200, "GUY"},
	{200, "DUDE"},      {200, "CHAP"},      {200, "FELLOW"},
	{201, "HER"},       {201, "GIRL"},      {201, "WOMAN"},
	{202, "IT"},        {202, "THING"},

	{203, "MONK"},       {204, "BARMAN"},    {204, "BARTENDER"},

	/* Prepositions, 225-249 */
	{225, "TO"},        {226, "AT"},        {227, "UP"},
	{228, "INTO"},      {228, "INSIDE"},    {229, "OFF"},
	{230, "UP"},        {231, "DOWN"},      {232, "ON"},


	/* Please, 251 */
	{251, "PLEASE"},

	/* About, 252 */
	{252, "ABOUT"}, {252, "CONCERNING"},

	/* Swear words, 253 */
	/*              I M P O R T A N T    M E S S A G E

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
	/* and other even ruder words. You didn't read them, did you? Good. */

	/* Answer-back smart-alec words, 249 */
	{249, "YES"},       {249, "NO"},        {249, "BECAUSE"},

	/* Noise words, 255 */
	{255, "THE"},       {255, "A"},         {255, "NOW"},
	{255, "SOME"},      {255, "AND"},       {255, "THAT"},
	{255, "POCUS"},     {255, "HIS"},
	{255, "THIS"},      {255, "SENTINEL"}
	
};        /* for "Ken SENT Me" */


const char Acci::what[] = "That's not possible!";

const Acci::ranktype Acci::ranks[9] = {
	{0, "Beginner"},    {10, "Novice"},
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

void Acci::clearwords() {
	for (byte i = 0; i < 11; i++)
		if (!realwords[i].empty())
			realwords[i].clear();
}

byte Acci::wordnum(Common::String x) {
	if (x.empty())
		return 0;
	
	for (int32 fv = nowords - 1; fv >= 0; fv--) 
		if (words[fv].w == x)
			return words[fv].n;

	// If not found as a whole, we look for it as a substring.
	for (int32 fv = nowords - 1; fv >= 0; fv--) 
		if (Common::String(words[fv].w.c_str(), x.size()) == x)
			return words[fv].n;

	return pardon;
}

void Acci::replace(Common::String old1, byte new1) {
	int16 q = _vm->_parser->pos(old1, thats);
	while (q != -1) {
		if (new1 == 0)
			thats.deleteChar(q);
		else {
			for (byte i = q; i < q + old1.size(); i++)
				thats.deleteChar(q);
			thats.insertChar(new1, q);
		}
		q = _vm->_parser->pos(old1, thats);
	}
}

/*procedure ninetydump;
var f:file; y:int16; bit:byte; a:byte absolute $A000:800;
begin
	off;
	assign(f,'avvydump.avd');
	rewrite(f,1);
	blockwrite(f,_vm->_gyro->dna,177); { just anything }
	for bit:=0 to 3 do
	begin
	port[$3c4]:=2; port[$3ce]:=4; port[$3C5]:=1 shl bit; port[$3CF]:=bit;
	blockwrite(f,a,12080);
	end;
	close(f); on;
	_vm->_scrolls->display('Dumped.');
end;*/

Common::String Acci::rank() {
	for (byte fv = 0; fv < 8; fv++) {
		if ((_vm->_gyro->dna.score >= ranks[fv].score) && (_vm->_gyro->dna.score < ranks[fv + 1].score)) {
			return ranks[fv].title;
		}
	}
	return "";
}

Common::String Acci::totaltime() {
	const double ticks_in_1_sec = (double)(65535) / 3600;
	uint16 h, m, s;
	Common::String a;

	// There are 65535 clock ticks in a second, 1092.25 in a minute, and 65535 in an hour.
	h = _vm->_gyro->dna.total_time / ticks_in_1_sec; // No. of seconds.
	if (h < 0)
		h = ceil((float)h);
	else
		h = floor((float)h);
	m = h % 3600;
	h = h / 3600;
	s = m % 60;
	m = m / 60;

	a = "You've been playing for ";
	if (h > 0)
		a = a + _vm->_gyro->strf(h) + " hours, ";
	if ((m > 0) || (h != 0))
		a = a + _vm->_gyro->strf(m) + " minutes and ";
	a = a + _vm->_gyro->strf(s) + " seconds.";

	return a;
}



void Acci::number(Common::String &codes) {
	warning("STUB: Acci::number()");
}

void Acci::cheatparse(Common::String codes) {
	uint16 num;
	int16 e;

	char cmd;
	int16 se, sx, sy;

	warning("STUB: Acci::cheatparse()");
}

void Acci::punctustrip(Common::String &x) {        /* Strips punctuation from x. */
	const char punct[] = "~`!@#$%^&*()_+-={}[]:\"|;'\\,./<>?";

	for (byte fv = 0; fv < 32; fv++)
		do {
			int16 p = _vm->_parser->pos(Common::String(punct[fv]), x);
			if (p == -1)
				break;
			x.deleteChar(p);
		} while (true);
}



void Acci::displaywhat(byte ch, bool animate, bool &ambigous) { /* << it's an adjective! */
	if (ch == pardon) {
		ambigous = true;
		if (animate)
			_vm->_scrolls->display("Whom?");
		else
			_vm->_scrolls->display("What?");
	} else {
		if (animate)
			_vm->_scrolls->display(Common::String("{ ") + _vm->_gyro->getname(ch) + " }");
		else {
			Common::String z = _vm->_gyro->get_better(ch);
			if (z != "") 
				_vm->_scrolls->display(Common::String("{ ") + z + " }");
		}
	}
}

bool Acci::do_pronouns() {
	bool ambiguous = false;

	for (byte fv = 0; fv < thats.size(); fv++) {
		byte word = thats[fv];
		switch (word) {
		case 200: {
			displaywhat(_vm->_gyro->him, true, ambiguous);
			thats.setChar(_vm->_gyro->him, fv);
			}
			break;
		case 201: {
			displaywhat(_vm->_gyro->her, true, ambiguous);
			thats.setChar(_vm->_gyro->her, fv);
			}
			break;
		case 202: {
			displaywhat(_vm->_gyro->it, false, ambiguous);
			thats.setChar(_vm->_gyro->it, fv);
			}
			break;
		}
	}

	return ambiguous;
}

void Acci::lowercase() {
	byte fv;

	warning("STUB: Acci::lowercase()");
}

void Acci::propernouns() {
	byte fv;

	lowercase();
	warning("STUB: Acci::propernouns()");
}

void Acci::sayit() { /* This makes Avalot say the response. */
	Common::String x;

	warning("STUB: Acci::sayit()");
}

void Acci::store_interrogation(byte interrogation) {
	byte fv;

	warning("STUB: Acci::store_interrogation()");
}



void Acci::clearuint16s() {
	warning("STUB: Acci::clearuint16s()");
}

void Acci::parse() {
	byte n, fv, ff;
	Common::String c, cc, thisword;
	byte answer;
	bool notfound;

	// First parsing - word identification

	if (!thats.empty())
		thats.clear();
	c = _vm->_parser->_inputText + ' ';
	n = 0;
	polite = false;
	verb = pardon;
	thing = pardon;
	thing2 = pardon;
	person = pardon;
	clearwords();


	// A cheat mode attempt.
	if (_vm->_parser->_inputText[0] == '.') {
		cheatparse(_vm->_parser->_inputText);
		thats = nowt;
		return;
	}

	// Are we being interrogated right now?
	if (_vm->_gyro->interrogation > 0) {
		store_interrogation(_vm->_gyro->interrogation);
		_vm->_gyro->weirdword = true;
		return;
	}

	// Actually process the command.
	cc = c;
	c.toUppercase();
	while (!c.empty()) {
		while ((!c.empty()) && (c[0] == ' ')) {
			c.deleteChar(0);
			cc.deleteChar(0);
		}
		if (c.empty())
			break;

		// Get the following word of the strings.
		byte size = _vm->_parser->pos(Common::String(' '), c) + 1;
		char *subStr = new char[size];
		Common::strlcpy(subStr, c.c_str(), size);
		thisword = subStr;
		Common::strlcpy(subStr, cc.c_str(), size);
		realwords[n] = subStr;
		delete[] subStr;

		punctustrip(c);

		notfound = true;

		// Check also[] first, which conatins words about the actual room.
		if (!thisword.empty()) {
			for (ff = 0; ff < 31; ff++) {
				if ((_vm->_gyro->also[ff][0] != 0) && (_vm->_parser->pos(',' + thisword, *_vm->_gyro->also[ff][0]) > -1)) {
					thats = thats + Common::String(99 + ff);
					notfound = false;
				}
			}
		}

		// Check Accis's own table (words[]) for "global" commands.
		if (notfound) {
			answer = wordnum(thisword);
			if (answer == pardon) {
				notfound = true;
				thats = thats + pardon;
			} else
				thats = thats + answer;
			n++;
		}

		// Delete words we already processed.
		int16 spacePos = _vm->_parser->pos(Common::String(' '), c);
		if (spacePos > -1)
			for (byte i = 0; i <= spacePos; i++)
				c.deleteChar(0);
		spacePos = _vm->_parser->pos(Common::String(' '), cc);
		if (spacePos > -1)
			for (byte i = 0; i <= spacePos; i++)
				cc.deleteChar(0);
	}

	if (_vm->_parser->pos(Common::String(254), thats) > -1) 
		unknown = realwords[_vm->_parser->pos(Common::String(254), thats)];
	else
		if (!unknown.empty())
			unknown.clear();

	// Replace words' codes that mean the same.
	replace(Common::String(255), 0); /* zap noise words */
	replace(Common::String(13) + 226, 1); // "look at" = "examine"
	replace(Common::String(13) + 228, 1); // "look in" = "examine"
	replace(Common::String(4) + 230, 17); // "get up" = "stand" 
	replace(Common::String(4) + 231, 17); // "get down" = "stand"... well, why not?
	replace(Common::String(18) + 228, 2); // "go in" = "open [door]"
	replace(Common::String(28) + 229, 253); // "P' off" is a swear word
	replace(Common::String(4) + 6, 6); // "Take inventory" (remember Colossal Adventure?)
	replace(Common::String(40) + 232, 21); // "put on" = "don"
	replace(Common::String(4) + 229, 20); // "take off" = "doff"

	// Words that could mean more than one person
	if (_vm->_gyro->dna.room == r__nottspub)
		replace(Common::String(204), 164); // Barman = Port
	else
		replace(Common::String(204), 154); // Barman = Malagauche

	switch (_vm->_gyro->dna.room) {
	case r__aylesoffice:
		replace(Common::String(203), 163); // Monk = Ayles
		break;       
	case r__musicroom:
		replace(Common::String(203), 166); // Monk = Jacques
		break;          
	default:
		replace(Common::String(203), 162); // Monk = Ibythneth
	}
	
	if (do_pronouns()) {
		_vm->_gyro->weirdword = true;
		thats = nowt;
		return;
	}

	// Second parsing.
	if (!_vm->_gyro->subject.empty())
		_vm->_gyro->subject.clear();
	_vm->_gyro->subjnumber = 0; // Find subject of conversation.
	
	fv = 0;
	while ((fv < 11) && !realwords[fv].empty()) {
		if ((realwords[fv][0] == '\'') || (realwords[fv][0] == '\"')) {
			_vm->_gyro->subjnumber = (byte)thats[fv];
			thats.setChar(moved, fv);
			break;
		}
		fv++;
	}

	if ((_vm->_gyro->subjnumber == 0) && !thats.empty()) // Still not found.
		for (fv = 0; fv < thats.size() - 1; fv++)
			if ((byte)thats[fv] == 252) { // The word is "about", or something similar.
				_vm->_gyro->subjnumber = (byte)thats[fv + 1];
				thats.setChar(0, fv + 1);
				break;
			}

	if ((_vm->_gyro->subjnumber == 0) && !thats.empty())// STILL not found! Must be the word after "say".
		for (fv = 0; fv < thats.size() - 1; fv++)
			if (((byte)thats[fv] == 7) && ((byte)thats[fv + 1] != 0) && !((225 <= (byte)thats[fv + 1]) && ((byte)thats[fv + 1] <= 229))) {
				// SAY not followed by a preposition
				_vm->_gyro->subjnumber = (byte)thats[fv + 1];
				thats.setChar(0, fv + 1);
				break;
			}

	for (int8 fv = thats.size() - 1; fv >= 0; fv--) // Reverse order, so first will be used.
		if (((byte)thats[fv] == 253) || ((byte)thats[fv] == 249) || ((1 <= (byte)thats[fv]) && ((byte)thats[fv] <= 49)))
			verb = (byte)thats[fv];
		else if ((50 <= (byte)thats[fv]) && ((byte)thats[fv] <= 149)) {
			thing2 = thing;
			thing = (byte)thats[fv];
		} else if ((150 <= (byte)thats[fv]) && ((byte)thats[fv] <= 199))
			person = (byte)thats[fv];
		else if ((byte)thats[fv] == 251)
			polite = true;

		if ((!unknown.empty()) && (verb != vb_exam) && (verb != vb_talk) && (verb != vb_save) && (verb != vb_load) && (verb != vb_dir)) {
				_vm->_scrolls->display(Common::String("Sorry, but I have no idea what \"") + unknown + "\" means. Can you rephrase it?");
				_vm->_gyro->weirdword = true;
		} else
			_vm->_gyro->weirdword = false;

	if (thats.empty())
		thats = nowt;

	if (thing != pardon)
		_vm->_gyro->it = thing;

	if (person != pardon)
		if (person < 175)
			_vm->_gyro->him = person;
		else
			_vm->_gyro->her = person;
}

void Acci::examobj() {   
	if (thing != _vm->_gyro->thinks)
		_vm->_lucerna->thinkabout(thing, _vm->_gyro->a_thing);
	switch (thing) {
	case _vm->_gyro->wine :
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
	case _vm->_gyro->onion:
		if (_vm->_gyro->dna.rotten_onion)
			_vm->_visa->dixi('q', 21); // Yucky onion.
		else
			_vm->_visa->dixi('t', 18);  // Normal onion scroll
		break;       
	default:
		_vm->_visa->dixi('t', thing); // <<< Ordinarily
	}
}

bool Acci::personshere() { // Person equivalent of "holding".
	if ((person == pardon) || (person == 0) || (_vm->_gyro->whereis[person - 150] == _vm->_gyro->dna.room))
		return true;
	else {
		if (person < 175)
			_vm->_scrolls->display(Common::String('H') + _vm->_scrolls->kControlToBuffer);
		else
			_vm->_scrolls->display(Common::String("Sh") + _vm->_scrolls->kControlToBuffer);
		_vm->_scrolls->display("e isn't around at the moment.");
		return false;
	}
}

void Acci::exampers() {
	if (personshere()) {
		if (thing != _vm->_gyro->thinks)
			_vm->_lucerna->thinkabout(person, _vm->_gyro->a_person);
		person -= 149;
		switch (person) { /* Special cases */
		case 11:
			if (_vm->_gyro->dna.wonnim)
				_vm->_visa->dixi('Q', 8); // "I'm Not Playing!"
			return;
		case 99:
			if (_vm->_gyro->dna.lustie_is_asleep)
				_vm->_visa->dixi('Q', 65); // He's asleep. (65! Wow!)
			return;
		}
		// Otherwise...
		_vm->_visa->dixi('p', person);
	} 
	
	// And afterwards...
	if ((person == 14) && (!_vm->_gyro->dna.ayles_is_awake))
		_vm->_visa->dixi('Q', 13);
}

bool Acci::holding() {
	if ((51 <= thing) && (thing <= 99)) // Also.
		return true;

	bool holdingResult = false;

	if (thing > 100)
		_vm->_scrolls->display("Be reasonable!");
	else if (!_vm->_gyro->dna.obj[thing])  // Verbs that need "thing" to be in the inventory.
		_vm->_scrolls->display("You're not holding it, Avvy.");
	else 
		holdingResult = true;

	return holdingResult;
}



void Acci::openBox(bool before) {
	_vm->_celer->show_one(5);

	_vm->_celer->pics_link();
	_vm->_trip->trippancy_link();
	_vm->_graphics->refreshScreen();

	_vm->_system->delayMillis(55);

	if (!before) {
		_vm->_celer->show_one(6);
		_vm->_celer->pics_link();
		_vm->_trip->trippancy_link();
		_vm->_graphics->refreshScreen();
	}
}

void Acci::examine() {
	// EITHER it's an object OR it's an Also OR it's a person OR it's something else.
	if ((person == pardon) && (thing != pardon)) {
		if (holding()) {
			// Remember: it's been Slipped! Ie subtract 49.
			if ((1 <= thing) && (thing <= 49)) // Standard object
				examobj();
			else if ((50 <= thing) && (thing <= 100)) { // Also thing
				openBox(true);
				_vm->_scrolls->display(*_vm->_gyro->also[thing - 50][1]);
				openBox(false);
			}
		}
	} else if (person != pardon)
		exampers();
	else _vm->_scrolls->display("It's just as it looks on the picture.");  // Don't know: guess.
}



void Acci::inv() {   /* the time-honoured command... */
	byte q = 0;
	_vm->_scrolls->display(Common::String("You're carrying ") + _vm->_scrolls->kControlToBuffer);
	{
		for (byte fv = 0; fv < numobjs; fv++)
			if (_vm->_gyro->dna.obj[fv]) {
				q++;
				if (q == _vm->_gyro->dna.carrying)
					_vm->_scrolls->display(Common::String("and ") + _vm->_scrolls->kControlToBuffer);
				_vm->_scrolls->display(_vm->_gyro->get_better(fv + 1) + _vm->_scrolls->kControlToBuffer);
				if ((fv + 1) == _vm->_gyro->dna.wearing)
					_vm->_scrolls->display(Common::String(", which you're wearing") + _vm->_scrolls->kControlToBuffer);
				if (q < _vm->_gyro->dna.carrying)
					_vm->_scrolls->display(Common::String(", ") + _vm->_scrolls->kControlToBuffer);
			}
			if (_vm->_gyro->dna.wearing == nowt)
				_vm->_scrolls->display(Common::String("...") + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "...and you're stark naked!");
			else
				_vm->_scrolls->display(".");
	}
}

void Acci::swallow() {   /* Eat something. */
	switch (thing) {
	case _vm->_gyro->wine:
		switch (_vm->_gyro->dna.winestate) { /* 4 is perfect */
		case 1: {
			if (_vm->_gyro->dna.teetotal)  {
				_vm->_visa->dixi('D', 6);
				return;
			}
			_vm->_visa->dixi('U', 1);
			_vm->_pingo->wobble();
			_vm->_visa->dixi('U', 2);
			_vm->_gyro->dna.obj[_vm->_gyro->wine - 1] = false;
			_vm->_lucerna->objectlist();
			have_a_drink();
				}
				break;
		case 2:
		case 3:
			_vm->_visa->dixi('d', 8);
			break; /* You can't drink it! */
		}
		break;
	case _vm->_gyro->potion: {
			_vm->_gyro->background(4);
			_vm->_visa->dixi('U', 3);
			_vm->_lucerna->gameover();
			_vm->_gyro->background(0);
		}
		break;
	case _vm->_gyro->ink:
		_vm->_visa->dixi('U', 4);
		break;
	case _vm->_gyro->chastity:
		_vm->_visa->dixi('U', 5);
		break;
	case _vm->_gyro->mushroom: {
			_vm->_visa->dixi('U', 6);
			_vm->_lucerna->gameover();
		}
		break;
	case _vm->_gyro->onion:
		if (_vm->_gyro->dna.rotten_onion)  _vm->_visa->dixi('U', 11);
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
		/* Constant- leave this one */
	}
}

void Acci::others() {
/* This lists the other people in the room. */
	warning("STUB: Acci::others()");
}

void Acci::lookaround() {
/* This is called when you say "look." */
	_vm->_scrolls->display(*_vm->_gyro->also[0][1]);
	switch (_vm->_gyro->dna.room) {
	case r__spludwicks:
		if (_vm->_gyro->dna.avaricius_talk > 0)
			_vm->_visa->dixi('q', 23);
		else
			others();
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
			_vm->_visa->dixi('q', 80);
			break; /* Inside art gallery */
		case 514 :
			_vm->_visa->dixi('q', 81);
			break; /* Outside ditto */
		case 260 :
			_vm->_visa->dixi('q', 82);
			break; /* Outside Geida's room. */
		}
		break;
	default:
		others();
	}
}

void Acci::opendoor() {   /* so whaddya THINK this does?! */
	switch (_vm->_gyro->dna.room) {   /* Special cases. */
	case r__yours:
		if (_vm->_trip->infield(2)) {
			/* Opening the box. */
			thing = 54; /* The box. */ person = pardon;
			examine();
			return;
		}
		break;
	case r__spludwicks:
		if (thing == 61) {
			_vm->_visa->dixi('q', 85);
			return;
		}
		break;
	}


	if ((!_vm->_gyro->dna.user_moves_avvy) && (_vm->_gyro->dna.room != r__lusties))
		return; /* No doors can open if you can't move Avvy. */

	for (byte fv = 8; fv < 15; fv++)
		if (_vm->_trip->infield(fv + 1)) {
			fv -= 8;

			switch (_vm->_gyro->portals[fv].op) {
			case _vm->_gyro->exclaim:
				_vm->_trip->tr[0].bounce();
				_vm->_visa->dixi('x', _vm->_gyro->portals[fv].data);
				break;
			case _vm->_gyro->transport:
				_vm->_trip->fliproom((_vm->_gyro->portals[fv].data) >> 8 /*High byte*/, (_vm->_gyro->portals[fv].data) & 0x0F /*Low byte*/);
				break;
			case _vm->_gyro->unfinished:
				_vm->_trip->tr[0].bounce();
				_vm->_scrolls->display("Sorry. This place is not available yet!");
				break;
			case _vm->_gyro->special:
				_vm->_trip->call_special(_vm->_gyro->portals[fv].data);
				break;
			case _vm->_gyro->mopendoor:
				_vm->_trip->open_the_door((_vm->_gyro->portals[fv].data) >> 8, (_vm->_gyro->portals[fv].data) & 0x0F, fv);
				break;
			}
			
			return;
		}

	if (_vm->_gyro->dna.room == r__map)
		_vm->_scrolls->display(Common::String("Avvy, you can complete the whole game without ever going "
				"to anywhere other than Argent, Birmingham, Cardiff, "
				"Nottingham and Norwich."));
	else
		_vm->_scrolls->display("Door? What door?");
}




void Acci::silly() {
	_vm->_scrolls->display("Don't be silly!");
}

void Acci::putproc() {   /* Called when you call vb_put. */
	if (!holding())
		return;

	thing2 -= 49;   /* Slip the second object */
	char temp = thing;
	thing = thing2;
	if (!holding())
		return;
	thing = temp;

	// Thing is the thing which you're putting in. Thing2 is where you're putting it.
	switch (thing2) {
	case _vm->_gyro->wine:
		if (thing == _vm->_gyro->onion) {
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
			if (_vm->_gyro->dna.box_contents != nowt)
				_vm->_scrolls->display("There's something in the box already, Avvy. Try taking that out first.");
			else {
				switch (thing) {
				case _vm->_gyro->money:
					_vm->_scrolls->display("You'd better keep some ready cash on you!");
					break;
				case _vm->_gyro->bell:
					_vm->_scrolls->display("That's a silly place to keep a bell.");
					break;
				case _vm->_gyro->bodkin:
					_vm->_scrolls->display("But you might need it!");
					break;
				case _vm->_gyro->onion:
					_vm->_scrolls->display("Just give it to Spludwick, Avvy!");
					break;
				default: // Put the object into the box...
					if (_vm->_gyro->dna.wearing == thing)
						_vm->_scrolls->display(Common::String("You'd better take ") + _vm->_gyro->get_better(thing) + " off first!");
					else {
						//_vm->_celer->show_one(5); // Open box.
						openBox(true);

						_vm->_gyro->dna.box_contents = thing;
						_vm->_gyro->dna.obj[thing - 1] = false;
						_vm->_lucerna->objectlist();
						_vm->_scrolls->display("OK, it's in the box.");

						//_vm->_celer->show_one(6); // Shut box.
						openBox(false);
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


/* The result of this fn is whether or not he says "Hey, thanks!" */
void Acci::not_in_order() {
	_vm->_scrolls->display(Common::String("Sorry, I need the ingredients in the right order for this potion. What I need next is ")
			+ _vm->_gyro->get_better(_vm->_gyro->spludwick_order[_vm->_gyro->dna.given2spludwick])
			+ _vm->_scrolls->kControlRegister + 2 + _vm->_scrolls->kControlSpeechBubble);
}

void Acci::go_to_cauldron() {
	_vm->_trip->tr[1].call_eachstep = false; /* Stops Geida_Procs. */
	_vm->_timeout->set_up_timer(1, _vm->_timeout->procspludwick_goes_to_cauldron, _vm->_timeout->reason_spludwalk);
	_vm->_trip->tr[1].walkto(2);
}

bool Acci::give2spludwick() {
	bool give2spludwick_result = false;

	if (_vm->_gyro->spludwick_order[_vm->_gyro->dna.given2spludwick] != thing) {
		not_in_order();
		return give2spludwick_result;
	}

	switch (thing) {
	case _vm->_gyro->onion:
		_vm->_gyro->dna.obj[_vm->_gyro->onion - 1] = false;
		if (_vm->_gyro->dna.rotten_onion)
			_vm->_visa->dixi('q', 22);
		else {
			_vm->_gyro->dna.given2spludwick++;
			_vm->_visa->dixi('q', 20);
			go_to_cauldron();
			_vm->_lucerna->points(3);
		}
		_vm->_lucerna->objectlist();
		break;
	case _vm->_gyro->ink:
		_vm->_gyro->dna.obj[_vm->_gyro->ink - 1] = false;
		_vm->_lucerna->objectlist();
		_vm->_gyro->dna.given2spludwick++;
		_vm->_visa->dixi('q', 24);
		go_to_cauldron();
		_vm->_lucerna->points(3);
		break;
	case _vm->_gyro->mushroom:
		_vm->_gyro->dna.obj[_vm->_gyro->mushroom - 1] = false;
		_vm->_visa->dixi('q', 25);
		_vm->_lucerna->points(5);
		_vm->_gyro->dna.given2spludwick++;
		go_to_cauldron();
		_vm->_gyro->dna.obj[_vm->_gyro->potion - 1] = true;
		_vm->_lucerna->objectlist();
		break;
	default:
		give2spludwick_result = true;
	}

	return give2spludwick_result;
}

void Acci::have_a_drink() {
	_vm->_gyro->dna.alcohol += 1;
	if (_vm->_gyro->dna.alcohol == 5) {
		_vm->_gyro->dna.obj[_vm->_gyro->key - 1] = true; /* Get the key. */
		_vm->_gyro->dna.teetotal = true;
		_vm->_gyro->dna.avvy_is_awake = false;
		_vm->_gyro->dna.avvy_in_bed = true;
		_vm->_lucerna->objectlist();
		_vm->_lucerna->dusk();
		_vm->_gyro->hang_around_for_a_while();
		_vm->_trip->fliproom(1, 1);
		_vm->_gyro->background(14);
		_vm->_trip->new_game_for_trippancy(); /* Not really */
	}
}

void Acci::cardiff_climbing() {
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

/* Called when you ask Avvy to stand. */
void Acci::already() {
	_vm->_scrolls->display("You're already standing!");
}

void Acci::stand_up() {
	switch (_vm->_gyro->dna.room) {
	case r__yours: /* Avvy isn't asleep. */
		if (_vm->_gyro->dna.avvy_is_awake && _vm->_gyro->dna.avvy_in_bed) {  /* But he's in bed. */
			if (_vm->_gyro->dna.teetotal) {
				_vm->_visa->dixi('d', 12);
				_vm->_gyro->background(0);
				_vm->_visa->dixi('d', 14);
			}
			_vm->_trip->tr[0].visible = true;
			_vm->_gyro->dna.user_moves_avvy = true;
			_vm->_trip->apped(1, 2);
			_vm->_gyro->dna.rw = _vm->_gyro->left;
			_vm->_celer->show_one(4); /* Picture of empty pillow. */
			_vm->_lucerna->points(1);
			_vm->_gyro->dna.avvy_in_bed = false;
			_vm->_timeout->lose_timer(_vm->_timeout->reason_arkata_shouts);
		} else
			already();
		break;

	case r__insidecardiffcastle:
		cardiff_climbing();
		break;

	case r__nottspub:
		if (_vm->_gyro->dna.sitting_in_pub)  {
			_vm->_celer->show_one(4); /* Not sitting down. */
			_vm->_trip->tr[0].visible = true; /* But standing up. */
			_vm->_trip->apped(1, 4); /* And walking away. */
			_vm->_gyro->dna.sitting_in_pub = false; /* Really not sitting down. */
			_vm->_gyro->dna.user_moves_avvy = true; /* And ambulant. */
		} else
			already();
		break;
	default:
		already();
	}
}


void Acci::getproc(char thing) {
	switch (_vm->_gyro->dna.room) {
	case r__yours:
		if (_vm->_trip->infield(2)) {
			if (_vm->_gyro->dna.box_contents == thing) {
				_vm->_celer->show_one(5);
				_vm->_scrolls->display("OK, I've got it.");
				_vm->_gyro->dna.obj[thing - 1] = true;
				_vm->_lucerna->objectlist();
				_vm->_gyro->dna.box_contents = nowt;
				_vm->_celer->show_one(6);
			} else
				_vm->_scrolls->display(Common::String("I can't see ") + _vm->_gyro->get_better(thing) + " in the box.");
		} else
			_vm->_visa->dixi('q', 57);
		break;
	case r__insidecardiffcastle:
		switch (thing) {
		case _vm->_gyro->pen:
			if (_vm->_trip->infield(2)) {
				/* Standing on the dais. */

				if (_vm->_gyro->dna.taken_pen)
					_vm->_scrolls->display("It's not there, Avvy.");
				else {
					/* OK: we're taking the pen, and it's there. */
					_vm->_celer->show_one(4); /* No pen there now. */
					_vm->_trip->call_special(3); /* Zap! */
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
		case _vm->_gyro->bolt:
			_vm->_visa->dixi('q', 52);
			break;
		default:
			_vm->_visa->dixi('q', 57);
		}
		break;
	case r__robins:
		if ((thing == _vm->_gyro->mushroom) & (_vm->_trip->infield(1)) & (_vm->_gyro->dna.mushroom_growing)) {
			_vm->_celer->show_one(3);
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

void Acci::give_geida_the_lute() {
	if (_vm->_gyro->dna.room != r__lustiesroom) {
		_vm->_scrolls->display("Not yet. Try later!\232\2");
		return;
	}
	_vm->_gyro->dna.obj[_vm->_gyro->lute - 1] = false;
	_vm->_lucerna->objectlist();
	_vm->_visa->dixi('q', 64); /* She plays it. */

	/* And the rest has been moved to Timeout... under give_lute_to_Geida. */

	_vm->_timeout->set_up_timer(1, _vm->_timeout->procgive_lute_to_geida, _vm->_timeout->reason_geida_sings);
	_vm->_enid->back_to_bootstrap(4);
}

void Acci::play_harp() {
	if (_vm->_trip->infield(7))
		_vm->_scrolls->musical_scroll();
	else
		_vm->_scrolls->display("Get a bit closer to it, Avvy!");
}

void Acci::winsequence() {
	_vm->_visa->dixi('q', 78);
	_vm->_sequence->first_show(7);
	_vm->_sequence->then_show(8);
	_vm->_sequence->then_show(9);
	_vm->_sequence->start_to_close();
	_vm->_timeout->set_up_timer(30, _vm->_timeout->procwinning, _vm->_timeout->reason_winning);
}

void Acci::person_speaks() {
	bool found;
	byte fv;
	char cfv;


	if ((person == pardon) || (person == 0))
		if ((_vm->_gyro->him == pardon) || (_vm->_gyro->whereis[_vm->_gyro->him - 150] != _vm->_gyro->dna.room))
			person = _vm->_gyro->her;
		else
			person = _vm->_gyro->him;

	if (_vm->_gyro->whereis[person - 150] != _vm->_gyro->dna.room) {
		_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + '1' + _vm->_scrolls->kControlToBuffer); // Avvy himself!
		return;
	}

	found = false; /* The person we're looking for's code is in Person. */

	for (fv = 0; fv < _vm->_trip->numtr; fv++) {
		if (_vm->_trip->tr[fv].quick && ((_vm->_trip->tr[fv].a.accinum + 149) == person)) {
			_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + byte(fv + 49) + _vm->_scrolls->kControlToBuffer);
			found = true;
		}
	}

	if (!found)
		for (fv = 0; fv < 16; fv++) 
			if ((_vm->_gyro->quasipeds[fv].who == person) && (_vm->_gyro->quasipeds[fv].room == _vm->_gyro->dna.room))
				_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlRegister) + byte(fv + 65) + _vm->_scrolls->kControlToBuffer);
}

void Acci::heythanks() {
	person_speaks();
	_vm->_scrolls->display(Common::String("Hey, thanks!") + _vm->_scrolls->kControlSpeechBubble + "(But now, you've lost it!)");
	_vm->_gyro->dna.obj[thing - 1] = false;
}

void Acci::do_that() {
	const Common::String booze[] = {"Bitter", "GIED", "Whisky", "Cider", "", "", "", "Mead"};
	byte fv, ff;
	int16 sx, sy;
	bool ok;


	if (thats == Common::String(nowt))  {
		if (!thats.empty())
			thats.clear();
		return;
	}

	if (_vm->_gyro->weirdword)
		return;

	if (thing < 200)
		thing -= 49; // "Slip"


	if ((verb != vb_load) && (verb != vb_save) && (verb != vb_quit) && (verb != vb_info) && (verb != vb_help)
		&& (verb != vb_larrypass) && (verb != vb_phaon) && (verb != vb_boss) && (verb != vb_cheat) && (verb != vb_restart) && (verb != vb_dir)
		&& (verb != vb_score) && (verb != vb_highscores) && (verb != vb_smartalec)) {
			if (!_vm->_gyro->alive) {
				_vm->_scrolls->display(Common::String("You're dead, so don't talk. What are you, a ghost or something? Try restarting, or restoring a saved game!"));
				return;
			}
			if (!_vm->_gyro->dna.avvy_is_awake  && (verb != vb_die) && (verb != vb_expletive) && (verb != vb_wake)) {
				_vm->_scrolls->display("Talking in your sleep? Try waking up!");
				return;
			}
	}

	switch (verb) {
	case vb_exam:
		examine();
		break;
	case vb_open:
		opendoor();
		break;
	case vb_pause: // Note that the original game doesn't care about the "O.K." box neither, it accepts clicks from everywhere on the screen to continue. Just like my code.
		_vm->_scrolls->display(Common::String("Game paused.") + _vm->_scrolls->kControlCenter + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine
			+ "Press Enter, Esc, or click the mouse on the \"O.K.\" box to continue.");
		break;
	case vb_get:
		if (thing != pardon) {
			/* Legitimate try to pick something up. */
			if (_vm->_gyro->dna.carrying >= maxobjs)
				_vm->_scrolls->display("You can't carry any more!");
			else
				getproc(thing);

		} else {
			/* Not... ditto. */
			if (person != pardon)
				_vm->_scrolls->display("You can't sweep folk off their feet!");
			else
				_vm->_scrolls->display("I assure you, you don't need it.");
		}
		break;
	case vb_drop:
		_vm->_scrolls->display(Common::String("Two years ago you dropped a florin in the street. Three days ") +
				"later it was gone! So now you never leave ANYTHING lying around. OK?");
		break;
		/*       begin _vm->_gyro->dna.obj[thing]:=false; _vm->_lucerna->objectlist(); end;*/
	case vb_inv:
		inv();
		break;
	case vb_talk:
		if (person == pardon) {
			if (_vm->_gyro->subjnumber == 99) /* They typed "say password". */
				_vm->_scrolls->display(Common::String("Yes, but what ") + _vm->_scrolls->kControlItalic + "is" + _vm->_scrolls->kControlRoman + " the password?");
			else if (((1 <= _vm->_gyro->subjnumber) && (_vm->_gyro->subjnumber <= 49)) || (_vm->_gyro->subjnumber == 253) || (_vm->_gyro->subjnumber == 449)) {
				thats.deleteChar(0);

				for (byte i = 0; i < 10; i++)
					realwords[i] = realwords[i + 1];
				//move(realwords[2], realwords[1], sizeof(realwords) - sizeof(realwords[1]));

				verb = _vm->_gyro->subjnumber;
				do_that();
				return;
			} else {
				person = _vm->_gyro->subjnumber;
				_vm->_gyro->subjnumber = 0;
				if ((person == 0) || (person == pardon))
					_vm->_scrolls->display("Talk to whom?");
				else if (personshere())
					_vm->_visa->talkto(person);
			}
		} else if (person == pardon)
			_vm->_scrolls->display("Talk to whom?");
		else if (personshere())
			_vm->_visa->talkto(person);
		break;
	case vb_give:
		if (holding()) {
			if (person == pardon)
				_vm->_scrolls->display("Give to whom?");
			else if (personshere()) {
				switch (thing) {
				case _vm->_gyro->money :
					_vm->_scrolls->display("You can't bring yourself to give away your moneybag.");
					break;
				case _vm->_gyro->bodkin:
				case _vm->_gyro->bell:
				case _vm->_gyro->clothes:
				case _vm->_gyro->habit :
					_vm->_scrolls->display("Don't give it away, it might be useful!");
					break;
				default:
					switch (person) {
					case _vm->_gyro->pcrapulus:
						if (thing == _vm->_gyro->wine) {
							_vm->_scrolls->display("Crapulus grabs the wine and gulps it down.");
							_vm->_gyro->dna.obj[_vm->_gyro->wine - 1] = false;
						} else
							heythanks();
						break;
					case _vm->_gyro->pcwytalot:
						if ((thing == _vm->_gyro->crossbow) || (thing == _vm->_gyro->bolt))
							_vm->_scrolls->display(Common::String("You might be able to influence Cwytalot more if you used it!"));
						else
							heythanks();
						break;
					case _vm->_gyro->pspludwick:
						if (give2spludwick())
							heythanks();
						break;
					case _vm->_gyro->pibythneth:
						if (thing == _vm->_gyro->badge) {
							_vm->_visa->dixi('q', 32); /* Thanks! Wow! */
							_vm->_lucerna->points(3);
							_vm->_gyro->dna.obj[_vm->_gyro->badge - 1] = false;
							_vm->_gyro->dna.obj[_vm->_gyro->habit - 1] = true;
							_vm->_gyro->dna.givenbadgetoiby = true;
							_vm->_celer->show_one(8);
							_vm->_celer->show_one(9);
						} else
							heythanks();
						break;
					case _vm->_gyro->payles:
						if (_vm->_gyro->dna.ayles_is_awake) {
							if (thing == _vm->_gyro->pen) {
								_vm->_gyro->dna.obj[_vm->_gyro->pen - 1] = false;
								_vm->_visa->dixi('q', 54);
								_vm->_gyro->dna.obj[_vm->_gyro->ink - 1] = true;
								_vm->_gyro->dna.given_pen_to_ayles = true;
								_vm->_lucerna->objectlist();
								_vm->_lucerna->points(2);
							} else
								heythanks();
						} else
							_vm->_scrolls->display("But he's asleep!");
						break;
					case _vm->_gyro->pgeida:
						switch (thing) {
						case _vm->_gyro->potion:
							_vm->_gyro->dna.obj[_vm->_gyro->potion - 1] = false;
							_vm->_visa->dixi('u', 16); /* She drinks it. */
							_vm->_lucerna->points(2);
							_vm->_gyro->dna.geida_given_potion = true;
							_vm->_lucerna->objectlist();
							break;
						case _vm->_gyro->lute:
							give_geida_the_lute();
							break;
						default:
							heythanks();
						}
						break;
					case _vm->_gyro->parkata:
						switch (thing) {
						case _vm->_gyro->potion:
							if (_vm->_gyro->dna.geida_given_potion)
								winsequence();
							else
								_vm->_visa->dixi('q', 77);
							break;             /* That Geida woman! */
						default:
							heythanks();
						}
						break;
					default:
						heythanks();
					}
				}
			}
			_vm->_lucerna->objectlist(); /* Just in case... */
		}
		break;

	case vb_eat:
	case vb_drink:
		if (holding())
			swallow();
		break;

	case vb_load:
		break;
	case vb_save:
		break;
	// We don't handle these two because we use ScummVM's save/load system.

	case vb_pay:
		_vm->_scrolls->display("No money need change hands.");
		break;
	case vb_look:
		lookaround();
		break;
	case vb_break:
		_vm->_scrolls->display("Vandalism is prohibited within this game!");
		break;
	case vb_quit: /* quit */
		if (_vm->_gyro->demo) {
			warning("STUB: Acci::do_that() - case vb_quit");
		//	_vm->_visa->dixi('q', 31);
		//	close(demofile);
		//	exit(0); /* Change this later!!! */
		}
		if (!polite)
			_vm->_scrolls->display("How about a `please\", Avvy?");
		else if (_vm->_scrolls->ask(Common::String(_vm->_scrolls->kControlRegister) + 'C' + _vm->_scrolls->kControlIcon + "Do you really want to quit?"))
			_vm->_gyro->lmo = true;
		break;
	case vb_go:
		_vm->_scrolls->display("Just use the arrow keys to walk there.");
		break;
	case vb_info: {
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
	case vb_undress:
		if (_vm->_gyro->dna.wearing == nowt)
			_vm->_scrolls->display("You're already stark naked!");
		else if (_vm->_gyro->dna.avvys_in_the_cupboard) {
			_vm->_scrolls->display(Common::String("You take off ") + _vm->_gyro->get_better(_vm->_gyro->dna.wearing) + '.');
			_vm->_gyro->dna.wearing = nowt;
			_vm->_lucerna->objectlist();
		} else
			_vm->_scrolls->display("Hadn't you better find somewhere more private, Avvy?");
		break;
	case vb_wear:
		if (holding()) { /* wear something */
			switch (thing) {
			case _vm->_gyro->chastity:
				_vm->_scrolls->display("Hey, what kind of a weirdo are you??!");
				break;
			case _vm->_gyro->clothes:
			case _vm->_gyro->habit: { /* Change this! */
					if (_vm->_gyro->dna.wearing != nowt) {
						if (_vm->_gyro->dna.wearing == thing)
							_vm->_scrolls->display("You're already wearing that.");
						else
							_vm->_scrolls->display("You'll be rather warm wearing two sets of clothes!");
						return;
					} else
						_vm->_gyro->dna.wearing = thing;

					_vm->_lucerna->objectlist();

					if (thing == _vm->_gyro->habit)
						fv = 3;
					else
						fv = 0;

					if (_vm->_trip->tr[0].whichsprite != fv) {
						sx = _vm->_trip->tr[0].x;
						sy = _vm->_trip->tr[0].y;
						_vm->_trip->tr[0].done();
						_vm->_trip->tr[0].init(fv, true, _vm->_trip);
						_vm->_trip->tr[0].appear(sx, sy, _vm->_trip->left);
						_vm->_trip->tr[0].visible = false;
					}
				}
				break;
			default:
				_vm->_scrolls->display(what);
			}
		}
		break;
	case vb_play:
		if (thing == pardon) {
			switch (_vm->_gyro->dna.room) { /* They just typed "play"... */
			case r__argentpub:
				// play_nim();
				warning("STUB: Acci::do_that() - case vb_play");
				break; /* ...in the pub, => play Nim. */
			case r__musicroom:
				play_harp();
				break;
			}
		} else if (holding()) {
			switch (thing) {
			case _vm->_gyro->lute : {
					_vm->_visa->dixi('U', 7);

					if (_vm->_gyro->whereis[_vm->_gyro->pcwytalot - 150] == _vm->_gyro->dna.room)
						_vm->_visa->dixi('U', 10);

					if (_vm->_gyro->whereis[_vm->_gyro->pdulustie - 150] == _vm->_gyro->dna.room)
						_vm->_visa->dixi('U', 15);
				}
				break;
			case 52:
				if (_vm->_gyro->dna.room == r__musicroom)
					play_harp();
				else
					_vm->_scrolls->display(what);
				break;
			case 55:
				if (_vm->_gyro->dna.room == r__argentpub)
					// play_nim();
					warning("STUB: Acci::do_that() - case vb_play");
				else
					_vm->_scrolls->display(what);
				break;
			default:
				_vm->_scrolls->display(what);
			}
		}
		break;
	case vb_ring:
		if (holding()) {
			if (thing == _vm->_gyro->bell) {
				_vm->_scrolls->display("Ding, dong, ding, dong, ding, dong, ding, dong...");
				if ((_vm->_gyro->dna.ringing_bells) & (_vm->_gyro->flagset('B')))
					_vm->_scrolls->display("(Are you trying to join in, Avvy??!)");
			} else
				_vm->_scrolls->display(what);
		}
		break;
	case vb_help:
		// boot_help();
		warning("STUB: Acci::do_that() - case vb_help");
		break;
	case vb_larrypass:
		_vm->_scrolls->display("Wrong game!");
		break;
	case vb_phaon:
		_vm->_scrolls->display("Hello, Phaon!");
		break;
	case vb_boss:
		// bosskey();
		warning("STUB: Acci::do_that() - case vb_boss");
		break;
	case vb_pee:
		if (_vm->_gyro->flagset('P')) {
			_vm->_scrolls->display("Hmm, I don't think anyone will notice...");
			_vm->_timeout->set_up_timer(4, _vm->_timeout->procurinate, _vm->_timeout->reason_gototoilet);
		} else
			_vm->_scrolls->display(Common::String("It would be ") + _vm->_scrolls->kControlItalic + "VERY"
			+ _vm->_scrolls->kControlRoman + " unwise to do that here, Avvy!");
		break;
	case vb_cheat:
		_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlItalic) + "Cheat mode now enabled.");
		_vm->_gyro->cheat = true;
		break;
	case vb_magic:
		if (_vm->_gyro->dna.avaricius_talk > 0)
			_vm->_visa->dixi('q', 19);
		else {
			if ((_vm->_gyro->dna.room == 12) & (_vm->_trip->infield(2))) { // Avaricius appears!
				_vm->_visa->dixi('q', 17);
				if (_vm->_gyro->whereis[1] == 12)
					_vm->_visa->dixi('q', 18);
				else {
					_vm->_trip->tr[1].init(1, false, _vm->_trip); /* Avaricius */
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
	case vb_smartalec:
		_vm->_scrolls->display("Listen, smart alec, that was just rhetoric.");
		break;
	case vb_expletive:
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
	case vb_listen:
		if ((_vm->_gyro->dna.ringing_bells) & (_vm->_gyro->flagset('B')))
			_vm->_scrolls->display("All other noise is drowned out by the ringing of the bells.");
		else if (_vm->_gyro->listen.empty())
			_vm->_scrolls->display("You can't hear anything much at the moment, Avvy.");
		else
			_vm->_scrolls->display(_vm->_gyro->listen);
		break;
	case vb_buy: // What are they trying to buy?
		switch (_vm->_gyro->dna.room) {
		case r__argentpub:
			if (_vm->_trip->infield(6)) { // We're in a pub, and near the bar.
				switch (thing) {
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

					_vm->_celer->show_one(12);
					_vm->_scrolls->display(booze[thing - 51] + ", please." + _vm->_scrolls->kControlRegister + '1' + _vm->_scrolls->kControlSpeechBubble);
					_vm->_gyro->dna.drinking = thing;

					_vm->_celer->show_one(10);
					_vm->_gyro->dna.malagauche = 177;
					_vm->_timeout->set_up_timer(27, _vm->_timeout->procbuydrinks, _vm->_timeout->reason_drinks);
					break;
				case 52:
					examine();
					break; // We have a right one here - buy Pepsi??!
				case _vm->_gyro->wine:
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

						_vm->_celer->show_one(12);
						_vm->_scrolls->display("Wine, please." + _vm->_scrolls->kControlRegister + '1' + _vm->_scrolls->kControlSpeechBubble);
						if (_vm->_gyro->dna.alcohol == 0)
							_vm->_lucerna->points(3);
						_vm->_celer->show_one(10);
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
				if (thing == _vm->_gyro->onion) {
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
	case vb_attack:
		if ((_vm->_gyro->dna.room == r__brummieroad) &&
				((person == 157) || (thing == _vm->_gyro->crossbow) || (thing == _vm->_gyro->bolt))
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
	case vb_password:
		if (_vm->_gyro->dna.room != r__bridge)
			_vm->_visa->dixi('Q', 12);
		else {
			ok = true;
			for (ff = 0; ff < thats.size(); ff++) {
				for (fv = 0; fv < words[_vm->_gyro->dna.pass_num + first_password - 1].w.size(); fv++) {
					Common::String temp = realwords[ff];
					temp.toUppercase();
					if (words[_vm->_gyro->dna.pass_num + first_password - 1].w[fv] != temp[fv])
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
	case vb_dir:
		_vm->_enid->dir(realwords[1]);
		break;
	case vb_die:
		_vm->_lucerna->gameover();
		break;
	case vb_score:
		_vm->_scrolls->display(Common::String("Your score is ") + _vm->_gyro->strf(_vm->_gyro->dna.score) + ',' + _vm->_scrolls->kControlCenter
			+ _vm->_scrolls->kControlNewLine + "out of a possible 128." + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine
			+ "This gives you a rank of " + rank() + '.' + _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + totaltime());
		break;
	case vb_put:
		putproc();
		break;
	case vb_stand:
		stand_up();
		break;
	case vb_kiss:
		if (person == pardon)
			_vm->_scrolls->display("Kiss whom?");
		else if (personshere()) {
			switch (person) {
			case _vm->_gyro->parkata:
				_vm->_visa->dixi('U', 12);
				break;
			case _vm->_gyro->pgeida:
				_vm->_visa->dixi('U', 13);
				break;
			case _vm->_gyro->pwisewoman:
				_vm->_visa->dixi('U', 14);
				break;
			default:
				_vm->_visa->dixi('U', 5); /* You WHAT? */
			}
		} else if ((150 <= person) && (person <= 174))
			_vm->_scrolls->display("Hey, what kind of a weirdo are you??");

		break;
	case vb_climb:
		if (_vm->_gyro->dna.room == r__insidecardiffcastle)
			cardiff_climbing();
		else /* In the wrong room! */
			_vm->_scrolls->display("Not with your head for heights, Avvy!");
		break;
	case vb_jump:
		_vm->_timeout->set_up_timer(1, _vm->_timeout->procjump, _vm->_timeout->reason_jumping);
		_vm->_gyro->dna.user_moves_avvy = false;
		break;
	case vb_highscores:
		//	show_highs();
		warning("STUB: Acci::do_that() - case vb_highscores");
		break;
	case vb_wake:
		if (personshere())
			switch (person) {
			case pardon:
			case _vm->_gyro->pavalot:
			case 0:
				if (!_vm->_gyro->dna.avvy_is_awake) {
					_vm->_gyro->dna.avvy_is_awake = true;
					_vm->_lucerna->points(1);
					_vm->_gyro->dna.avvy_in_bed = true;
					_vm->_celer->show_one(3); /* Picture of Avvy, awake in bed. */
					if (_vm->_gyro->dna.teetotal)
						_vm->_visa->dixi('d', 13);
				} else
					_vm->_scrolls->display("You're already awake, Avvy!");
				break;
			case _vm->_gyro->payles:
				if (!_vm->_gyro->dna.ayles_is_awake)
					_vm->_scrolls->display("You can't seem to wake him by yourself.");
				break;
			case _vm->_gyro->pjacques:
				_vm->_scrolls->display(Common::String("Brother Jacques, Brother Jacques, are you asleep?") + _vm->_scrolls->kControlRegister + '1' + 
					_vm->_scrolls->kControlSpeechBubble + "Hmmm... that doesn't seem to do any good...");
				break;
			default:
				_vm->_scrolls->display("It's difficult to awaken people who aren't asleep...!");
		}
		break;
	case vb_sit:
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
	case vb_restart:
		if (_vm->_scrolls->ask("Restart game and lose changes?"))  {
			_vm->_lucerna->dusk();
			_vm->_gyro->newgame();
			_vm->_lucerna->dawn();
		}
		break;
	case pardon:
		_vm->_scrolls->display("Hey, a verb would be helpful!");
		break;
	case vb_hello:
		person_speaks();
		_vm->_scrolls->display(Common::String("Hello.") + _vm->_scrolls->kControlSpeechBubble);
		break;
	case vb_thanks:
		person_speaks();
		_vm->_scrolls->display(Common::String("That's OK.") + _vm->_scrolls->kControlSpeechBubble);
		break;
	default:
		_vm->_scrolls->display(Common::String(_vm->_scrolls->kControlBell) + "Parser bug!");
	}
}

void Acci::verbopt(char n, Common::String &answer, char &anskey) {
	switch (n) {
	case vb_exam:
		answer = "Examine";
		anskey = 'x';
		break; /* the ubiqutous one */
	/* vb_give isn't dealt with by this procedure, but by ddm__with */
	case vb_drink:
		answer = "Drink";
		anskey = 'D';
		break;
	case vb_wear:
		answer = "Wear";
		anskey = 'W';
		break;
	case vb_ring:
		answer = "Ring";
		anskey = 'R';
		break; /* only the bell! */
	case vb_play:
		answer = "Play";
		anskey = 'P';
		break;
	case vb_eat:
		answer = "Eat";
		anskey = 'E';
		break;
	default:
		answer = "? Unknown!";    /* Bug! */
		anskey = '?';
	}
}



} // End of namespace Avalanche.
