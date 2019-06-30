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

 // Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// HACK to allow building with the SDL backend on MinGW
// see bug #1800764 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif // main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/language.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "zlib.h"
#include "winexe_pe.h"
#include "file.h"
#include "script_preresponses.h"
#include "script_quotes.h"
#include "script_responses.h"
#include "script_ranges.h"
#include "script_states.h"
#include "tag_maps.h"

/**
 * Format of the access.dat file that will be created:
 * 4 Bytes - Magic string 'SVTN' to identify valid data file
 * 2 bytes - Version number
 *
 * Following is a series of index entries with the following fields:
 * 4 bytes - offset in file of entry
 * 4 bytes - size of entry in the file
 * ASCIIZ  - name of the resource
 */

#define VERSION_NUMBER 5
#define HEADER_SIZE 0x1700

Common::File inputFile, outputFile;
Common::PEResources resEng, resGer;
uint headerOffset = 6;
uint dataOffset = HEADER_SIZE;

#define ENGLISH_10042C_FILESIZE 4099072
#define ENGLISH_10042B_FILESIZE 4095488
#define ENGLISH_10042_FILESIZE 4094976
#define GERMAN_10042D_FILESIZE 4542464

enum {
	ENGLISH_10042C_DIFF = 0x401C00,
	ENGLISH_10042B_DIFF = 0x401400,
	ENGLISH_10042_DIFF = 0x402000,
	GERMAN_DIFF = 0x401200
};
enum Version {
	ENGLISH_10042C = 0,
	ENGLISH_10042B = 1,
	ENGLISH_10042 = 2,
	GERMAN = 3
};
Version _version;

const int FILE_DIFF[4] = {
	ENGLISH_10042C_DIFF, ENGLISH_10042B_DIFF, ENGLISH_10042_DIFF,
	GERMAN_DIFF
};

static const char *const ITEM_NAMES[46] = {
	"LeftArmWith", "LeftArmWithout", "RightArmWith", "RightArmWithout", "BridgeRed",
	"BridgeYellow", "BridgeBlue", "BridgeGreen", "Parrot", "CentralCore", "BrainGreen",
	"BrainYellow", "BrainRed", "BrainBlue", "ChickenGreasy", "ChickenPlain", "ChickenPurple",
	"ChickenRed", "ChickenYellow", "CrushedTV", "Ear", "Ear1", "Eyeball", "Eyeball1",
	"Feather", "Lemon", "GlassEmpty", "GlassPurple", "GlassRed", "GlassYellow", "Hammer",
	"Hose", "HoseEnd", "LiftHead", "LongStick", "Magazine", "Mouth", "MusicKey", "Napkin",
	"Nose", "Perch", "PhonoCylinder", "PhonoCylinder1", "PhonoCylinder2", "PhonoCylinder3",
	"Photo"
};

static const char *const ITEM_DESCRIPTIONS[46] = {
	"The Maitre d'Bot's left arm holding a key", "The Maitre d'Bot's left arm",
	"The Maitre d'Bot's right arm holding Titania's auditory center",
	"The Maitre d'Bot's right arm", "Red Fuse", "Yellow Fuse", "Blue Fuse",
	"Green Fuse", "The Parrot", "Titania's central intelligence core",
	"Titania's auditory center", "Titania's olfactory center",
	"Titania's speech center", "Titania's vision center", "rather greasy chicken",
	"very plain chicken", "chicken smeared with starling pur$e",
	"chicken covered with tomato sauce", "chicken coated in mustard sauce",
	"A crushed television set", "Titania's ear", "Titania's ear", "Titania's eye",
	"Titania's eye", "A parrot feather", "A nice fat juicy lemon",
	"An empty beer glass", "A beer glass containing pur$ed flock of starlings",
	"A beer glass containing tomato sauce", "A beer glass containing mustard sauce",
	"A hammer", "A hose", "The other end of a hose", "The LiftBot's head",
	"A rather long stick", "A magazine", "Titania's mouth", "A key",
	"A super-absorbent napkin", "Titania's nose", "A perch", "A phonograph cylinder",
	"A phonograph cylinder", "A phonograph cylinder", "A phonograph cylinder",
	"A photograph"
};
static const char *const ITEM_DESCRIPTIONS_DE[46] = {
	"Der linke Arm des OberkellnerBots, im Besitz eines Schl\xFC""ssels ",
	"Der linke Arm des OberkellnerBots",
	"Der rechte Arm des OberkellnerBots, im Besitz von Titanias Geh\xF6""rmodul",
	"Der rechte Arm des OberkellnerBots",
	"Rote Sicherung",
	"Gelbe Sicherung",
	"Blaue Sicherung",
	"Gr\xFC""ne Sicherung",
	"Der Papagei",
	"Titanias Gro\xDF""hirn",
	"Titanias Geh\xF6""rmodul",
	"Titanias Geruchsmodul",
	"Titanias Sprachmodul",
	"Titanias Gesichtsmodul",
	"ziemlich fettiges H\xFC""hnchen",
	"H\xFC""hnchen ohne alles",
	"mit Starenp\xFC""ree beschmiertes H\xFC""hnchen",
	"mit Tomatensauce garniertes H\xFC""hnchen",
	"mit Senfso\xDF""e \xFC""berzogenes H\xFC""hnchen",
	"Ein zerschmetterter Fernsehapparat",
	"Titanias Ohr",
	"Titanias Ohr",
	"Titanias Auge",
	"Titanias Auge",
	"Eine Papageienfeder",
	"Eine sch\xF6""ne fette saftige Zitrone",
	"Ein leeres Bierglas",
	"Ein Starenp\xFC""ree enthaltendes Bierglas",
	"Ein Tomatenso\xDF""e enthaltendes Bierglas",
	"Ein Senfso\xDF""e enthaltendes Bierglas",
	"Ein Hammer",
	"Ein Schlauch",
	"Das andere Ende eines Schlauchs",
	"Der Kopf eines LiftBots",
	"Ein ziemlich langer Stab",
	"Ein Magazin",
	"Titanias Mund",
	"Ein Schl\xFC""ssel",
	"Eine supersaugf\xE4""hige Serviette",
	"Titanias Nase",
	"Eine Vogelstange",
	"Ein Grammophonzylinder",
	"Ein Grammophonzylinder",
	"Ein Grammophonzylinder",
	"Ein Grammophonzylinder",
	"Ein Foto"
};

static const char *const ITEM_IDS[40] = {
	"MaitreD Left Arm", "MaitreD Right Arm", "OlfactoryCentre", "AuditoryCentre",
	"SpeechCentre", "VisionCentre", "CentralCore", "Perch", "SeasonBridge",
	"FanBridge", "BeamBridge", "ChickenBridge", "CarryParrot", "Chicken",
	"CrushedTV", "Feathers", "Lemon", "BeerGlass", "BigHammer", "Ear1", "Ear 2",
	"Eye1", "Eye2", "Mouth", "Nose", "NoseSpare", "Hose", "DeadHoseSpare",
	"HoseEnd", "DeadHoseEndSpare", "BrokenLiftbotHead", "LongStick", "Magazine",
	"Napkin", "Phonograph Cylinder", "Phonograph Cylinder 1", "Phonograph Cylinder 2",
	"Phonograph Cylinder 3", "Photograph", "Music System Key"
};

static const char *const ROOM_NAMES[34] = {
	"1stClassLobby", "1stClassRestaurant", "1stClassState",
	"2ndClassLobby", "secClassState", "Arboretum", "FrozenArboretum",
	"Bar", "BilgeRoom", "BilgeRoomWith", "BottomOfWell", "Bridge",
	"CreatorsChamber", "CreatorsChamberOn", "Dome", "Home", "Lift",
	"EmbLobby", "MoonEmbLobby", "MusicRoomLobby", "MusicRoom",
	"ParrotLobby", "Pellerator", "PromenadeDeck", "SculptureChamber",
	"SecClassLittleLift", "ServiceElevator", "SGTLeisure", "SGTLittleLift",
	"SgtLobby", "SGTState", "Titania", "TopOfWell", "PlayersRoom"
};

struct NumberEntry {
	const char *_text;
	int _value;
	uint _flags;
};

const NumberEntry NUMBERS[76] = {
	{ "a", 1, 3 },
	{ "and", 0, 1 },
	{ "negative", 0, 10 },
	{ "minus", 0, 10 },
	{ "below zero", 0, 8 },
	{ "degrees below zero", 0, 8 },
	{ "nil", 0, 2 },
	{ "zero", 0, 2 },
	{ "one", 1, 0x12 },
	{ "two", 2, 0x12 },
	{ "three", 3, 0x12 },
	{ "four", 4, 0x12 },
	{ "five", 5, 0x12 },
	{ "six", 6, 0x12 },
	{ "seven", 7, 0x12 },
	{ "eight", 8, 0x12 },
	{ "nine", 9, 0x12 },
	{ "0", 0, 2 },
	{ "1", 1, 2 },
	{ "2", 2, 2 },
	{ "3", 3, 2 },
	{ "4", 4, 2 },
	{ "5", 5, 2 },
	{ "6", 6, 2 },
	{ "7", 7, 2 },
	{ "8", 8, 2 },
	{ "9", 9, 2 },
	{ "first", 1, 2 },
	{ "second", 2, 2 },
	{ "third", 3, 2 },
	{ "fourth", 4, 2 },
	{ "fifth", 5, 2 },
	{ "sixth", 6, 2 },
	{ "seventh", 7, 2 },
	{ "eighth", 8, 2 },
	{ "ninth", 9, 2 },
	{ "ten", 10, 2 },
	{ "eleven", 11, 2 },
	{ "twelve", 12, 2 },
	{ "thirteen", 13, 2 },
	{ "fourteen", 14, 2 },
	{ "fifteen", 15, 2 },
	{ "sixteen", 16, 2 },
	{ "seventeen", 17, 2 },
	{ "eighteen", 18, 2 },
	{ "nineteen", 19, 2 },
	{ "tenth", 10, 2 },
	{ "eleventh", 11, 2 },
	{ "twelfth", 12, 2 },
	{ "thirteenth", 13, 2 },
	{ "fourteenth", 14, 2 },
	{ "fifteenth", 15, 2 },
	{ "sixteenth", 16, 2 },
	{ "seventeenth", 17, 2 },
	{ "eighteenth", 18, 2 },
	{ "nineteenth", 19, 2 },
	{ "twenty", 20, 0x12 },
	{ "thirty", 30, 0x12 },
	{ "forty", 40, 0x12 },
	{ "fourty", 40, 0x12 },
	{ "fifty", 50, 0x12 },
	{ "sixty", 60, 0x12 },
	{ "seventy", 70, 0x12 },
	{ "eighty", 80, 0x12 },
	{ "ninety", 90, 0x12 },
	{ "twentieth", 20, 2 },
	{ "thirtieth", 30, 2 },
	{ "fortieth", 40, 2 },
	{ "fiftieth", 50, 2 },
	{ "sixtieth", 60, 2 },
	{ "seventieth", 70, 2 },
	{ "eightieth", 80, 2 },
	{ "ninetieth", 90, 2 },
	{ "hundred", 100, 4 },
	{ "hundredth", 100, 6 }
};

struct CommonPhrase {
	const char *_str;
	uint _dialogueId;
	uint _roomNum;
	uint _val1;
};

