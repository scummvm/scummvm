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

#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/object.h"
#include "sci/engine/state.h"
#include "sci/engine/vm.h"
#include "sci/engine/script_patches.h"
#include "sci/engine/workarounds.h"

#define SCI_WORKAROUNDENTRY_TERMINATOR { (SciGameId)0, -1, -1, 0, NULL, NULL, NULL, 0, 0, { WORKAROUND_NONE, 0 } }
#define SCI_MESSAGEWORKAROUNDENTRY_TERMINATOR { (SciGameId)0, (SciMedia)0, (kLanguage)0, -1, -1, 0, 0, 0, 0, { MSG_WORKAROUND_NONE, -1, 0, 0, 0, 0, 0, 0, 0, NULL } }

namespace Sci {

// Attention:
//  To identify locally called procedures, code signatures are used.
//   Offsets change a lot between different versions of games, especially between different language releases.
//   That's why it isn't good to hardcode the offsets of those procedures.
//
//  Those signatures are just like the script patcher signatures (for further study: engine\script_patches.cpp)
//   However you may NOT use command SIG_SELECTOR8 nor SIG_SELECTOR16 atm. Proper support for those may be added later.


// Empty signature for matching against any procedure.
//
// Use this when maintaining signatures is unnecessary or difficult.
//
// For example, a local procedure in the Hoyle4 bridge game has an uninit bug.
// This procedure is also in Hoyle5, where is is compiled as SCI32 bytecode
// with debug instructions, and compiled again without debug instructions in
// the Mac version of Hoyle5. This would require three different signatures,
// but there are only two procedures in the script so it doesn't matter; it's
// enough to identify that the bug occurs in a local procedure in the script.
static const uint16 sig_any_procedure[] = {
	SIG_END
};

//                Game: Conquests of Camelot
//      Calling method: endingCartoon2::changeState
//   Subroutine offset: English 0x020d (script 92)
// Applies to at least: English PC floppy
static const uint16 sig_arithmetic_camelot_1[] = {
	0x83, 0x32,                      // lal local[32h]
	0x30, SIG_UINT16(0x001d),        // bnt [...]
	0x7a,                            // push2
	0x39, 0x08,                      // pushi 08
	0x36,                            // push
	0x43,                            // callk Graph
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name,       local-call-signature, index-range,   workaround
const SciWorkaroundEntry arithmeticWorkarounds[] = {
	{ GID_CAMELOT,         92,   92,  0,     "endingCartoon2", "changeState", sig_arithmetic_camelot_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_lai: during the ending, sub gets called with no parameters, uses parameter 1 which is theGrail in this case - bug #5237
	{ GID_FANMADE,        516,  983,  0,             "Wander", "setTarget",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_mul: The Legend of the Lost Jewel Demo (fan made): called with object as second parameter when attacked by insects - bug #5124
	{ GID_FANMADE,         -1,  935,  0,             "Scaler", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_mul: Zork: The Great Underground Empire (fan made): called with object as second parameter when changing rooms
	{ GID_GK1,            800,64992,  0,                "Fwd", "doit",                         nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // op_gt: when Mosely finds Gabriel and Grace near the end of the game, compares the Grooper object with 7
	{ GID_GK1,             -1,   91,  0,            "Walkman", "showTapes",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_bnot: Italian fan translation when exiting Walkman. bnot is just an extra 00 byte from the fan patch; the result isn't used.
	{ GID_HOYLE4,         700,   -1,  1,               "Code", "doit",                         nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // op_add: while bidding in Bridge, an object ("Bid") is added to an object in another segment ("hand3")
	{ GID_ICEMAN,         199,  977,  0,            "Grooper", "doit",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_add: While dancing with the girl
	{ GID_MOTHERGOOSE256,  -1,  999,  0,              "Event", "new",                          nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_and: constantly during the game (SCI1 version)
	{ GID_MOTHERGOOSE256,  -1,    4,  0,              "rm004", "doit",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_or: when going north and reaching the castle (rooms 4 and 37) - bug #5101
	{ GID_MOTHERGOOSEHIRES,90,   90,  0,      "newGameButton", "select",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_ge: MUMG Deluxe, when selecting "New Game" in the main menu. It tries to compare an integer with a list. Needs to return false for the game to continue.
	{ GID_PHANTASMAGORIA, 902,    0,  0,                   "", "export 7",                     nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_shr: when starting a chapter in Phantasmagoria
	{ GID_QFG1VGA,        301,  928,  0,              "Blink", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_div: when entering the inn, Blink:init called with 1 parameter, but 2nd parameter is used for div which happens to be an object.
	{ GID_QFG2,           200,  200,  0,              "astro", "messages",                     nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_lsi: when getting asked for your name by the astrologer - bug #5152
	{ GID_QFG3,           780,  999,  0,                   "", "export 6",                     nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // op_add: trying to talk to yourself at the top of the giant tree - bug #6692
	{ GID_TORIN,        51400,64928,  0,              "Blink", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // op_div: when Lycentia knocks Torin out after he removes her collar
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Fan-Made "Ocean Battle"
//      Calling method: RoomScript::doit
//   Subroutine offset: 0x1f17
// Applies to at least: Ocean Battle
static const uint16 sig_uninitread_fanmade_1[] = {
	0x3f, 0x04,                      // link 04
	0x88, SIG_UINT16(0x023b),        // lsg global[23Bh]
	SIG_END
};

//                Game: Hoyle 1
//      Calling method: export 0
//   Subroutine offset: 0x037c (script 16)
// Applies to at least: English PC floppy
static const uint16 sig_uninitread_hoyle1_1[] = {
	0x3f, 0x05,                      // link 05
	0x78,                            // push1
	0x76,                            // push0
	0x40,                            // call [...]
	SIG_END
};

//                Game: Jones in the fast lane
//      Calling method: weekendText::draw
//   Subroutine offset: 0x03d3 (script 232)
// Applies to at least: English PC CD
static const uint16 sig_uninitread_jones_1[] = {
	0x3f, 0x02,                      // link 02
	0x8d, 0x00,                      // lst temp[0]
	0x35, 0x01,                      // ldi 01
	0x22,                            // lt?
	SIG_END
};

//                Game: Conquests of the Longbow
//      Calling method: letter::handleEvent
//   Subroutine offset: English PC/Amiga 0x00a8 (script 213)
// Applies to at least: English PC floppy, English Amiga floppy
static const uint16 sig_uninitread_longbow_1[] = {
	0x3f, 0x02,                      // link 02
	0x35, 0x00,                      // ldi 00
	0xa5, 0x00,                      // sat temp[0]
	0x8d, 0x00,                      // lst temp[0]
	SIG_END
};

//                Game: Quest for Glory 1 / Hero's Quest 1
//      Calling method: Encounter::init
//   Subroutine offset: English Hero's Quest 0x0bd0, English Quest for Glory 1 0x0be4 (script 210)
// Applies to at least: English PC floppy (Hero's Quest, Quest For Glory 1), Japanese PC-9801 floppy
static const uint16 sig_uninitread_qfg1_1[] = {
	0x3f, 0x02,                      // link 02
	0x87, 0x00,                      // lap param[0]
	0x30, SIG_UINT16(0x000c),        // bnt [...]
	0x87, 0x01,                      // lap param[1]
	0x30, SIG_UINT16(0x0007),        // bnt [...]
	0x87, 0x01,                      // lap param[1]
	0xa5, 0x01,                      // sat temp[1]
	SIG_END
};

//                Game: Quest for Glory 1 VGA
//      Calling method: Encounter::init
//   Subroutine offset: English w/o patch 0x0cee, w/ patch 0x0ce7 (script 210)
// Applies to at least: English PC floppy
static const uint16 sig_uninitread_qfg1vga_1[] = {
	0x3f, 0x02,                      // link 02
	0x87, 0x00,                      // lap param[0]
	0x31, 0x0b,                      // bnt [...]
	0x87, 0x01,                      // lap param[1]
	0x31, 0x07,                      // bnt [...]
	0x87, 0x01,                      // lap param[1]
	0xa5, 0x01,                      // sat temp[1]
	// following jump is different for patched and unpatched game
	SIG_END
};

//                Game: Quest for Glory 1 VGA
//      Calling method: Encounter::init (although class names are blank in Mac)
//   Subroutine offset: 0x0f22 (script 210)
// Applies to at least: Mac floppy
static const uint16 sig_uninitread_qfg1vga_2[] = {
	0x3f, 0x02,                      // link 02
	0x87, 0x00,                      // lap param[0]
	0x30, SIG_UINT16(0x000c),        // bnt [...]
	0x87, 0x01,                      // lap param[1]
	0x30, SIG_UINT16(0x0007),        // bnt [...]
	0x87, 0x01,                      // lap param[1]
	0xa5, 0x01,                      // sat temp[1]
	SIG_END
};

//                Game: Quest for Glory 2
//      Calling method: abdulS::changeState, jabbarS::changeState
//   Subroutine offset: English 0x2d22 (script 260)
// Applies to at least: English PC floppy
static const uint16 sig_uninitread_qfg2_1[] = {
	0x3f, 0x03,                      // link 03
	0x39, 0x3b,                      // pushi 3Bh
	0x76,                            // push0
	0x81, 0x00,                      // lag global[0]
	0x4a, 0x04,                      // send 04
	SIG_END
};

//                Game: Quest for Glory 3
//      Calling method: rm140::init
//   Subroutine offset: English 0x1008 (script 140)
// Applies to at least: English, French, German, Italian, Spanish PC floppy
static const uint16 sig_uninitread_qfg3_1[] = {
	0x3f, 0x01,                      // link 01
	0x89, 0x7d,                      // lsg global[7Dh]
	0x35, 0x03,                      // ldi 03
	SIG_END
};

//                Game: Quest for Glory 3
//      Calling method: computersMove::changeState
//   Subroutine offset: English/etc. 0x0f53 (script 490)
// Applies to at least: English, French, German, Italian, Spanish PC floppy
static const uint16 sig_uninitread_qfg3_2[] = {
	0x3f, 0x1d,                      // link 1Dh
	0x35, 0x01,                      // ldi 01
	0xa5, 0x18,                      // sat temp[18h]
	0x35, 0xce,                      // ldi CEh
	0xa5, 0x19,                      // sat temp[19h]
	0x35, 0x00,                      // ldi 00
	0xa5, 0x00,                      // sat temp[0]
	SIG_END
};

//                Game: RAMA
//      Calling method: securityKeypad::newRoom
//   Subroutine offset: English 0x3b20 (script 6110)
// Applies to at least: English
static const uint16 sig_uninitread_rama_1[] = {
	0x7e, SIG_UINT16(0x121),                                                      // line 289
	0x7d, 0x73, 0x65, 0x63, 0x75, 0x72, 0x6b, 0x65, 0x79, 0x2e, 0x73, 0x63, 0x00, // file "securkey.sc"
	SIG_END
};

//                Game: RAMA
//      Calling method: key1::doVerb, key2, key3, etc.
//   Subroutine offset: English 0x5254 (script 6107)
// Applies to at least: English
static const uint16 sig_uninitread_rama_2[] = {
	0x7e, SIG_UINT16(0xf2),                                                       // line 242
	0x7d, 0x61, 0x76, 0x73, 0x65, 0x63, 0x75, 0x72, 0x37, 0x2e, 0x73, 0x63, 0x00, // file "avsecur7.sc"
	SIG_END
};

//                Game: Space Quest 1
//      Calling method: firePulsar::changeState
//   Subroutine offset: English 0x018a (script 703)
// Applies to at least: English PC floppy, English Amiga floppy
static const uint16 sig_uninitread_sq1_1[] = {
	0x3f, 0x01,                      // link 01
	0x38, SIG_ADDTOOFFSET(+2),       // pushi 0242 (selector egoStatus)
	0x76,                            // push0
	0x72, SIG_ADDTOOFFSET(+2),       // lofsa DeltaurRegion
	0x4a, 0x04,                      // send 04
	SIG_END
};

// Workarounds for uninitialized reads for parameters
//    gameID,           room,script,lvl,          object-name, method-name,       local-call-signature, index-range,  workaround
const SciWorkaroundEntry uninitializedReadForParamWorkarounds[] = {
	{ GID_ALL,            -1,   990, -1,            "Restore", "doit",                         nullptr,     1,     1,{ WORKAROUND_FAKE,   0 } }, // When opening original restore dialog in SCI16 games
	{ GID_ALL,            -1, 64990, -1,            "Restore", "doit",                         nullptr,     1,     1,{ WORKAROUND_FAKE,   0 } }, // When opening original restore dialog in SCI32 games
	{ GID_GK1,            -1,    12, -1,          "GKIconbar", "showInvItem",                  nullptr,     1,     1,{ WORKAROUND_FAKE,   1 } }, // When showing the icon bar containing an inventory item
	{ GID_HOYLE5,         -1,    15, -1,               "Hand", "add",                          nullptr,     1,     1,{ WORKAROUND_FAKE,   0 } }, // When the game adds cards to your hand in any mini-game
	{ GID_HOYLE5,        700,   730,  0,              nullptr, "runningSuit",                  nullptr,     2,     2,{ WORKAROUND_FAKE,   0 } }, // when an opponent is playing in Bridge
	{ GID_HOYLE5,       1100,    22, -1,           "HandPile", "show",                         nullptr,     1,     1,{ WORKAROUND_FAKE,   0 } }, // when showing money piles in Poker
	{ GID_LAURABOW2,      -1,    90,  0,          "MuseumRgn", "crowdInRoom",                  nullptr,     3,     4,{ WORKAROUND_FAKE,   0 } }, // When actors walk around the museum. Out-of-bounds parameters are read but not used. Doesn't matter, but generates a lot of warnings.
	{ GID_PHANTASMAGORIA2,-1, 63019,  0,     "WynDocTextView", "cue",                          nullptr,     2,     2,{ WORKAROUND_FAKE,   0 } }, // When dragging the slider next to an e-mail message
	{ GID_QFG3,           -1,    28,  0,               "hero", "changeGait",                   nullptr,     2,     2,{ WORKAROUND_FAKE,   0 } }, // When ego's walk style is adjusted throughout the game
	{ GID_QFG4,           -1,    28,  0,               "hero", "changeGait",                   nullptr,     1,     2,{ WORKAROUND_FAKE,   0 } }, // When ego's walk style is adjusted throughout the game (some scripts don't pass either parameter)
	{ GID_QFG4,           -1, 64924,  0,      "gloryMessager", "nextMsg",                      nullptr,     1,     5,{ WORKAROUND_FAKE,   0 } }, // Floppy: when saying a message. Passes 5 non-existent parameters to GloryTalker:say which never reads them, but generates a lot of warnings.
	{ GID_SHIVERS,        -1, 64918,  0,                "Str", "strip",                        nullptr,     1,     1,{ WORKAROUND_FAKE,   0 } }, // When starting a new game and entering a name
	{ GID_SQ4,            35,   928,  0,           "Narrator", "say",                          nullptr,     1,     1,{ WORKAROUND_FAKE,  11 } }, // Clicking smell on sidewalk, fixes message due to missing say parameter in sidewalk1:doVerb(6) - bug #10917
	SCI_WORKAROUNDENTRY_TERMINATOR
};

// Workarounds for uninitialized reads for temporary variables
//    gameID,           room,script,lvl,          object-name, method-name,       local-call-signature, index-range,  workaround
const SciWorkaroundEntry uninitializedReadWorkarounds[] = {
	{ GID_ALL,            -1,   990,  0,            "Restore", "doit",                         nullptr,   364,   364, { WORKAROUND_FAKE,   0 } }, // When pressing Restore button in original restore dialog when list is empty
	{ GID_ALL,            -1,   937, -1,  /*IconBar*/ nullptr, "dispatchEvent",                nullptr,    58,    58, { WORKAROUND_FAKE,  64 } }, // Pressing Enter on SCI1.1 control panel while mouse isn't over a button, also affects many dialogs. Empty name for Mac. Value must have 0x40 set to claim event. Bugs #5241, #6603, #13045, #13161
	{ GID_CAMELOT,        40,    40,  0,               "Rm40", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when looking at the ground at the pool of Siloam - bug #6401
	{ GID_CASTLEBRAIN,   280,   280,  0,         "programmer", "dispatchEvent",                nullptr,     0,     0, { WORKAROUND_FAKE, 0xf } }, // pressing 'q' on the computer screen in the robot room, and closing the help dialog that pops up (bug #5143). Moves the cursor to the view with the ID returned (in this case, the robot hand)
	{ GID_CASTLEBRAIN,   320,   325,  0,               "word", "dispatchEvent",                nullptr,    14,    15, { WORKAROUND_FAKE,   0 } }, // holding down enter key during the word search puzzle, temp 14 and 15 - bug #9783
	{ GID_CNICK_KQ,       -1,     0,  1,          "Character", "say",                          nullptr,   504,   505, { WORKAROUND_FAKE,   0 } }, // checkers/backgammon, like in hoyle 3 - temps 504 and 505 - bug #6255
	{ GID_CNICK_KQ,       -1,   700,  0,           "gcWindow", "open",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when entering the control menu, like in hoyle 3
	{ GID_CNICK_KQ,      300,   303,  0,      "theDoubleCube", "<noname520>",                  nullptr,     5,     5, { WORKAROUND_FAKE,   0 } }, // while playing backgammon with doubling enabled - bug #6426 (same as the theDoubleCube::make workaround for Hoyle 3)
	{ GID_CNICK_KQ,      300,   303,  0,      "theDoubleCube", "<noname519>",                  nullptr,     9,     9, { WORKAROUND_FAKE,   0 } }, // when accepting a double, while playing backgammon with doubling enabled (same as the theDoubleCube::accept workaround for Hoyle 3)
	{ GID_CNICK_LAURABOW,500,     0,  1,          "<no name>", "<noname446>",                  nullptr,   504,   505, { WORKAROUND_FAKE,   0 } }, // Yacht, like in hoyle 3 - temps 504 and 505 - bug #6424
	{ GID_CNICK_LAURABOW, -1,   700,  0,              nullptr, "open",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when entering control menu - bug #6423 (same as the gcWindow workaround for Hoyle 3)
	{ GID_CNICK_LAURABOW,100,   100,  0,              nullptr, "<noname144>",                  nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // while playing domino - bug #6429 (same as the dominoHand2 workaround for Hoyle 3)
	{ GID_CNICK_LAURABOW,100,   110,  0,              nullptr, "doit",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when changing the "Dominoes per hand" setting - bug #6430
	{ GID_CNICK_LSL,     250,   250,  0,           "increase", "handleEvent",                  nullptr,     2,     2, { WORKAROUND_FAKE,   0 } }, // when increasing own bet for blackjack - bug #10184
	{ GID_CNICK_LONGBOW,   0,     0,  0,          "RH Budget", "init",                         nullptr,     0,     1, { WORKAROUND_FAKE,   0 } }, // when starting the game
	{ GID_ECOQUEST,       -1,    -1,  0,              nullptr, "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // almost clicking anywhere triggers this in almost all rooms
	{ GID_ECOQUEST2,      -1,    50,  0,         "talkButton", "cue",                          nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // clicking Ecorder talk button before clicking power button
	{ GID_FANMADE,       516,   979,  0,                   "", "export 0",                     nullptr,    20,    20, { WORKAROUND_FAKE,   0 } }, // Happens in Grotesteing after the logos
	{ GID_FANMADE,       528,   990,  0,            "GDialog", "doit",                         nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // Happens in Cascade Quest when closing the glossary - bug #5116
	{ GID_FANMADE,       488,     1,  0,         "RoomScript", "doit",        sig_uninitread_fanmade_1,     1,     1, { WORKAROUND_FAKE,   0 } }, // Happens in Ocean Battle while playing - bug #5335
	{ GID_FANMADE,        -1,     0,  0,             "Tetris", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // SCI Tetris 1.1: at start of game, broken high score script passes uninit temp variable instead of reference to temp variable
	{ GID_FANMADE,        -1,   997,  0,         "TheMenuBar", "handleEvent",                  nullptr,   507,   507, { WORKAROUND_FAKE,   1 } }, // Betrayed Alliance 1.3 when selecting Notes from menu. Uninitialized variable must be > 0
	{ GID_FREDDYPHARKAS,  -1,    24,  0,              "gcWin", "open",                         nullptr,     5,     5, { WORKAROUND_FAKE, 0xf } }, // is used as priority for game menu
	{ GID_FREDDYPHARKAS,  -1,    31,  0,            "quitWin", "open",                         nullptr,     5,     5, { WORKAROUND_FAKE, 0xf } }, // is used as priority for game menu
	{ GID_FREDDYPHARKAS, 540,   540,  0,          "WaverCode", "init",                         nullptr,     0,     1, { WORKAROUND_FAKE,   0 } }, // Gun practice mini-game, all temps - 0+1 - bug #5232
	{ GID_GK1,            -1, 64950, -1,            "Feature", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // sometimes when walk-clicking
	{ GID_GK1,            -1, 64937, -1,         "GKControls", "dispatchEvent",                nullptr,     6,     6, { WORKAROUND_FAKE,   0 } }, // when using keyboard navigation (tab) in the game settings and hitting 'enter' when over a slider
	{ GID_GK1,            -1, 64994, -1,               "Game", "save",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when saving in Mac version
	{ GID_GK1,            -1, 64994, -1,               "Game", "restore",                      nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when restoring in Mac version
	{ GID_GK2,            -1,    11,  0,                   "", "export 10",                    nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // When game starts and throughout game. temp1 in Italian version, temp3 in others
	{ GID_GK2,            -1, 64921, -1,              "Print", "addEdit",                      nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // When trying to use the game debugger's flag setting command
	{ GID_HOYLE1,          4,   104,  0,   "GinRummyCardList", "calcRuns",                     nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // Gin Rummy / right when the game starts
	{ GID_HOYLE1,          5,   204,  0,            "tableau", "checkRuns",                    nullptr,     2,     2, { WORKAROUND_FAKE,   0 } }, // Cribbage / during the game
	{ GID_HOYLE1,          3,    16,  0,                   "", "export 0",     sig_uninitread_hoyle1_1,     3,     3, { WORKAROUND_FAKE,   0 } }, // Hearts / during the game - bug #5299
	{ GID_HOYLE1,         -1,   997,  0,            "MenuBar", "doit",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // When changing game speed settings - bug #5512
	{ GID_HOYLE3,         -1,     0,  1,          "Character", "say",                          nullptr,   504,   505, { WORKAROUND_FAKE,   0 } }, // when starting checkers or dominoes, first time a character says something - temps 504 and 505
	{ GID_HOYLE3,         -1,   700,  0,           "gcWindow", "open",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when entering control menu
	{ GID_HOYLE3,        100,   100,  0,        "dominoHand2", "cue",                          nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // while playing domino - bug #5042
	{ GID_HOYLE3,        100,   110,  0,           "OKButton", "doit",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when changing the "Dominoes per hand" setting - bug #6430
	{ GID_HOYLE3,        300,   303,  0,      "theDoubleCube", "make",                         nullptr,     5,     5, { WORKAROUND_FAKE,   0 } }, // while playing backgammon with doubling enabled
	{ GID_HOYLE3,        300,   303,  0,      "theDoubleCube", "accept",                       nullptr,     9,     9, { WORKAROUND_FAKE,   0 } }, // when accepting a double, while playing backgammon with doubling enabled
	{ GID_HOYLE4,         -1,     0,  0,              nullptr, "open",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when selecting "Control" from the menu (temp vars 0-3) - bug #5132
	{ GID_HOYLE4,        910,    18,  0,              nullptr, "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // during tutorial - bug #5213
	{ GID_HOYLE4,        910,   910,  0,              nullptr, "setup",                        nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // when selecting "Tutorial" from the main menu - bug #5132
	{ GID_HOYLE4,        700,   700,  1,         "BridgeHand", "calcQTS",                      nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // when placing a bid in bridge (always)
	{ GID_HOYLE4,        700,   710,  1, "BridgeStrategyPlay", "checkSplitTops",               nullptr,    10,    10, { WORKAROUND_FAKE,   0 } }, // while playing bridge, objects LeadReturn_Trump, SecondSeat_Trump, ThirdSeat_Trump and others - bug #5794
	{ GID_HOYLE4,        700,   730,  1,      "BridgeDefense", "beatTheirBest",                nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // rarely while playing bridge
	{ GID_HOYLE4,        700,    -1,  1,      "BridgeDefense", "makeContractMinusAce",         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when playing Bridge - bug #11162
	{ GID_HOYLE4,        700,    -1,  1,      "BridgeDefense", "think",                        nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // sometimes while playing bridge, temp var 3, 17 and others, objects LeadReturn_Trump, ThirdSeat_Trump and others
	{ GID_HOYLE4,        700,    -1,  1,               "Code", "doit",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when placing a bid in bridge (always), temp var 11, 24, 27, 46, 75, objects compete_tree, compwe_tree, other1_tree, b1 - bugs #5663 and #5794
	{ GID_HOYLE4,        700,   921,  0,              "Print", "addEdit",                      nullptr,    -1,    -1, { WORKAROUND_FAKE, 118 } }, // when saving the game (may also occur in other situations) - bug #6601, bug #6614
	{ GID_HOYLE4,        400,   400,  1,            "GinHand", "calcRuns",                     nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // sometimes while playing Gin Rummy (e.g. when knocking and placing a card) - bug #5665
	{ GID_HOYLE4,        500,    17,  1,          "Character", "say",                          nullptr,   504,   504, { WORKAROUND_FAKE,   0 } }, // sometimes while playing Cribbage (e.g. when the opponent says "Last Card") - bug #5662
	{ GID_HOYLE4,        700,   753,  0,   "LeadSeat_NoTrump", "think",              sig_any_procedure,     4,     6, { WORKAROUND_FAKE,   0 } }, // when playing Bridge
	{ GID_HOYLE4,        800,   870,  0,     "EuchreStrategy", "thinkLead",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // while playing Euchre, happens at least on 2nd or 3rd turn - bug #6602
	{ GID_HOYLE4,         -1,   937,  0,            "IconBar", "dispatchEvent",                nullptr,   407,   408, { WORKAROUND_FAKE,   0 } }, // pressing ENTER on scoreboard while mouse is not on OK button, may not happen all the time - bug #6603
	{ GID_HOYLE5,         -1,    14, -1,              nullptr, "select",                       nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // dragging the sliders in game settings
	{ GID_HOYLE5,         -1, 64937, -1,              nullptr, "select",                       nullptr,     7,     7, { WORKAROUND_FAKE,   0 } }, // clicking the "control" and "options" buttons in the icon bar
	{ GID_HOYLE5,         -1, 64937, -1,            "IconBar", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // clicking on any button in the icon bar
	{ GID_HOYLE5,        400,   400,  1,            "GinHand", "calcRuns",                     nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // when starting Gin
	{ GID_HOYLE5,        700,   700,  1,         "BridgeHand", "calcQTS",                      nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // when an opponent is playing in Bridge
	{ GID_HOYLE5,        700,   710,  1, "BridgeStrategyPlay", "checkSplitTops",               nullptr,    10,    10, { WORKAROUND_FAKE,   0 } }, // when playing Bridge - bug #11167
	{ GID_HOYLE5,        700,   730,  1,      "BridgeDefense", "beatTheirBest",                nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // when playing Bridge - bug #11171
	{ GID_HOYLE5,        700,    -1,  1,      "BridgeDefense", "makeContractMinusAce",         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when playing Bridge
	{ GID_HOYLE5,        700,    -1,  1,      "BridgeDefense", "think",                        nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when an opponent is playing in Bridge, objects LeadSeat_NoTrump and others
	{ GID_HOYLE5,        700,    -1,  1,               "Code", "doit",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when placing a bid in Bridge, objects c2_tree, other1_tree, compwe_tree - bugs #11168, #11169, #11170, #11183
	{ GID_HOYLE5,        700,   753,  0,   "LeadSeat_NoTrump", "think",              sig_any_procedure,     4,     6, { WORKAROUND_FAKE,   0 } }, // when playing Bridge
	{ GID_HOYLE5,        700,  1115,  0,              nullptr, "select",                       nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when adjusting the attitude slider in Bridge - bug #11166
	{ GID_HOYLE5,       1100,    18,  0,               "Tray", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when playing Poker
	{ GID_HOYLE5,       1100,  1100,  0,         "anteButton", "handleEvent",                  nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when exiting Poker
	{ GID_HOYLE5,       6029,  6029,  1,        "ControlIcon", "select",                       nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // Solitaire: when changing any slider in the Card Flip mini-game's options window
	{ GID_HOYLE5,         -1,  6000,  1,              "sHand", "handleEvent",                  nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // Solitaire: when clicking on an empty card base in any game
	{ GID_ISLANDBRAIN,   140,   140,  0,              "piece", "init",                         nullptr,     3,     3, { WORKAROUND_FAKE,   1 } }, // first puzzle right at the start, some initialization variable. bnt is done on it, and it should be non-0
	{ GID_ISLANDBRAIN,   160,   165,  0,          "cartesian", "buyClue",                      nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // microscope puzzle: upon click of buy clue - depends on non-deterministic initialization for puzzle. Before div occurs, temp0 must be non-0 - bug #14546
	{ GID_ISLANDBRAIN,   180,   190,  0,               "word", "dispatchEvent",                nullptr,    14,    15, { WORKAROUND_FAKE,   0 } }, // holding down enter key during the word search puzzle, temps 14 and 15
	{ GID_ISLANDBRAIN,   200,   268,  0,          "anElement", "select",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // elements puzzle, gets used before super TextIcon
	{ GID_JONES,           1,   232,  0,        "weekendText", "draw",          sig_uninitread_jones_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // jones/cd only - gets called during the game
	{ GID_JONES,           1,   255,  0,                   "", "export 0",                     nullptr,    13,    14, { WORKAROUND_FAKE,   0 } }, // jones/cd only - called when a game ends, temps 13 and 14
	{ GID_JONES,         764,   255,  0,                   "", "export 0",                     nullptr,    13,    14, { WORKAROUND_FAKE,   0 } }, // jones/ega&vga only - called when the game starts, temps 13 and 14
	//{ GID_KQ5,            -1,     0,  0,                   "", "export 29",                       NULL,     3,     3, { WORKAROUND_FAKE,   0xf } }, // called when playing harp for the harpies or when aborting dialog in toy shop, is used for kDoAudio - bug #4961
	// ^^ shouldn't be needed anymore, we got a script patch instead (kq5PatchCdHarpyVolume)
	{ GID_KQ5,            25,    25,  0,              "rm025", "doit",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // inside witch forest, when going to the room where the walking rock is
	{ GID_KQ5,            55,    55,  0,         "helpScript", "doit",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when giving the tambourine to the monster in the labyrinth (only happens at one of the locations) - bug #5198
	{ GID_KQ5,            -1,   755,  0,              "gcWin", "open",                         nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when entering control menu in the FM-Towns version
	{ GID_KQ5,            -1,   764,  0,              "trash", "select",                       nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when clicking delete button on save/restore dialog in the FM-Towns version
	{ GID_KQ6,            -1,    30,  0,               "rats", "changeState",                  nullptr,     0,     5, { WORKAROUND_FAKE,   0 } }, // rats in the catacombs (temps 0-5, all temps!) - bugs #4958, #4998, #5017
	{ GID_KQ6,           210,   210,  0,              "rm210", "scriptCheck",                  nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // using inventory in that room - bug #4953
	{ GID_KQ6,           500,   500,  0,              "rm500", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // going to island of the beast
	{ GID_KQ6,           520,   520,  0,              "rm520", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // going to boiling water trap on beast isle
	{ GID_KQ6,           720,   720,  0,              "rm720", "warnUser",                     nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when guard opens guard-room door after waiting too long during wedding music
	{ GID_KQ6,            -1,   903,  0,         "controlWin", "open",                         nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // when opening the controls window (save, load etc)
	{ GID_KQ6,            -1,   907,  0,             "tomato", "doVerb",                       nullptr,     2,     2, { WORKAROUND_FAKE,   0 } }, // when looking at the rotten tomato in the inventory - bug #5331
	{ GID_KQ6,            -1,   921, -1,              "Print", "addEdit",                      nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when displaying the game debugger's teleport dialog
	{ GID_KQ6,            -1,   928,  0,              nullptr, "startText",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // gets caused by Text+Audio support (see script patcher)
	{ GID_KQ7,            -1, 64996,  0,               "User", "handleEvent",                  nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // called when pushing a keyboard key
	{ GID_KQ7,          2450,  2450,  0,           "exBridge", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // called when walking up to the throne in the cave in chapter 2
	{ GID_KQ7,          2450,  2450,  0,       "maliciaComes", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when malicia appears at the southeast exit of the main chamber near the end of chapter 2
	{ GID_KQ7,          5300,  5302,  0,          "putOnMask", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // in chapter 3, after using the mask on Valanice, click the jackalope hair in inventory - bug Trac#9759
	{ GID_KQ7,          6060, 64964,  0,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // after entering the harp crystal in chapter 5
	{ GID_KQ7,            -1, 64994, -1,               "Game", "restore",                      nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when restoring from ScummVM launcher in Mac version
	{ GID_LAURABOW,       37,     0,  0,                "CB1", "doit",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when going up the stairs - bug #5084
	{ GID_LAURABOW,       -1,   967,  0,             "myIcon", "cycle",                        nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // having any portrait conversation coming up - initial bug #4971
	{ GID_LAURABOW2,      -1,    24,  0,              "gcWin", "open",                         nullptr,     5,     5, { WORKAROUND_FAKE, 0xf } }, // is used as priority for game menu
	{ GID_LAURABOW2,      -1,    21,  0,      "dropCluesCode", "doit",                         nullptr,     1,     1, { WORKAROUND_FAKE, 0x7fff } }, // when asking some questions (e.g. the reporter about the burglary, or the policeman about Ziggy). Must be big, as the game scripts perform lt on it and start deleting journal entries - bugs #4979, #5026
	{ GID_LAURABOW2,      -1,    90,  1,        "MuseumActor", "init",                         nullptr,     6,     6, { WORKAROUND_FAKE,   0 } }, // Random actors in museum - bug #5197
	{ GID_LAURABOW2,     240,   240,  0,     "sSteveAnimates", "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Steve Dorian's idle animation at the docks - bug #5028
	{ GID_LAURABOW2,      -1,   928,  0,              nullptr, "startText",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // gets caused by Text+Audio support (see script patcher)
	{ GID_LIGHTHOUSE,     -1,     0,  0,          "globalVMD", "play",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when playing a VMD while inventory is disabled (opening a portal in room 360) - bug #14924
	{ GID_LIGHTHOUSE,     -1,    17,  0,              nullptr, "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when operating the joystick in the puzzle to lower the bridge at the entrance to the workshop, or the joystick that moves the robotic arm in the mini-sub
	{ GID_LONGBOW,        -1,     0,  0,            "Longbow", "restart",                      nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // When canceling a restart game - bug #5244
	{ GID_LONGBOW,        -1,   213,  0,              "clear", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // When giving an answer using the druid hand sign code in any room
	{ GID_LONGBOW,        -1,   213,  0,             "letter", "handleEvent", sig_uninitread_longbow_1,     1,     1, { WORKAROUND_FAKE,   0 } }, // When using the druid hand sign code in any room - bug #5035
	{ GID_LSL1,          250,   250,  0,           "increase", "handleEvent",                  nullptr,     2,     2, { WORKAROUND_FAKE,   0 } }, // casino, playing game, increasing bet
	{ GID_LSL1,          720,   720,  0,              "rm720", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // age check room
	{ GID_LSL2,           38,    38,  0,        "cloudScript", "changeState",                  nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // entering the room in the middle deck of the ship - bug #5034
	{ GID_LSL3,          340,   340,  0,        "ComicScript", "changeState",                  nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // right after entering the 3 ethnic groups inside comedy club (temps 200, 201, 202, 203)
	{ GID_LSL5,           -1,   175,  0,                   "", "export 0",                     nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // mac: when asking for password the procedure returns an uninitialized variable. this return value is never used.
	{ GID_LSL6,          820,    82,  0,                   "", "export 0",                     nullptr,     0,   326, { WORKAROUND_FAKE,   0 } }, // when touching the electric fence (temp 193 for English release, temp 293 for French/German, temp 313 for Spanish - used for setting the loop of the death animation), it's not setting it for this death - bug #5103
	{ GID_LSL6,           -1,    85,  0,          "washcloth", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // washcloth in inventory
	{ GID_LSL6,           -1,   928, -1,           "Narrator", "startText",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // used by various objects that are even translated in foreign versions, that's why we use the base-class
	{ GID_LSL6HIRES,      -1,    85,  0,             "LL6Inv", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // when creating a new game. must be non-zero for correct inventory background color - bug #13497
	{ GID_LSL6HIRES,      -1,    85,  0,          "washcloth", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when interacting with the wet washcloth in the inventory - Trac#9811
	{ GID_LSL6HIRES,     820,    82,  0,                   "", "export 0",                     nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // when touching the electric fence - bug #10361
	{ GID_LSL6HIRES,      -1, 64950,  1,            "Feature", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // at least when entering swimming pool area
	{ GID_LSL6HIRES,      -1, 64964,  0,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // during the game
	{ GID_LSL6HIRES,      -1, 64994, -1,               "Game", "save",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when saving in Mac version
	{ GID_LSL6HIRES,      -1, 64994, -1,               "Game", "restore",                      nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when restoring in Mac version
	{ GID_LSL7,           -1, 64029,  0,          "oMessager", "nextMsg",                      nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // when running the game with subtitles only
	{ GID_LSL7,           -1, 64017,  0,             "oFlags", "clear",                        nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // demo version, when it starts, and whenever the player chooses to go to the "Strip Liar's Dice" mini game
	{ GID_LSL7,           -1, 64017,  0,        "oActorFlags", "clear",                        nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // after an NPC walks off the left side of the screen at the Clothing Optional Pool
	{ GID_LSL7,           -1, 64892,  0,      "oEventHandler", "killAllEventHogs",             nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when looking at the swordfish in the kitchen
	{ GID_MOTHERGOOSE256, -1,     0,  0,                 "MG", "doit",                         nullptr,     5,     5, { WORKAROUND_FAKE,   0 } }, // SCI1.1: When moving the cursor all the way to the left during the game - bug #5224
	{ GID_MOTHERGOOSE256, -1,   992,  0,             "AIPath", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Happens in the demo and full version. In the demo, it happens when walking two screens from mother goose's house to the north. In the full version, it happens in rooms 7 and 23 - bug #5269
	{ GID_MOTHERGOOSE256, 90,    90,  0,        "introScript", "changeState",                  nullptr,    64,    65, { WORKAROUND_FAKE,   0 } }, // At the very end, after the game is completed and restarted (floppy: temp 64, CD: temp 65) - bug #5626
	{ GID_MOTHERGOOSEHIRES,-1,64950, -1,            "Feature", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // right when clicking on a child at the start and probably also later
	{ GID_MOTHERGOOSEHIRES,-1,64950, -1,               "View", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // see above
	{ GID_PEPPER,         -1,   894,  0,            "Package", "doVerb",                       nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // using the hand on the book in the inventory - bug #5154
	{ GID_PEPPER,        150,   928,  0,           "Narrator", "startText",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // happens during the non-interactive demo of Pepper
	{ GID_PEPPER,        260,   260,  0,            "glutton", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // using tomato on General Lee
	{ GID_PHANTASMAGORIA, -1, 64921, -1,              "Print", "addEdit",                      nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // When trying to use the game debugger's flag setting command
	{ GID_PQ4,            -1,    25,  0,         "iconToggle", "select",                       nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when toggling the icon bar to auto-hide or not
	{ GID_PQ4,           170,   170, -1,        "hideAndSeek", "handleEvent",                  nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when clicking to move right while still moving left during the Emo shootout - bug #9847
	{ GID_PQ4,           275, 64964, -1,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when Sherry walks out of the morgue on day 3
	{ GID_PQ4,           240, 64964, -1,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when encountering Sherry and the reporter outside the morgue at the end of day 3
	{ GID_PQ4,          2010,  2010, -1,         "enemyShip1", "cantBeHere",                   nullptr,     7,     7, { WORKAROUND_FAKE,   0 } }, // when crashing into a UFO in the asteroids minigame in the Shortstop Bar
	{ GID_PQ4,          2010,  2010, -1,         "enemyShip2", "cantBeHere",                   nullptr,     7,     7, { WORKAROUND_FAKE,   0 } }, // when crashing into a UFO in the asteroids minigame in the Shortstop Bar
	{ GID_PQ4,            -1, 64937, -1,     "pqGameControls", "dispatchEvent",                nullptr,     6,     6, { WORKAROUND_FAKE,   0 } }, // pressing the Enter key on control panel screen while mouse isn't over a button
	{ GID_PQ4,            -1, 64950, -1,            "Feature", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // floppy: when walking within room 395 (city hall, day 3)
	{ GID_PQ4,            -1, 64994, -1,               "Game", "save",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when saving in Mac version
	{ GID_PQ4,            -1,     0, -1,                "pq4", "restore",                      nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when restoring in Mac version
	{ GID_PQSWAT,         -1, 64950,  0,              nullptr, "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Using any menus in-game
	{ GID_PQSWAT,         -1,    73,  0,   "theLashInterface", "transmit",                     nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Clicking the transmit button in LASH
	{ GID_PQSWAT,       2990,  2990,  0,    "talkToSchienbly", "changeState",                  nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // When the video of Schienbly talking for the first time ends
	{ GID_QFG1,           -1,   210,  0,          "Encounter", "init",           sig_uninitread_qfg1_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // qfg1/hq1: going to the brigands hideout
	{ GID_QFG1VGA,        -1,   210,  0,          "Encounter", "init",        sig_uninitread_qfg1vga_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // qfg1vga: going to the brigands hideout - bug #5515
	{ GID_QFG1VGA,        -1,   210,  0,          "Encounter", "init",        sig_uninitread_qfg1vga_2,     0,     0, { WORKAROUND_FAKE,   0 } }, // qfg1vga mac: going to the brigands hideout - bug #5515
	{ GID_QFG1VGA,        58,    58,  0,               "rm58", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,  18 } }, // qfg1vga: casting "detect magic" at giant's cave, temp 0 used instead of spell number
	{ GID_QFG1VGA,        96,    96,  0,              nullptr, "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // qfg1vga mac: when yorick throws an object
	{ GID_QFG1VGA,       320,   320,  0,              nullptr, "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // qfg1vga mac: first time entering room 320 when centaur offers fruits and vegetables
	{ GID_QFG2,           -1,    71,  0,        "theInvSheet", "doit",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // accessing the inventory
	{ GID_QFG2,           -1,    79,  0,        "TryToMoveTo", "onTarget",                     nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when throwing pot at air elemental, happens when client coordinates are the same as airElemental coordinates. happened to me right after room change - bug #6859
	{ GID_QFG2,           -1,   701, -1,              "Alley", "at",                           nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when walking inside the alleys in the town - bug #5019 & #5106
	{ GID_QFG2,          260,   260,  0,             "abdulS", "changeState",    sig_uninitread_qfg2_1,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // During the thief's first mission (in the house), just before Abdul is about to enter the house (where you have to hide in the wardrobe), bug #5153, temps 1 and 2
	{ GID_QFG2,          260,   260,  0,            "jabbarS", "changeState",    sig_uninitread_qfg2_1,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // During the thief's first mission (in the house), just before Jabbar is about to enter the house (where you have to hide in the wardrobe), bug #5164, temps 1 and 2
	{ GID_QFG2,          500,   500,  0,   "lightNextCandleS", "changeState",                  nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // Inside the last room, while Ad Avis performs the ritual to summon the genie - bug #5566
	{ GID_QFG2,           -1,   700,  0,              nullptr, "showSign",                     nullptr,    10,    10, { WORKAROUND_FAKE,   0 } }, // Occurs sometimes when reading a sign in Raseir, Shapeir et al - bugs #5627, #5635
	{ GID_QFG3,          140,   140,  0,              "rm140", "init",           sig_uninitread_qfg3_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // when importing a character and selecting the previous profession - bug #5163
	{ GID_QFG3,          330,   330, -1,             "Teller", "doChild",                      nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // when talking to King Rajah about "Rajah" (bug #5033, temp 1) or "Tarna" (temp 0), or when clicking on yourself and saying "Greet" (bug #5148, temp 1)
	{ GID_QFG3,          700,   700, -1,      "monsterIsDead", "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // in the jungle, after winning any fight, bug #5169
	{ GID_QFG3,          450,   450, -1,    "castOpenOnCrack", "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // in laibon's hut, wait for night and cast open on crack in wall
	{ GID_QFG3,          470,   470, -1,              "rm470", "notify",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // closing the character screen in the Simbani village in the room with the bridge, bug #5165
	{ GID_QFG3,          470,   470, -1,     "<invalid name>", "notify",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // same as previous, with rm470::name used for temp storage by fan patches added by GOG
	{ GID_QFG3,          490,   490, -1,      "computersMove", "changeState",    sig_uninitread_qfg3_2,     4,     4, { WORKAROUND_FAKE,   0 } }, // when finishing awari game, bug #5167
	{ GID_QFG3,           -1,    32, -1,            "ProjObj", "doit",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // near the end, when throwing the spear of death or sword, bugs #5282, #11477
	{ GID_QFG4,           -1,    15, -1,     "charInitScreen", "dispatchEvent",                nullptr,     5,     5, { WORKAROUND_FAKE,   0 } }, // floppy version, when viewing the character screen
	{ GID_QFG4,           -1,    23, -1,     "tellerControls", "dispatchEvent",                nullptr,     6,     6, { WORKAROUND_FAKE,   0 } }, // floppy version, when using keyboard controls in the conversation interface
	{ GID_QFG4,           -1,    50, -1,     "sSearchMonster", "changeState",                  nullptr,     2,     2, { WORKAROUND_FAKE,   0 } }, // CD version, when searching a chernovy or revenant with speech disabled
	{ GID_QFG4,           -1, 64917, -1,       "controlPlane", "setBitmap",                    nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // floppy version, when entering the game menu
	{ GID_QFG4,           -1, 64917, -1,              "Plane", "setBitmap",                    nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // floppy version, happens sometimes in fight scenes
	{ GID_QFG4,          380,    80, -1,           "myButton", "select",                       nullptr,     2,     2, { WORKAROUND_FAKE,   1 } }, // CD version, when clicking on a puzzle piece for the keyhole scrambled picture puzzle
	{ GID_QFG4,           -1, 64950, -1,            "Feature", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // CD version, at the lake, when meeting the Rusalka and attempting to leave, and after Ad Avis catches you, and finishes his monologue
	{ GID_QFG4,           -1, 64964,  0,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // CD version, walking down to the monastery basement or entering the Gypsy Camp
	{ GID_QFG4,           -1, 64950,  0,               "View", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // CD version, in the room with the spider pillar, when climbing on the pillar, or when using the Glide spell in the swamp
	{ GID_RAMA,           -1, 64950, -1,              nullptr, "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // When clicking on the main game interface, or the main menu buttons, or mousing over things in the main game window
	{ GID_RAMA,           -1, 64923, -1,              "Inset", "init",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // When receiving a message on the pocket computer at the start of the game
	{ GID_RAMA,          6107, 6107, -1,              nullptr, "doVerb",         sig_uninitread_rama_2,     0,     0, { WORKAROUND_FAKE,   0 } }, // When pressing keys on the final console in the Avian Lair
	{ GID_RAMA,          6110, 6110, -1,     "securityKeypad", "newRoom",        sig_uninitread_rama_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // When entering the correct key combination on the security console in the Avian Lair
	{ GID_SHIVERS,        -1,   952,  0,       "SoundManager", "stop",                         nullptr,     2,     2, { WORKAROUND_FAKE,   0 } }, // Just after Sierra logo
	{ GID_SHIVERS,        -1, 64950,  0,            "Feature", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // When clicking on the locked door at the beginning
	{ GID_SHIVERS,        -1, 64950,  0,               "View", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // When clicking on the gargoyle eye at the beginning
	{ GID_SHIVERS,     20311, 64964,  0,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // Just after door puzzle is solved and the metal balls start to roll
	{ GID_SHIVERS,     29260, 29260,  0,             "spMars", "handleEvent",                  nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // When clicking mars after seeing fortune to align earth etc...
	{ GID_SHIVERS,     29260, 29260,  0,            "spVenus", "handleEvent",                  nullptr,     4,     4, { WORKAROUND_FAKE,   0 } }, // When clicking venus after seeing fortune to align earth etc...
	{ GID_SQ1,           103,   103,  0,               "hand", "internalEvent",                nullptr,    -1,    -1, { WORKAROUND_FAKE,   0 } }, // Spanish (and maybe early versions?) only: when moving cursor over input pad, temps 1 and 2
	{ GID_SQ1,            -1,   703,  0,                   "", "export 1",                     nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // sub that's called from several objects while on sarien battle cruiser
	{ GID_SQ1,            -1,   703,  0,         "firePulsar", "changeState",     sig_uninitread_sq1_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // export 1, but called locally (when shooting at aliens)
	{ GID_SQ1,            -1,   703,  0,      "DeltaurRegion", "init",            sig_uninitread_sq1_1,     0,     0, { WORKAROUND_FAKE,   0 } }, // export 1, but called locally (when teleporting to a deltaur room)
	{ GID_SQ4,            -1,   398,  0,            "showBox", "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // CD: called when rummaging in Software Excess bargain bin
	{ GID_SQ4,            -1,   928, -1,           "Narrator", "startText",                    nullptr,  1000,  1000, { WORKAROUND_FAKE,   1 } }, // CD: happens in the options dialog and in-game when speech and subtitles are used simultaneously
	{ GID_SQ4,           395,   395, -1,    "fromStoreScript", "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // CD: happens when shoplifting in Galaxy Galleria - bug #10229
	{ GID_SQ4,            -1,   708, -1,            "exitBut", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Floppy: happens, when looking at the "close" button in the sq4 hintbook - bug #6447
	{ GID_SQ4,            -1,   708, -1,                   "", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Floppy: happens, when looking at the "close" button... in Russian version - bug #5573
	{ GID_SQ4,            -1,   708, -1,            "prevBut", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Floppy: happens, when looking at the "previous" button in the sq4 hintbook - bug #6447
	{ GID_SQ4,            -1,   708, -1, "\xA8\xE6\xE3 \xAD\xA0\xA7\xA0\xA4.", "doVerb",          nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Floppy: happens, when looking at the "previous" button... in Russian version - bug #5573
	{ GID_SQ4,            -1,   708, -1,            "nextBut", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Floppy: happens, when looking at the "next" button in the sq4 hintbook - bug #6447
	{ GID_SQ4,            -1,   708, -1,                  ".", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // Floppy: happens, when looking at the "next" button... in Russian version - bug #5573
	{ GID_SQ5,           201,   201,  0,        "buttonPanel", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   1 } }, // when looking at the orange or red button - bug #5112
	{ GID_SQ6,            -1,     0,  0,                "SQ6", "init",                         nullptr,     2,     2, { WORKAROUND_FAKE,   0 } }, // Demo and full version: called when the game starts (demo: room 0, full: room 100)
	{ GID_SQ6,           390,   666, -1,                   "", "export 0",                     nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // Demo: death dialog in room 390 (early version)
	{ GID_SQ6,            -1, 64950, -1,            "Feature", "handleEvent",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // called when pressing "Start game" in the main menu, when entering the Orion's Belt bar (room 300), and perhaps other places
	{ GID_SQ6,            -1, 64964,  0,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // during the game
	{ GID_SQ6,           210,   210,  0,       "buttonSecret", "doVerb",                       nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // after winning the first round of stooge fighter 3
	{ GID_SQ6,            -1, 64994, -1,               "Game", "restore",                      nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // When trying to load an invalid save game from the launcher
	{ GID_SQ6,            -1, 64921, -1,              "Print", "addEdit",                      nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // When trying to use the game debugger's flag setting command
	{ GID_SQ6,           105,   105, -1,    "doRestoreScript", "changeState",                  nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when restoring from main menu in Mac version
	{ GID_SQ6,            -1, 64994, -1,               "Game", "save",                         nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when saving in Mac version
	{ GID_SQ6,            -1, 64994, -1,               "Game", "restore",                      nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // when restoring in Mac version
	{ GID_TORIN,          -1, 64017,  0,             "oFlags", "clear",                        nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // entering Torin's home in the French version
	{ GID_TORIN,          -1, 64029,  0,          "oMessager", "nextMsg",                      nullptr,     3,     3, { WORKAROUND_FAKE,   0 } }, // start of chapter one, or when running with subtitles only
	{ GID_TORIN,          -1, 64892,  0,      "oEventHandler", "killAllEventHogs",             nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // when pressing the hint button when the game is about to transition to a new room (race condition) - Trac#9810
	{ GID_TORIN,       20100, 64964,  0,              "DPath", "init",                         nullptr,     1,     1, { WORKAROUND_FAKE,   0 } }, // going down the cliff at the first screen of chapter 2 (washing area)
	{ GID_TORIN,       61100, 64888,  0,              "Torin", "autorestore",                  nullptr,    11,    11, { WORKAROUND_FAKE,   0 } }, // after attempting to restore a save game saved with the wrong game version
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kAbs_workarounds[] = {
	{ GID_HOYLE1,          1,     1,  0,              "room1", "doit",                   nullptr,     0,     0, { WORKAROUND_FAKE,  0x3e9 } }, // crazy eights - called with objects instead of integers
	{ GID_HOYLE1,          2,     2,  0,              "room2", "doit",                   nullptr,     0,     0, { WORKAROUND_FAKE,  0x3e9 } }, // old maid - called with objects instead of integers
	{ GID_HOYLE1,          3,     3,  0,              "room3", "doit",                   nullptr,     0,     0, { WORKAROUND_FAKE,  0x3e9 } }, // hearts - called with objects instead of integers
	{ GID_QFG1VGA,        -1,    -1,  0,              nullptr, "doit",                   nullptr,     0,     0, { WORKAROUND_FAKE,  0x3e9 } }, // when the game is patched with the NRS patch
	{ GID_QFG3   ,        -1,    -1,  0,              nullptr, "doit",                   nullptr,     0,     0, { WORKAROUND_FAKE,  0x3e9 } }, // when the game is patched with the NRS patch - bugs #6042, #6043
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kAnimate_workarounds[] = {
	{ GID_FANMADE,        -1,    76,  0,              "rm076", "init",                   nullptr,     0,     0, { WORKAROUND_FAKE,  0 } }, // Betrayed Alliance Book 2 Demo: when starting game, script passes stale accumulator (8) instead of list
	{ GID_FANMADE,        -1,    76,  0,         "RoomScript", "changeState",            nullptr,     0,     0, { WORKAROUND_FAKE,  0 } }, // Betrayed Alliance Book 2 Demo: when starting game, script passes stale accumulator (theMusic object) instead of list
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kArraySetElements_workarounds[] = {
	{ GID_GK1,           302, 64918,  0,                "Str", "callKernel",             nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // when erasing a letter on the wall in St Louis Cemetery
	{ GID_PHANTASMAGORIA, -1, 64918,  0,                "Str", "callKernel",             nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // when starting a new game and selecting a chapter above 1, or when quitting the chase (in every chase room), or when completing chase successfully
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kArrayFill_workarounds[] = {
	// Phantasmagoria Mac calls kArrayFill to zero-initialize new string buffers, but almost
	//  half of these calls are missing the start-index parameter and so they had no effect.
	//  Our SCI strings are already zero-initialized and so these calls can be ignored.
	{ GID_PHANTASMAGORIA, -1,    38,  0,        "SaveManager", "init",                   nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } },
	{ GID_PHANTASMAGORIA, -1,    38,  0,        "SaveManager", "save",                   nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } },
	{ GID_PHANTASMAGORIA, -1,    38,  0,        "SaveManager", "getSaveType",            nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } },
	{ GID_PHANTASMAGORIA, -1,    38,  0,        "SaveManager", "testFlag",               nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } },
	{ GID_PHANTASMAGORIA, -1,    38,  0,        "SaveManager", "delete",                 nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } },
	{ GID_PHANTASMAGORIA, -1,    90,  0,           "logoRoom", "init",                   nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } },
	{ GID_PHANTASMAGORIA, -1, 45950,  0,            "rm45950", "init",                   nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } },
	{ GID_PQ4,           540, 64918,  0,                "Str", "callKernel",             nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when clicking on Hate Crimes in the computer on day 2
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kCelHigh_workarounds[] = {
	{ GID_KQ5,            -1,   255,  0,          "deathIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // english floppy: when getting beaten up in the inn and probably more, called with 2nd parameter as object - bug #5049
	{ GID_PQ2,            -1,   255,  0,              "DIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when showing picture within windows, called with 2nd/3rd parameters as objects
	{ GID_SQ1,             1,   255,  0,              "DIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // DEMO: Called with 2nd/3rd parameters as objects when clicking on the menu - bug #5012
	{ GID_FANMADE,        -1,   979,  0,              "DIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // In The Gem Scenario and perhaps other fanmade games, this is called with 2nd/3rd parameters as objects - bug #5144
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kCelWide_workarounds[] = {
	{ GID_KQ5,            -1,   255,  0,          "deathIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // english floppy: when getting beaten up in the inn and probably more, called with 2nd parameter as object - bug #5049
	{ GID_PQ2,            -1,   255,  0,              "DIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when showing picture within windows, called with 2nd/3rd parameters as objects
	{ GID_SQ1,             1,   255,  0,              "DIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // DEMO: Called with 2nd/3rd parameters as objects when clicking on the menu - bug #5012
	{ GID_FANMADE,        -1,   979,  0,              "DIcon", "setSize",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // In The Gem Scenario and perhaps other fanmade games, this is called with 2nd/3rd parameters as objects - bug #5144
	{ GID_LSL6HIRES,      -1,    94,  0,    "ll6ControlPanel",    "init",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when opening the "controls" panel from the main menu, the third argument is missing
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDeleteKey_workarounds[] = {
	{ GID_HOYLE4,        300,   999,  0,     "handleEventList", "delete",                nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // restarting hearts, while tray is shown - bug #6604
	{ GID_HOYLE4,        500,   999,  0,     "handleEventList", "delete",                nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // restarting cribbage, while tray is shown - bug #6604
	{ GID_HOYLE4,        975,   999,  0,     "handleEventList", "delete",                nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // going back to gamelist from hearts/cribbage, while tray is shown - bug #6604
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Fan-Made games (SCI Studio)
//      Calling method: Game::save, Game::restore
//   Subroutine offset: Al Pond 2:          0x0e5c (ldi 0001)
//                      Black Cauldron:     0x000a (ldi 01)
//                      Cascade Quest:      0x0d1c (ldi 0001)
//                      Demo Quest:         0x0e55 (ldi 0001)
//                      I want my C64 back: 0x0e57 (ldi 0001)
// Applies to at least: games listed above
static const uint16 sig_kDeviceInfo_Fanmade_1[] = {
	0x3f, 0x79,                      // link 79h
	0x34, SIG_UINT16(0x0001),        // ldi 0001
	0xa5, 0x00,                      // sat temp[0]
	SIG_END
};
static const uint16 sig_kDeviceInfo_Fanmade_2[] = {
	0x3f, 0x79,                      // link 79h
	0x35, 0x01,                      // ldi 01
	0xa5, 0x00,                      // sat temp[0]
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name,      local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDeviceInfo_workarounds[] = {
	{ GID_FANMADE,        -1,   994,  1,               "Game", "save",      sig_kDeviceInfo_Fanmade_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games (SCI Studio), this is called with one parameter for CurDevice LDI 01 variant
	{ GID_FANMADE,        -1,   994,  1,               "Game", "save",      sig_kDeviceInfo_Fanmade_2,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games (SCI Studio), this is called with one parameter for CurDevice LDI 0001 variant
	{ GID_FANMADE,        -1,   994,  0,              "Black", "save",      sig_kDeviceInfo_Fanmade_1,     0,     0, { WORKAROUND_IGNORE,    0 } }, // In fanmade games (SCI Studio), this is called with one parameter for CurDevice (Black Cauldron Remake)
	{ GID_FANMADE,        -1,   994,  1,               "Game", "restore",   sig_kDeviceInfo_Fanmade_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games (SCI Studio), this is called with one parameter for CurDevice LDI 01 variant
	{ GID_FANMADE,        -1,   994,  1,               "Game", "restore",   sig_kDeviceInfo_Fanmade_2,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // In fanmade games (SCI Studio), this is called with one parameter for CurDevice LDI 0001 variant
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Police Quest 2
//      Calling method: rm23Script::elements
//   Subroutine offset: English 1.001.000: 0x04ae, English 1.002.011: 0x04ca, Japanese: 0x04eb (script 23)
// Applies to at least: English PC floppy, Japanese PC-9801
static const uint16 sig_kDisplay_pq2_1[] = {
	0x35, 0x00,                      // ldi 00
	0xa3, 0x09,                      // sal local[9]
	0x35, 0x01,                      // ldi 01
	0xa3, 0x0a,                      // sal local[0Ah]
	0x38, SIG_ADDTOOFFSET(+2),       // pushi selector[drawPic] TODO: implement selectors
	0x7a,                            // push2
	0x39, 0x5a,                      // pushi 5Ah
	0x7a,                            // push2
	0x81, 0x02,                      // lag global[2]
	0x4a, 0x08,                      // send 08
	SIG_END
};

//                Game: Space Quest 4
//      Calling method: doCatalog::mode
//   Subroutine offset: English PC CD: 0x0084 (script 391)
// Applies to at least: English PC CD
static const uint16 sig_kDisplay_sq4_1[] = {
	0x38, SIG_UINT16(0x0187),        // pushi 0187h (drawPic)
	0x78,                            // push1
	0x38, SIG_UINT16(0x0189),        // pushi 0189h (reflectPosn)
	0x81, 0x02,                      // lag global[2]
	0x4a, 0x06,                      // send 06
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDisplay_workarounds[] = {
	{ GID_ISLANDBRAIN,   300,   300,  0,           "geneDude", "show",                    nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // when looking at the gene explanation chart - a parameter is an object
	{ GID_LONGBOW,        95,    95,  0,          "countDown", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // DEMO: during title screen "Robin Hood! Your bow is needed"
	{ GID_LONGBOW,       220,   220,  0,             "moveOn", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // DEMO: during second room "Outwit and outfight..."
	{ GID_LONGBOW,       210,   210,  0,               "mama", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // DEMO: during third room "Fall under the spell..."
	{ GID_LONGBOW,       320,   320,  0,              "flyin", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // DEMO: during fourth room "Conspiracies, love..."
	{ GID_PQ2,            23,    23,  0,         "rm23Script", "handleEvent",  sig_kDisplay_pq2_1,     0,     0, { WORKAROUND_IGNORE,    0 } }, // when looking at the 2nd page of files in jail - 0x75 as id - bugs #5223, #9670
	{ GID_QFG1,           11,    11,  0,             "battle", "init",                    nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // DEMO: When entering battle, 0x75 as id
	{ GID_SQ3,             1,     1,  0,              "rm001", "handleEvent",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // MAC: during intro when pressing enter - a parameter is an object
	{ GID_SQ4,           397,     0,  0,                   "", "export 12",               nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // FLOPPY: when going into the computer store - bug #5227
	{ GID_SQ4,           391,   391,  0,          "doCatalog", "changeState",  sig_kDisplay_sq4_1,     0,     0, { WORKAROUND_IGNORE,    0 } }, // CD: clicking on catalog in roboter sale - a parameter is an object
	{ GID_SQ4,           391,   391,  0,         "choosePlug", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // CD: ordering connector in roboter sale - a parameter is an object
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDirLoop_workarounds[] = {
	{ GID_KQ4,             4,   992,  0,              "Avoid", "doit",                   nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // when the ogre catches you in front of his house, second parameter points to the same object as the first parameter, instead of being an integer (the angle) - bug #5217
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDisposeScript_workarounds[] = {
	{ GID_LAURABOW,      777,   777,  0,             "myStab", "changeState",            nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // DEMO: after the will is signed, parameter 0 is an object - bug #4967
	{ GID_LSL2,           -1,    54,  0,               "rm54", "dispose",                nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // Amiga: room 55, script tries to kDisposeScript an object (does not happen for DOS) - bug #6818
	{ GID_MOTHERGOOSEHIRES,37,  337,  0,        "rhymeScript", "changeState",            nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // after the rhyme with the king
	{ GID_QFG1,           -1,    64,  0,               "rm64", "dispose",                nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // when leaving graveyard, parameter 0 is an object
	{ GID_SQ4,            -1,   151,  0,        "fightScript", "dispose",                nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // during fight with Vohaul, parameter 0 is an object, happens in at least room 150
	{ GID_SQ4,            -1,   152,  0,       "driveCloseUp", "dispose",                nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // when choosing "beam download", parameter 0 is an object, may happen in room 150 and 900 (900 see bug #9812)
	{ GID_SQ4,           150,   152,  0,                   "", "dispose",                nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // when choosing "beam download"... in Russian version - bug #5573
	{ GID_SQ4,           900,   152,  0,                   "", "dispose",                nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // same as above, but for "game over" room
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDoAudioResume_workarounds[] = {
	{ GID_HOYLE5,         -1,    17,  0,              nullptr, "startAudio",             nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when a character talks during a game
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDoSoundPlay_workarounds[] = {
	{ GID_LSL6HIRES,    -1,  64989,   0,          nullptr,          "play",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // always passes an extra null argument
	{ GID_QFG4,         -1,  64989,   0,          nullptr,          "play",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // always passes an extra null argument
	{ GID_PQ4,          -1,  64989,   0,          nullptr,          "play",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // always passes an extra null argument
	{ GID_KQ7,          -1,  64989,   0,          nullptr,          "play",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // always passes an extra null argument
	{ GID_SQ6,          -1,      0,   0,          nullptr,          "play",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Demo passes an extra null argument on startup
	{ GID_GK1,          -1,  64989,   0,          nullptr,          "play",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Mac version always passes an extra null argument
	{ GID_GK2,          -1,  64989,   0,          nullptr,          "play",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Demo passes an extra null argument when clicking on buttons
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kDoSoundFade_workarounds[] = {
	{ GID_KQ5,           213,   989,  0,       "globalSound3", "fade",                   nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // english floppy: when bandits leave the secret temple, parameter 1 is an object followed by 3 non-existent parameters. Ignore; rideOut2 calls the same fade correctly at the same time - bug #5078
	{ GID_KQ6,           105,   989,  0,        "globalSound", "fade",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // floppy: during intro, parameter 4 is an object
	{ GID_KQ6,           460,   989,  0,       "globalSound2", "fade",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // after pulling the black widow's web on the isle of wonder, parameter 4 is an object - bug #4954
	{ GID_QFG4,           -1, 64989,  1,          "GlorySong", "fade",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // CD version: many places, parameter 4 is an object (the sound object itself)
	{ GID_SQ5,           800,   989,  0,          "sq5Music1", "fade",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when cutting the wrong part of Goliath with the laser, parameter 4 is an object - bug #6341
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGetAngle_workarounds[] = {
	{ GID_FANMADE,       516,   992,  0,             "Motion", "init",                   nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // The Legend of the Lost Jewel Demo (fan made): called with third/fourth parameters as objects
	{ GID_KQ6,            -1,   752,  0,        "throwDazzle", "changeState",            nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // room 740/790 after the Genie is exposed in the Palace (short and long ending), it starts shooting lightning bolts around. An extra 5th parameter is passed - bug #4959 & #5203
	{ GID_SQ1,            -1,   927,  0,           "PAvoider", "doit",                   nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // all rooms in Ulence Flats after getting the Pilot Droid: called with a single parameter when the droid is in Roger's path - bug #6016
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGetCWD_workarounds[] = {
	{ GID_LSL6,           -1,     0,  0,               "LSL6", "play",                   nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // Mac version passes uninitialized global (zero) on startup, then immediately overwrites it with kGetSaveDir
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGetSaveFiles_workarounds[] = {
	{ GID_KQ5,            -1,   764,  0,              "trash", "select",                 nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // FM-Towns version when clicking delete save button, save-catalog code passes buffers by value instead of address
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kFileIOOpen_workarounds[] = {
	{ GID_HOYLE5,         -1, 64990,  0,            "Restore", "doit",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Missing second argument when checking for bridgesg.cat or poker.cat when showing restore dialog
	{ GID_TORIN,       61000, 61000,  0,       "roSierraLogo", "init",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Missing second argument when the game checks for autosave.cat after the Sierra logo
	{ GID_TORIN,       61100, 61100,  0,     "roPickAChapter", "init",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Missing second argument when the game checks for autosave.cat after the Sierra logo in the demo
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Hoyle 5
//      Calling method: subroutine at offset 0x1074
//   Subroutine offset: 0x1432 (script 64990)
// Applies to at least: English PC
static const uint16 sig_kFileIOCheckFreeSpace_hoyle5_1[] = {
	0x7e, SIG_ADDTOOFFSET(2),        // line N
	0x7d, 0x73, 0x61, 0x76, 0x65,    // file "save.sc"
	0x2e, 0x73, 0x63, 0x00,
	0x3f, 0x02,                      // link 02
	0x7e, SIG_ADDTOOFFSET(2),        // line N
	0x38, SIG_UINT16(0x008d),        // pushi new
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name,              local-call-signature, index-range,   workaround
const SciWorkaroundEntry kFileIOCheckFreeSpace_workarounds[] = {
	{ GID_HOYLE5,         -1, 64990,  0,              "Save", "update",   sig_kFileIOCheckFreeSpace_hoyle5_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Extra argument when checking for free space when showing save dialog
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,              local-call-signature, index-range,   workaround
const SciWorkaroundEntry kFileIOReadString_workarounds[] = {
	{ GID_FANMADE,        -1,   800,  0,       "TitleScreen", "init",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // SCI Tetris 1.0: at start of game, broken high score script passes temp variable instead of reference to temp variable
	{ GID_FANMADE,        -1,   993,  0,           "hiscore", "read",                    nullptr,     0,     0, { WORKAROUND_FAKE,   0 } }, // SCI Tetris 1.1: at start of game, broken high score script passes temp variable instead of reference to temp variable
	{ GID_HOYLE5,         -1, 64993,  0,           "version", "readString",              nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // Zero passed as string when game initializes and VERSION file is present, which only Mac includes. Result is unused
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kFindKey_workarounds[] = {
	{ GID_ECOQUEST2,     100,   999,  0,            "myList", "contains",                nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // When Noah Greene gives Adam the Ecorder, and just before the game gives a demonstration, a null reference to a list is passed - bug #4987
	{ GID_HOYLE4,        300,   999,  0,             "Piles", "contains",                nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // When passing the three cards in Hearts, a null reference to a list is passed - bug #5664
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kFrameOut_workarounds[] = {
	{ GID_PQ4,           360,   360,  0,       "csFrontInset", "init",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When clicking hand on the impounded white car on day 4 - bug #9848
	{ GID_PQ4,           360,   360,  0,       "csFrontInset", "dispose",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When clicking hand on the impounded white car on day 4
	{ GID_PQ4,           360,   360,  0,        "copCarInset", "init",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When clicking hand on the impounded police car on day 3
	{ GID_PQ4,           360,   360,  0,        "copCarInset", "dispose",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When exiting the impounded police car on day 3
	{ GID_PQ4,           275,   275,  0,        "checkSherry", "changeState",            nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When encountering Sherry and Sam in the morgue on day 3
	{ GID_PQ4,           725,   725,  0,        "fridgeInset", "init",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When opening the refrigerator at the end of day 4
	{ GID_PQ4,           725,   725,  0,        "fridgeInset", "dispose",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When exiting the refrigerator at the end of day 4
	{ GID_PQ4,           735,   735,  0,           "medInset", "dispose",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 1 } }, // When exiting the medicine cabinet at the end of day 4
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGraphDrawLine_workarounds[] = {
	{ GID_ISLANDBRAIN,   300,   300,  0,         "dudeViewer", "show",                    nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when looking at the gene explanation chart, gets called with 1 extra parameter
	{ GID_SQ1,            43,    43,  0,        "someoneDied", "changeState",             nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when ordering beer, gets called with 1 extra parameter
	{ GID_SQ1,            71,    71,  0,       "destroyXenon", "changeState",             nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // during the Xenon destruction cutscene (which results in death), gets called with 1 extra parameter - bug #5176
	{ GID_SQ1,            53,    53,  0,           "blastEgo", "changeState",             nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when Roger is found and zapped by the cleaning robot, gets called with 1 extra parameter - bug #5177
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Island of Dr. Brain
//      Calling method: upElevator::changeState, downElevator::changeState, correctElevator::changeState
//   Subroutine offset: 0x201f (script 291)
// Applies to at least: English PC floppy
static const uint16 sig_kGraphSaveBox_ibrain_1[] = {
	0x3f, 0x01,                      // link 01
	0x87, 0x01,                      // lap param[1]
	0x30, SIG_UINT16(0x0043),        // bnt [...]
	0x76,                            // push0
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name,         local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGraphSaveBox_workarounds[] = {
	{ GID_CASTLEBRAIN,   420,   427,  0,          "alienIcon", "select",                         nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when selecting a card during the alien card game, gets called with 1 extra parameter
	{ GID_ISLANDBRAIN,   290,   291,  0,         "upElevator", "changeState", sig_kGraphSaveBox_ibrain_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when testing in the elevator puzzle, gets called with 1 argument less - 15 is on stack - bug #4943
	{ GID_ISLANDBRAIN,   290,   291,  0,       "downElevator", "changeState", sig_kGraphSaveBox_ibrain_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // see above
	{ GID_ISLANDBRAIN,   290,   291,  0,    "correctElevator", "changeState", sig_kGraphSaveBox_ibrain_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // see above (when testing the correct solution)
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "movePt",                         nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters - bug #5099
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGraphRestoreBox_workarounds[] = {
	{ GID_LSL6,           -1,    86,  0,             "LL6Inv", "hide",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // happens during the game, gets called with 1 extra parameter
	{ GID_MOTHERGOOSE256, -1,    90,  0,        "introScript", "changeState",            nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // happens when restoring after completing a game in SCI1.1 floppy, 2nd parameter is an object from previous game
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,   local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGraphFillBoxForeground_workarounds[] = {
	{ GID_LSL6,           -1,     0,  0,               "LSL6", "hideControls",             nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // happens when giving the bungee key to merrily (room 240) and at least in room 650 too - gets called with additional 5th parameter
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGraphFillBoxAny_workarounds[] = {
	{ GID_SQ4,            -1,   818,  0,     "iconTextSwitch", "show",                    nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // CD: game menu "text/speech" display - parameter 5 is missing, but the right color number is on the stack
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Space Quest 4
//      Calling method: laserScript::changeState
//   Subroutine offset: English/German/French/Russian PC floppy, Japanese PC-9801: 0x0016, English PC CD: 0x00b2 (script 150)
// Applies to at least: English/German/French/Russian PC floppy, English PC CD, Japanese PC-9801
static const uint16 sig_kGraphRedrawBox_sq4_1[] = {
	0x3f, 0x07,                      // link 07
	0x39, SIG_ADDTOOFFSET(+1),       // pushi 2Ah for PC floppy, pushi 27h for PC CD
	0x76,                            // push0
	0x72,                            // lofsa laserSound
	SIG_END
};

// same as above but for NRS SQ4 Update, which has a different first instruction
static const uint16 sig_kGraphRedrawBox_sq4_2[] = {
	0x3e, SIG_UINT16(0x0007),        // link 0007
	0x39, SIG_ADDTOOFFSET(+1),       // pushi 2Ah for PC floppy, pushi 27h for PC CD
	0x76,                            // push0
	0x72,                            // lofsa laserSound
	SIG_END
};

//                Game: Space Quest 4
//      Calling method: shootEgoScript::changeState
//   Subroutine offset: English/German/French/Russian PC floppy, Japanese PC-9801: 0x0f8c, English PC CD: 0x0c4d (script 703)
// Applies to at least: English/German/French/Russian PC floppy, English PC CD, Japanese PC-9801
static const uint16 sig_kGraphRedrawBox_sq4_3[] = {
	0x3f, 0x03,                      // link 03
	0x39, SIG_ADDTOOFFSET(+1),       // pushi [ number ]
	0x78,                            // push1
	0x39, 0x69,                      // pushi 69h
	SIG_END
};

// same as above but for NRS SQ4 Update, which has a different first instruction
static const uint16 sig_kGraphRedrawBox_sq4_4[] = {
	0x3e, SIG_UINT16(0x0003),        // link 0003
	0x39, SIG_ADDTOOFFSET(+1),       // pushi [ number ]
	0x78,                            // push1
	0x39, 0x69,                      // pushi 69h
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name,        local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGraphRedrawBox_workarounds[] = {
	{ GID_SQ4,           405,   405,  0,       "swimAfterEgo", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,           405,   405,  0,                   "", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air... Russian version - bug #5573
	{ GID_SQ4,           406,   406,  0,        "egoFollowed", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when getting shot by the police - accidental additional parameter specified
	{ GID_SQ4,            -1,   406,  0,       "swimAndShoot", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,            -1,   406,  0,                   "", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air... Russian version - bug #5573 (is for both egoFollowed and swimAndShoot)
	{ GID_SQ4,           410,   410,  0,       "swimAfterEgo", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,           410,   410,  0,                   "", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air... Russian version - bug #5573
	{ GID_SQ4,           411,   411,  0,        "egoFollowed", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when getting shot by the police - accidental additional parameter specified
	{ GID_SQ4,            -1,   411,  0,       "swimAndShoot", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air - accidental additional parameter specified
	{ GID_SQ4,            -1,   411,  0,                   "", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // skateOrama when "swimming" in the air... Russian version - bug #5573
	{ GID_SQ4,           150,   150,  0,        "laserScript", "changeState", sig_kGraphRedrawBox_sq4_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when visiting the pedestral where Roger Jr. is trapped, before trashing the brain icon in the programming chapter, accidental additional parameter specified - bug #5479, German - bug #5527
	{ GID_SQ4,           150,   150,  0,                   "", "changeState", sig_kGraphRedrawBox_sq4_1,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // same as above, for the Russian version - bug #5573
	{ GID_SQ4,           150,   150,  0,        "laserScript", "changeState", sig_kGraphRedrawBox_sq4_2,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // same as above, for NRS SQ4 Update
	{ GID_SQ4,            -1,   703,  0,     "shootEgoScript", "changeState", sig_kGraphRedrawBox_sq4_3,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // estros when getting shot by the police - accidental additional parameter specified
	{ GID_SQ4,            -1,   703,  0,                   "", "changeState", sig_kGraphRedrawBox_sq4_3,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // same as above, for the Russian version
	{ GID_SQ4,            -1,   703,  0,     "shootEgoScript", "changeState", sig_kGraphRedrawBox_sq4_4,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // same as above, for NRS SQ4 Update
	{ GID_SQ4,            -1,   704,  0,           "shootEgo", "changeState",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // When shot by Droid in Super Computer Maze (Rooms 500, 505, 510...) - accidental additional parameter specified
	{ GID_KQ5,            -1,   981,  0,           "myWindow",     "dispose",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the floppy version, when closing any dialog box, accidental additional parameter specified - bug #5031
	{ GID_KQ5,            -1,   995,  0,               "invW",        "doit",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the floppy version, when closing the inventory window, accidental additional parameter specified
	{ GID_KQ5,            -1,   995,  0,                   "",    "export 0",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the floppy version, when opening the gem pouch, accidental additional parameter specified - bug #5138
	{ GID_KQ5,            -1,   403,  0,          "KQ5Window",     "dispose",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Happens in the FM Towns version when closing any dialog box, accidental additional parameter specified
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kGraphUpdateBox_workarounds[] = {
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "addPt",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters - bug #5099
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "movePt",                  nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters - bug #5099
	{ GID_PQ3,           202,   202,  0,            "MapEdit", "dispose",                 nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // when plotting crimes, gets called with 2 extra parameters
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    local-call-signature, index-range,   workaround
const SciWorkaroundEntry kIsObject_workarounds[] = {
	{ GID_GK1DEMO,       50,   999,  0,                "List", "eachElementDo",             nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // GK1 demo, when asking Grace for messages it gets called with an invalid parameter (type "error") - bug #4950
	{ GID_ISLANDBRAIN,   -1,   999,  0,                "List", "eachElementDo",             nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // when going to the game options, choosing "Info" and selecting anything from the list, gets called with an invalid parameter (type "error") - bug #4989
	{ GID_QFG3,          -1,   999,  0,                "List", "eachElementDo",             nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // when asking for something, gets called with type error parameter
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,    local-call-signature, index-range,   workaround
const SciWorkaroundEntry kListAt_workarounds[] = {
	{ GID_HOYLE5,        100, 64999,  0,           "theHands", "at",                        nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // After the first hand is dealt in Crazy Eights game in demo, an object is passed instead of a number
	{ GID_LIGHTHOUSE,     24, 64999,  0,           "LightInv", "at",                        nullptr,     0,     0, { WORKAROUND_FAKE, 0 } }, // When taking the car keys from the table at the start of the game
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kLock_workarounds[] = {
	{ GID_PQSWAT,       6400,  6400,  0,      "rookerTrains3", "changeState",               nullptr,     0,     0, { WORKAROUND_STILLCALL,    0 } }, // After completing Small Arms Training, at the end of the Tactics Training
	{ GID_PQSWAT,       6400,  6400,  0,      "rookerTrains5", "changeState",               nullptr,     0,     0, { WORKAROUND_STILLCALL,    0 } }, // After completing Small Arms Training, at the end of the Tactics Training
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kMemory_workarounds[] = {
	{ GID_LAURABOW2,      -1,   999,  0,                   "", "export 6",               nullptr,     0,     0, { WORKAROUND_FAKE,    0 } }, // during the intro, when exiting the train (room 160), talking to Mr. Augustini, etc. - bug #4944
	{ GID_SQ1,            -1,   999,  0,                   "", "export 6",               nullptr,     0,     0, { WORKAROUND_FAKE,    0 } }, // during walking Roger around Ulence Flats - bug #6017
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kMoveCursor_workarounds[] = {
	{ GID_KQ5,            -1,   937,  0,            "IconBar", "handleEvent",             nullptr,     0,     0, { WORKAROUND_IGNORE,  0 } }, // when pressing escape to open the menu, gets called with one parameter instead of 2 - bug #5575
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kNewWindow_workarounds[] = {
	{ GID_ECOQUEST,       -1,   981,  0,          "SysWindow", "open",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // EcoQuest 1 demo uses an in-between interpreter from SCI1 to SCI1.1. It's SCI1.1, but uses the SCI1 semantics for this call - bug #4976
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kPalVarySetVary_workarounds[] = {
	{ GID_KQ7,          4600,  4600,  0,      "sRosDogDeath",  "changeState",            nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // when dying under Malicia's house, gets called with no parameters (bug #9763)
	{ GID_KQ7,          4600,  4600,  0,      "sRosDogDeath2", "changeState",            nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // when dying under Malicia's house, gets called with no parameters (bug #9763)
	{ GID_KQ7,          4600,  4600,  0,      "sValDogDeath",  "changeState",            nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // when dying under Malicia's house, gets called with no parameters (bug #9763)
	{ GID_KQ7,          4600,  4600,  0,      "sZapVal",       "changeState",            nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // when dying under Malicia's house, gets called with no parameters (bug #9763)
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kPalVarySetPercent_workarounds[] = {
	{ GID_GK1,           370,   370,  0,        "graceComeOut", "changeState",           nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // there's an extra parameter in GK1, when changing chapters. This extra parameter seems to be a bug or just unimplemented functionality, as there's no visible change from the original in the chapter change room
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kPalVarySetStart_workarounds[] = {
	{ GID_PQ4,           170,   170,  0,             "getHit", "changeState",            nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Three extra parameters passed during the gunfight at the end of day 1
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kPalVaryMergeStart_workarounds[] = {
	{ GID_PQ4,           170,   170,  0,             "getHit", "changeState",            nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // Three extra parameters passed during the gunfight at the end of day 1
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kPlatform32_workarounds[] = {
	{ GID_HOYLE5,         -1,     0,  0,             "hoyle4", "newRoom",                nullptr,     0,     0, { WORKAROUND_FAKE,      1 } }, // at the start of the game, incorrectly uses SCI16 calling convention for kPlatform
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kRandom_workarounds[] = {
	{ GID_TORIN,       51400, 64928,  0,              "Blink", "init",                   nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // at the end of the game, during the cutscene after touching the collar on Lycentia; Trac#9779
	{ GID_TORIN,       51400, 64928,  0,              "Blink", "cycleDone",              nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // at the end of the game, during the cutscene after touching the collar on Lycentia; Trac#9779
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kReadNumber_workarounds[] = {
	{ GID_CNICK_LAURABOW,100,   101,  0,          "dominoes.opt", "doit",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // When dominoes.opt is present, the game scripts call kReadNumber with an extra integer parameter - bug #6425
	{ GID_HOYLE3,        100,   101,  0,          "dominoes.opt", "doit",                nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // When dominoes.opt is present, the game scripts call kReadNumber with an extra integer parameter - bug #6425
	{ GID_FANMADE,        -1,     0,  0,                "Tetris", "init",                nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // SCI Tetris 1.1: at start of game, broken high score script passes temp variable instead of reference to temp variable
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kPaletteUnsetFlag_workarounds[] = {
	{ GID_QFG4DEMO,      100,   100,  0,            "doMovie", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // after the Sierra logo, no flags are passed, thus the call is meaningless - bug #4947
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kSetCursor_workarounds[] = {
	{ GID_KQ5,            -1,   768,  0,           "KQCursor", "init",                   nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // CD: gets called with 4 additional "900d" parameters
	{ GID_MOTHERGOOSEHIRES,-1,    0, -1,                 "MG", "setCursor",              nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // At the start of the game, an object is passed as the cel number
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kSetPort_workarounds[] = {
	{ GID_LSL6,          740,   740,  0,              "rm740", "drawPic",                 nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // ending scene, is called with additional 3 (!) parameters
	{ GID_QFG3,          830,   830,  0,        "portalOpens", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE,    0 } }, // when the portal appears during the end, gets called with 4 parameters - bug #5174
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Island of Dr. Brain
//      Calling method: childBreed::changeState
//   Subroutine offset: 0x1c7c (script 310)
// Applies to at least: English PC floppy
static const uint16 sig_kStrAt_ibrain_1[] = {
	0x3f, 0x16,                      // link 16
	0x78,                            // push1
	0x8f, 0x01,                      // lsp param[1]
	0x43,                            // callk StrLen
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kStrAt_workarounds[] = {
	{ GID_CASTLEBRAIN,   220,   220,  0,         "robotJokes", "animateOnce",             nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // when trying to view the terminal at the end of the maze without having collected any robot jokes - bug #5127
	{ GID_ISLANDBRAIN,   300,   310,  0,         "childBreed", "changeState", sig_kStrAt_ibrain_1,     0,     0, { WORKAROUND_FAKE,      0 } }, // when clicking Breed to get the second-generation cyborg hybrid (Standard difficulty), the two parameters are swapped - bug #5088
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,   local-call-signature, index-range,   workaround
const SciWorkaroundEntry kStrCpy_workarounds[] = {
	{ GID_MOTHERGOOSE,    23,    23,  0,          "talkScript", "changeState",             nullptr,     0,     0, { WORKAROUND_FAKE,      0 } }, // when talking to the girl in scene 23, there's no destination parameter (script bug - wrong instruction order). The original source is used directly afterwards in kDisplay, to show the girl's text - bug #6485
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//                Game: Quest for Glory 2
//      Calling method: export 21 of script 2
//   Subroutine offset: English 0x0deb (script 2)
// Applies to at least: English PC floppy
static const uint16 sig_kStrLen_qfg2_1[] = {
	0x3f, 0x04,                      // link 04
	0x78,                            // push1
	0x8f, 0x02,                      // lsp param[2]
	0x43,                            // callk StrLen
	SIG_END
};

//    gameID,           room,script,lvl,          object-name, method-name, local-call-signature, index-range,   workaround
const SciWorkaroundEntry kStrLen_workarounds[] = {
	{ GID_QFG2,          210,     2,  0,                   "", "export 21",   sig_kStrLen_qfg2_1,     0,     0, { WORKAROUND_FAKE,      0 } }, // When saying something incorrect at the WIT, an integer is passed instead of a reference - bug #5489
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kUnLoad_workarounds[] = {
	{ GID_ECOQUEST,      380,    61,  0,              "gotIt", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // CD version: after talking to the dolphin the first time, a 3rd parameter is passed by accident
	{ GID_ECOQUEST,      380,    69,  0,   "lookAtBlackBoard", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // German version, when closing the blackboard closeup in the dolphin room, a 3rd parameter is passed by accident - bug #5483
	{ GID_LAURABOW2,      -1,    -1,  0,           "sCartoon", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // DEMO: during the intro, a 3rd parameter is passed by accident - bug #4966
	{ GID_LSL6,          130,   130,  0,    "recruitLarryScr", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // during intro, a 3rd parameter is passed by accident
	{ GID_LSL6,          740,   740,  0,        "showCartoon", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // during ending, 4 additional parameters are passed by accident
	{ GID_LSL6HIRES,     740,   740,  0,        "showCartoon", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // during ending, multiple additional parameters are passed by accident
	{ GID_LSL6HIRES,     130,   130,  0,    "recruitLarryScr", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // during intro, a 3rd parameter is passed by accident
	{ GID_SQ1,            43,   303,  0,            "slotGuy", "dispose",                 nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // when leaving ulence flats bar, parameter 1 is not passed - script error
	{ GID_QFG4,           -1,   110,  0,            "dreamer", "dispose",                 nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // during the dream sequence, a 3rd parameter is passed by accident
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kWait_workarounds[] = {
	{ GID_SQ1,             1,     1,  0,              "demo1", "changeState",             nullptr,     0,     0, { WORKAROUND_IGNORE, 0 } }, // DEMO: calls kWait with no parameters
	SCI_WORKAROUNDENTRY_TERMINATOR
};

//    gameID,           room,script,lvl,          object-name, method-name,  local-call-signature, index-range,   workaround
const SciWorkaroundEntry kScrollWindowAdd_workarounds[] = {
	{ GID_PHANTASMAGORIA, 45, 64907,  0,   "ScrollableWindow", "addString",               nullptr,     0,     0, { WORKAROUND_STILLCALL, 0 } }, // ScrollWindow interface passes the last two parameters twice
	SCI_WORKAROUNDENTRY_TERMINATOR
};

SciWorkaroundSolution trackOriginAndFindWorkaround(int index, const SciWorkaroundEntry *workaroundList, SciCallOrigin *trackOrigin) {
	const EngineState *state = g_sci->getEngineState();
	const ExecStack *lastCall = state->xs;
	const SciGameId gameId = g_sci->getGameId();

	*trackOrigin = state->getCurrentCallOrigin();
	const Common::String &curObjectName = trackOrigin->objectName;
	const Common::String &curMethodName = trackOrigin->methodName;
	const int &curRoomNumber = trackOrigin->roomNr;
	const int &curScriptNr = trackOrigin->scriptNr;
	const int &curLocalCallOffset = trackOrigin->localCallOffset;

	if (workaroundList) {
		// Search if there is a workaround for this one
		int16 inheritanceLevel = 0;
		Common::String searchObjectName = g_sci->getSciLanguageString(curObjectName, K_LANG_ENGLISH);
		reg_t searchObject = lastCall->sendp;
		const byte *curScriptPtr = nullptr;
		uint32 curScriptSize = 0;
		bool matched = false;

		do {
			const SciWorkaroundEntry *workaround = workaroundList;
			while (workaround->methodName) {
				bool objectNameMatches = (workaround->objectName == nullptr) ||
										 (workaround->objectName == searchObjectName);

				if (((workaround->gameId == GID_ALL) || (workaround->gameId == gameId))
						&& ((workaround->scriptNr == -1) || (workaround->scriptNr == curScriptNr))
						&& ((workaround->roomNr == -1) || (workaround->roomNr == curRoomNumber))
						&& ((workaround->inheritanceLevel == -1) || (workaround->inheritanceLevel == inheritanceLevel))
						&& (objectNameMatches || !g_sci->_features->hasScriptObjectNames())
						&& workaround->methodName == g_sci->getSciLanguageString(curMethodName, K_LANG_ENGLISH)
						&& ((workaround->fromIndex == -1) || ((workaround->fromIndex <= index) && (workaround->toIndex >= index)))) {
					// Workaround found
					if ((workaround->localCallSignature) || (curLocalCallOffset >= 0)) {
						// local call signature found and/or subcall was made
						if ((workaround->localCallSignature) && (curLocalCallOffset >= 0)) {
							// local call signature found and subcall was made -> check signature accordingly
							if (!curScriptPtr) {
								// get script data
								int segmentId = g_sci->getEngineState()->_segMan->getScriptSegment(curScriptNr);
								SegmentObj *segmentObj = nullptr;
								if (segmentId) {
									segmentObj = g_sci->getEngineState()->_segMan->getScriptIfLoaded(segmentId);
								}
								if (!segmentObj) {
									workaround++;
									continue;
								}
								Script *scriptObj = (Script *)segmentObj;
								curScriptPtr = scriptObj->getBuf();
								curScriptSize = scriptObj->getScriptSize();
							}

							// now actually check for signature match
							if (g_sci->getScriptPatcher()->verifySignature(curLocalCallOffset, workaround->localCallSignature, "workaround signature", SciSpan<const byte>(curScriptPtr, curScriptSize))) {
								matched = true;
							}

						} else {
							// mismatch, so workaround doesn't match
							workaround++;
							continue;
						}
					} else {
						// no localcalls involved -> workaround matches
						matched = true;
					}
					if (matched) {
						debugC(kDebugLevelWorkarounds, "Workaround: '%s:%s' in script %d, localcall %x", workaround->objectName, workaround->methodName, curScriptNr, curLocalCallOffset);
						return workaround->newValue;
					}
				}
				workaround++;
			}

			// Go back to the parent
			inheritanceLevel++;
			searchObject = state->_segMan->getObject(searchObject)->getSuperClassSelector();
			if (!searchObject.isNull())
				searchObjectName = state->_segMan->getObjectName(searchObject);
		} while (!searchObject.isNull()); // no parent left?
	}

	SciWorkaroundSolution noneFound;
	noneFound.type = WORKAROUND_NONE;
	noneFound.value = 0;
	return noneFound;
}

// Workarounds for known broken messages
//
// The most common message bug is a script passing the wrong tuple or a message
//  having the wrong tuple. This results in a "missing message" message. These
//  requests just need to be remapped to the right message. In some cases the
//  message text is incorrect, or missing, in which case it can be extracted
//  from another record or faked with a hard-coded response.
//
// Workarounds can be optionally scoped to media (floppy vs cd), language, and
//  room number. If a message is remapped, this will also remap any audio36 and
//  sync36 resources with the same tuple, unless those resources have their own
//  remapping workarounds due to their tuples being out of sync with their
//  already broken messages, which we've seen twice.
//
// The one kind of broken message that we don't handle yet is when the audio is
//  completely missing from the game. QFG4 has most of those. In speech-only
//  mode this means no audio or text and in dual mode the text disappears
//  quickly as most games use audio length for the message delay. That logic is
//  in each game's Messager/Narrator/Talker scripts.
static const SciMessageWorkaroundEntry messageWorkarounds[] = {
	// game              media             language       room   mod    n    v    c   s    workaround-type          mod    n    v    c   s tlk  idx  len  text
	// Clicking Do on bulldozer at camp after Gonzales leaves. Message has wrong cond.
	{ GID_ECOQUEST2,     SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  560,   1,   4,   0,  1, { MSG_WORKAROUND_REMAP,    560,   1,   4,   4,  1, 99,   0,   0, nullptr } },
	// FPFP CD has several message sequences where audio and text were left out of sync - bug #10964
	//  Some of the texts just say "Dummy Msg" and the real values are concatenated in the first record.
	// Lever Brothers' intro
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  1, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99,   0,  25, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  2, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99,  26,  20, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  3, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99,  47,  58, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  4, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99, 106,  34, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  5, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99, 141,  27, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  6, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99, 169,  29, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  7, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99, 199,  52, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  24,   0,   0,  8, { MSG_WORKAROUND_EXTRACT,  220,  24,   0,   0,  1, 99, 252,  37, nullptr } },
	// Kenny's intro
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  30,   0,   0,  3, { MSG_WORKAROUND_EXTRACT,  220,  30,   0,   0,  3, 99,   0,  14, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  30,   0,   0,  4, { MSG_WORKAROUND_EXTRACT,  220,  30,   0,   0,  3, 99,  15, 245, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  220,  30,   0,   0,  5, { MSG_WORKAROUND_EXTRACT,  220,  30,   0,   0,  4, 99,   0,   0, nullptr } },
	// Helen swatting flies
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  660,  35,   0,   0,  1, { MSG_WORKAROUND_EXTRACT,  660,  35,   0,   0,  1, 53,   0,  42, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  660,  35,   0,   0,  2, { MSG_WORKAROUND_EXTRACT,  660,  35,   0,   0,  1, 53,  43,  93, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  660,  35,   0,   0,  3, { MSG_WORKAROUND_EXTRACT,  660,  35,   0,   0,  1, 53, 137,  72, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  660,  35,   0,   0,  4, { MSG_WORKAROUND_EXTRACT,  660,  35,   0,   0,  2, 53,   0,   0, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  660,  35,   0,   0,  5, { MSG_WORKAROUND_EXTRACT,  660,  35,   0,   0,  1, 53, 210,  57, nullptr } },
	{ GID_FREDDYPHARKAS, SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  660,  35,   0,   0,  6, { MSG_WORKAROUND_EXTRACT,  660,  35,   0,   0,  3, 12,   0,   0, nullptr } },
	// Wrong talker when clicking guns on Srini in room 300
	{ GID_FREDDYPHARKAS, SCI_MEDIA_FLOPPY, K_LANG_NONE,     -1,  300,  17,  47,   0,  1, { MSG_WORKAROUND_EXTRACT,  300,  17,  47,   0,  1, 40,   0,   0, nullptr } },
	// Missing message when clicking full bottles of water on spigot in room 320
	{ GID_FREDDYPHARKAS, SCI_MEDIA_FLOPPY, K_LANG_NONE,     -1,  320,  16,  34,   7,  1, { MSG_WORKAROUND_REMAP,    320,  37,  34,   7,  1,  0,   0,   0, nullptr } },
	// Missing message when clicking flashlight and other items on Magentia in room 290, floppy 1.0 - bug #10782
	{ GID_GK1,           SCI_MEDIA_FLOPPY, K_LANG_NONE,     -1,  290,   8,   0,  35,  1, { MSG_WORKAROUND_REMAP,    290,   8,   0,   0,  1,  0,   0,   0, nullptr } },
	// Missing message when clicking photocopy of the veve on the artist after giving sketch and original veve in room 410 - bug #10818
	{ GID_GK1,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  410,   3,  48,  31,  1, { MSG_WORKAROUND_REMAP,    410,   3,  24,  31,  1,  0,   0,   0, nullptr } },
	// Missing message when clicking operate on Loreli's right chair in room 420, floppy 1.0 - bug #10820
	{ GID_GK1,           SCI_MEDIA_FLOPPY, K_LANG_NONE,     -1,  420,   4,   8,   3,  1, { MSG_WORKAROUND_REMAP,    420,   4,   8,   7,  1,  0,   0,   0, nullptr } },
	// Clicking money on Loreli when sitting or dancing in room 420 - bug #10819
	//  The script transposes sitting vs dancing responses, passes an invalid cond for one of them, and the
	//  audio36 for the other has the wrong tuple, which we fix in the audio36 workarounds.
	{ GID_GK1,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  420,   2,  32,   3,  1, { MSG_WORKAROUND_REMAP,    420,   2,  32,   0,  1,  0,   0,   0, nullptr } },
	{ GID_GK1,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  420,   2,  32,   0,  1, { MSG_WORKAROUND_REMAP,    420,   2,  32,   2,  1,  0,   0,   0, nullptr } },
	// Clicking one of Gabriel's letters on Gerde in room 120 after getting his address in some versions
	{ GID_GK2,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  120,  18,  63,   0,  1, { MSG_WORKAROUND_REMAP,    120,  18,  44,   0,  1,  0,   0,   0, nullptr } },
	{ GID_GK2,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  120,  18,  64,   0,  1, { MSG_WORKAROUND_REMAP,    120,  18,  44,   0,  1,  0,   0,   0, nullptr } },
	// Clicking any item other than the dagger on theater vent in room 11853
	{ GID_GK2,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1, 1185,   4,   0,   0,  1, { MSG_WORKAROUND_REMAP,   1185,   4,  62,   0,  1,  0,   0,   0, nullptr } },
	// During the common Game Over cutscene, the "Ticket, please!" message has the incorrect audio
	//  "Tickets only!", which is the response the skeleton gives when talking to him in the land
	//  of the dead. Normally people wouldn't have noticed, but ScummVM supports audio + subtitles
	//  in this game at the same time. We could fix this either way because there are non-buggy
	//  versions of each message. "Ticket, please!" is the correct message but "Tickets only!"
	//  is the audio that everyone is used to, so we make the text match "Tickets only!" on CD.
	{ GID_KQ6,           SCI_MEDIA_CD,     K_LANG_NONE,     -1,  640,   1,   0,   1,  1, { MSG_WORKAROUND_REMAP,    640,   5,   0,   0,  2, 83,   0,   0, nullptr } },
	// Clicking the drink-me potion on ego in the castle basement hallways while guards are around
	{ GID_KQ6,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  840,   3,  14,   1,  1, { MSG_WORKAROUND_REMAP,    899,   0,   0, 198,  1, 99,   0,   0, nullptr } },
	{ GID_KQ6,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  899,   1,  14,   1,  1, { MSG_WORKAROUND_REMAP,    899,   0,   0, 198,  1, 99,   0,   0, nullptr } },
	// Clicking Do on the horse on the wall in room 870; the message resource has the wrong verb.
	{ GID_KQ6,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  870,  12,   5,   0,  1, { MSG_WORKAROUND_REMAP,    870,  12,  75,   0,  1, 99,   0,   0, nullptr } },
	// "Tips for playing King's Quest VI" displays a message that's too long to display on the screen
	// with Macintosh fonts and causes the graphics code to crash. In the original this "worked" because
	// of a script bug that truncated all CD and Mac messages to 400 characters, even though KQ6 has
	// several important messages that are much longer. We fix the truncation bug with a script patch,
	// but this one message needs to remain shorter.
	{ GID_KQ6,           SCI_MEDIA_MAC,    K_LANG_NONE,     -1,  908,   0,   0,  16, 27, { MSG_WORKAROUND_EXTRACT,  908,   0,   0,  16, 27, 99,   0, 466, nullptr } },
	// Asking Yvette about Tut in act 2 party in floppy version - bug #10723
	//  The last two sequences in this five part message reveal a murder that hasn't occurred yet.
	//  We skip these as to not spoil the plot, but only in the act 2 rooms, as the message is used
	//  in later acts where all five parts are appropriate. Sierra fixed this in the CD version by
	//  creating a new three-sequence message for act 2 to accomplish the same thing.
	{ GID_LAURABOW2,     SCI_MEDIA_FLOPPY, K_LANG_NONE,    350, 1885,   1,   6,  16,  4, { MSG_WORKAROUND_REMAP,   1885,   1,   6,  16,  6,  0,   0,   0, nullptr } },
	{ GID_LAURABOW2,     SCI_MEDIA_FLOPPY, K_LANG_NONE,    360, 1885,   1,   6,  16,  4, { MSG_WORKAROUND_REMAP,   1885,   1,   6,  16,  6,  0,   0,   0, nullptr } },
	{ GID_LAURABOW2,     SCI_MEDIA_FLOPPY, K_LANG_NONE,    370, 1885,   1,   6,  16,  4, { MSG_WORKAROUND_REMAP,   1885,   1,   6,  16,  6,  0,   0,   0, nullptr } },
	// Missing message when clicking carbon paper on desk lamp in room 550, floppy 1.0 - bug #10706
	{ GID_LAURABOW2,     SCI_MEDIA_FLOPPY, K_LANG_NONE,     -1,  550,   5,  39,   6,  1, { MSG_WORKAROUND_REMAP,    550,  45,  39,   6,  1,  0,   0,   0, nullptr } },
	// Looking at coal in room 720, message is prepended with carriage return and newline
	{ GID_LAURABOW2,     SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  720,  12,   1,   0,  1, { MSG_WORKAROUND_EXTRACT,  720,  12,   1,   0,  1, 99,   2, 255, nullptr } },
	// Asking Olympia about Pippin Carter during Act 2 in German version - bug #5710
	//  This message contains over seventy trailing newlines, causing the graphics code to
	//  draw a window larger than the screen and crash. We trim the trailing newlines.
	{ GID_LAURABOW2,     SCI_MEDIA_FLOPPY, K_LANG_GERMAN,   -1, 1892,   1,   6,   3,  1, { MSG_WORKAROUND_EXTRACT, 1892,   1,   6,   3,  1, 25,   0, 249, nullptr } },
	// Asking Ramses about snake oil in German version. Same as above: excessive trailing newlines.
	{ GID_LAURABOW2,     SCI_MEDIA_FLOPPY, K_LANG_GERMAN,   -1, 1891,   1,   6,  41,  1, { MSG_WORKAROUND_EXTRACT, 1891,   1,   6,  41,  1, 27,   0, 146, nullptr } },
	// Using the game's debugger and clicking "Museum", wrong resource number for "T. Rex Room"
	{ GID_LAURABOW2,     SCI_MEDIA_ALL,    K_LANG_NONE,     29,    0,   2,   0,   0, 16, { MSG_WORKAROUND_REMAP,     10,   2,   0,   0, 16, 97,   0,   0, nullptr } },
	// Using the hand icon on Keith in the Blue Room (missing message) - bug #6253
	{ GID_PQ1,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,   38,  10,   4,   8,  1, { MSG_WORKAROUND_REMAP,     38,  10,   4,   9,  1,  0,   0,   0, nullptr } },
	// Using the eye icon on Keith in the Blue Room (no message and wrong talker) - bug #6253
	{ GID_PQ1,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,   38,  10,   1,   0,  1, { MSG_WORKAROUND_EXTRACT,   38,  10,   1,  13,  1, 99,   0,   0, nullptr } },
	// Talking to Kaspar the shopkeeper - bug #6250
	{ GID_QFG1VGA,       SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  322,  14,   1,  19,  1, { MSG_WORKAROUND_REMAP,    322,  14,   2,  19,  1,  0,   0,   0, nullptr } },
	// Remove the "a:" floppy drive prefix from the default character export
	//  filename "a:shadows.sav". We do this for SCI16 games in kDrawControl.
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_NONE,     -1,   52,   2,   0,  10,  1, { MSG_WORKAROUND_EXTRACT,   52,   2,   0,  10,  1, 99,   2,  11, nullptr } },
	// Talking with the Leshy and telling him about "bush in goo" - bug #10137
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  579,   0,   0,   0,  1, { MSG_WORKAROUND_REMAP,    579,   0,   1,   0,  1,  0,   0,   0, nullptr } },
	// Examining the statue inventory item from the monastery - bug #10770
	// The description says "squid-like monster", yet the icon is
	// clearly an insect. It turned Chief into "an enormous beetle". We
	// change the phrase to "monstrous insect". This message is text-only.
	// Note: The German string contains accented characters.
	//  0x84 "a with diaeresis"
	//  0x94 "o with diaeresis"
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_ENGLISH,  -1,   16,  49,   1,   0,  2, { MSG_WORKAROUND_FAKE,      16,  49,   1,   0,  2, 99,   0,   0, "Carefully wrapped in a shopping bag is the grotesque sculpture of a horrible, monstrous insect." } },
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_GERMAN,   -1,   16,  49,   1,   0,  2, { MSG_WORKAROUND_FAKE,      16,  49,   1,   0,  2, 99,   0,   0, "Die groteske Skulptur eines schrecklichen, monstr\x94sen insekts ist sorgf\x84ltig in die Einkaufstasche eingewickelt." } },
	// The CD edition mangled the Rusalka flowers dialogue. - bug #10849
	// In the floppy edition, there are 3 lines, the first from
	// the narrator, then two from Rusalka. The CD edition omits
	// narration and only has the 3rd text, with the 2nd audio! The
	// 3rd audio is orphaned but available.
	// We only restore Rusalka's lines, providing the correct text
	// for seq:1 to match the audio. We respond to seq:2 requests
	// with Rusalka's last text. The orphaned audio (seq:3) has its
	// tuple remapped to seq:2 in an audio workaround below.
	{ GID_QFG4,          SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  520,   2,  59,   0,  1, { MSG_WORKAROUND_FAKE,     520,   2,  59,   0,  1, 28,   0,   0, "Thank you for the beautiful flowers.  No one has been so nice to me since I can remember." } },
	{ GID_QFG4,          SCI_MEDIA_CD,     K_LANG_ENGLISH,  -1,  520,   2,  59,   0,  2, { MSG_WORKAROUND_EXTRACT,  520,   2,  59,   0,  1, 28,   0,   0, nullptr } },
	// Wrong talker when asking gnome about himself in room 320 - bug #11030.
	//  This incorrect value caused the narrator to accidentally voice these
	//  messages in the CD version instead of the actor who played the gnome.
	//  There's nothing we can do about that but we can at least set the correct
	//  talker so that the message boxes appear in the correct screen location.
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  322,  10, 149,   1,  1, { MSG_WORKAROUND_EXTRACT,  322,  10, 149,   1,  1, 13,   0,   0, nullptr } },
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  322,  10, 149,   1,  2, { MSG_WORKAROUND_EXTRACT,  322,  10, 149,   1,  2, 13,   0,   0, nullptr } },
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  322,  10, 149,   1,  3, { MSG_WORKAROUND_EXTRACT,  322,  10, 149,   1,  3, 13,   0,   0, nullptr } },
	// Wrong verb when clicking lock pick or toolkit on gnome's door in room 320
	//  while not having enough lock picking skill.
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  320,   9,   9,   7,  1, { MSG_WORKAROUND_REMAP,    320,   9,  42,   7,  1, 99,   0,   0, nullptr } },
	// Wrong modNum (missing parameter) when clicking your only dagger in room 625
	{ GID_QFG4,          SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  625,   4,   6,  34,  1, { MSG_WORKAROUND_REMAP,    620,   4,   6,  34,  1, 99,   0,   0, nullptr } },
	// This fixes the error message shown when speech and subtitles are
	// enabled simultaneously in SQ4 - the (very) long dialog when Roger
	// is talking with the aliens is missing - bug #6067.
	// The missing text is just too big to fit in one speech bubble, and
	// if it's added here manually and drawn on screen, it's painted over
	// the entrance in the back where the Sequel Police enters, so it
	// looks very ugly. Perhaps this is why this particular text is missing,
	// as the text shown in this screen is very short (one-liners).
	// Just output an empty string here instead of showing an error.
	{ GID_SQ4,           SCI_MEDIA_CD,     K_LANG_NONE,     -1,   16,   7,   0,   3,  1, { MSG_WORKAROUND_FAKE,      16,   7,   0,   3,  1,  7,   0,   0, "" } },
	// Missing message when clicking talk in room 520 - bug #10915
	{ GID_SQ4,           SCI_MEDIA_CD,     K_LANG_NONE,     -1,  510,  99,   0,   3,  1, { MSG_WORKAROUND_REMAP,    500,  99,   0,   3,  1,  0,   0,   0, nullptr } },
	// Wrong talker when looking at Cliffy in room 240 after returning from Genetix
	{ GID_SQ5,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  240,   5,   1,   5,  1, { MSG_WORKAROUND_EXTRACT,  240,   5,   1,   5,  1, 99,   0,   0, nullptr } },
	// Missing message when looking at hole in room 740
	{ GID_SQ6,           SCI_MEDIA_ALL,    K_LANG_NONE,     -1,  740,   3,   1,   8,  1, { MSG_WORKAROUND_REMAP,    740,   3,  88,   8,  1, 99,   0,   0, nullptr } },
	SCI_MESSAGEWORKAROUNDENTRY_TERMINATOR
};

// Audio36 workarounds are for when an audio36 resource's tuple is out of sync
//  with its corresponding message, in which case it can be remapped. Any sync36
//  resource with the same tuple will also be remapped. Remapping is the only
//  workaround type available to this table. If a tuple is remapped both here
//  and in message workarounds, this remapping is used.
static const SciMessageWorkaroundEntry audio36Workarounds[] = {
	// game              media             language       room   mod    n    v    c   s    workaround-type          mod    n    v    c   s tlk  idx  len  text
	// Clicking money on Lorelei when dancing - bug #10819 (see message workarounds above)
	{ GID_GK1,           SCI_MEDIA_CD,     K_LANG_NONE,     -1,  420,   2,  32,   0,  1, { MSG_WORKAROUND_REMAP,    420,   2,  32,   3,  1,  0,   0,   0, nullptr } },
	// Clicking Von Glower's letter on Gabriel in room 1120
	{ GID_GK2,           SCI_MEDIA_ALL,    K_LANG_ENGLISH,  -1, 1120,   5,  96,   0,  1, { MSG_WORKAROUND_REMAP,   1120,   5,   2,   0,  1,  0,   0,   0, nullptr } },
	// Clicking Look on floor grate in room 510 - bug #10848
	{ GID_QFG4,          SCI_MEDIA_CD,     K_LANG_NONE,     -1,  510,  23,   1,   0,  1, { MSG_WORKAROUND_REMAP,    510, 199,   1,   0,  1,  0,   0,   0, nullptr } },
	// Clicking flowers on Rusalka - bug #10849 (see message workarounds above)
	{ GID_QFG4,          SCI_MEDIA_CD,     K_LANG_NONE,     -1,  520,   2,  59,   0,  2, { MSG_WORKAROUND_REMAP,    520,   2,  59,   0,  3,  0,   0,   0, nullptr } },
	// Casting glide spell in room 593 when standing on water
	{ GID_QFG4,          SCI_MEDIA_CD,     K_LANG_NONE,     -1,  593,   5,   6,   7,  1, { MSG_WORKAROUND_REMAP,    570,   2,   6,  14,  1,  0,   0,   0, nullptr } },
	SCI_MESSAGEWORKAROUNDENTRY_TERMINATOR
};

// Sync36 workarounds are for when a sync36 resource's tuple is out of sync with
//  its corresponding message, in which case it can be remapped. This hasn't
//  been encountered yet, but remapping solves a similar LB2 sync bug.
static const SciMessageWorkaroundEntry sync36Workarounds[] = {
	// game              media             language       room   mod    n    v    c   s    workaround-type          mod    n    v    c   s tlk  idx  len  text
	// Asking yvette about tut in act 2 is missing a sync resource but a duplicate message has a sync resource - bug #9956
	{ GID_LAURABOW2,     SCI_MEDIA_CD,     K_LANG_NONE,     -1, 1885,   1,   6,  30,  2, { MSG_WORKAROUND_REMAP,   1885,   1,   6,  10,  2,  0,   0,   0, nullptr } },
	SCI_MESSAGEWORKAROUNDENTRY_TERMINATOR
};

static SciMessageWorkaroundSolution findMessageWorkaround(int module, byte noun, byte verb, byte cond, byte seq, const SciMessageWorkaroundEntry *workaroundList) {
	const SciMessageWorkaroundEntry *workaround = workaroundList;
	while (workaround->solution.type != MSG_WORKAROUND_NONE) {
		if (workaround->gameId == g_sci->getGameId() &&
			(workaround->media == SCI_MEDIA_ALL ||
			(workaround->media == SCI_MEDIA_FLOPPY && !g_sci->isCD()) ||
			(workaround->media == SCI_MEDIA_CD && g_sci->isCD()) ||
			(workaround->media == SCI_MEDIA_MAC && g_sci->getPlatform() == Common::kPlatformMacintosh && !g_sci->isCD())) &&
			(workaround->language == K_LANG_NONE ||
			workaround->language == g_sci->getSciLanguage()) &&
			(workaround->roomNumber == -1 ||
			workaround->roomNumber == g_sci->getEngineState()->currentRoomNumber()) &&
			workaround->module == module &&
			workaround->noun == noun &&
			workaround->verb == verb &&
			workaround->cond == cond &&
			workaround->seq == seq) {
			break;
		}
		workaround++;
	}
	return workaround->solution;
}

SciMessageWorkaroundSolution findMessageWorkaround(int module, byte noun, byte verb, byte cond, byte seq) {
	return findMessageWorkaround(module, noun, verb, cond, seq, messageWorkarounds);
}

ResourceId remapAudio36ResourceId(const ResourceId &resourceId) {
	int module = resourceId.getNumber();
	byte noun = resourceId.getTuple() >> 24;
	byte verb = (resourceId.getTuple() >> 16) & 0xff;
	byte cond = (resourceId.getTuple() >> 8) & 0xff;
	byte seq = resourceId.getTuple() & 0xff;

	SciMessageWorkaroundSolution workaround = findMessageWorkaround(module, noun, verb, cond, seq, audio36Workarounds);
	if (workaround.type != MSG_WORKAROUND_REMAP) {
		workaround = findMessageWorkaround(module, noun, verb, cond, seq, messageWorkarounds);
	}

	if (workaround.type == MSG_WORKAROUND_REMAP) {
		return ResourceId(resourceId.getType(), workaround.module, workaround.noun, workaround.verb, workaround.cond, workaround.seq);
	}
	return resourceId;
}

ResourceId remapSync36ResourceId(const ResourceId &resourceId) {
	int module = resourceId.getNumber();
	byte noun = resourceId.getTuple() >> 24;
	byte verb = (resourceId.getTuple() >> 16) & 0xff;
	byte cond = (resourceId.getTuple() >> 8) & 0xff;
	byte seq = resourceId.getTuple() & 0xff;

	SciMessageWorkaroundSolution workaround = findMessageWorkaround(module, noun, verb, cond, seq, sync36Workarounds);
	if (workaround.type != MSG_WORKAROUND_REMAP) {
		workaround = findMessageWorkaround(module, noun, verb, cond, seq, audio36Workarounds);
	}
	if (workaround.type != MSG_WORKAROUND_REMAP) {
		workaround = findMessageWorkaround(module, noun, verb, cond, seq, messageWorkarounds);
	}

	if (workaround.type == MSG_WORKAROUND_REMAP) {
		return ResourceId(resourceId.getType(), workaround.module, workaround.noun, workaround.verb, workaround.cond, workaround.seq);
	}
	return resourceId;
}

} // End of namespace Sci