static const CommonPhrase BELLBOT_COMMON_PHRASES_EN[] = {
	{ "what is wrong with her", 0x30FF9, 0x7B, 0 },
	{ "what is wrong with titania", 0x30FF9, 0x7B, 0 },
	{ "something for the weekend", 0x30D8B, 0x00, 0 },
	{ "other food", 0x30E1D, 0x00, 3 },
	{ "different food", 0x30E1D, 0x00, 3 },
	{ "alternative food", 0x30E1D, 0x00, 3 },
	{ "decent food", 0x30E1D, 0x00, 3 },
	{ "nice food", 0x30E1D, 0x00, 3 },
	{ "nicer food", 0x30E1D, 0x00, 3 },
	{ "make me happy", 0x31011, 0x00, 0 },
	{ "cheer me up", 0x31011, 0x00, 0 },
	{ "help me if im unhappy", 0x31011, 0x00, 0 },
	{ "i obtain a better room", 0x30E8A, 0x00, 3 },
	{ "i obtain a better room", 0x30E8A, 0x00, 2 },
	{ "i get a better room", 0x30E8A, 0x00, 3 },
	{ "i get a better room", 0x30E8A, 0x00, 2 },
	{ "i want a better room", 0x30E8A, 0x00, 3 },
	{ "i want a better room", 0x30E8A, 0x00, 2 },
	{ "i understood", 0x30D75, 0x6D, 0 },
	{ "i knew", 0x30D75, 0x6D, 0 },
	{ "i know", 0x30D75, 0x6D, 0 },
	{ "not stupid", 0x30D75, 0x6D, 0 },
	{ "cheeky", 0x30D75, 0x6D, 0 },
	{ "not help", 0x30D6F, 0x6D, 0 },
	{ "not helpful", 0x30D6F, 0x6D, 0 },
	{ "dont help", 0x30D6F, 0x6D, 0 },
	{ "no help", 0x30D6F, 0x6D, 0 },
	{ "sorry", 0x30D76, 0x6D, 0 },
	{ "not mean that", 0x30D76, 0x6D, 0 },
	{ "didnt mean that", 0x30D76, 0x6D, 0 },
	{ "apologise", 0x30D76, 0x6D, 0 },
	{ "play golf", 0x313B6, 0x00, 0 },
	{ "is not the captain meant to go down with the ship", 0x31482, 0x00, 0 },
	{ "is not the captain supposed to go down with the ship", 0x31482, 0x00, 0 },
	{ "sauce sticks to the chicken", 0x3156B, 0x00, 0 },
	{ "sauce gets stuck to the chicken", 0x3156B, 0x00, 0 },
	{ nullptr, 0, 0, 0 }
};
static const CommonPhrase BELLBOT_COMMON_PHRASES_DE[] = {
	{ "was ist mit ihr los", 0x30ff9, 0x7b, 0 },
	{ "was fehlt ihr", 0x30ff9, 0x7b, 0 },
	{ "was hat sie denn", 0x30ff9, 0x7b, 0 },
	{ "was ist denn mit ihr", 0x30ff9, 0x7b, 0 },
	{ "was stimmt denn nicht mit ihr", 0x30ff9, 0x7b, 0 },
	{ "was ist mit titania los", 0x30ff9, 0x7b, 0 },
	{ "was fehlt titania", 0x30ff9, 0x7b, 0 },
	{ " was ist denn mit titania", 0x30ff9, 0x7b, 0 },
	{ "was ist denn mit titania los", 0x30ff9, 0x7b, 0 },
	{ "etwas anderes zu essen", 0x30e1d, 0, 3 },
	{ "noch zu essen", 0x30e1d, 0, 3 },
	{ "noch essen", 0x30e1d, 0, 3 },
	{ "anderes essen", 0x30e1d, 0, 3 },
	{ "etwas gutes zu essen", 0x30e1d, 0, 3 },
	{ "vernuenftiges essen", 0x30e1d, 0, 3 },
	{ "leckeres essen", 0x30e1d, 0, 3 },
	{ "lecker essen", 0x30e1d, 0, 3 },
	{ "besseres essen", 0x30e1d, 0, 3 },
	{ "gutes zu essen", 0x30e1d, 0, 3 },
	{ "aufheitern", 0x31011, 0, 0 },
	{ "heiter mich mal auf", 0x31011, 0, 0 },
	{ "kriege ich ein besseres zimmer", 0x30e8a, 0, 3 },
	{ "kriege ich ein besseres zimmer", 0x30e8a, 0, 2 },
	{ "kriege ich eine bessere kabine", 0x30e8a, 0, 3 },
	{ "kriege ich eine bessere kabine", 0x30e8a, 0, 2 },
	{ "komme ich an eine schoenere kabine", 0x30e8a, 0, 3 },
	{ "komme ich an eine schoenere kabine", 0x30e8a, 0, 2 },
	{ "kriege ich eine schoenere kabine", 0x30e8a, 0, 3 },
	{ "kriege ich eine schoenere kabine", 0x30e8a, 0, 2 },
	{ "bekomme ich eine schoenere kabine", 0x30e8a, 0, 3 },
	{ "bekomme ich eine schoenere kabine", 0x30e8a, 0, 2 },
	{ "komme ich an eine bessere kabine", 0x30e8a, 0, 3 },
	{ "komme ich an eine bessere kabine", 0x30e8a, 0, 2 },
	{ "ich verstehe", 0x30d75, 0x6d, 0 },
	{ "das wusste ich", 0x30d75, 0x6d, 0 },
	{ "wusste ich es doch", 0x30d75, 0x6d, 0 },
	{ "ich weiss", 0x30d75, 0x6d, 0 },
	{ "ich kenne", 0x30d75, 0x6d, 0 },
	{ "hilfst du nicht", 0x30d6f, 0x6d, 0 },
	{ "du hilfst nicht", 0x30d6f, 0x6d, 0 },
	{ "nicht hilfsbereit", 0x30d6f, 0x6d, 0 },
	{ "nicht freundlich", 0x30d6f, 0x6d, 0 },
	{ "nicht zuvorkommend", 0x30d6f, 0x6d, 0 },
	{ "hilfst nicht", 0x30d6f, 0x6d, 0 },
	{ "keine hilfe", 0x30d6f, 0x6d, 0 },
	{ "hilfst du mir nicht", 0x30d6f, 0x6d, 0 },
	{ "entschuldig", 0x30d76, 0x6d, 0 },
	{ "verzeihung", 0x30d76, 0x6d, 0 },
	{ "tut mir leid", 0x30d76, 0x6d, 0 },
	{ "tut mir aber leid", 0x30d76, 0x6d, 0 },
	{ "ich meinte es nicht so", 0x30d76, 0x6d, 0 },
	{ "wollte ich nicht sagen", 0x30d76, 0x6d, 0 },
	{ "nicht so gemeint", 0x30d76, 0x6d, 0 },
	{ "nichts fuer ungut", 0x30d76, 0x6d, 0 },
	{ "spielst du golf", 0x313b6, 0, 0 },
	{ "golf spielen", 0x313b6, 0, 0 },
	{ "spiele golf", 0x313b6, 0, 0 },
	{ "sosse klebt am huhn fest", 0x3156b, 0, 0 },
	{ "die sosse nicht von dem huehnchen los", 0x3156b, 0, 0 },
	{ nullptr, 0, 0, 0 }
};

struct FrameRange {
	int _startFrame;
	int _endFrame;
};

static const FrameRange BARBOT_FRAME_RANGES[60] = {
	{ 558, 585 }, { 659, 692 }, { 802, 816 }, { 1941, 1977 }, { 1901, 1941 },
	{ 810, 816 }, { 857, 865}, { 842, 857 }, { 821, 842 }, { 682, 692 },
	{ 1977, 2018 }, { 2140, 2170 }, { 2101, 2139 }, { 2018, 2099}, { 1902, 2015 },
	{ 1811, 1901 }, { 1751, 1810 }, { 1703, 1750 }, { 1681, 1702 }, { 1642, 1702 },
	{ 1571, 1641 }, { 1499, 1570 }, { 1403, 1463 }, { 1464, 1499 }, { 1288, 1295 },
	{ 1266, 1287 }, { 1245, 1265 }, { 1208, 1244 }, { 1171, 1207 }, { 1120, 1170 },
	{ 1092, 1120 }, { 1092, 1092 }, { 1044, 1091 }, { 1011, 1043 }, { 1001, 1010 },
	{ 985, 1001 }, { 927, 984 }, { 912, 926 }, { 898, 906 }, { 802, 896 },
	{ 865, 896 }, { 842, 865 }, { 816, 842 }, { 802, 842 }, { 740, 801 },
	{ 692, 740 }, { 610, 692 }, { 558, 610 }, { 500, 558 }, { 467, 500 },
	{ 421, 466 }, { 349, 420 }, { 306, 348 }, { 305, 306 }, { 281, 305 },
	{ 202, 281 }, { 182, 202 }, { 165, 182 }, { 96, 165 }, { 0, 95 }
};

static const char *const MISSIVEOMAT_MESSAGES[3] = {
	"Welcome, Leovinus.\n"
	"\n"
	"This is your Missive-O-Mat.\n"
	"\n"
	"You have received 1827 Electric Missives.\n"
	"\n"
	"For your convenience I have deleted:\n"
	" 453 things that people you don't know thought it would be "
	"terribly witty to forward to you,\n"
	" 63 Missives containing double or triple exclamation marks,\n"
	" 846 Missives from mailing-lists you once thought might be quite "
	"interesting and now can't figure out how to cancel,\n"
	" 962 Chain Missives,\n"
	" 1034 instructions on how to become a millionaire using butter,\n"
	" 3 Yassaccan Death Threats (slightly down on last week which is"
	" pleasing news),\n"
	" and a Missive from your Mother which I have answered reassuringly.\n"
	"\n"
	"I have selected the following Missives for your particular attention. "
	"You will not need to run Fib-Finder to see why.  Something Is Up and I "
	"suspect those two slippery urchins Brobostigon and Scraliontis are behind it.",

	"Hello Droot.  I have evaluated your recent missives.\n"
	"Contents break down as follows:\n"
	"\n"
	"Good news 49%\n"
	"Bad news 48%\n"
	"Indifferent news 4%\n"
	"Petty mailings and Family Missives 5%\n"
	"Special Offers from the Blerontin Sand Society 1% (note - there's"
	" a rather pretty dune for hire on p4)\n"
	"\n"
	"In general terms you Thrive.  You continue to Prosper.   Your shares are"
	" Secure.  Your hair, as always, looks Good.  Carpet 14 needs cleaning.  \n"
	"\n"
	"I am pleased to report there have been no further comments about "
	"foot odor.\n"
	"\n"
	"Recommend urgently you sell all fish paste shares as Market jittery.\n"
	"\n"
	"As your Great Scheme nears completion I have taken the liberty of"
	" replying to all non-urgent Missives and list below only communic"
	"ations with Manager Brobostigon and His Pain in the Ass Loftiness"
	" Leovinus.  \n"
	"\n"
	"Beware - Leovinus grows suspicious.  Don't take your eye off B"
	"robostigon.  \n"
	"\n"
	"Weather for the Launch tomorrow is bright and sunny.  Hazy clouds"
	" will be turned on at eleven.  I suggest the red suit with the st"
	"reamers.\n"
	"\n"
	"All money transfers will be completed through alias accounts by m"
	"oonsup.\n"
	"\n"
	"Eat well.  Your fish levels are down and you may suffer indecisio"
	"n flutters mid-morning.\n"
	"\n"
	"Here are your Missives...",

	"Hello Antar, this is your Missive-o-Mat.\n"
	"Not that you need reminding but today is the Glorious Dawning of "
	"a New Age in Luxury Space Travel.\n"
	"\n"
	"Generally my assessment of your position this morning is that you"
	" are well, albeit not as rich as you would like to be.  I hope yo"
	"ur interesting collaboration with Mr Scraliontis will soon bear f"
	"ruit. \n"
	"\n"
	"I trust your flatulence has eased during the night.  Such a distr"
	"essing condition for a man in your position.\n"
	"\n"
	"Most of your Missives are routine construction matters which I ha"
	"ve dealt with and deleted.  All Missives from Mr Scraliontis and "
	"His Loftiness Leovinus are here."
};

static const char *const MISSIVEOMAT_MESSAGES_DE[3] = {
	"Willkommen, Leovinus.\n"
	"\n"
	"Dies ist Ihr Depesch-O-Mat.\n"
	"\n"
	"Sie haben 1827 Elektrische Depeschen erhalten.\n"
	"\n"
	"Aus praktischen Gr\xFC""nden habe ich 453 Nachrichten von Leuten, "
	"die Sie nicht kennen und die dachten, es w\xE4""re unheimlich geistreich, "
	"sie an Sie weiterzusenden, gel\xF6""scht, darunter 63 Depeschen mit "
	"doppelten oder dreifachen Ausrufezeichen,\n"
	"846 Depeschen von Mailing-Listen, die Sie einmal f\xFC"
	"r sehr interessant hielten, und von denen Sie jetzt keine Ahnung haben, "
	"wie man sie l\xF6""schen kann, \n"
	"962 Kettendepeschen,\n"
	"1034 Anweisungen, wie man durch den Einsatz von Butter zum Million\xE4""r wird,\n"
	"3 Yassakkanische Morddrohungen (diese Zahl ist im Vergleich zur Vorwoche leicht "
	"gesunken, was durchaus erfreulich ist), \n"
	"und eine Depesche von Ihrer Mutter, die ich mit beruhigenden Worten beantwortet habe.\n"
	"\n"
	"Auf folgende Depeschen m\xF6""chte ich Ihre besondere Aufmerksamkeit lenken. "
	"Sie brauchen den Flunker-Finder nicht zu aktivieren um zu sehen, warum. "
	"Irgend etwas ist faul, und ich habe den Verdacht, da\xDF"" die beiden schleimigen "
	"Quallen Brobostigon und Scraliontis wieder dahinter stecken.",

	"Hallo Droot.  Ich habe Ihre letzten Depeschen ausgewertet.\n"
	"Der Inhalt gliedert sich wie folgt:\n"
	"\n"
	"Gute Nachrichten 49%\n"
	"Schlechte Nachrichten 48%\n"
	"Mittelm\xE4\xDF""ige Nachrichten 4%\n"
	"Belanglose Mailings und Familiendepeschen 5%\n"
	"Sonderangebote der Blerontinischen Sand Gesellschaft 1% "
	"(beachtenswert: die ziemlich h\xFC""bsche Miet-D\xFC""ne auf Seite 4)\n"
	"\n"
	"Insgesamt gesehen sind Sie \xE4""u\xDF""erst Erfolgreich.  Die Gesch\xE4""fte "
	"Florieren weiterhin.  Ihre Aktien sind in Sicherheit. Ihr Haar "
	"sieht wie immer Toll aus. Teppich 14 mu\xDF"" gereinigt werden.\n"
	"\n"
	"Es freut mich, berichten zu d\xFC""rfen, da\xDF"" keine weiteren "
	"Kommentare zu Schwei\xDF""f\xFC\xDF""en gemeldet wurden.\n"
	"\n"
	"Empfehle dringend, alle Fischpaste-Aktien zu verkaufen, da Marktschwankungen.\n"
	"\n"
	"Da Ihr Gro\xDF""er Plan beinahe vollendet ist, war ich so frei, alle "
	"nicht dringenden Depeschen zu beantworten und f\xFC""hre hierauffolgend nur "
	"die Korrespondenz mit Manager Brobostigon und Ihrer Durchlauchtigsten Gro\xDF""kotz "
	"Nervens\xE4""ge Leovinus auf.\n"
	"\n"
	"Achtung: Leovinus sch\xF6""pft langsam Verdacht.  Lassen Sie auch Brobostigon "
	"nicht aus den Augen.\n"
	"\n"
	"F\xFC""r den morgigen Stapellauf ist das Wetter heiter und sonnig.  "
	"Um elf Uhr wird leichte Bew\xF6""lkung eingeschaltet. Ich schlage "
	"den roten Anzug mit der Sch\xE4""rpe vor.\n"
	"\n"
	"Alle Geld\xFC""berweisungen werden \xFC""ber Decknamenkonten vor Mondaufgang get\xE4""tigt.\n"
	"\n"
	"Achten Sie auf eine ausgewogene Ern\xE4""hrung.  Ihr Fischstand ist niedrig und Sie k\xF6"
	"nnten am sp\xE4""ten Vormittag unter Entscheidungsschwankungen leiden.\n"
	"\n"
	"Hier sind Ihre Depeschen...",

	"Hallo Antar, dies ist Ihr Depesch-O-Mat.\n"
	"\n"
	"Nicht, da\xDF"" Sie daran noch erinnert werden m\xFC"
	"ssen, aber heute ist die Glorreiche D\xE4""mmerung eines Neuen Zeitalters "
	"in der Luxusraumfahrt. \n"
	"\n"
	"Im Allgemeinen zeigt meine Bewertung Ihrer Verfassung an diesem Morgen, "
	"da\xDF"" Sie wohlauf sind, wenn auch nicht so reich, wie Sie es gerne w\xE4""ren.  "
	"Ich hoffe, die interessante Zusammenarbeit mit Herrn Scraliontis wird bald "
	"Fr\xFC""chte tragen. \n"
	"\n"
	"Ich hoffe, Ihre Bl\xE4""hungen haben in der Nacht etwas nachgelassen.  "
	"Was f\xFC""r ein betr\xFC""bliches Leiden f\xFC""r  "
	"einen Mann in Ihrer Position.\n"
	"\n"
	"Bei den meisten eingegangenen Depeschen handelt es sich um routinem\xE4\xDF"
	"ige Bauangelegenheiten, die ich bearbeitet und dann gel\xF6""scht "
	"habe.  Alle Depeschen von Herrn Scraliontis und Ihrer Durchlauchtigen "
	"Aufgeblasenheit Leovinus folgen."
};

struct BedheadEntry {
	const char *_name1;
	const char *_name2;
	const char *_name3;
	const char *_name4;
	int _startFrame;
	int _endFrame;
};

static const BedheadEntry ON_CLOSED[4] = {
	{ "Closed", "Closed", "Open", "Open", 0, 12 },
	{ "Open", "Any", "Any", "RestingUTV", 0, 4 },
	{ "Closed", "Open", "Any", "RestingV", 0, 6 },
	{ "Closed", "Closed", "Closed", "RestingG", 0, 21 }
};
static const BedheadEntry ON_RESTING_TV[2] = {
	{ "Any", "Closed", "Open", "Open", 6, 12 },
	{ "Any", "Closed", "Closed", "RestingG", 6, 21 }
};
static const BedheadEntry ON_RESTING_UV[2] = {
	{ "Any", "Any", "Open", "Open", 8, 12 },
	{ "Any", "Any", "Closed", "RestingG", 8, 21 }
};
static const BedheadEntry ON_CLOSED_WRONG[2] = {
	{ "Any", "Any", "Closed", "OpenWrong", 42, 56 },
	{ "Any", "Any", "Open", "RestingDWrong", 42, 52 }
};

static const BedheadEntry OFF_OPEN[3] = {
	{ "Closed", "Closed", "Open", "Closed", 27, 41 },
	{ "Any", "Open", "Any", "RestingUV", 27, 29 },
	{ "Open", "Closed", "Any", "RestingTV", 27, 33 }
};
static const BedheadEntry OFF_RESTING_UTV[1] = {
	{ "Any", "Any", "Any", "Closed", 36, 41 }
};
static const BedheadEntry OFF_RESTING_V[1] = {
	{ "Closed", "Any", "Any", "Closed", 32, 41 }
};
static const BedheadEntry OFF_RESTING_G[3] = {
	{ "Closed", "Closed", "Closed", "Closed", 21, 41 },
	{ "Any", "Open", "Closed", "RestingUV", 21, 29 },
	{ "Open", "Closed", "Closed", "RestingTV", 21, 33 }
};
static const BedheadEntry OFF_OPEN_WRONG[1] = {
	{ "Any", "Any", "Any", "ClosedWrong", 56, 70 }
};
static const BedheadEntry OFF_RESTING_D_WRONG[1] = {
	{ "Any", "Any", "Any", "ClosedWrong", 59, 70 }
};

static const char *const STRINGS_EN[156] = {
	"",
	"You are standing outside the Pellerator.",
	"I'm sorry, you cannot enter this pellerator at present as a bot is in the way.",
	"I'm sorry, you cannot enter this pellerator at present as it's frozen shut",
	"The Succ-U-Bus is in Standby, or \"Off\" mode at present.",
	"There is currently nothing to deliver.",
	"There is currently nothing in the tray to send.",
	"The Succ-U-Bus is a Single Entity Delivery Device.",
	"Chickens are allocated on a one-per-customer basis.",
	"Only one piece of chicken per passenger. Thank you.",
	"You have been upgraded to 1st Class status. Enjoy hugely.",
	"You have been upgraded to 2nd Class status. Enjoy.",
	"This room is reserved for the exclusive use of first class passengers."
	" That does not currently include you",
	"No losers.",
	"Passengers of your class are not permitted to enter this area.",
	"Please exit from the other side.",
	"For mysterious and unknowable reasons, this transport is temporarily out of order.",
	"Unfortunately this fan controller has blown a fuse.",
	"In case of emergency hammer requirement, poke with long stick.",
	"This stick is too short to reach the branches.",
	"You are standing outside Elevator %d",
	"I'm sorry, you cannot enter this elevator at present as a bot is in the way.",
	"This elevator is currently in an advanced state of non-functionality.",
	"That light appears to be loose.",
	"Lumi-Glow(tm) Lights.  They glow in the dark!",
	"You already have one.",
	"This glass is totally and utterly unbreakable.",
	"For emergency long stick, smash glass.",
	"This dispenser has suddenly been fitted with unbreakable glass "
	"to prevent unseemly hoarding of sticks.",
	"The Chicken is already quite clean enough, thank you.",
	"Now would be an excellent opportunity to adjust your viewing apparatus.",
	"You cannot take this because the cage is locked shut.",
	"You are already at your chosen destination.",
	"Passengers of your class are not permitted to enter this area.",
	"Sorry, you must be at least 3rd class before you can summon for help.",
	"You have not assigned a room to go to.",
	"Sorry, this elevator does not go below floor 27.",
	"You must select a game to load first.",
	"You must select a game to save first.",
	"Please supply Galactic reference material.",
	"This is the restaurant music system.  It appears to be locked.",
	"You can't pick this up on account of it being stuck to the branch.",
	"You cannot get this, it is frozen to the branch.",
	"Please check in at the reception desk.",
	"This foodstuff is already sufficiently garnished.",
	"Sadly, this dispenser is currently empty.",
	"Please place food source beneath dispenser for sauce delivery.",
	"The Seasonal Adjustment switch is not operational at the present time.",
	"This is your stateroom. It is for sleeping. If you desire "
	"entertainment or relaxation, please visit your local leisure lounge.",
	"The bed will not currently support your weight."
	" We are working on this problem but are unlikely to be able to fix it.",
	"This is not your assigned room. Please do not enjoy.",
	"Sadly, this is out of your reach.",
	"The Succ-U-Bus is a Single Entity Delivery Device.",
	"Sadly, the Grand Canal transport system is closed for the winter.",
	"This area is off limits to passengers.",
	"Go where?",
	"It would be nice if you could take that but you can't.",
	"A bowl of pistachio nuts.",
	"Not a bowl of pistachio nuts.",

	"Sadly, it is not possible to summon the DoorBot from this location.",
	"Sadly, it is not possible to summon the BellBot from this location.",
	"There is no one here to talk to",
	"Talking to ",
	"the DoorBot",
	"the DeskBot",
	"a LiftBot",
	"the Parrot",
	"the BarBot",
	"a ChatterBot",
	"the BellBot",
	"the Maitre d'Bot",
	"a Succ-U-Bus",
	"Unknown",
	"The arm is already holding something.",
	"You can't get this.",
	"That doesn't seem to do anything.",
	"It doesn't seem to want this.",
	"This does not reach.",
	"The chicken is already clean.",
	"Succ-U-Bus auxiliary hose attachment incompatible with sliding glass cover.",
	"This item is incorrectly calibrated.",
	"Only First Class passengers are allowed to use the Gondoliers.",
	"There is currently nothing available for your viewing pleasure on this channel.",
	"Television control",
	"Operate visual entertainment device",
	"Operate the lights",
	"Deploy floral enhancement",
	"Deploy fully recumbent relaxation device",
	"Deploy comfort workstation",
	"Deploy minor horizontally mobile storage compartment",
	"Deploy major semi-recumbent relaxation device",
	"Inflate fully recumbent relaxation device",
	"Deploy personal maintenance hub",
	"Deploy executive horizontal worksurface",
	"Deploy minor semi-recumbent relaxation device",
	"Deploy aqueous cleansing receptacle",
	"Deploy major horizontally mobile storage compartment",
	"Succ-U-Bus delivery system control",
	"Navigation controller",
	"Let Titania figure out where Earth is (skip puzzle)",
	"Summon Elevator",
	"Summon Pellerator",
	"Go to the Bottom of the Well",
	"Go to the Top of the Well",
	"Go to your stateroom",
	"Go to the Bar",
	"Go to the Promenade Deck",
	"Go to the Arboretum",
	"Go to the Music Room",
	"Go to the First Class Restaurant",
	"The Parrot Lobby",
	"The Creators' Chamber",
	"The Bridge",
	"The Bilge Room",
	"The Sculpture Chamber",
	"The Arboretum",
	"The Bottom of the Well",
	"The Promenade Deck",
	"The 1st class restaurant",
	"Titania's Room",
	"The Bar",
	"The Embarkation Lobby",
	"The Music Room",
	"Unknown Room",
	"The Service Elevator",
	"The Super Galactic Leisure Lounge",
	"The Elevator",
	"The Dome",
	"The Pellerator",
	"The Top of the Well",
	"Nowhere you're likely to want to go.",
	"1st class",
	"2nd class",
	"SGT class",
	"no class",
	"Your assigned room: ",
	"A previously assigned room: ",
	"Saved Chevron: ",
	"Current location: ",
	"Elevator %d",
	"Floor %d",
	"Room %d",
	" (shift-click edits)",
	"A hot",
	"A cold",
	"Load the game.",
	"Save the game.",
	"Empty",
	"Quit the game.",
	"Are you sure you want to quit?",
	"Change the volume settings",
	"Master volume",
	"Music volume",
	"Parrot volume",
	"Speech volume"
};

static const char *const STRINGS_DE[202] = {
	"",
	"Sie befinden sich vor dem Pellerator.",
	"Wir bedauern, da ein Bot den Weg versperrt, ist Ihnen der "
		"Zutritt zum Pellerator Ihnen gegenwSrtig verwehrt.",
	"Wir bedauern, Zutritt zu diesem Pellerator ist nicht m\xF6"
		"glich, da die T\xFC" "r zugefroren ist.",
	"Der Sukk-U-Bus befindet sich gegenwSrtig im Standby-oder \"AUS\"-Betrieb.",
	"Zur Zeit gibt es nichts zuzustellen.",
	"Gegenw\xE4rtig befindet sich nichts im Ablagekorb.",
	"Der Sukk-U-Bus ist ein Einzel-St\xFC" "ck-Liefergerst.",
	"Nur ein H\xFChnchen pro Passagier. Wir bedanken uns f\xFC"
		"r Ihr Verst\xE4ndnis.",
	"H\xFChner werden nur in Eine-Einheit-Pro-Person-Rationen zugeteilt.",
	"Sie sind in die Erste Klasse h\xF6hergestuft worden. Genie\xDF"
		"en Sie es in vollen Z\xFCgen.",
	"Sie sind in die Zweite Klasse h\xF6hergestuft worden. Genie\xDF"
		"en Sie es.",
	"Diese Kabine ist ausschlie\xDFlich f\xFCr Erste-Klasse-Passagiere "
		"reserviert worden. Zur Zeit schlie\xDFt das Sie nicht ein.",
	"Bitte keine Versager.",
	"Passagieren Ihrer Klasse ist der Zugang zu diesem Bereich nicht gestattet.",
	"Benutzen Sie bitte den Ausgang auf der anderen Seite.",
	"Aus mysteri\xF6sen, v\xF6llig unbekannten Gr\xFCnden ist dieses "
		"Transportmittel vor\xFC" "bergehend au\xDF" "er Betrieb.",
	"Leider ist diesem Ventilatorschalter eine Sicherung durchgebrannt.",
	"Im Falle eines dringenden Hammerbed\xFCrfnisses, bedienen Sie sich eines Stabs.",
	"Dieser Stab ist zu kurz um die Aste zu erreichen.",
	"Sie befinden sich vor dem Aufzug %d",
	"Wir bedauern, da ein Bot den Weg versperrt, ist Ihnen der Zutritt "
		"zu diesem Aufzug gegenwSrtig verwehrt.",
	"Dieser Aufzug ist gegenwSrtig im fortgeschrittenen Zustand des "
		"Nicht-Funktionierens.",
	"Die Lampe scheint irgendwie los zu sein.",
	"Lumina Leuchten. Sie leuchten im Dunkeln!",
	"Sie haben doch schon eins.",
	"Dieses Glas ist ganz und gar unzerbrechlich.",
	"Im Falle von dringend ben\xF6tigtem Langen Stab, zertr\xFCmmern Sie das Glas.",
	"Dieser Automat ist v\xF6llig unerwartet mit unzerbrechlichem Glas "
		"ausgestattet worden um ungeb\xFC hrlichem Horten von "
		"St\xF6" "cken vorzubeugen.",
	"Das H\xFChnchen ist eigentlich schon sauber genug, danke vielmals.",
	"Jetzt wSre der ideale Zeitpunkt, Ihre Sehhilfe zur Hand zu nehmen.",
	"Dieses Objekt k\xF6nnen Sie nicht mitnehmen, da die T\xFCr fest verschlossen ist.",
	"Sie befinden sich bereits an Ihrem gew\xFCnschten Reiseziel.",
	"Passagieren Ihrer Klasse ist der Zugang zu diesem Bereich nicht gestattet.",
	"Wir bedauern, aber Sie m\xFCssen mindestens Dritte Klasse sein "
		"bevor Sie um Hilfe bitten k\xF6nnen.",
	"Ihnen wurde keine Kabine zugeteilt.",
	"Wir bedauern, aber dieser Aufzug geht nicht tiefer als bis in den 27. Stock.",
	"Sie m\xFCssen zuerst das Spiel selektieren, das Sie laden m\xF6" "chten.",
	"Sie m\xFCssen zuerst das Spiel selektieren, das Sie speichern m\xF6" "chten.",
	"Stellen Sie bitte das Galaktische Referenzmaterial zur Verf\xFCgung.",
	"Dies ist das Musiksystem des Restaurants. Scheinbar ist es verschlossen.",
	" Aufgrund der Tatsache, da\xDF dieses Objekt an einem Ast festhSngt, "
		"k\xF6nnen Sie es nicht entfernen.",
	"Sie k\xF6nnen dieses Objekt nicht entfernen, es ist am Ast festgefroren.",
	"Melden Sie sich bitte an der Rezeption an.",
	"Dieses Nahrungsmittel ist bereits ausreichend garniert.",
	"Leider ist dieser Automat gegenwSrtig leer.",
	"Bei So\xAF" "enbedarf positionieren Sie bitte die Nahrungsquelle direkt "
		"unter den Automaten.",
	"Der Jahreszeitenschalter befindet sich zur Zeit au\xDF" "er Betrieb.",
	"Dies ist Ihre Kabine. Sie dient zum Schlafen. Wenn Sie Unterhaltung "
		"oder Entspannung w\xFCnschen, statten Sie bitte Ihrem "
		"nSchstgelegenen Salon einen Besuch ab.",
	"Das Bett ist Ihrem Gewicht momentan nicht gewachsen. Wir geben "
		"uns gro\xDF" "e M\xFChe, aber es ist unwahrscheinlich, "
		"da\xDF wir das Problem beheben k\xF6nnen.",
	"Dies ist nicht die Ihnen zugeteilte Kabine. Genie\xDF" "en Sie den "
		"Aufenthalt bitte nicht.",
	"Leider ist dies f\xFCr Sie au\xDF" "er Reichweite.",
	"Der Sukk-U-Bus ist ein Einzel-St\xFC" "ck-Lieferger\xE4t.",
	"Leider ist das Gro\xAF" "e-Kanal-Bef\xF6rderungssystem im Winter geschlossen.",
	"Passagieren ist der Zutritt zu diesem Bereich nicht gestattet.",
	"Wohin m\xF6" "chten Sie gehen?",
	"Es wSre zwar ganz nett, wenn Sie das mitnehmen k\xF6nnten, "
		"aber das k\xF6nnen Sie eben nicht.",
	"Eine Schale Pistazien.",
	"Keine Schale Pistazien.",

	"Leider ist es nicht m\xF6""glich den T\xFC""r-Bot von diesem Ort aus herbeizurufen.",
	"Leider ist es nicht m\xF6""glich den Klingel-Bot von diesem Ort aus herbeizurufen.",
	"Es ist niemand hier mit dem du sprechen k\xF6""nntest",
	"Im Gespr\xE4""ch mit ",
	"der T\xFC""r-Bot",
	"der Empfangs-Bot",
	"der Aufzugs-Bot",
	"der Papagei",
	"der Bar-Bot",
	"ein Quassel-Bot",
	"der Klingel-Bot",
	"der Chef-Bot",
	"ein Sukk-U-Bus",
	"Unbekannt",
	"Der Arm h\xE4""lt bereits etwas.",
	"Das kannst du nicht bekommen.",
	"Das scheint nichts zu tun.",
	"Es scheint das nicht zu wollen.",
	"Das erreicht es nicht.",
	"Das H\xFC""hnchen ist bereits sauber.",
	"Sukk-U-Bus Hilfsschlauch-Aufsatz nicht kompatibel mit gl\xE4""serner Schiebeabdeckung.",
	"Der Gegenstand ist falsch kalibriert.",
	"Nur Passagiere erster Klasse ist es erlaubt den Gondoliere zu benutzen.",
	"Zurzeit kommt auf diesem Kanal nichts sehenswertes.",
	"Fernbedienung",
	"Bediene visuelles Unterhaltungssystem",
	"Bediene die Lichter",
	"Setze florale Verbesserung ein",
	"Stelle vollkommend-liegendes Entspannungsger\xE4""t bereit",
	"Stelle Wohlf\xFC""hl-Arbeitsplatz bereit",
	"Stelle kleineres horizontal bewegliches Abstellfach bereit",
	"Stelle gr\xF6""\xDF""eres halbliegendes Entspannungsger\xE4""t bereit",
	"Pumpe vollkommend-liegendes Entspannungsger\xE4""t auf",
	"Stelle pers\xF6""nliche Instandsetzungsstation bereit",
	"Stelle horizontale F\xFC""hrungskrafts-Arbeitsfl\xE4""che bereit",
	"Stelle kleineres halbliegendes Entspannungsger\xE4""t bereit",
	"Stelle mit Wasser reinigendes Beh\xE4""ltnis bereit",
	"Stelle gr\xF6""\xDF""eres horizontal tragbares Abstellfach bereit",
	"Sukk-U-Bus Zustell-Kontrollsystem",
	"Navigations-Kontroller",
	"Lass Titania herausbekommen wo die Erde ist (\xFC""berspringe Puzzle)",
	"Rufe Aufzug",
	"Rufe Pellerator",
	"Gehe zum Grund des Brunnens",
	"Gehe zur Oberfl\xE4""che des Brunnens",
	"Gehe zu deiner Kabine",
	"Gehe zur Bar",
	"Gehe zum Promenadendeck",
	"Gehe zum Baumgarten",
	"Gehe zum Musikzimmer",
	"Gehe zum First-Class-Restaurant",
	"Die Papagei-Lobby",
	"Das Zimmer des Erschaffers",
	"Die Br\xFC""cke",
	"Der Kielraum",
	"Das Skulpturenzimmer",
	"Der Baumgarten",
	"Der Grund des Brunnens",
	"Das Promenadendeck",
	"Das First-Class-Restaurant",
	"Titanias Zimmer",
	"Die Bar",
	"Die Empfangshalle",
	"Das Musikzimmer",
	"Unbekanntes Zimmer",
	"Der G\xFC""teraufzug",
	"Die Supergalaktische-Freizeitlounge",
	"Der Aufzug",
	"Die Kuppel",
	"Der Pellerator",
	"Die Oberfl\xE4""che des Brunnens",
	"Nirgends, wo du hingehen m\xF6""chtest.",
	"Erste Klasse",
	"Zweite Klasse",
	"SGT Klasse",
	"keine Klasse",
	"Dein zugewiesenes Zimmer: ",
	"Dein zuvor zugewiesenes Zimmer: ",
	"Gespeichertes Abzeichen: ",
	"Derzeitige Position: ",
	"Aufzug %d",
	"Stock %d",
	"Kabine %d",
	" (Chevrons ver\xE4" "ndern mit Umschalt-Taste+Klicken)",
	"Eine hei\xDF""e",
	"Eine kalte",
	"Laden Sie das Spiel.",
	"Speichern Sie das Spiel.",
	"Leer",
	"Beenden Sie das Spiel.",
	"Sind Sie sicher, da\xDF"" Sie das Spiel verlassen m\xF6""chten?",
	"\xC4""ndern der Lautst\xE4""rkeeinstellungen",
	"Grundlautst\xE4""rke",
	"Musiklautst\xE4""rke",
	"Papageienlautst\xE4""rke",
	"Sprachlautst\xE4""rke",

	"Sommer",
	"Herbst",
	"Winter",
	"Fr\xFC" "nhling",
	"Sn'ood",
	"J'af'ah",
	"Bitta",
	"Fr\xAA" "ic",
	"Pflanzen bitte nicht ber\xFC" "nhren.",
	"!\xBC" "ta'\xAD" "ta! !T\xAA" "z n\xAA" " sappibundli t\xAA"
		"cn\xAA" "z!",

	"Stop",
	"!Hanaz!",
	"VorwSrts",
	"!Panaz!",
	"T\xAA" "z k'b\xAA" "z",
	"Ein wenig herumkurven",
	"Otundo a\x92" " doom\xAA" "n n\x92" "sanza",
	"Sinnlose Drehung des Steuerrads",
	"!0xBC" "ta\x92\xAD" "ta!  T\xAA""z vidsta\x92" "jaha i\xAC"
		"in\x92" "qu\xAA" " m\xAA" "n\xAA" "z",
	"Sternenpanorama des Reiseziels hier einf\xFC" "ngen.",

	"V'lo\xAC",
	"Geschwindigkeit",
	"Pan",
	"Ein",
	"Han",
	"Aus",
	"Turgo",
	"Langsam",
	"Pido",
	"Schnell",

	"\xBC" "lu\xAD" " q\xB0 scu'b\xAA" "rri",
	"H\xFC" "hnchen a la sauce tomate",
	"\xBC" "lu\xAD" " q\xB0 scu'jajaja",
	"H0xFC" "hnchen a la sauce moutarde",
	"\xBC" "lu0xAD q\xB0 scu'\xAD" "lu\xAD",
	"H\xFC" "hnchen a la sauce 'Vogel'",
	"\xBC" "lu\xAD" " sanza scu, n\xAA n\xAA n\xAA",
	"H\xFC" "hnchen bar jeglicher sauce",

	"!\xB2" "la! !\xB2" "la!  !!!Sizzlo ab\x92\xAA\xAA" "o s\xAA"
		"nza cr\xAA" "dibo!!!  N\xAA" "nto p\xAA" "rificio i\xAC" "ind\xAA",
	"Achtung, Lebensgefahr. Unglaublich hohe Voltzahl!!! Innen keine "
		"artungsbed\xFC" "rftigen Teile vorhanden.",
	"!!!Birin\xAC" "i sp\xAA" "culato t\xAA" "z n\xAA n\xAA n\xAA"
		" ouvraditiniz!  J\x92" "in n\xAA n\xAA upraximus stifibilimus"
		" j\x92" "in sigorto funct",
	"Sie hStten die erste Kontrollt\xFC" "r nicht \xF6"
		"ffnen sollen! Dies ist nicht nur ungeheuer gef\xE4"
		"hrlich, Sie verlieren auch jegliche Garantie-Anspr\xFC" "che.",
	"!T\xAA" "z n\xAA bleabaz t\xAA" "z n\xAA j\x92" "abaz!  Coco?",
	"Und sagen Sie hinterher blo\xFC nicht, niemand hStte Sie gewarnt.",
	"Pin\xAA" "z-pin\xAA" "z stot \xAF" "r\xB0 jibbli",
	"Dr\xFC" "cken Sie den Knopf um die Bombe zu entschSrfen."
};

static const char *const MUSIC_DATA[4] = {
	"64,^|^|^|^|^|^|^|^|^|^|^|^|^|^|^|^|8,^^^^ 5:A///|64,/|/|/|/|/|/|/"
	"|/|/|/|/|/|/|/|/|/|^|^|^|^|^|^|^|^|^|16, ^B//|64,/|/|/|/|^|16,^C/"
	"/|64,/|/|/|/|",
	"2:8,^^^^B//a|//g//B//|g///B//a|//g//A//|B//^C//b|//a//a//|BCb/b//"
	"a|//g//A//|g/+f/D//c|//b//gA/|g/^^C//C|//C//a//|BCb////a|//g//g//"
	"|g/g//B/a|/g//////|//^^B//a|//g//B//|g///B//a|//g//B//|g//^C//b|/"
	"/a//a//|BCb/b//a|//g//B//|g/+f/D//c|//b//gA/|g/^^C//C|//C//a//|BC"
	"b////a|//g//g//|g/g//B/a|/g//////|3:^^B//a//|g//A//g/|/^B//a//|g/"
	"/A//B/|b^ 3:C//b//|a//g//+f/|+fG/G/GA/|B/a/g///|B///+f//G|G/G/+f/"
	"G/|^^e//d//|c//b//gA|g/B//a//|g//g//g/|g//B/a/g|//////^^|^^Ga///G"
	"|////////|////////|////////|",
	"2:8,^^^^^^D/|/E//E//E|/d//^^d/|/E//E//E|/E//^^G/|/d//d//d|/^^^^^d"
	"/|/E//E//E|/d/^^^E/|/E//d/+F/|bD^^^^G/|/e//e//e|^^^^^^d/|/E//E//E"
	"|//d///d/|//b/////|^^^^^^D/|/E//E//E|/d//^^d/|/E//E//E|/E//^^G/|/"
	"d//d//d|/^^^^^d/|/E//E//E|/d/^^^E/|/E//d/d/|d/^^^^G/|/e//e//e|^^^"
	"^^^d/|/E//E//E|//d///d/|//b/////|3:D///c//b|//b//b//|D///c//b|//b"
	"//g//|E///d//c|//b//a//|aB/B/BC/|D/c/b///|^^^D//DE|/E/E/d/d|//E/g"
	"//g|//g//d//|^^^^g//E|//E//E//|d///d///|b///////|// 3:Db///C|///b"
	"/// 5:A|64,/|/|/|/|/|/|/|/|",
	"2:8,^^G//+f//|e//e//e/|//G//+f//|e//e//+F/|G/a//g//|+f//+f//+f/|/"
	"/G//+F//|e//e//e/|//B//a//|g//e///d|//c//b//|a//a//a/|+f/G// 2:+F"
	"//|e//e//C/|//b/g/+f/|//G/////|^^G//+f//|e//e//e/|//G//+f//|e//e/"
	"/e/|//a//g//|+f//+f//+f/|//G//+F//|e//e//e/|//B//a//|g//e///d|/  "
	"2:dC//b//|a//a//a/|+f/G//+F//|e//e//C/|//b/g/+f/|//G/////|d//d//d"
	"/|/E//E//d|d//d//E/|/+F//G//b|a//a//a/|/D//D// 3:D|//g/g//D|/d/G/"
	"///|^^b//b//|b//ba/B/|c//B//a/|/g//+f//+f|G//+F//e/|/c//C///|b/g/"
	"+f///|G///////|G///////|C///////|////////|////////|"
};

void NORETURN_PRE error(const char *s, ...) {
	printf("%s\n", s);
	exit(1);
}

void writeEntryHeader(const char *name, uint offset, uint size, uint flags) {
	assert(headerOffset < HEADER_SIZE);
	outputFile.seek(headerOffset);
	outputFile.writeLong(offset);
	outputFile.writeLong(size);
	outputFile.writeWord(flags);
	outputFile.writeString(name);

	headerOffset += 10 + strlen(name) + 1;
}

void writeEntryHeader(const char *name, uint offset, uint size) {
	writeEntryHeader(name, offset, size, 0);
}

void writeEntryHeader(const char *name, uint offset, uint size, bool isCompressed) {
	uint flags = isCompressed ? 1 : 0;
	writeEntryHeader(name, offset, size, flags);
}

void writeFinalEntryHeader() {
	assert(headerOffset <= (HEADER_SIZE - 8));
	outputFile.seek(headerOffset);
	outputFile.writeLong(0);
	outputFile.writeLong(0);
}

void writeCompressedRes(Common::File *src) {

}

void writeStringArray(const char *name, uint offset, int count) {
	outputFile.seek(dataOffset);

	inputFile.seek(offset - FILE_DIFF[_version]);
	uint *offsets = new uint[count];
	for (int idx = 0; idx < count; ++idx)
		offsets[idx] = inputFile.readLong();

	// Iterate through reading each string
	for (int idx = 0; idx < count; ++idx) {
		if (offsets[idx]) {
			inputFile.seek(offsets[idx] - FILE_DIFF[_version]);
			outputFile.writeString(inputFile);
		} else {
			outputFile.writeString("");
		}
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;

	delete[] offsets;
}

void writeStringArray(const char *name, const char *const *strings, int count) {
	outputFile.seek(dataOffset);

	// Iterate through writing each string
	for (int idx = 0; idx < count; ++idx) {
		outputFile.writeString(strings[idx]);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

Common::WinResourceID getResId(uint id) {
	return Common::WinResourceID(id);
}

Common::WinResourceID getResId(const char *id) {
	if (!strcmp(id, "Bitmap"))
		return Common::WinResourceID(2);

	return Common::WinResourceID(id);
}

void writeResource(const char *name, Common::File *file) {
	outputFile.seek(dataOffset);
	outputFile.write(*file, file->size());

	writeEntryHeader(name, dataOffset, file->size());
	dataOffset += file->size();
	delete file;
}

void writeResource(const char *resName, const char *sectionStr, uint32 resId, bool isEnglish = true) {
	Common::PEResources &res = isEnglish ? resEng : resGer;
	Common::File *file = res.getResource(getResId(sectionStr), resId);
	assert(file);
	writeResource(resName, file);
}

void writeResource(const char *sectionStr, uint32 resId, bool isEnglish = true) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%u", sectionStr, resId);
	if (!isEnglish)
		strcat(nameBuffer, "/DE");

	writeResource(nameBuffer, sectionStr, resId, isEnglish);
}

void writeResource(const char *resName, const char *sectionStr, const char *resId, bool isEnglish = true) {
	Common::PEResources &res = isEnglish ? resEng : resGer;
	Common::File *file = res.getResource(getResId(sectionStr),
		Common::WinResourceID(resId));
	assert(file);
	writeResource(resName, file);
}

void writeResource(const char *sectionStr, const char *resId, bool isEnglish = true) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%s", sectionStr, resId);
	if (!isEnglish)
		strcat(nameBuffer, "/DE");

	writeResource(nameBuffer, sectionStr, resId, isEnglish);
}

void writeBitmap(const char *name, Common::File *file) {
	outputFile.seek(dataOffset);

	// Set up a memory stream for the compressed data, and wrap
	// it with a zlib compressor
	Common::MemoryWriteStreamDynamic *compressedData =
		new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
	Common::WriteStream *zlib = Common::wrapCompressedWriteStream(compressedData);

	// Write out the necessary bitmap header so that the ScummVM
	// BMP decoder can properly handle decoding the bitmaps
	zlib->write("BM", 2);
	zlib->writeUint32LE(file->size() + 14);	// Filesize
	zlib->writeUint32LE(0);					// res1 & res2
	zlib->writeUint32LE(0x436);				// image offset

	// Transfer the bitmap data
	int srcSize = file->size();
	byte *data = new byte[srcSize];
	file->read(data, srcSize);
	zlib->write(data, srcSize);

	delete[] data;
	zlib->finalize();

	// Write out the compressed data
	outputFile.write(compressedData->getData(), compressedData->size());

	writeEntryHeader(name, dataOffset, compressedData->size() + 14, true);
	dataOffset += compressedData->size() + 14;

	// Free the zlib write stream
	delete zlib;
}

void writeBitmap(const char *sectionStr, const char *resId, bool isEnglish = true) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%s%s", sectionStr, resId,
		isEnglish ? "" : "/DE");

	Common::PEResources &res = isEnglish ? resEng : resGer;
	Common::File *file = res.getResource(getResId(sectionStr),
		Common::WinResourceID(resId));
	assert(file);
	writeBitmap(nameBuffer, file);
}

void writeBitmap(const char *sectionStr, uint32 resId, bool isEnglish = true) {
	char nameBuffer[256];
	sprintf(nameBuffer, "%s/%u%s", sectionStr, resId,
		isEnglish ? "" : "/DE");

	Common::PEResources &res = isEnglish ? resEng : resGer;
	Common::File *file = res.getResource(getResId(sectionStr),
		Common::WinResourceID(resId));
	assert(file);
	writeBitmap(nameBuffer, file);
}

void writeNumbers() {
	outputFile.seek(dataOffset);

	// Iterate through writing each string
	for (int idx = 0; idx < 76; ++idx) {
		outputFile.writeString(NUMBERS[idx]._text);
		outputFile.writeLong(NUMBERS[idx]._value);
		outputFile.writeLong(NUMBERS[idx]._flags);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("TEXT/NUMBERS", dataOffset, size);
	dataOffset += size;
}

void writeString(uint offset) {
	if (offset == 0) {
		outputFile.writeByte(0);
	} else {
		inputFile.seek(offset - FILE_DIFF[_version]);
		char c;
		do {
			c = inputFile.readByte();
			outputFile.writeByte(c);
		} while (c);
	}
}

void writeResponseTree() {
	outputFile.seek(dataOffset);

	const uint OFFSETS[3] = { 0x619520, 0x618340, 0x617380 };
	for (int idx = 0; idx < 1022; ++idx) {
		inputFile.seek(OFFSETS[_version] - FILE_DIFF[_version] + idx * 8);
		uint id = inputFile.readLong();
		uint offset = inputFile.readLong();

		outputFile.writeLong(id);
		if (!id) {
			// An end of list id
		} else if (offset >= OFFSETS[_version] && offset <= (OFFSETS[_version] + 0x1FF0)) {
			// Offset to another table
			outputFile.writeByte(0);
			outputFile.writeLong((offset - OFFSETS[_version]) / 8);
		} else {
			// Offset to ASCIIZ string
			outputFile.writeByte(1);
			writeString(offset);
		}
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("TEXT/TREE", dataOffset, size);
	dataOffset += size;
}

void writeSentenceEntries(const char *name, uint tableOffset) {
	outputFile.seek(dataOffset);

	uint v1, category, v4, v9, v11, v12, v13;
	uint offset3, offset5, offset6, offset7, offset8, offset10;

	for (uint idx = 0; ; ++idx) {
		inputFile.seek(tableOffset - FILE_DIFF[_version] + idx * 0x34);
		v1 = inputFile.readLong();
		if (!v1)
			// Reached end of list
			break;

		// Read data fields
		category = inputFile.readLong();
		offset3 = inputFile.readLong();
		v4 = inputFile.readLong();
		offset5 = inputFile.readLong();
		offset6 = inputFile.readLong();
		offset7 = inputFile.readLong();
		offset8 = inputFile.readLong();
		v9 = inputFile.readLong();
		offset10 = inputFile.readLong();
		v11 = inputFile.readLong();
		v12 = inputFile.readLong();
		v13 = inputFile.readLong();

		outputFile.writeLong(v1);
		outputFile.writeLong(category);
		writeString(offset3);
		outputFile.writeLong(v4);
		writeString(offset5);
		writeString(offset6);
		writeString(offset7);
		writeString(offset8);
		outputFile.writeLong(v9);
		writeString(offset10);
		outputFile.writeLong(v11);
		outputFile.writeLong(v12);
		outputFile.writeLong(v13);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeWords(const char *name, uint tableOffset, int recordCount = 2) {
	outputFile.seek(dataOffset);
	int recordSize = recordCount * 4;

	uint val, strOffset;
	for (uint idx = 0; ; ++idx) {
		inputFile.seek(tableOffset - FILE_DIFF[_version] + idx * recordSize);
		val = inputFile.readLong();
		strOffset = inputFile.readLong();

		if (!val) {
			// Reached end of list
			assert(idx >= 128);
			break;
		}

		outputFile.writeLong(val);
		writeString(strOffset);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeSentenceMappings(const char *name, uint offset, int numValues) {
	inputFile.seek(offset - FILE_DIFF[_version]);
	outputFile.seek(dataOffset);

	uint id;
	while ((id = inputFile.readLong()) != 0) {
		outputFile.writeLong(id);

		for (int ctr = 0; ctr < numValues; ++ctr)
			outputFile.writeLong(inputFile.readLong());
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader(name, dataOffset, size);
	dataOffset += size;
}

void writeStarfieldPoints() {
	outputFile.seek(dataOffset);

	const int OFFSETS[3] = { 0x59DE4C, 0x59DBEC, 0x59CC1C };
	inputFile.seek(OFFSETS[_version] - FILE_DIFF[_version]);
	uint size = 876 * 12;

	outputFile.write(inputFile, size);
	writeEntryHeader("STARFIELD/POINTS", dataOffset, size);
	dataOffset += size;
}

void writeStarfieldPoints2() {
	outputFile.seek(dataOffset);

	const int OFFSETS[3] = { 0x5A2F28, 0x5A2CC8, 0x5A1CF8 };
	for (int rootCtr = 0; rootCtr < 80; ++rootCtr) {
		inputFile.seek(OFFSETS[_version] - FILE_DIFF[_version] + rootCtr * 8);
		uint offset = inputFile.readUint32LE();
		uint count = inputFile.readUint32LE();

		outputFile.writeLong(count);
		inputFile.seek(offset - FILE_DIFF[_version]);
		outputFile.write(inputFile, count * 4 * 4);
	}

	uint size = outputFile.size() - dataOffset;
	outputFile.write(inputFile, size);
	writeEntryHeader("STARFIELD/POINTS2", dataOffset, size);
	dataOffset += size;
}

void writePhrases(const char *name, const CommonPhrase *phrases) {
	outputFile.seek(dataOffset);

	for (uint idx = 0; phrases->_str; ++idx, ++phrases) {
		outputFile.writeString(phrases->_str);
		outputFile.writeLong(phrases->_dialogueId);
		outputFile.writeLong(phrases->_roomNum);
		outputFile.writeLong(phrases->_val1);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("Phrases/Bellbot", dataOffset, size);
	dataOffset += size;
}

void writeBarbotFrameRanges() {
	outputFile.seek(dataOffset);

	for (int idx = 0; idx < 60; ++idx) {
		outputFile.writeLong(BARBOT_FRAME_RANGES[idx]._startFrame);
		outputFile.writeLong(BARBOT_FRAME_RANGES[idx]._endFrame);
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("FRAMES/BARBOT", dataOffset, size);
	dataOffset += size;
}

void writeMissiveOMatMessages() {
	outputFile.seek(dataOffset);

	for (int idx = 0; idx < 3; ++idx)
		outputFile.writeString(MISSIVEOMAT_MESSAGES[idx]);

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("TEXT/MISSIVEOMAT/WELCOME", dataOffset, size);
	dataOffset += size;

	static const int MESSAGES[3] = { 0x5A63C0, 0x5A5BA8, 0x5A4A18 };
	writeStringArray("TEXT/MISSIVEOMAT/MESSAGES", MESSAGES[_version], 58);
	static const int FROM[3] = { 0x5A61F0, 0x5A59D8, 0x5A4BE8 };
	writeStringArray("TEXT/MISSIVEOMAT/FROM", FROM[_version], 58);
	static const int TO[3] = { 0x5A62D8, 0x5A5AC0, 0x5A4B00 };
	writeStringArray("TEXT/MISSIVEOMAT/TO", TO[_version], 58);
}

void writeMissiveOMatMessagesDE() {
	outputFile.seek(dataOffset);

	for (int idx = 0; idx < 3; ++idx)
		outputFile.writeString(MISSIVEOMAT_MESSAGES_DE[idx]);

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("TEXT/MISSIVEOMAT/WELCOME/DE", dataOffset, size);
	dataOffset += size;

	writeStringArray("TEXT/MISSIVEOMAT/MESSAGES/DE", 0x5A9988, 58);
	writeStringArray("TEXT/MISSIVEOMAT/FROM/DE", 0x5A97B8, 58);
	writeStringArray("TEXT/MISSIVEOMAT/TO/DE", 0x5A98A0, 58);
}

void writeBedheadGroup(const BedheadEntry *data, int count) {
	for (int idx = 0; idx < count; ++idx, ++data) {
		outputFile.writeString(data->_name1);
		outputFile.writeString(data->_name2);
		outputFile.writeString(data->_name3);
		outputFile.writeString(data->_name4);
		outputFile.writeLong(data->_startFrame);
		outputFile.writeLong(data->_endFrame);
	}
}

void writeBedheadData() {
	outputFile.seek(dataOffset);

	writeBedheadGroup(ON_CLOSED, 4);
	writeBedheadGroup(ON_RESTING_TV, 2);
	writeBedheadGroup(ON_RESTING_UV, 2);
	writeBedheadGroup(ON_CLOSED_WRONG, 2);
	writeBedheadGroup(OFF_OPEN, 3);
	writeBedheadGroup(OFF_RESTING_UTV, 1);
	writeBedheadGroup(OFF_RESTING_V, 1);
	writeBedheadGroup(OFF_RESTING_G, 3);
	writeBedheadGroup(OFF_OPEN_WRONG, 1);
	writeBedheadGroup(OFF_RESTING_D_WRONG, 1);

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("DATA/BEDHEAD", dataOffset, size);
	dataOffset += size;
}

void writeParrotLobbyLinkUpdaterEntries() {
	static const int OFFSETS[3] = { 0x5A5B38, 0x5A5320, 0x5A4360 };
	static const int COUNTS[5] = { 7, 5, 6, 9, 1 };
	static const int SKIP[5] = { 36, 36, 40, 36, 0 };
	uint recordOffset = OFFSETS[_version], linkOffset;
	byte vals[8];

	outputFile.seek(dataOffset);

	for (int groupNum = 0; groupNum < 4; ++groupNum) {
		for (int entryNum = 0; entryNum < COUNTS[groupNum];
				++entryNum, recordOffset += 36) {
			inputFile.seek(recordOffset - FILE_DIFF[_version]);
			linkOffset = inputFile.readUint32LE();
			for (int idx = 0; idx < 8; ++idx)
				vals[idx] = inputFile.readUint32LE();

			// Write out the entry
			inputFile.seek(linkOffset - FILE_DIFF[_version]);
			outputFile.writeString(inputFile);
			outputFile.write(vals, 8);
		}

		// Skip space between groups
		recordOffset += SKIP[groupNum];
	}

	uint size = outputFile.size() - dataOffset;
	writeEntryHeader("DATA/PARROT_LOBBY_LINK_UPDATOR", dataOffset, size);
	dataOffset += size;
}

void writeHeader() {
	// Write out magic string
	const char *MAGIC_STR = "SVTN";
	outputFile.write(MAGIC_STR, 4);

	// Write out version number
	outputFile.writeWord(VERSION_NUMBER);
}

void writeData() {
	for (int idx = 0; idx < (resGer.empty() ? 1 : 2); ++idx) {
		bool isEnglish = idx == 0;

		writeBitmap("Bitmap", "BACKDROP", isEnglish);
		writeBitmap("Bitmap", "EVILTWIN", isEnglish);
		writeBitmap("Bitmap", "RESTORED", isEnglish);
		writeBitmap("Bitmap", "RESTOREF", isEnglish);
		writeBitmap("Bitmap", "RESTOREU", isEnglish);
		writeBitmap("Bitmap", "STARTD", isEnglish);
		writeBitmap("Bitmap", "STARTF", isEnglish);
		writeBitmap("Bitmap", "STARTU", isEnglish);
		writeBitmap("Bitmap", "TITANIC", isEnglish);
		writeBitmap("Bitmap", 133, isEnglish);
		writeBitmap("Bitmap", 164, isEnglish);
		writeBitmap("Bitmap", 165, isEnglish);
	}

	writeResource("STFONT", 149);
	writeResource("STFONT", 151);
	writeResource("STFONT", 152);
	writeResource("STFONT", 153);

	writeResource("TEXT/STVOCAB", "TEXT", "STVOCAB.TXT");
	writeResource("TEXT/JRQUOTES", "TEXT", "JRQUOTES.TXT");
	writeResource("TEXT", 155);

	writeResource("STARFIELD", 132);
	writeStarfieldPoints();
	writeStarfieldPoints2();

	writeStringArray("TEXT/ITEM_DESCRIPTIONS", ITEM_DESCRIPTIONS, 46);
	writeStringArray("TEXT/ITEM_DESCRIPTIONS/DE", ITEM_DESCRIPTIONS_DE, 46);
	writeStringArray("TEXT/ITEM_NAMES", ITEM_NAMES, 46);
	writeStringArray("TEXT/ITEM_IDS", ITEM_IDS, 40);
	writeStringArray("TEXT/ROOM_NAMES", ROOM_NAMES, 34);
	writeStringArray("TEXT/STRINGS", STRINGS_EN, 156);
	writeStringArray("TEXT/STRINGS/DE", STRINGS_DE, 202);
	const int TEXT_PHRASES[3] = { 0x61D3C8, 0x618340, 0x61B1E0 };
	const int TEXT_REPLACEMENTS1[3] = { 0x61D9B0, 0x61C788, 0x61B7C8 };
	const int TEXT_REPLACEMENTS2[3] = { 0x61DD20, 0x61CAF8, 0x61BB38 };
	const int TEXT_REPLACEMENTS3[3] = { 0x61F5C8, 0x61E3A0, 0x61D3E0 };
	const int TEXT_PRONOUNS[3] = { 0x631318, 0x6300F8, 0x62F138 };
	writeStringArray("TEXT/PHRASES", TEXT_PHRASES[_version], 376);
	writeStringArray("TEXT/REPLACEMENTS1", TEXT_REPLACEMENTS1[_version], 218);
	writeStringArray("TEXT/REPLACEMENTS2", TEXT_REPLACEMENTS2[_version], 1576);
	writeStringArray("TEXT/REPLACEMENTS3", TEXT_REPLACEMENTS3[_version], 82);
	writeStringArray("TEXT/PRONOUNS", TEXT_PRONOUNS[_version], 15);
	writeStringArray("MUSIC/PARSER", MUSIC_DATA, 4);

	const int SENTENCES_DEFAULT[3] = { 0x5C0130, 0x5BEFC8, 0x5BE008 };
	const int SENTENCES_BARBOT[2][3] = {
		{ 0x5ABE60, 0x5AACF8, 0x5A9D38 }, { 0x5BD4E8, 0x5BC380, 0x5BB3C0 }
	};
	const int SENTENCES_BELLBOT[20][3] = {
		{ 0x5C2230, 0x5C10C8, 0X5C0108 }, { 0x5D1670, 0x5D0508, 0x5CF548 },
		{ 0x5D1A80, 0x5D0918, 0x5CF958 }, { 0x5D1AE8, 0x5D0980, 0x5CF9C0 },
		{ 0x5D1B88, 0x5D0A20, 0x5CFA60 }, { 0x5D2A60, 0x5D18F8, 0x5D0938 },
		{ 0x5D2CD0, 0x5D1B68, 0x5D0BA8 }, { 0x5D3488, 0x5D2320, 0x5D1360 },
		{ 0x5D3900, 0x5D2798, 0x5D17D8 }, { 0x5D3968, 0x5D2800, 0x5D1840 },
		{ 0x5D4668, 0x5D3500, 0x5D2540 }, { 0x5D47A0, 0x5D3638, 0x5D2678 },
		{ 0x5D4EC0, 0x5D3D58, 0x5D2D98 }, { 0x5D5100, 0x5D3F98, 0x5D2FD8 },
		{ 0x5D5370, 0x5D4208, 0x5D3248 }, { 0x5D5548, 0x5D43E0, 0x5D3420 },
		{ 0x5D56B8, 0x5D4550, 0x5D3590 }, { 0x5D57C0, 0x5D4658, 0x5D3698 },
		{ 0x5D5B38, 0x5D49D0, 0x5D3A10 }, { 0x5D61B8, 0x5D5050, 0x5D4090 }
	};
	writeSentenceEntries("Sentences/Default", SENTENCES_DEFAULT[_version]);
	writeSentenceEntries("Sentences/Barbot", SENTENCES_BARBOT[0][_version]);
	writeSentenceEntries("Sentences/Barbot2", SENTENCES_BARBOT[1][_version]);
	writeSentenceEntries("Sentences/Bellbot", SENTENCES_BELLBOT[0][_version]);
	writeSentenceEntries("Sentences/Bellbot/1", SENTENCES_BELLBOT[1][_version]);
	writeSentenceEntries("Sentences/Bellbot/2", SENTENCES_BELLBOT[2][_version]);
	writeSentenceEntries("Sentences/Bellbot/3", SENTENCES_BELLBOT[3][_version]);
	writeSentenceEntries("Sentences/Bellbot/4", SENTENCES_BELLBOT[4][_version]);
	writeSentenceEntries("Sentences/Bellbot/5", SENTENCES_BELLBOT[5][_version]);
	writeSentenceEntries("Sentences/Bellbot/6", SENTENCES_BELLBOT[6][_version]);
	writeSentenceEntries("Sentences/Bellbot/7", SENTENCES_BELLBOT[7][_version]);
	writeSentenceEntries("Sentences/Bellbot/8", SENTENCES_BELLBOT[8][_version]);
	writeSentenceEntries("Sentences/Bellbot/9", SENTENCES_BELLBOT[9][_version]);
	writeSentenceEntries("Sentences/Bellbot/10", SENTENCES_BELLBOT[10][_version]);
	writeSentenceEntries("Sentences/Bellbot/11", SENTENCES_BELLBOT[11][_version]);
	writeSentenceEntries("Sentences/Bellbot/12", SENTENCES_BELLBOT[12][_version]);
	writeSentenceEntries("Sentences/Bellbot/13", SENTENCES_BELLBOT[13][_version]);
	writeSentenceEntries("Sentences/Bellbot/14", SENTENCES_BELLBOT[14][_version]);
	writeSentenceEntries("Sentences/Bellbot/15", SENTENCES_BELLBOT[15][_version]);
	writeSentenceEntries("Sentences/Bellbot/16", SENTENCES_BELLBOT[16][_version]);
	writeSentenceEntries("Sentences/Bellbot/17", SENTENCES_BELLBOT[17][_version]);
	writeSentenceEntries("Sentences/Bellbot/18", SENTENCES_BELLBOT[18][_version]);
	writeSentenceEntries("Sentences/Bellbot/19", SENTENCES_BELLBOT[19][_version]);

	const int SENTENCES_DESKBOT[3][3] = {
		{ 0x5DCD10, 0x5DBBA8, 0x5DABE8 }, { 0x5E8E18, 0x5E7CB0, 0x5E6CF0 },
		{ 0x5E8BA8, 0x5E7A40, 0x5E6A80 }
	};
	writeSentenceEntries("Sentences/Deskbot", SENTENCES_DESKBOT[0][_version]);
	writeSentenceEntries("Sentences/Deskbot/2", SENTENCES_DESKBOT[1][_version]);
	writeSentenceEntries("Sentences/Deskbot/3", SENTENCES_DESKBOT[2][_version]);

	const int SENTENCES_DOORBOT[12][3] = {
		{ 0x5EC110, 0x5EAFA8, 0x5E9FE8 }, { 0x5FD930, 0x5FC7C8, 0x5FB808 },
		{ 0x5FDD0C, 0x5FCBA4, 0x5FBBE4 }, { 0x5FE668, 0x5FD500, 0x5FC540 },
		{ 0x5FDD40, 0x5FCBD8, 0X5FBC18 }, { 0x5FFF08, 0x5FEDA0, 0x5FDDE0 },
		{ 0x5FE3C0, 0x5FD258, 0x5FC298 }, { 0x5FF0C8, 0x5FDF60, 0x5FCFA0 },
		{ 0x5FF780, 0x5FE618, 0x5FD658 }, { 0x5FFAC0, 0x5FE958, 0x5FD998 },
		{ 0x5FFC30, 0x5FEAC8, 0x5FDB08 }, { 0x6000E0, 0x5FEF78, 0x5FDFB8 }
	};
	writeSentenceEntries("Sentences/Doorbot", SENTENCES_DOORBOT[0][_version]);
	writeSentenceEntries("Sentences/Doorbot/2", SENTENCES_DOORBOT[1][_version]);
	writeSentenceEntries("Sentences/Doorbot/100", SENTENCES_DOORBOT[2][_version]);
	writeSentenceEntries("Sentences/Doorbot/101", SENTENCES_DOORBOT[3][_version]);
	writeSentenceEntries("Sentences/Doorbot/102", SENTENCES_DOORBOT[4][_version]);
	writeSentenceEntries("Sentences/Doorbot/107", SENTENCES_DOORBOT[5][_version]);
	writeSentenceEntries("Sentences/Doorbot/110", SENTENCES_DOORBOT[6][_version]);
	writeSentenceEntries("Sentences/Doorbot/111", SENTENCES_DOORBOT[7][_version]);
	writeSentenceEntries("Sentences/Doorbot/124", SENTENCES_DOORBOT[8][_version]);
	writeSentenceEntries("Sentences/Doorbot/129", SENTENCES_DOORBOT[9][_version]);
	writeSentenceEntries("Sentences/Doorbot/131", SENTENCES_DOORBOT[10][_version]);
	writeSentenceEntries("Sentences/Doorbot/132", SENTENCES_DOORBOT[11][_version]);

	const int SENTENCES_LIFTBOT[3] = { 0x6026B0, 0x601548, 0x600588 };
	const int SENTENCES_MAITRED[2][3] = {
		{ 0x60CFD8, 0x60BE70, 0x60AEB0 }, { 0x614288, 0x613120, 0x612160 }
	};
	const int SENTENCES_PARROT[3] = { 0x615858, 0x6146F0, 0x613730 };
	const int SENTENCES_SUCCUBUS[3] = { 0x616698, 0x615530, 0x614570 };
	const int MAPPINGS_BARBOT[3] = { 0x5B28A0, 0x5B173E, 0x5B0778 };
	const int MAPPINGS_BELLBOT[3] = { 0x5CD830, 0x5CC6C8, 0x5CB708 };
	const int MAPPINGS_DESKBOT[3] = { 0x5E2BB8, 0x5E1A50, 0x5E0A90 };
	const int MAPPINGS_DOORBOT[3] = { 0x5F7950, 0x5F67E8, 0x5F5828 };
	const int MAPPINGS_LIFTBOT[3] = { 0x608660, 0x6074F8, 0x606538 };
	const int MAPPINGS_MAITRED[3] = { 0x6125C8, 0x611460, 0x6104A0 };
	const int MAPPINGS_PARROT[3] = { 0x615B68, 0x614A00, 0x613A40 };
	const int MAPPINGS_SUCCUBUS[3] = { 0x6189F0, 0x617888, 0x6168C8 };
	const int WORDS_BARBOT[3] = { 0x5BE2E0, 0x5BD178, 0x5BC1B8 };
	const int WORDS_BELLBOT[3] = { 0x5D8230, 0x5D70C8, 0x5D6108 };
	const int WORDS_DESKBOT[3] = { 0x5EAAA8, 0x5E9940, 0x5E8980 };
	const int WORDS_DOORBOT[3] = { 0x601098, 0x5FFF30, 0x5FEF70 };
	const int WORDS_LIFTBOT[3] = { 0x60C788, 0x60B620, 0x60A660 };
	writeSentenceEntries("Sentences/Liftbot", SENTENCES_LIFTBOT[_version]);
	writeSentenceEntries("Sentences/MaitreD", SENTENCES_MAITRED[0][_version]);
	writeSentenceEntries("Sentences/MaitreD/1", SENTENCES_MAITRED[1][_version]);
	writeSentenceEntries("Sentences/Parrot", SENTENCES_PARROT[_version]);
	writeSentenceEntries("Sentences/SuccUBus", SENTENCES_SUCCUBUS[_version]);
	writeSentenceMappings("Mappings/Barbot", MAPPINGS_BARBOT[_version], 8);
	writeSentenceMappings("Mappings/Bellbot", MAPPINGS_BELLBOT[_version], 1);
	writeSentenceMappings("Mappings/Deskbot", MAPPINGS_DESKBOT[_version], 4);
	writeSentenceMappings("Mappings/Doorbot", MAPPINGS_DOORBOT[_version], 4);
	writeSentenceMappings("Mappings/Liftbot", MAPPINGS_LIFTBOT[_version], 4);
	writeSentenceMappings("Mappings/MaitreD", MAPPINGS_MAITRED[_version], 1);
	writeSentenceMappings("Mappings/Parrot", MAPPINGS_PARROT[_version], 1);
	writeSentenceMappings("Mappings/SuccUBus", MAPPINGS_SUCCUBUS[_version], 1);
	writeWords("Words/Barbot", WORDS_BARBOT[_version]);
	writeWords("Words/Bellbot", WORDS_BELLBOT[_version], 3);
	writeWords("Words/Deskbot", WORDS_DESKBOT[_version], 3);
	writeWords("Words/Doorbot", WORDS_DOORBOT[_version], 3);
	writeWords("Words/Liftbot", WORDS_LIFTBOT[_version]);
	writePhrases("Phrases/Bellbot", BELLBOT_COMMON_PHRASES_EN);

	writeResponseTree();
	writeNumbers();
	writeAllScriptQuotes();
	writeAllScriptResponses();
	writeAllScriptRanges();

	writeAllTagMappings();
	writeAllUpdateStates();
	writeAllScriptPreResponses();
	writeBarbotFrameRanges();
	writeMissiveOMatMessages();
	writeBedheadData();
	writeParrotLobbyLinkUpdaterEntries();
}

void writeGermanData() {
	writeResource("TEXT/STVOCAB/DE", "TEXT", "STVOCABDE.TXT", false);
	writeResource("TEXT/JRQUOTES/DE", "TEXT", "JRQUOTESDE.TXT", false);
	writeResource("TEXT/155/DE", "TEXT", 155, false);

	writeStringArray("TEXT/PHRASES/DE", 0x23EEC8 + GERMAN_DIFF, 178);
	writeStringArray("TEXT/REPLACEMENTS1/DE", 0x23F198 + GERMAN_DIFF, 1362);
	writeStringArray("TEXT/REPLACEMENTS2/DE", 0x2406E8 + GERMAN_DIFF, 816);
	writeStringArray("TEXT/REPLACEMENTS3/DE", 0x2413B0 + GERMAN_DIFF, 608);
	writeStringArray("TEXT/REPLACEMENTS4/DE", 0x241D38 + GERMAN_DIFF, 195);
	writeStringArray("TEXT/PRONOUNS/DE", 0x248610 + GERMAN_DIFF, 15);

	writeSentenceMappings("Mappings/Barbot/DE", 0x1BA388 + GERMAN_DIFF, 8);
	writeSentenceMappings("Mappings/Bellbot/DE", 0x1E1D20 + GERMAN_DIFF, 1);
	writeSentenceMappings("Mappings/Deskbot/DE", 0x1F5A18 + GERMAN_DIFF, 4);
	writeSentenceMappings("Mappings/Doorbot/DE", 0x214E00 + GERMAN_DIFF, 4);
	writeSentenceMappings("Mappings/Liftbot/DE", 0x224AE0 + GERMAN_DIFF, 4);
	writeSentenceMappings("Mappings/MaitreD/DE", 0x232E30 + GERMAN_DIFF, 1);
	writeSentenceMappings("Mappings/Parrot/DE", 0x235FA8 + GERMAN_DIFF, 1);
	writeSentenceMappings("Mappings/SuccUBus/DE", 0x2399C8 + GERMAN_DIFF, 1);
	writeWords("Words/Barbot/DE", 0x5C68B8);
	writeWords("Words/Bellbot/DE", 0x5E8378, 3);
	writeWords("Words/Deskbot/DE", 0x5FE8D8, 3);
	writeWords("Words/Doorbot/DE", 0x61B398, 3);
	writeWords("Words/Liftbot/DE", 0x629618);
	writePhrases("Phrases/Bellbot/DE", BELLBOT_COMMON_PHRASES_DE);

	const int SENTENCES_BARBOT[2] = { 0x5B00C0, 0x5C5AC8 };
	const int SENTENCES_BELLBOT[20] = { 0x5CACF8, 0x5D1670 };
	const int SENTENCES_DESKBOT[3] = { 0x5ED428, 0x5FCEA0, 0x5FCC30 };
	const int SENTENCES_DOORBOT[4] = { 0x5FFFC8, 0x61A690, 0x61AA38 };

	writeSentenceEntries("Sentences/Default/DE", 0x5C8C70);
	writeSentenceEntries("Sentences/Barbot/DE", SENTENCES_BARBOT[0]);
	writeSentenceEntries("Sentences/Barbot2/DE", SENTENCES_BARBOT[1]);
	writeSentenceEntries("Sentences/Bellbot/DE", SENTENCES_BELLBOT[0]);
	writeSentenceEntries("Sentences/Bellbot/1/DE", SENTENCES_BELLBOT[1]);
	writeSentenceEntries("Sentences/Deskbot/DE", SENTENCES_DESKBOT[0]);
	writeSentenceEntries("Sentences/Deskbot/2/DE", SENTENCES_DESKBOT[1]);
	writeSentenceEntries("Sentences/Deskbot/3/DE", SENTENCES_DESKBOT[2]);
	writeSentenceEntries("Sentences/Doorbot/DE", SENTENCES_DOORBOT[0]);
	writeSentenceEntries("Sentences/Doorbot/1/DE", SENTENCES_DOORBOT[1]);
	writeSentenceEntries("Sentences/Doorbot/2/DE", SENTENCES_DOORBOT[2]);
	writeSentenceEntries("Sentences/Liftbot/DE", 0x61CAD0);
	writeSentenceEntries("Sentences/MaitreD/DE", 0x629EE8);
	writeSentenceEntries("Sentences/Parrot/DE", 0x633FFC);
	writeSentenceEntries("Sentences/SuccUBus/DE", 0x637CD8);

	writeMissiveOMatMessagesDE();

	writeResource("STFONT", 149, false);
	writeResource("STFONT", 151, false);
	writeResource("STFONT", 152, false);
	writeResource("STFONT", 153, false);
}

void createScriptMap() {
	Common::File inFile;
	char line[80];
	char c[2];
	c[0] = c[1] = '\0';
	int counter = 0;

	inFile.open("d:\\temp\\map.txt");
	printf("static const TagMapping xxxx_ID_MAP[] = {\n");

	do {
		strcpy(line, "");

		while (!inFile.eof()) {
			c[0] = inFile.readByte();
			if (c[0] == '\n')
				c[0] = ' ';
			else if (c[0] == '\r')
				continue;
			strcat(line, c);
			if (inFile.eof() || strlen(line) == (2 * 9))
				break;
		}

		uint v1, v2;
		sscanf(line, "%x %x", &v1, &v2);

		if (counter != 0 && (counter % 3) == 0)
			printf("\r\n");
		if ((counter % 3) == 0)
			printf("\t");

		printf("{ 0x%.5x, 0x%.5x }, ", v1, v2);
		++counter;
	} while (!inFile.eof());

	printf("};\r\n");
	inFile.close();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Format: %s ST.exe [ST_german.exe] [titanic.dat]\n", argv[0]);
		exit(0);
	}

	if (!inputFile.open(argv[1])) {
		error("Could not open input file");
	}
	resEng.loadFromEXE(argv[1]);

	if (argc == 4) {
		resGer.loadFromEXE(argv[2]);
	}

	if (inputFile.size() == ENGLISH_10042C_FILESIZE)
		_version = ENGLISH_10042C;
	else if (inputFile.size() == ENGLISH_10042B_FILESIZE)
		_version = ENGLISH_10042B;
	else if (inputFile.size() == ENGLISH_10042_FILESIZE)
		_version = ENGLISH_10042;
	else if (inputFile.size() == GERMAN_10042D_FILESIZE) {
		printf("German version detected. You must use an English versoin "
			"for the primary input file\n");
		exit(0);
	} else {
		printf("Unknown version of ST.exe specified\n");
		exit(0);
	}

	if (!outputFile.open(argc == 4 ? argv[3] : "titanic.dat", Common::kFileWriteMode)) {
		printf("Could not open output file\n");
		exit(0);
	}

	writeHeader();
	writeData();

	if (argc == 4) {
		inputFile.open(argv[2]);
		_version = GERMAN;
		writeGermanData();
	}
	writeFinalEntryHeader();
	inputFile.close();
	outputFile.close();
	return 0;
}
