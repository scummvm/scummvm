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

#include "bagel/spacebar/sraf_computer.h"
#include "bagel/spacebar/sraf_msg.h"
#include "bagel/spacebar/sraf_file.h"
#include "bagel/spacebar/boflib/app.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/spacebar/baglib/link_object.h"
#include "bagel/bagel.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define SRAF_DIR         "$SBARDIR\\SRAFFA\\CLOSEUP\\COMPUTER\\"
#define SRAF_MALE_DIR    "$SBARDIR\\SRAFFA\\char\\GMALE\\"
#define SRAF_FEMALE_DIR  "$SBARDIR\\SRAFFA\\char\\GFEMALE\\"
#define SRAF_AUDIO_DIR   "$SBARDIR\\SRAFFA\\AUDIO\\EVENTS\\"
#define SRAF_TIMEOUT     "SFTIMOUT.WAV"

#define USE_TEXT_WIDTHS       true

// Local constants
//
// Deal summary constants
#define kUnknownTermStr     "(Unknown)"
#define kRightColumnAlign   25

// Sraffan flashback max turns...
#define kSrafMaxTurns       200

// Amount of time to figure out who is driving...
#define kDrivingTime        5

// Constants not linked to any one screen
#define kCheckMark 'x'

#define kStandardIndentation        4
#define kStandardDoubleIndentation  8

// Buyer bids summary
#define kFirstMineralColumn     12

#define kBuyerBidsPointSize     14
#define kOtherPointSize         13
#define kMineralColWidth        4

#define kBuyerBidsPointWidth    6
#define kLineItemHeight         (kBuyerBidsPointSize + 4)

// Add instructions to the main screen

#define kActivateFooterStr1  "NOTE: On any subsequent screens where the information displayed does"
#define kActivateFooterStr2  "not fit on the screen, scroll up or down one line using up-arrow and"
#define kActivateFooterStr3  "down-arrow.  Scroll up or down a page using page-up or page-down."

#define kBuyerBidsHeaderStr  "BUYER      Zn  Ba  Rg  Ut  Pn  Sz   0  H20 LH  CH  ME  TE  AS  PD   ACCEPT"
#define kBuyerBidsMessage1   "Click on any buyer to see their biography.  Click in the 'ACCEPT' column "
#define kBuyerBidsMessage2   "to incorporate that buyer into the current offer."

// EMail constants
#define kEmailHeaderStr     "SUBJECT                  FROM                   TO"
#define kMessageFromColumn  25
#define kMessageToColumn    48

// Audio constants
#define kAudioHeaderStr     "TITLE                           AUTHOR               PLANET"
#define kAudioAuthorCol     32
#define kAudioPlanetCol     53

// Robobutler constants
#define kRobobutlerHeaderStr "                    TODAY'S SPECIAL OFFERINGS"
#define kRobobutlerCol      20

// Check teams constants
#define kCheckTeamHeaderStr1 "MEETING           DISPATCH  YOUR STAFF ATTENDING MEETING"
#define kCheckTeamHeaderStr2 "WITH              TIME"
//                            0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
#define kCheckTeamTimeCol   18
#define kCheckTeamStaffCol  28

#define kCheckDispatchTeamFooter1 "To check the status of a meeting, click in the 'MEETING WITH' column"
#define kCheckDispatchTeamFooter2 "on the same line as the meeting attendees."

// Staffer constants
#define kMaxStafferNameLen  9

// Dispatch negotiating team constants
#define kDispatchHeaderStr  "MEET WITH:                      TEAM TO INCLUDE:    AVAILABLE    INCLUDE ON TEAM"
#define kMeetTitleColumn    5
#define kMeetSubColumn      9
#define kTeamMemberColumn   36
#define kTeamAvailableColumn   56
#define kTeamIncludeColumn   73
#define kDispatchFooterStr1  "To meet with an individual, click in the 'MEET WITH' column to the"
#define kDispatchFooterStr2  "left of the name.  To read an individual's biography, click on that"
#define kDispatchFooterStr3  "individual's name.  To include a staff member on the negotiating "
#define kDispatchFooterStr4  "team, click in the 'INCLUDE ON TEAM' column.  Click on 'Dispatch "
#define kDispatchFooterStr5  "Team' when the negotiating team is fully assembled."

// Code word constants
#define kCodeWordsHeader1   "Your offer was accepted by the sellers!  Please choose a code name"
#define kCodeWordsHeader2   "for the deal by selecting one word from each group."
#define kCodeWordsHeader3   "    ***GROUP ONE***                             ***GROUP TWO***"
//                           123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define kGroup1Col2     15
#define kGroup2Col1     45
#define kGroup2Col2     60

// Have to know some mineral constants in the grid

enum {
	ZINC = 0,
	BARIUM,
	RIGELLIUM,
	UPTITE,
	PLATINUM,
	SANZIUM,
	OXYGEN,
	WATER,
	LUMBER,
	CROP,
	MEDICINE,
	TURF,
	STRUCTURE,
	POLLUTION,
	WEAPONS
};

// Main menu items

enum {
	DEALSUMMARY = 0,
	BUYERBIDS,
	DEALBACKGROUND,
	SELLERBIOS,
	OTHERBIOS,
	STAFFBIOS,
	DISPATCHTEAM,
	CURRENTEMAIL,
	AUDIOSETTINGS,
	ROBOBUTLER,
	CHECKTEAMS
};


// Local structs
struct ST_BUTTONS {
	const char *_pszName;
	const char *_pszUp;
	const char *_pszDown;
	const char *_pszFocus;
	const char *_pszDisabled;
	int _nLeft;
	int _nTop;
	int _nWidth;
	int _nHeight;
	int _nID;

};

#define SRAF_NO_MEETING 0
#define SRAF_GOOD_MEETING 1
#define SRAF_BAD_MEETING 2

struct SELLERITEM {
	const char *_pszName;
	int16       _nAmount;
	SELLERS     _nSellerID;
	const char *_pszSellerBio;
	bool        _bMeetWith;
	bool        _bAvailable;
	int16       _nMeetingResult;
};

// Struct for email messages

struct EMAILITEM {
	const char *_pszMessageSubj;
	const char *_pszMessageFrom;
	const char *_pszMessageTo;
	const char *_pszMessageFile;
};

// Struct for buyer bids grid
struct BUYERBIDSREC {
	const char *_pszName;
	int16       _nMineralVal[NUM_MINERALS];
	int16       _nBidSum;
	bool        _bAccept;
	BUYERS      _nBuyerID;
	const char *_pszBuyerBio;
	bool        _bMeetWith;
	bool        _bAvailable;
	int16       _nFlags;
	int16       _nMeetingResult;
};

// Mineral information
struct MINERAL_NAMES {
	const char *_pszMineralName;
	const char *_pszMineralAbbrev;
};

// Staffer bio information
struct STAFFERITEM {
	const char *_pszStafferName;
	const char *_pszStafferBio;
	const char *_pszStafferBmp;
	bool     _bAvailable;
	bool     _bOnCurrentTeam;
	int      _nFlags;
	int      _nNegotiatingStrength;
	int      _nMeetingTime;
};

// Staffer bio information
struct OTHERITEM {
	const char *_pszName;
	const char *_pszOtherPartyBio;
	bool     _bMeetWith;
	bool     _bAvailable;
	int16    _nPaymentAmount;
	int16    _nMeetingResult;
};

struct AUDIOITEM {
	const char *_pszTitle;
	const char *_pszAuthor;
	const char *_pszPlanet;
	const char *_pszAudioFile;
	CBofSound *_pMidiTrack;
};

#define kRandomPlayTime 15

struct OFFERINGITEM {
	const char *_pszType;
	const char *_pszOffering;
	const char *_pszFile;
	const char *_pszRcvOfferingFile;
};

// Codewords
struct CODEWORDITEM {
	const char *_pszCodeWord1;
	const char *_pszCodeWord2;
	const char *_pszCodeWord3;
	const char *_pszCodeWord4;
};


// Local globals
//
// there are 6 regular staffers, but durteen, the 7th, is the one that notifies deven that
// time has run out... we need room in the structure for him also, although he won't be
// doing any negotiating or meetings for us.
static STAFFERITEM g_staffers[NUM_STAFFERS + 1] = {
	{ "Norg-72",   "nor72bio.txt", "sanenorg.bmp",   true, false, mStafferMale,    1,  30 },
	{ "Pnurth-81", "Pnu81bio.txt", "sanepnur.bmp",   true, false, mStafferFemale,  3,  30 },
	{ "Zig-64",    "Zig64bio.txt", "sanezig.bmp",    true, false, mStafferFemale,  6,  18 },
	{ "Lentil-24", "Len24bio.txt", "sanelent.bmp",   true, false, mStafferFemale,  5,  15 },
	{ "Vargas-20", "Var20bio.txt", "sanevarg.bmp",   true, false, mStafferMale,   10,  25 },
	{ "Churg-53",  "Chu53bio.txt", "sanechur.bmp",   true, false, mStafferMale,    5,  22 },
	{ "Durteen-97", "DUR97bio.txt", "sanedurt.bmp",   true, false, mStafferMale,    0,  0 },
};

static const char *g_stMainItems[NUM_MAIN_ITEMS] = {
	"* Deal Summary",
	"* Buyer Bids and Biographies",
	"* Deal Background Data",
	"* Seller Biographies",
	"* Biographies of Other Parties",
	"* Staff Biographies",
	"* Dispatch Negotiating Team",
	"* Current E-Mail",
	"* Household Audio",
	"* Robobutler Service",
	"* Status of Negotiations"
};

static SELLERITEM g_stSellerNames[NUM_SELLERS] = {
	{ "Irk-4", -1, IRK4, "IRK4BIO.TXT", false, true, SRAF_NO_MEETING },
	{ "Yeef-8", 38, YEEF8, "YEE8BIO.TXT", false, true, SRAF_NO_MEETING },
	{ "Quosh-23", -1, QUOSH23, "QUO11BIO.TXT", false, true, SRAF_NO_MEETING },
};

// Yeef can be talked down!

#define kYeefTalkedDownAmount       33

#define mBuyerMale      0x0001
#define mBuyerFemale    0x0002

// Can talk these buyers up... here are there amounts.

#define kSinjinTalkedUpAmount       7
#define kSinjinMineralID            STRUCTURE

#define kGungTalkedUpAmount         8
#define kGungMineralID              BARIUM

#define kDorkTalkedUpAmount         8
#define kDorkMineralID              UPTITE

#define kGildTalkedUpAmount1        11
#define kGildTalkedUpAmount2        8
#define kGildMineralID1             ZINC
#define kGildMineralID2             PLATINUM

#define MIN_MART_ENTRIES              6

// Buyer bids grid, dork, gung sinjin and gild can be talked up, but the
// rest of them are non-negotiable. Only dork and gung are part of the
// final solution.
static BUYERBIDSREC g_stBuyerBids[NUM_BUYERS] = {
	{ "Pylon-3",    { 0,  0, 0, 0, 0, 0,  7,  9, 2, 3, 4, 8, 0, 0 }, 33, false, PYLON3, "PYL3BIO.TXT", false, true, mBuyerFemale, SRAF_NO_MEETING },
	{ "Dippik-10",  { 0,  0, 0, 0, 0, 0,  5,  4, 0, 0, 0, 0, 6, 8 }, 23, false, DIPPIK10, "DIP10BIO.TXT", false, true, mBuyerMale, SRAF_NO_MEETING },
	{ "Vebbil-18",  { 0,  0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 7, 0, 0 }, 7, false, VEBBIL18, "VEB18BIO.TXT", false, true, mBuyerFemale, SRAF_NO_MEETING },
	{ "Gung-14",    { 0,  4, 2, 0, 0, 10, 0,  0, 0, 0, 0, 0, 0, 0 }, 16, false, GUNG14, "GUN14BIO.TXT", false, true, mBuyerMale, SRAF_NO_MEETING },
	{ "Reyes-24",   { 10, 0, 0, 7, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 17, false, REYES24, "REY24BIO.TXT", false, true, mBuyerMale, SRAF_NO_MEETING },
	{ "Gild-13",    { 6,  0, 0, 0, 4, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 10, false, GILD13, "GIL13BIO.TXT", false, true, mBuyerFemale, SRAF_NO_MEETING },
	{ "Hem-20",     { 0,  0, 3, 0, 5, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 8, false, HEM20, "HEM20BIO.TXT", false, true, mBuyerFemale, SRAF_NO_MEETING },
	{ "Dork-44",    { 0,  6, 0, 4, 0, 11, 0,  0, 0, 0, 0, 0, 0, 0 }, 21, false, DORK44, "DOR44BIO.TXT", false, true, mBuyerMale, SRAF_NO_MEETING },
	{ "Rawley-23",  { 0,  0, 0, 0, 0, 0,  0,  0, 0, 0, 8, 0, 0, 0 }, 8, false, RAWLEY23, "RAW23BIO.TXT", false, true, mBuyerFemale, SRAF_NO_MEETING },
	{ "Jella-37",   { 0,  0, 0, 0, 0, 0,  0,  0, 4, 0, 0, 0, 0, 0 }, 4, false, JELLA37, "JEL37BIO.TXT", false, true, mBuyerFemale, SRAF_NO_MEETING },
	{ "Sinjin-11",  { 0,  0, 0, 0, 0, 0,  12, 0, 0, 0, 0, 0, 5, 6 }, 23, false, SINJIN11, "SIN11BIO.TXT", false, true, mBuyerMale, SRAF_NO_MEETING },
	{ "Hundey-42",  { 0,  0, 4, 0, 0, 0,  0,  0, 3, 0, 0, 0, 0, 6 }, 13, false, HUNDEY42, "HUN42BIO.TXT", false, true, mBuyerMale, SRAF_NO_MEETING },
	{ "Chandra-15", { 0,  0, 0, 0, 0, 0,  0, 13, 0, 2, 4, 0, 0, 0 }, 19, false, CHANDRA15, "CHA15BIO.TXT", false, true, mBuyerFemale, SRAF_NO_MEETING },
	{ "Clang-2",    { 0,  0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 9 }, 9, false, CLANG2, "CLA2BIO.TXT", false, true, mBuyerMale, SRAF_NO_MEETING },
	{ "Min. Mart",  { 4,  0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 4, false, MINMARTZN, "MINBIO.TXT", false, true, 0, SRAF_NO_MEETING },
	{ "Min. Mart",  { 0,  3, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 3, false, MINMARTBA, "MINBIO.TXT", false, true, 0, SRAF_NO_MEETING },
	{ "Min. Mart",  { 0,  0, 1, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 1, false, MINMARTRG, "MINBIO.TXT", false, true, 0, SRAF_NO_MEETING },
	{ "Min. Mart",  { 0,  0, 0, 3, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 3, false, MINMARTUT, "MINBIO.TXT", false, true, 0, SRAF_NO_MEETING },
	{ "Min. Mart",  { 0,  0, 0, 0, 3, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 3, false, MINMARTPN, "MINBIO.TXT", false, true, 0, SRAF_NO_MEETING },
	{ "Min. Mart",  { 0,  0, 0, 0, 0, 7,  0,  0, 0, 0, 0, 0, 0, 0 }, 7, false, MINMARTSZ,  "MINBIO.TXT", false, true, 0, SRAF_NO_MEETING },
};

static MINERAL_NAMES g_stMinerals[NUM_MINERALS] = {
	{ "Zinc Mining Rights", "Zn" },
	{ "Barium Mining Rights", "Ba" },
	{ "Rigellium Mining Rights", "Rg" },
	{ "Uptite Mining Rights", "Ut" },
	{ "Pandemonium Mining Rights", "Pn" },
	{ "Sanzium Mining Rights", "Sz" },
	{ "Oxygen Mining Rights", "O" },
	{ "Water Extraction Rights", "H2O" },
	{ "Lumber Harvesting Rights", "LH" },
	{ "Crop Harvesting Rights", "CH" },
	{ "Medicine Extraction Rights", "ME" },
	{ "Turf Extraction Rights", "TE" },
	{ "Artificial Structure Ownership and Removal Rights", "AS" },
	{ "Exclusive Pollution Dumping Rights", "PD" },
};

static EMAILITEM g_stEmailMessages[NUM_MAIL_MESSAGES] = {
	{ "Armpit III",       "Irk-4",                "Deven-7",      "SRAMAIL1.TXT" },
	{ "Pool",             "Devo-185",             "Deven-7",      "SRAMAIL2.TXT" },
	{ "Rumor",            "Durteen-97",           "Deven-7",      "SRAMAIL3.TXT" },
	{ "Mona Lisa",        "Fezint-40",            "Deven-7",      "SRAMAIL4.TXT" },
	{ "A Special Offer",  "Acme Upholstery Corp", "Deven-7",      "SRAMAIL5.TXT" },
	{ "Swonza-5",         "Churg-53",             "Deven-7",      "SRAMAIL6.TXT" },
	{ "Pinna-6C",         "Chippik-9",            "Deven-7",      "SRAMAIL7.TXT" },
	{ "Armpit III",       "Yeef-8",               "Deven-7",      "SRAMAIL8.TXT" },
};

enum OTHERPARTYS {
	SWONZA5 = 0, POLITICIANS, ENVIRONMENTALISTS
};

static OTHERITEM g_stOtherPartys[NUM_OTHER_PARTYS] = {
	{ "Swonza-5", "SWO5BIO.TXT", false, true, -1, SRAF_NO_MEETING },
	{ "Politicians", "POLITBIO.TXT", false, true, -30, SRAF_NO_MEETING },
	{ "Environmentalists", "ENVIRBIO.TXT", false, true, -1, SRAF_NO_MEETING },
};

// Sraffin computer buttons
#define ON_BUTTON               0
#define OFF_BUTTON              1
#define QUIT_BUTTON             2
#define RETURN_TO_MAIN_BUTTON   3
#define SUBMIT_BUTTON           4
#define DISPLAY_KEY_BUTTON      5
#define DISPATCH_TEAM_BUTTON    6
#define NO_MUSIC_BUTTON         7
#define DONE_BUTTON             8
#define ORDER_BEVERAGE_BUTTON   9
#define ORDER_SNACK_BUTTON      10

// Define our buttons to go on the bottom of the main screen
static ST_BUTTONS g_stButtons[NUM_SRAFCOMPBUTT] = {
	{ "On", "onup.bmp", "ondn.bmp", "onup.bmp", "onup.bmp", 20, 445, 200, 30, ON_BUTTON },
	{ "Off", "offup.bmp", "offdn.bmp", "offup.bmp", "offup.bmp", 20, 445, 200, 30, OFF_BUTTON },
	{ "Quit", "quitup.bmp", "quitdn.bmp", "quitup.bmp", "quitup.bmp", 407, 445, 200, 30, QUIT_BUTTON },
	{ "Return to Main", "mainup.bmp", "maindn.bmp", "mainup.bmp", "mainup.bmp", 430, 445, 200, 30, RETURN_TO_MAIN_BUTTON },
	{ "Submit Offer", "offerup.bmp", "offerdn.bmp", "offerup.bmp", "offerup.bmp", 220, 445, 200, 30, SUBMIT_BUTTON },
	{ "Display the Key", "keyup.bmp", "keydn.bmp", "keyup.bmp", "keyup.bmp", 220, 445, 200, 30, DISPLAY_KEY_BUTTON },
	{ "Dispatch Team", "teamup.bmp", "teamdn.bmp", "teamup.bmp", "teamup.bmp", 220, 445, 200, 30, DISPATCH_TEAM_BUTTON },
	{ "No Music", "musicup.bmp", "musicdn.bmp", "musicup.bmp", "musicup.bmp", 220, 445, 200, 30, NO_MUSIC_BUTTON },
	{ "Done", "doneup.bmp", "donedn.bmp", "doneup.bmp", "doneup.bmp", 430, 445, 200, 30, DONE_BUTTON },
	{ "Order Beverage", "bevup.bmp", "bevdn.bmp", "bevup.bmp", "bevup.bmp", 20, 445, 200, 30, ORDER_BEVERAGE_BUTTON },
	{ "Order Snack", "snackup.bmp", "snackdn.bmp", "snackup.bmp", "snackup.bmp", 225, 445, 200, 30, ORDER_SNACK_BUTTON }
};

static AUDIOITEM g_audioTracks[NUM_MUSICAL_SCORES] = {
	{ "Harpsichord Invention #1", "J. S. Bach",     "Earth",      "SRAFFAN1.MID", nullptr },
	{ "Jazz Theme #44981",        "Urgon-Thmo",     "Thelbia",    "SRAFFAN2.MID", nullptr },
	{ "Bamboo Breeze",            "H. Fugimachi",   "Earth",      "SRAFFAN3.MID", nullptr },
	{ "Power of Crystal, OP. 12", "Brak-44",        "H'poctyl",   "SRAFFAN4.MID", nullptr },
	{ "String Quartet",           "J. Salesin",     "Earth",      "SRAFFAN5.MID", nullptr },
	{ "The Womp Song",            "The Womps",      "Armpit IV",  "SRAFFAN6.MID", nullptr },
	{ "Mixed Selections",         " ",              " ",          nullptr,        nullptr },
};
static AUDIOITEM g_chickenDance = \
{ "Chicken Dance",            "Unknown Origin", "Mars",       "CHIKDANC.MID", nullptr };
// Pointers to the g_audioTracks entries. For the Mac, The Womp Song
// is replaced with the Chicken Dance
static AUDIOITEM *g_stAudioSetting[NUM_MUSICAL_SCORES] = {
	&g_audioTracks[0],
	&g_audioTracks[1],
	&g_audioTracks[2],
	&g_audioTracks[3],
	&g_audioTracks[4],
	&g_audioTracks[5],
	&g_audioTracks[6]
};

static OFFERINGITEM g_stOfferings[NUM_OFFERINGS] = {
	{ "BEVERAGE:", "Distilled Fleeblunx Sap", "BEVDESC.TXT", "DRINKRCV.TXT" },
	{ "LIGHT SNACK:", "Br'thl'gian Centipede Legs", "SNKDESC.TXT", "SNACKRCV.TXT" }
};

// Codewords

static CODEWORDITEM g_stCodeWords[NUM_CODE_WORDS] = {
	{ "ANGRY",     "LUCKY",      "BAGPIPE",    "MOSS" },
	{ "BRASS",     "MELTED",     "BANANA",     "MUFFIN" },
	{ "BURNT",     "MOIST",      "BANSHEE",    "PAJAMAS" },
	{ "DEAD",      "NOISY",      "BULLET",     "PUCK" },
	{ "ELVISH",    "POINTY",     "COOKIE",     "SHRUB" },
	{ "ENCRUSTED", "SIMPLE",     "FRISBEE",    "SWORD" },
	{ "FUZZY",     "SMOKY",      "GALOSHES",   "TORPEDO" },
	{ "GIGGLING",  "TWITCHING",  "GUITAR",     "TUNIC" },
	{ "HUNGRY",    "UNITED",     "HYENA",      "UMBRELLA" },
	{ "HURLING",   "VARNISHED",  "LANTERN",    "ZUCCHINI" }
};

static bool gTextScreenFrontmost = false;
static int gDispatchCurState = 0;   // Retain current state of dispatch negotiating team screen
static int gTurncountLineNo = 0;    // Keep track of turncount line number
// Member static initializations

bool SrafComputer::_bSwonzaEnlightened = false;
SrafComputer *SrafComputer::_pHead = nullptr;   // Only sraffan computer
int SrafComputer::_nStartingTime = 0;
bool SrafComputer::_bRandomAudio = false;
int SrafComputer::_nRandomTime = 0;
bool SrafComputer::_bFailureNotified = false;

// States that the dispatch negotiating team can take on
#define mSellersExpanded 0x01
#define mBuyersExpanded  0x02
#define mOthersExpanded  0x04

CBofWindow *SrafTextScreen::_pSaveActiveWin = nullptr;

// Local prototype functions
const char *buildSrafDir(const char *pszFile);
const char *buildAudioDir(const char *pszFile);
const char *buildMaleSrafDir(const char *pszFile);
const char *buildFemaleSrafDir(const char *pszFile);

// Local prototypes

SrafComputer::SrafComputer() :
	gCompDisplay(40, 40, 600, 440), gTextWindow(0, 0, 640 - 1, 480 - 1),
	gStafferDisplay(0, 0, 640 - 1, 480 - 1), gSrafTextWindow(0, 440, 640 - 1, 480 - 1) {
	int i;

	_cTextColor = CTEXT_WHITE;          // Start out with white as our text color
	_cTextHiliteColor = RGB(255, 255, 255); // and some other color as our hilite
	_cTextLineColor = CTEXT_YELLOW;

	// Computer starts as off and at the main screen
	_eMode = SC_OFF;
	_eCurScreen = SC_MAIN;

	// initialize our three buttons
	for (i = 0; i < NUM_SRAFCOMPBUTT; i++) {
		_pButtons[i] = nullptr;
	}

	// initialize bitmaps (one extra staffer, durteen)
	for (i = 0; i < (NUM_STAFFERS + 1); i++) {
		_pStafferBmp[i] = nullptr;
	}

	// List and text box
	_pLBox = nullptr;
	_pMainList = nullptr;
	_nSelection = -1;
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = 20;

	// Subscreen initializations
	_pSellerSummaryList = nullptr;
	_pBuyerSummaryList = nullptr;
	_pTeamList = nullptr;

	// Text only screen initializations
	_pTextOnlyScreen = nullptr;

	_pszGroup1Word = nullptr;
	_pszGroup2Word = nullptr;
	_bSrafAttached = false;

	_pHead = this;
}

// FYI: It only uses the seller summary list to build the onscreen
// list, we use the global to store intermediate data.
void SrafComputer::recalcSellerSummaryList() {
	// initialize the initial state of the seller summary
	if (_pSellerSummaryList == nullptr) {
		_pSellerSummaryList = new CBofList<DealSummarySellerItem>;
	} else {
		delete _pSellerSummaryList;
		_pSellerSummaryList = nullptr;
	}

	// Lets verify that its all set before we go to use it
	if (_pSellerSummaryList == nullptr) {
		_pSellerSummaryList = new CBofList<DealSummarySellerItem>;
	}

	int i = 0;
	while (i < NUM_SELLERS) {
		DealSummarySellerItem sellerItem;
		sellerItem._eSellerID = g_stSellerNames[i]._nSellerID;
		sellerItem._nSellerOffer = g_stSellerNames[i]._nAmount;
		sellerItem._pSellerName = g_stSellerNames[i]._pszName;

		_pSellerSummaryList->addToTail(sellerItem);
		i++;
	}
}


void SrafComputer::recalcBuyerSummaryList() {
	// initialize the initial state of the deal summary
	if (_pBuyerSummaryList == nullptr) {
		_pBuyerSummaryList = new CBofList<DealSummaryBuyerItem>;
	} else {
		delete _pBuyerSummaryList;
		_pBuyerSummaryList = nullptr;
	}

	// Lets verify that its all set before we go to use it
	if (_pBuyerSummaryList == nullptr) {
		_pBuyerSummaryList = new CBofList<DealSummaryBuyerItem>;
	}

	int i = 0;
	while (i < NUM_BUYERS) {
		if (g_stBuyerBids[i]._bAccept) {
			DealSummaryBuyerItem *pBuyerItem = new DealSummaryBuyerItem();

			pBuyerItem->_eBuyerID = g_stBuyerBids[i]._nBuyerID;
			pBuyerItem->_nBuyerOffer = g_stBuyerBids[i]._nBidSum;
			pBuyerItem->_pBuyerName = g_stBuyerBids[i]._pszName;

			_pBuyerSummaryList->addToTail(*pBuyerItem);
			delete pBuyerItem;
		}

		i++;
	}
}

bool SrafComputer::verifyDispatchTeam() {
	bool bValidTeam = true;
	char szFailureReason[256];
	int nTeam = 0;

	// Make sure that we have someone to meet
	int nMeetOthers = getMeetMember(kOthersList);
	int nMeetSellers = getMeetMember(kSellersList);
	int nMeetBuyers = getMeetMember(kBuyersList);

	if (nMeetOthers == -1 && nMeetSellers == -1 && nMeetBuyers == -1) {
		bool bStafferSelected = false;

		// Nobody to meet with, see if we have any team members selected.
		// This changes the error message.
		for (int i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i]._bOnCurrentTeam) {
				bStafferSelected = true;
				break;
			}
		}

		if (bStafferSelected == false) {
			Common::strcpy_s(szFailureReason, kszNotEvenClose);
		} else {
			Common::strcpy_s(szFailureReason, kszNoNegotiators);
		}
		bValidTeam = false;
	}

	if (bValidTeam) {
		if ((nMeetOthers != -1 && nMeetSellers != -1 && nMeetBuyers != -1)) {
			Common::sprintf_s(szFailureReason, ksz3MeetingTargets,
			                  g_stSellerNames[nMeetSellers]._pszName,
			                  g_stOtherPartys[nMeetOthers]._pszName,
			                  g_stBuyerBids[nMeetBuyers]._pszName);
			bValidTeam = false;
		}
	}

	if (bValidTeam) {
		if ((nMeetOthers != -1 && nMeetSellers != -1)) {
			Common::sprintf_s(szFailureReason, ksz2MeetingTargets,
			                  g_stSellerNames[nMeetSellers]._pszName,
			                  g_stOtherPartys[nMeetOthers]._pszName);
			bValidTeam = false;
		}
	}

	if (bValidTeam) {
		if ((nMeetSellers != -1 && nMeetBuyers != -1)) {
			Common::sprintf_s(szFailureReason, ksz2MeetingTargets,
			                  g_stSellerNames[nMeetSellers]._pszName,
			                  g_stBuyerBids[nMeetBuyers]._pszName);
			bValidTeam = false;
		}
	}

	if (bValidTeam) {
		if ((nMeetOthers != -1 && nMeetBuyers != -1)) {
			Common::sprintf_s(szFailureReason, ksz2MeetingTargets,
			                  g_stOtherPartys[nMeetOthers]._pszName,
			                  g_stBuyerBids[nMeetBuyers]._pszName);
			bValidTeam = false;
		}
	}

	// Make sure at least one staff member is sent on this mission
	if (bValidTeam) {
		for (int i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i]._bOnCurrentTeam) {
				nTeam |= (1 << (i + 3));
			}
		}

		if (nTeam == 0) {
			Common::strcpy_s(szFailureReason, kszNoTeamMembers);
			bValidTeam = false;
		}
	}

	// No meetings with irk or quosh allowed.
	if (bValidTeam) {
		if (nMeetSellers == IRK4) {
			Common::strcpy_s(szFailureReason, kszCantMeetIrk);
			bValidTeam = false;
		}

		if (nMeetSellers == QUOSH23) {
			Common::strcpy_s(szFailureReason, kszCantMeetQuosh);
			bValidTeam = false;
		}
	}

	// Don't allow repeat negotiations with someone that
	// has already been successfully negotiated with.
	if (bValidTeam) {
		if (nMeetSellers != -1) {
			if (g_stSellerNames[nMeetSellers]._nMeetingResult == SRAF_GOOD_MEETING) {
				Common::strcpy_s(szFailureReason, kszCantRenegSeller);
				bValidTeam = false;
			}
		}

		if (nMeetOthers != -1) {
			if (g_stOtherPartys[nMeetOthers]._nMeetingResult == SRAF_GOOD_MEETING) {
				Common::strcpy_s(szFailureReason, kszCantRenegOther);
				bValidTeam = false;
			}
		}

		if (nMeetBuyers != -1) {
			if (g_stBuyerBids[nMeetBuyers]._nMeetingResult == SRAF_GOOD_MEETING) {
				Common::strcpy_s(szFailureReason, kszCantRenegBuyer);
				bValidTeam = false;
			}
		}
	}

	// Build a valid meeting list element.
	DispatchedTeamItem *pTeamItem = nullptr;
	if (bValidTeam) {

		int nDispatchFlags = (nMeetOthers != -1 ? mOtherParty : (nMeetSellers != -1 ? mSeller : mBuyer));
		int nMeetingWith = (nMeetOthers != -1 ? nMeetOthers : (nMeetSellers != -1 ? nMeetSellers : nMeetBuyers));

		if (_pTeamList == nullptr) {
			_pTeamList = new CBofList<DispatchedTeamItem>;
		}

		pTeamItem = new DispatchedTeamItem();
		pTeamItem->_nFlags = nTeam | nDispatchFlags;
		pTeamItem->_nMeetWithID = nMeetingWith;
		CBagVar *pVar = g_VarManager->getVariable("SRATURNCOUNT");
		assert(pVar != nullptr);

		pTeamItem->_nDispatchTime = pVar->getNumValue();
		pTeamItem->_nMeetingTime = calculateMeetingTime(pTeamItem->_nFlags);

		// Pick a team captain, must be same sex as the announcer.
		pTeamItem->_nTeamCaptain = getTeamCaptain(pTeamItem->_nFlags);

		// Team gender is based on sex of captain.
		pTeamItem->_nFlags |= getTeamGender(pTeamItem->_nFlags);

		_pTeamList->addToTail(*pTeamItem);

		// Finally, since we have a valid team, set those team members who are attending to
		// be unavailable for any other staff meetings, also reset them to false for
		// on current team

		for (int i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i]._bOnCurrentTeam) {
				g_staffers[i]._bOnCurrentTeam = false;
				g_staffers[i]._bAvailable = false;
			}
		}
	}

	// There are all kinds of ways to screw up a meeting, but we address those in some kind of
	// staff meeting grid (search StaffMeeting)
	if (bValidTeam == false) {
		displayMessage(szFailureReason);
	} else {
		char szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString  sStr(szLocalBuff, 256);

		// We're very excited, we notified the user, now clear up the globals and reset.
		if (nMeetOthers != -1) {
			g_stOtherPartys[nMeetOthers]._bMeetWith = false;
			g_stOtherPartys[nMeetOthers]._bAvailable = false;
		} else if (nMeetSellers != -1) {
			g_stSellerNames[nMeetSellers]._bMeetWith = false;
			g_stSellerNames[nMeetSellers]._bAvailable = false;
		} else {
			g_stBuyerBids[nMeetBuyers]._bMeetWith = false;
			g_stBuyerBids[nMeetBuyers]._bAvailable = false;
		}

		// Redraw the screen with the meeting with column collapsed and
		// the checkmarks out of the staffer columns.  This assures that
		// any screen capture will come back to us ready for display to the
		// user.

		initDispatchTeam();
		activateDispatchTeam();

		// Also have to give the boss a sound or text file to play.
		if (pTeamItem->_nFlags & mStafferMale) {
			sStr = buildMaleSrafDir(kGSM1SraMaleStr);
		} else {
			sStr = buildFemaleSrafDir(kGSM1SraFemStr);
		}

		// Notify Deven...
		notifyBoss(sStr, pTeamItem->_nTeamCaptain);

		// All done with our team item, trash it.
		delete pTeamItem;

		updateWindow();
	}

	return bValidTeam;
}


SrafComputer::~SrafComputer() {
	// These lists are persistent across turning the computer on and off, so
	// delete them only at the end of the game, not when you turn on/off the
	// computer (attach/detach)
	delete _pSellerSummaryList;
	_pSellerSummaryList = nullptr;

	delete _pBuyerSummaryList;
	_pBuyerSummaryList = nullptr;

	delete _pTeamList;
	_pTeamList = nullptr;

	delete _pLBox;
	_pLBox = nullptr;

	// We grab these bad babies in the attach sequence, but since
	// we need them to live past having the computer on, we need to
	// destruct them in the destructor.

	for (int i = 0; i < (NUM_MUSICAL_SCORES - 1); i++) {
		delete g_stAudioSetting[i]->_pMidiTrack;
		g_stAudioSetting[i]->_pMidiTrack = nullptr;
	}

	_pTextOnlyScreen = nullptr;
	_pHead = nullptr;
}

void SrafComputer::onMainLoop() {
	if (_bFirstPaint) {
		_bFirstPaint = false;
		attachActiveObjects();
	}
}

void SrafComputer::onPaint(CBofRect *pRect) {
	if (getBackdrop()) {
		assert(getWorkBmp() != nullptr);

		// erase everything from the background
		getWorkBmp()->paint(getBackdrop(), pRect, pRect);

		// paint all the objects to the background
		paintStorageDevice(nullptr, getBackdrop(), pRect);
	}

	// Paint the backdrop
	if (getBackdrop()) {
		paintBackdrop();
	}

	if (_eMode == SC_OFF) {
		_pButtons[ON_BUTTON]->paint(nullptr);
	} else {
		_pButtons[OFF_BUTTON]->paint(nullptr);
	}
}

ErrorCode SrafComputer::attach() {
	ErrorCode errorCode = CBagStorageDevWnd::attach();

	if (errorCode == ERR_NONE) {
		// Build our main menu list
		assert(_pMainList == nullptr);

		_pMainList = new CBofList<SrafCompItem>;
		fillMain();

		// If we're on the Mac version, slot in the Chicken Dance song
		if (g_engine->getPlatform() == Common::kPlatformMacintosh)
			g_stAudioSetting[5] = &g_chickenDance;

		// Bring in all our audio tracks
		for (int i = 0; i < (NUM_MUSICAL_SCORES - 1); i++) {
			if (g_stAudioSetting[i]->_pMidiTrack == nullptr) {
				g_stAudioSetting[i]->_pMidiTrack = new CBofSound(this, buildAudioDir(g_stAudioSetting[i]->_pszAudioFile), SOUND_MIDI | SOUND_ASYNCH | SOUND_LOOP, 32000);
			}
		}

		// Must have a valid backdrop by now
		assert(_pBackdrop != nullptr);
		CBofPalette *pPal = _pBackdrop->getPalette();

		for (int i = 0; i < NUM_SRAFCOMPBUTT; i++) {
			_pButtons[i] = new CBofBmpButton;
			CBofBitmap *pUp = loadBitmap(buildSrafDir(g_stButtons[i]._pszUp), pPal);
			CBofBitmap *pDown = loadBitmap(buildSrafDir(g_stButtons[i]._pszDown), pPal);
			CBofBitmap *pFocus = loadBitmap(buildSrafDir(g_stButtons[i]._pszFocus), pPal);
			CBofBitmap *pDis = loadBitmap(buildSrafDir(g_stButtons[i]._pszDisabled), pPal);

			_pButtons[i]->loadBitmaps(pUp, pDown, pFocus, pDis);
			_pButtons[i]->create(g_stButtons[i]._pszName, g_stButtons[i]._nLeft, g_stButtons[i]._nTop, g_stButtons[i]._nWidth, g_stButtons[i]._nHeight, this, g_stButtons[i]._nID);
			if (i != QUIT_BUTTON)
				_pButtons[i]->hide();
		}

		// Get our code words
		_pszGroup1Word = new CBofString();
		_pszGroup2Word = new CBofString();

		// Set the starting time...
		if (_nStartingTime == 0) {
			CBagVar *pVar = g_VarManager->getVariable("SRATURNCOUNT");
			assert(pVar != nullptr);
			_nStartingTime = pVar->getNumValue();
		}

		setOn();
		show();
		updateWindow();

		// Finally, use our regular system cursor, not the custom ones
		CBagCursor::showSystemCursor();

		// Bring in all the external variables
		restoreSraffanVars();

		// Finally, if we're hallucinating, turn off the hallucination filter.
		CBagVar *pVar = g_VarManager->getVariable("HALLUCINATE");
		if (pVar && pVar->getNumValue() > 0) {
			pVar->setValue(0);
		}
	}

	_bSrafAttached = true;
	_bFailureNotified = false;

	// No start state for dispatch screen
	gDispatchCurState = 0;

	return ERR_NONE;
}

ErrorCode SrafComputer::detach() {
	// Computer gets turned off
	_eMode = SC_OFF;

	// Get rid of the system cursor
	if (_bSrafAttached) {
		CBagCursor::hideSystemCursor();
		_bSrafAttached = false;
	}

	// Remove any capture/focus
	CBofApp::getApp()->setCaptureControl(nullptr);
	CBofApp::getApp()->setFocusControl(nullptr);


	// save all the external variables
	saveSraffanVars();

	// Delete our computer buttons
	for (int i = 0; i < NUM_SRAFCOMPBUTT; i++) {
		delete _pButtons[i];
		_pButtons[i] = nullptr;
	}

	delete _pLBox;
	_pLBox = nullptr;

	// (one extra staffer, durteen)
	for (int i = 0; i < (NUM_STAFFERS + 1); i++) {
		delete _pStafferBmp[i];
		_pStafferBmp[i] = nullptr;
	}

	// Get rid of our codewords...
	delete _pszGroup1Word;
	_pszGroup1Word = nullptr;

	delete _pszGroup2Word;
	_pszGroup2Word = nullptr;

	delete _pMainList;
	_pMainList = nullptr;

	// The attach method will restore the state of the dispatched teams...,
	// we can trash that info here.
	delete _pTeamList;
	_pTeamList = nullptr;

	CBagStorageDevWnd::detach();

	return ERR_NONE;
}


void SrafComputer::onLButtonUp(uint32 /*nFlags*/, CBofPoint */*xPoint*/, void *) {
}

void SrafComputer::onLButtonDown(uint32 /*nFlags*/, CBofPoint */*xPoint*/, void *) {
}

void SrafComputer::onKeyHit(uint32 lKey, uint32 nRepCount) {
	switch (lKey) {
	case BKEY_BACK:                 // Temporary, go back to main screen.
		deleteListBox();
		_eCurScreen = SC_MAIN;
		break;

	case BKEY_ALT_q:
		// Don't allow a quit from the sraf computer.
		//quit();
		break;

	case ' ':
		// Increment turn count each time screen is updated.
		incrementTurnCount();
		break;

	default:
		CBagStorageDevWnd::onKeyHit(lKey, nRepCount);
		break;
	}
}


void SrafComputer::onBofButton(CBofObject *pObject, int nState) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	// Count a turn if we have been clicked
	// Mouse downs on button clicks were causing too many weird
	// problems with meeting results... so get rid of them.
	CBofButton *pButton = (CBofButton *)pObject;

	switch (_eCurScreen) {
	case SC_MAIN:
		onButtonMainScreen(pButton, nState);
		break;
	case SC_DEAL:
		onButtonDealSummary(pButton, nState);
		break;
	case SC_BIDS:
		onButtonBuyerBids(pButton, nState);
		break;
	case SC_BACKGROUND_DATA:
		onButtonDealBackground(pButton, nState);
		break;
	case SC_SELLER_BIOS:
		onButtonSellerBios(pButton, nState);
		break;
	case SC_OTHER_BIOS:
		onButtonOtherBios(pButton, nState);
		break;
	case SC_STAFF_BIOS:
		onButtonStaffBios(pButton, nState);
		break;
	case SC_DISPATCH:
		onButtonDispatchTeam(pButton, nState);
		break;
	case SC_EMAIL:
		onButtonCurrentEMail(pButton, nState);
		break;
	case SC_AUDIO:
		onButtonAudioSettings(pButton, nState);
		break;
	case SC_ORDER:
		onButtonRoboButler(pButton, nState);
		break;
	case SC_CHECK_TEAMS:
		onButtonCheckTeams(pButton, nState);
		break;
	case SC_CODE_WORDS:
		onButtonCodeWords(pButton, nState);
		break;
	default:
		break;
	}
}


void SrafComputer::setOn() {
	_eMode = SC_ON;

	activateMainScreen();
}

void SrafComputer::setOff() {
	if (_eMode != SC_OFF) {
		_eMode = SC_OFF;

		_pButtons[QUIT_BUTTON]->hide();
		_pButtons[OFF_BUTTON]->hide();
		_pButtons[ON_BUTTON]->show();

		setFocus();
	}

	deleteListBox();
	invalidateRect(&gCompDisplay);
	updateWindow();
}

void SrafComputer::setQuit() {
	close();
}

ErrorCode SrafComputer::createListBox() {
	ErrorCode errorCode = ERR_NONE;

	if (_pLBox == nullptr) {
		// We need to create one
		_pLBox = new CBofListBox;
		errorCode = _pLBox->create("ListBox", &gCompDisplay, this);
		if (errorCode != ERR_NONE) {
			return errorCode;
		}
	}

	_pLBox->setPointSize(_nListPointSize);
	_pLBox->setItemHeight(_nListItemHeight);
	_pLBox->setTextColor(_cTextColor);
	_pLBox->setHighlightColor(_cTextHiliteColor);
	_pLBox->setFont(FONT_MONO);
	_pLBox->setFocus();

	return errorCode;
}


void SrafComputer::deleteListBox() {
	if (_pLBox) {
		_pLBox->deleteAll();    // Clears all in the text box
	}
}

void SrafComputer::fillMain() {
	int i = 0;

	while (i < NUM_MAIN_ITEMS) {
		SrafCompItem *pCompItem = new SrafCompItem();

		pCompItem->_pItem = g_stMainItems[i];

		_pMainList->addToTail(*pCompItem);
		delete pCompItem;

		i++;
	}
}

void SrafComputer::onBofListBox(CBofObject * /*pListBox*/, int nItemIndex) {
	_nSelection = nItemIndex;

	// Do all kinds of neat things based on our current screen.
	switch (_eCurScreen) {
	case SC_MAIN:
		onListMainScreen();
		break;
	case SC_DEAL:
		onListDealSummary();
		break;
	case SC_BIDS:
		onListBuyerBids();
		break;
	case SC_BACKGROUND_DATA:
		onListDealBackground();
		break;
	case SC_SELLER_BIOS:
		onListSellerBios();
		break;
	case SC_OTHER_BIOS:
		onListOtherBios();
		break;
	case SC_STAFF_BIOS:
		onListStaffBios();
		break;
	case SC_DISPATCH:
		onListDispatchTeam();
		break;
	case SC_EMAIL:
		onListCurrentEMail();
		break;
	case SC_AUDIO:
		onListAudioSettings();
		break;
	case SC_ORDER:
		onListRoboButler();
		break;
	case SC_CHECK_TEAMS:
		onListCheckTeams();
		break;
	case SC_CODE_WORDS:
		onListCodeWords();
		break;
	default:
		break;
	}

	incrementTurnCount();
	updateWindow();
}

void SrafComputer::activateDealSummary() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	// Current screen is now the DEAL screen.
	_eCurScreen = SC_DEAL;

	// initialize point size and item height
	_nListPointSize = kBuyerBidsPointSize;
	_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"

	deactivateMainScreen();

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Retrieve IRK's and QUOSH's terms
	CBagVar *pVar = g_VarManager->getVariable("IRKSTERMS");
	if (pVar != nullptr) {
		g_stSellerNames[IRK4]._nAmount = pVar->getNumValue();
	}

	pVar = g_VarManager->getVariable("QUOSHSTERMS");
	if (pVar != nullptr) {
		g_stSellerNames[QUOSH23]._nAmount = pVar->getNumValue();
	}

	recalcSellerSummaryList();
	recalcBuyerSummaryList();

	assert(_pSellerSummaryList != nullptr);
	assert(_pBuyerSummaryList != nullptr);

	// load up the main list now with our deal summary data.

	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	//  Populate first line of list with header

	sStr = "DEAL SUMMARY: Armpit III";
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Align text with others in column
	sStr = "COSTS";
	alignAtColumn(sStr, "CURRENT TERMS", kRightColumnAlign);
	_pLBox->addToTail(sStr, false);

	// Populate the listbox with the sellers
	int numItems = _pSellerSummaryList->getCount();
	int totalAsking = 0;

	char szRightCol[128];
	bool bAskKnown = true;
	for (int i = 0; i < numItems; i++) {
		DealSummarySellerItem sellerItem = _pSellerSummaryList->getNodeItem(i);
		sStr = sellerItem._pSellerName;

		// Put the appropriate info in the right hand column
		if (sellerItem._nSellerOffer == -1) {
			Common::strcpy_s(szRightCol, kUnknownTermStr);
			bAskKnown = false;
		} else {
			Common::sprintf_s(szRightCol, "%2dgZ", sellerItem._nSellerOffer);

			totalAsking += sellerItem._nSellerOffer;
		}

		alignAtColumn(sStr, szRightCol, kRightColumnAlign);
		_pLBox->addToTail(sStr, false);
	}

	for (int i = 0; i < NUM_OTHER_PARTYS; i++) {
		switch (i) {
		case POLITICIANS:
		case ENVIRONMENTALISTS:
			sStr = g_stOtherPartys[i]._pszName;

			if (g_stOtherPartys[i]._nPaymentAmount == -1) {
				Common::strcpy_s(szRightCol, kUnknownTermStr);
				bAskKnown = false;
			} else {
				totalAsking += -g_stOtherPartys[i]._nPaymentAmount;
				Common::sprintf_s(szRightCol, "%2dgZ", -g_stOtherPartys[i]._nPaymentAmount);
			}

			alignAtColumn(sStr, szRightCol, kRightColumnAlign);
			_pLBox->addToTail(sStr, false);
			break;
		case SWONZA5:
		default:
			break;
		}

	}

	sStr = "TOTAL COSTS";
	if (bAskKnown) {
		Common::sprintf_s(szRightCol, "%2dgZ", totalAsking);
	} else {
		Common::strcpy_s(szRightCol, kUnknownTermStr);
	}

	alignAtColumn(sStr, szRightCol, kRightColumnAlign);
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	sStr = "BUYERS";
	alignAtColumn(sStr, "CURRENT OFFER", kRightColumnAlign);

	_pLBox->addToTail(sStr, false);

	// Populate the listbox with the buyers
	numItems = _pBuyerSummaryList->getCount();
	totalAsking = 0;

	for (int i = 0; i < numItems; i++) {
		DealSummaryBuyerItem buyerItem = _pBuyerSummaryList->getNodeItem(i);
		sStr = buyerItem._pBuyerName;

		// Put the appropriate info in the right hand column, don't want it
		// if it's zero though.
		if (buyerItem._nBuyerOffer != 0) {
			Common::sprintf_s(szRightCol, "%2dgZ", buyerItem._nBuyerOffer);
			totalAsking += buyerItem._nBuyerOffer;
			alignAtColumn(sStr, szRightCol, kRightColumnAlign);
			_pLBox->addToTail(sStr, false);
		}
	}

	// Just take care of swonza here.
	// Don't even list swonza if we have not resolved his terms yet.
	if (g_stOtherPartys[SWONZA5]._nPaymentAmount != -1) {

		sStr = g_stOtherPartys[SWONZA5]._pszName;

		totalAsking += g_stOtherPartys[SWONZA5]._nPaymentAmount;
		Common::sprintf_s(szRightCol, "%2dgZ", g_stOtherPartys[SWONZA5]._nPaymentAmount);

		alignAtColumn(sStr, szRightCol, kRightColumnAlign);
		_pLBox->addToTail(sStr, false);
	}

	// Update the asking with the bribes
	sStr = "TOTAL OFFER PRICE  ";

	Common::sprintf_s(szRightCol, "%2dgZ", totalAsking);
	alignAtColumn(sStr, szRightCol, kRightColumnAlign);

	_pLBox->addToTail(sStr, false);

	// Show list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[SUBMIT_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();
}

void SrafComputer::activateBuyerBids() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	// Current screen is now the BIDS screen.
	_eCurScreen = SC_BIDS;

	// initialize point size and item height
	_nListPointSize = kBuyerBidsPointSize;
	_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"
	deactivateMainScreen();

	// Load up the main list now with our bid data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Put up the column header
	sStr = kBuyerBidsHeaderStr;
	_pLBox->addToTail(sStr, false);

	// Walk through the grid and build individual strings and add to the list.
	for (int i = 0; i < NUM_BUYERS; i++) {
		sStr = buildBidString(i);
		_pLBox->addToTail(sStr, false);
		if (g_stBuyerBids[i]._bAccept) {
			_pLBox->setTextLineColor(i + 1, _cTextLineColor);
		}
	}

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	sStr = kBuyerBidsMessage1;
	_pLBox->addToTail(sStr, false);

	sStr = kBuyerBidsMessage2;
	_pLBox->addToTail(sStr, false);

	// Show the list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[DISPLAY_KEY_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();
}

CBofString &SrafComputer::buildBidString(int index) {
	char szRightCol[256];
	gBidStr = g_stBuyerBids[index]._pszName;

	for (int j = 0; j < NUM_MINERALS; j++) {
		// Don't display zeroes.
		if (g_stBuyerBids[index]._nMineralVal[j] == 0) {
			Common::strcpy_s(szRightCol, "  ");
		} else {
			Common::sprintf_s(szRightCol, "%2d", g_stBuyerBids[index]._nMineralVal[j]);
		}

		alignAtColumn(gBidStr, szRightCol, kFirstMineralColumn + j * kMineralColWidth);
	}

	Common::strcpy_s(szRightCol, (g_stBuyerBids[index]._bAccept ? "[X]" : "[ ]"));

	alignAtColumn(gBidStr, szRightCol, kFirstMineralColumn + NUM_MINERALS * kMineralColWidth + 2);

	return gBidStr;
}

void SrafComputer::activateDealBackground() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	_eCurScreen = SC_BACKGROUND_DATA;

	sStr = buildSrafDir("SRAFDEAL.TXT");
	displayTextScreen(sStr);
}

void SrafComputer::hideAllButtons() {
	assert(isValidObject(this));

	for (int i = 0; i < NUM_SRAFCOMPBUTT; i++) {
		if (_pButtons[i] != nullptr) {
			_pButtons[i]->hide();
		}
	}
}

void SrafComputer::activateSellerBios() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	// Current screen is now the SELLER BIOS screen.
	_eCurScreen = SC_SELLER_BIOS;

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = 20;

	// Get rid of the main screen, who needs it anyway!!!
	deactivateMainScreen();

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Load up the main list now with our deal summary data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Populate first line of list with header
	sStr = "SELLER BIOGRAPHIES";
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Populate the listbox with the staff names
	for (int i = 0; i < NUM_SELLERS; i++) {
		sStr = g_stSellerNames[i]._pszName;
		_pLBox->addToTail(sStr, false);
	}

	// Show list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();
}


void SrafComputer::activateOtherBios() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	// Current screen is now the OTHER BIOS screen.
	_eCurScreen = SC_OTHER_BIOS;

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = 20;

	// Get rid of the main screen, who needs it anyway!!!
	deactivateMainScreen();

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Load up the main list now with our deal summary data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Populate first line of list with header
	sStr = "OTHER BIOGRAPHIES";
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Populate the listbox with the other names
	for (int i = 0; i < NUM_OTHER_PARTYS; i++) {
		sStr = g_stOtherPartys[i]._pszName;
		_pLBox->addToTail(sStr, false);
	}

	// Show list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();
}

void SrafComputer::activateStaffBios() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	// Current screen is now the STAFF BIOS screen.
	_eCurScreen = SC_STAFF_BIOS;

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = 20;

	// Get rid of the main screen, who needs it anyway!!!
	deactivateMainScreen();

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Load up the main list now with our deal summary data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Populate first line of list with header
	sStr = "STAFF BIOGRAPHIES";
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Populate the listbox with the staff names
	for (int i = 0; i < NUM_STAFFERS; i++) {
		sStr = g_staffers[i]._pszStafferName;
		_pLBox->addToTail(sStr, false);
	}

	// Show list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();
}

void SrafComputer::activateDispatchTeam() {
	// Current screen is now the DISPATCH screen.
	_eCurScreen = SC_DISPATCH;

	// initialize point size and item height
	_nListPointSize = kBuyerBidsPointSize;
	_nListItemHeight = kLineItemHeight;

	// This screen, like most of the subscreens, is implemented as a text box.
	deactivateMainScreen();

	// Load up the main list now with our bid data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	recalcDispatchList(gDispatchCurState);

	// Show the list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[DISPATCH_TEAM_BUTTON]->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->paint();
	_pButtons[DISPATCH_TEAM_BUTTON]->paint();

	_pButtons[QUIT_BUTTON]->hide();
}


void SrafComputer::recalcDispatchList(int mExpansionFlag) {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);
	char szRightCol[256];
	bool bDone = false;
	int nListEntries = 0;
	const char *pMeetWith[3] = { "A Seller", "A Buyer", "Other Interested Party" };
	int nMeetWithEntries = 3;
	bool baddToTail = true;
	int nSkipped = 0;

	// Put up the column header and a blank line
	sStr = kDispatchHeaderStr;
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Keep track of the currently expanded state
	gDispatchCurState = mExpansionFlag;

	// Figure out how big our list will be
	switch (mExpansionFlag) {
	case mSellersExpanded:
	case mOthersExpanded:
	case 0:
		nListEntries = NUM_STAFFERS;
		break;

	case mBuyersExpanded:
		nListEntries = NUM_BUYERS - MIN_MART_ENTRIES + 3;
		break;
	}

	// Walk through the grid and build individual strings and add to the list.
	int i = 0;
	while (!bDone) {
		// First line is a special case... if any of the lists are expanded, then
		// start with that for a header.
		sStr = "    ";
		switch (mExpansionFlag) {
		case 0:
			if (i < nMeetWithEntries)
				sStr += pMeetWith[i];
			break;

		case mSellersExpanded:
			switch (i) {
			case 0:
				sStr += pMeetWith[0];
				break;
			case NUM_SELLERS + 1:
				sStr += pMeetWith[1];
				break;

			case NUM_SELLERS + 2:
				sStr += pMeetWith[2];
				break;

			default:
				if (i <= NUM_SELLERS) {
					// Only include those that are not being met with already.
					if (g_stSellerNames[i - 1]._bAvailable == true) {
						sStr += "[ ] ";
						sStr += g_stSellerNames[i - 1]._pszName;
						if (g_stSellerNames[i - 1]._bMeetWith) {
							sStr.replaceCharAt(kStandardIndentation + 1, kCheckMark);
						}
					} else {
						baddToTail = false;
					}
				}
			}
			break;

		case mBuyersExpanded:
			switch (i) {
			case 0:
			case 1:
				sStr += pMeetWith[i];
				break;

			case NUM_BUYERS - MIN_MART_ENTRIES + 2:
				sStr += pMeetWith[2];
				break;

			default:
				// Don't display the min mart entries, there's really no reason to
				// show them.
				// щnly include those that are not being met with already.
				if (g_stBuyerBids[i - 2]._bAvailable == true) {
					if (i <= (NUM_BUYERS - MIN_MART_ENTRIES + 1)) {
						sStr += "[ ] ";
						sStr += g_stBuyerBids[i - 2]._pszName;
						if (g_stBuyerBids[i - 2]._bMeetWith) {
							sStr.replaceCharAt(kStandardIndentation + 1, kCheckMark);
						}
					}
				} else {
					baddToTail = false;
				}
			}
			break;

		case mOthersExpanded:
			switch (i) {
			case 0:
			case 1:
			case 2:
				sStr += pMeetWith[i];
				break;

			default:
				if (i <= NUM_OTHER_PARTYS + 2) {
					if (g_stOtherPartys[i - 3]._bAvailable == true) {
						sStr += "[ ] ";
						sStr += g_stOtherPartys[i - 3]._pszName;
						if (g_stOtherPartys[i - 3]._bMeetWith) {
							sStr.replaceCharAt(kStandardIndentation + 1, kCheckMark);
						}
					} else {
						baddToTail = false;
					}
				}
			}
			break;
		}

		// Add the right hand column
		if (baddToTail == true) {
			if ((i - nSkipped) < NUM_STAFFERS) {
				Common::strcpy_s(szRightCol, g_staffers[i - nSkipped]._pszStafferName);
				alignAtColumn(sStr, szRightCol, kTeamMemberColumn);

				Common::strcpy_s(szRightCol, (g_staffers[i - nSkipped]._bAvailable ? "YES" : "NO "));
				alignAtColumn(sStr, szRightCol, kTeamAvailableColumn);

				Common::strcpy_s(szRightCol, (g_staffers[i - nSkipped]._bOnCurrentTeam ? "[X]" : "[ ]"));

				alignAtColumn(sStr, szRightCol, kTeamIncludeColumn - 1);
			}
			_pLBox->addToTail(sStr, false);
		} else {
			nSkipped++;
		}

		baddToTail = true;

		// Find out if we're done
		i++;
		if ((i - nSkipped) >= nListEntries) {
			bDone = true;
		}
	}

	// Include our footer in this list
	sStr = "";
	_pLBox->addToTail(sStr, false);

	sStr = kDispatchFooterStr1;
	_pLBox->addToTail(sStr, false);

	sStr = kDispatchFooterStr2;
	_pLBox->addToTail(sStr, false);

	sStr = kDispatchFooterStr3;
	_pLBox->addToTail(sStr, false);

	sStr = kDispatchFooterStr4;
	_pLBox->addToTail(sStr, false);

	sStr = kDispatchFooterStr5;
	_pLBox->addToTail(sStr, false);
}


void SrafComputer::activateCurrentEMail() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	char szRightCol[256];

	// Current screen is now the EMAIL screen.
	_eCurScreen = SC_EMAIL;

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = kLineItemHeight;

	// This screen, like most of the subscreens, will be implemented
	// as a text box.  The only event that we care about on this screen
	// is a mouse down on our "Make current offer to sellers"

	deactivateMainScreen();

	// load up the main list now with our bid data.

	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kEmailHeaderStr;
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Walk through the grid and build individual strings and add to the list.
	for (int i = 0; i < NUM_MAIL_MESSAGES; i++) {
		sStr = g_stEmailMessages[i]._pszMessageSubj;

		Common::strcpy_s(szRightCol, g_stEmailMessages[i]._pszMessageFrom);
		alignAtColumn(sStr, szRightCol, kMessageFromColumn);

		Common::strcpy_s(szRightCol, g_stEmailMessages[i]._pszMessageTo);
		alignAtColumn(sStr, szRightCol, kMessageToColumn);

		_pLBox->addToTail(sStr, false);
	}

	// Show the list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();
}


void SrafComputer::activateAudioSettings() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	// current screen is now the AUDIO screen.
	_eCurScreen = SC_AUDIO;

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = kLineItemHeight;

	// This screen, like most of the subscreens, will be implemented
	// as a text box.  The only event that we care about on this screen
	// is a mouse down on our "Make current offer to sellers"
	deactivateMainScreen();

	// Load up the main list now with our bid data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kAudioHeaderStr;
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Walk through the titles and build the list
	char szRightCol[256];
	for (int i = 0; i < NUM_MUSICAL_SCORES; i++) {
		sStr = g_stAudioSetting[i]->_pszTitle;

		Common::strcpy_s(szRightCol, g_stAudioSetting[i]->_pszAuthor);
		alignAtColumn(sStr, szRightCol, kAudioAuthorCol);

		Common::strcpy_s(szRightCol, g_stAudioSetting[i]->_pszPlanet);
		alignAtColumn(sStr, szRightCol, kAudioPlanetCol);

		_pLBox->addToTail(sStr, false);
	}

	// Show the list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();

	// Only show the no music button if music is playing.
	bool bAnythingPlaying = CBofSound::midiSoundPlaying();
	if (bAnythingPlaying) {
		_pButtons[NO_MUSIC_BUTTON]->show();
	}
	_pButtons[QUIT_BUTTON]->hide();
}


void SrafComputer::activateRoboButler() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	char szRightCol[256];

	// Current screen is now the robobutler screen.
	_eCurScreen = SC_ORDER;

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = kLineItemHeight;

	// This screen, like most of the subscreens, will be implemented
	// as a text box.  The only event that we care about on this screen
	// is a mouse down on our "Make current offer to sellers"

	deactivateMainScreen();

	// Load up the main list now with our bid data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kRobobutlerHeaderStr;
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Walk through the titles and build the list
	for (int i = 0; i < NUM_OFFERINGS; i++) {
		sStr = g_stOfferings[i]._pszType;

		Common::strcpy_s(szRightCol, g_stOfferings[i]._pszOffering);
		alignAtColumn(sStr, szRightCol, kRobobutlerCol);

		_pLBox->addToTail(sStr, false);
	}

	// Show the list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();

	doShowChowButtons();
	updateWindow();
}

void SrafComputer::doShowChowButtons() {
	// Only list this stuff if it has not already been ordered.
	bool bAllowBeverage = true;
	bool bAllowSnack = true;

	CBagStorageDev *pSDev = g_SDevManager->getStorageDevice("INV_WLD");
	if (pSDev) {
		CBagObject *pBevObj = pSDev->getObject("SZTB", true);
		CBagObject *pSnackObj = pSDev->getObject("SZTA", true);

		if (pSnackObj) {
			bAllowSnack = false;
		}

		if (pBevObj) {
			bAllowBeverage = false;
		}
	}

	if (bAllowBeverage) {
		_pButtons[ORDER_BEVERAGE_BUTTON]->show();
	} else {
		_pButtons[ORDER_BEVERAGE_BUTTON]->hide();
	}

	if (bAllowSnack) {
		_pButtons[ORDER_SNACK_BUTTON]->show();
	} else {
		_pButtons[ORDER_SNACK_BUTTON]->hide();
	}
}

void SrafComputer::activateCheckTeams() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);
	char szAttendeesBuff[256];
	szAttendeesBuff[0] = '\0';
	CBofString aStr(szAttendeesBuff, 256);

	// Current screen is now the Check teams screen.
	_eCurScreen = SC_CHECK_TEAMS;

	// initialize point size and item height
	//
	// Set this stuff to a tiny point size so that it won't flow over the
	// right hand side of the screen.
	_nListPointSize = kBuyerBidsPointSize;
	_nListItemHeight = kLineItemHeight;

	// This screen, like most of the subscreens, will be implemented
	// as a text box.  The only event that we care about on this screen
	// is a mouse down on our "Make current offer to sellers"

	deactivateMainScreen();

	// Load up the main list now with our bid data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kCheckTeamHeaderStr1;
	_pLBox->addToTail(sStr, false);

	sStr = kCheckTeamHeaderStr2;
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	int numItems = (_pTeamList == nullptr ? 0 : _pTeamList->getCount());

	// Walk through the list and build individual strings and add to the list.
	int  nNumAttendees = 0;
	int  nMaxNumAttendees = 0;

	for (int i = 0; i < numItems; i++) {
		DispatchedTeamItem teamListItem = _pTeamList->getNodeItem(i);

		// Figure out who they are meeting with...
		if (teamListItem._nFlags & mOtherParty) {
			sStr = g_stOtherPartys[teamListItem._nMeetWithID]._pszName;
		} else if (teamListItem._nFlags & mBuyer) {
			sStr = g_stBuyerBids[teamListItem._nMeetWithID]._pszName;
		} else {
			sStr = g_stSellerNames[teamListItem._nMeetWithID]._pszName;
		}

		// List the dispatch time
		char szRightCol[256];
		Common::sprintf_s(szRightCol,
		                  "%02d:%02d",
		                  teamListItem._nDispatchTime / 100,
		                  teamListItem._nDispatchTime % 100);
		alignAtColumn(sStr, szRightCol, kCheckTeamTimeCol);

		// Now build the list of attendees.
		bool bFirstAttendee = true;
		int mFlag = 0;

		// Count number of attendees first, this will help us format our list
		for (int j = 0; j < NUM_STAFFERS; j++) {
			switch (j) {
			case 0:
				mFlag = mNorg72;
				break;
			case 1:
				mFlag = mPnurth81;
				break;
			case 2:
				mFlag = mZig64;
				break;
			case 3:
				mFlag = mLentil24;
				break;
			case 4:
				mFlag = mVargas20;
				break;
			case 5:
				mFlag = mChurg53;
				break;
			}

			if (teamListItem._nFlags & mFlag) {
				nNumAttendees++;
			}
		}

		for (int j = 0; j < NUM_STAFFERS; j++) {
			switch (j) {
			case 0:
				mFlag = mNorg72;
				break;
			case 1:
				mFlag = mPnurth81;
				break;
			case 2:
				mFlag = mZig64;
				break;
			case 3:
				mFlag = mLentil24;
				break;
			case 4:
				mFlag = mVargas20;
				break;
			case 5:
				mFlag = mChurg53;
				break;
			}

			if (teamListItem._nFlags & mFlag) {
				if (bFirstAttendee) {
					aStr = g_staffers[j]._pszStafferName;
					bFirstAttendee = false;
				} else {
					aStr += ", ";
					aStr += g_staffers[j]._pszStafferName;
				}
			}
		}

		// save high water mark.
		if (nNumAttendees > nMaxNumAttendees) {
			nMaxNumAttendees = nNumAttendees;
		}
		nNumAttendees = 0;

		alignAtColumn(sStr, szAttendeesBuff, kCheckTeamStaffCol);
		_pLBox->addToTail(sStr, false);
	}

	// Decrease point size if we got too much for one screen
	if (nMaxNumAttendees > 4) {
		_nListPointSize = kOtherPointSize;
		_nListItemHeight = kLineItemHeight;
		_pLBox->setPointSize(_nListPointSize);
		_pLBox->setItemHeight(_nListItemHeight);
	}

	// Display the current time...
	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Add a dummy line so we can overwrite it with the current turncount.
	sStr = " ";
	_pLBox->addToTail(sStr, false);

	int nLineNo = _pLBox->getNumItems();
	displayTurnCount(nLineNo - 1);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

#define kCheckSuccessfulNegotiations "SUCCESSFUL NEGOTIATIONS: "
#define kCheckUnsuccessfulNegotiations "UNSUCCESSFUL NEGOTIATIONS: "
	// Add a few lines indicating who has succeeded and who has failed.
	char        szGoodMeetings[256];
	char        szBadMeetings[256];
	CBofString  sGoodMeetings(szGoodMeetings, 256);
	CBofString  sBadMeetings(szBadMeetings, 256);
	bool        bSFirstTime = true;
	bool        bUFirstTime = true;

	sGoodMeetings = "";
	sBadMeetings = "";

	for (int i = 0; i < NUM_SELLERS; i++) {
		if (g_stSellerNames[i]._nMeetingResult != SRAF_NO_MEETING) {
			if (g_stSellerNames[i]._nMeetingResult == SRAF_GOOD_MEETING) {
				if (bSFirstTime == true) {
					bSFirstTime = false;
				} else {
					sGoodMeetings += ", ";
				}
				sGoodMeetings += g_stSellerNames[i]._pszName;
			} else {
				if (bUFirstTime == true) {
					bUFirstTime = false;
				} else {
					sBadMeetings += ", ";
				}
				sBadMeetings += g_stSellerNames[i]._pszName;
			}
		}
	}

	// Scan the buyers for meetings
	for (int i = 0; i < NUM_BUYERS; i++) {
		if (g_stBuyerBids[i]._nMeetingResult != SRAF_NO_MEETING) {
			if (g_stBuyerBids[i]._nMeetingResult == SRAF_GOOD_MEETING) {
				if (bSFirstTime == true) {
					bSFirstTime = false;
				} else {
					sGoodMeetings += ", ";
				}
				sGoodMeetings += g_stBuyerBids[i]._pszName;
			} else {
				if (bUFirstTime == true) {
					bUFirstTime = false;
				} else {
					sBadMeetings += ", ";
				}
				sBadMeetings += g_stBuyerBids[i]._pszName;
			}
		}
	}

	// Scan the other partys for meetings
	for (int i = 0; i < NUM_OTHER_PARTYS; i++) {
		if (g_stOtherPartys[i]._nMeetingResult != SRAF_NO_MEETING) {
			if (g_stOtherPartys[i]._nMeetingResult == SRAF_GOOD_MEETING) {
				if (bSFirstTime == true) {
					bSFirstTime = false;
				} else {
					sGoodMeetings += ", ";
				}
				sGoodMeetings += g_stOtherPartys[i]._pszName;
			} else {
				if (bUFirstTime == true) {
					bUFirstTime = false;
				} else {
					sBadMeetings += ", ";
				}
				sBadMeetings += g_stOtherPartys[i]._pszName;
			}
		}
	}

	// Display all this info we just calculated...
	sStr = kCheckSuccessfulNegotiations;
	_pLBox->addToTail(sStr, false);

	_pLBox->addToTail(sGoodMeetings, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	sStr = kCheckUnsuccessfulNegotiations;
	_pLBox->addToTail(sStr, false);

	_pLBox->addToTail(sBadMeetings, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	sStr = kCheckDispatchTeamFooter1;
	_pLBox->addToTail(sStr, false);

	sStr = kCheckDispatchTeamFooter2;
	_pLBox->addToTail(sStr, false);

	// show the list box
	_pLBox->show();

	_pButtons[RETURN_TO_MAIN_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();
}


void SrafComputer::activateCodeWords() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);
	char szRightCol[128];

	// Current screen is now the CODE WORDS screen.
	_eCurScreen = SC_CODE_WORDS;

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = kLineItemHeight;

	// Get rid of the main screen, who needs it anyway!!!
	deactivateMainScreen();

	// Make sure we start with the correct computer background
	if (getBackdrop()) {
		paintBackdrop();
	}

	// Load up the main list now with our deal summary data.
	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	// Populate first line of list with header
	sStr = kCodeWordsHeader1;
	_pLBox->addToTail(sStr, false);

	sStr = kCodeWordsHeader2;
	_pLBox->addToTail(sStr, false);

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	sStr = kCodeWordsHeader3;
	_pLBox->addToTail(sStr, false);

	// Populate the listbox with the staff names
	for (int i = 0; i < NUM_CODE_WORDS; i++) {
		sStr = g_stCodeWords[i]._pszCodeWord1;

		Common::strcpy_s(szRightCol, g_stCodeWords[i]._pszCodeWord2);
		alignAtColumn(sStr, szRightCol, kGroup1Col2);

		Common::strcpy_s(szRightCol, g_stCodeWords[i]._pszCodeWord3);
		alignAtColumn(sStr, szRightCol, kGroup2Col1);

		Common::strcpy_s(szRightCol, g_stCodeWords[i]._pszCodeWord4);
		alignAtColumn(sStr, szRightCol, kGroup2Col2);

		_pLBox->addToTail(sStr, false);
	}

	// Blank line and add to end

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	sStr = "CODE uint16 PAIR: (,)";
	_pLBox->addToTail(sStr, false);

	// Show list box
	_pLBox->show();

	// Use the done button instead of return to main.
	_pButtons[DONE_BUTTON]->show();
	_pButtons[QUIT_BUTTON]->hide();

	updateWindow();
}


void SrafComputer::initDealSummary() {
	// initialize the initial state of the deal summary
	recalcSellerSummaryList();
	recalcBuyerSummaryList();
}

//	Allocate everything we need here...
void SrafComputer::initBuyerBids() {
}

void SrafComputer::initDealBackground() {
}

void SrafComputer::initSellerBios() {
}

void SrafComputer::initOtherBios() {
}

void SrafComputer::initStaffBios() {
}

int SrafComputer::getMeetMember(int nListToSearch) {
	switch (nListToSearch) {
	case 0:
		break;

	case kOthersList:
		for (int i = 0; i < NUM_OTHER_PARTYS; i++) {
			if (g_stOtherPartys[i]._bMeetWith)
				return i;
		}
		break;

	case kSellersList:
		for (int i = 0; i < NUM_SELLERS; i++) {
			if (g_stSellerNames[i]._bMeetWith)
				return i;
		}
		break;


	case kBuyersList:
		for (int i = 0; i < NUM_BUYERS; i++) {
			if (g_stBuyerBids[i]._bMeetWith)
				return i;
		}
		break;


	case kStaffersList:
		for (int i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i]._bOnCurrentTeam)
				return i;
		}
		break;
	}
	return -1;
}

int SrafComputer::getAdjustedIndex(int nListToSearch, int nElementIndex, bool bScreenToIndex) {
	int nTotalNotAvail = 0;
	int nOrigElementIndex = nElementIndex;

	switch (nListToSearch) {
	case 0:
		break;

	case kOthersList:
		// Adjust for out of range.
		nElementIndex = (nElementIndex >= NUM_OTHER_PARTYS ? (NUM_OTHER_PARTYS - 1) : nElementIndex);

		for (int i = 0; i < NUM_OTHER_PARTYS; i++) {
			// Get all consecutive that are not available.
			while (i < NUM_OTHER_PARTYS && g_stOtherPartys[i]._bAvailable == false) {
				nTotalNotAvail++;
				i++;
			}

			// If we exceeded our bounds, or our counter, minus the number out is
			// the element index, then we're done.
			if (i >= NUM_OTHER_PARTYS || (i - nTotalNotAvail) >= nElementIndex) {
				break;
			}
		}
		break;

	case kSellersList:
		// Adjust for out of range.
		nElementIndex = (nElementIndex >= NUM_SELLERS ? (NUM_SELLERS - 1) : nElementIndex);

		for (int i = 0; i < NUM_BUYERS; i++) {
			// get all consecutive that are not available.
			while (i < NUM_SELLERS && g_stSellerNames[i]._bAvailable == false) {
				nTotalNotAvail++;
				i++;
			}

			// If we exceeded our bounds, or our counter, minus the number out is
			// the element index, then we're done.
			if (i >= NUM_SELLERS || (i - nTotalNotAvail) >= nElementIndex) {
				break;
			}
		}
		break;

	case kBuyersList:
		// Adjust for out of range.
		nElementIndex = (nElementIndex >= NUM_BUYERS ? (NUM_BUYERS - 1) : nElementIndex);

		for (int i = 0; i < NUM_BUYERS; i++) {
			// Get all consecutive that are not available.
			while (i < NUM_BUYERS && g_stBuyerBids[i]._bAvailable == false) {
				nTotalNotAvail++;
				i++;
			}

			// If we exceeded our bounds, or our counter, minus the number out is
			// the element index, then we're done.
			if (i >= NUM_BUYERS || (i - nTotalNotAvail) >= nElementIndex) {
				break;
			}
		}
		break;

	case kStaffersList:
		// Adjust for out of range.
		nElementIndex = (nElementIndex >= NUM_STAFFERS ? (NUM_STAFFERS - 1) : nElementIndex);

		for (int i = 0; i < NUM_STAFFERS; i++) {
			// Get all consecutive that are not available.
			while (i < NUM_STAFFERS && g_staffers[i]._bAvailable == false) {
				nTotalNotAvail++;
				i++;
			}

			// If we exceeded our bounds, or our counter, minus the number out is
			// the element index, then we're done.
			if (i >= NUM_STAFFERS || (i - nTotalNotAvail) >= nElementIndex) {
				break;
			}
		}
		break;
	}

	if (bScreenToIndex) {
		return nOrigElementIndex + nTotalNotAvail;
	}

	return nOrigElementIndex - nTotalNotAvail;
}

void SrafComputer::initDispatchTeam() {
	// Go through the entire list and make sure that they are all not on the
	// current team.

	for (int i = 0; i < NUM_STAFFERS; i++) {
		g_staffers[i]._bOnCurrentTeam = false;
	}

	for (int i = 0; i < NUM_SELLERS; i++) {
		g_stSellerNames[i]._bMeetWith = false;
	}

	for (int i = 0; i < NUM_BUYERS; i++) {
		g_stBuyerBids[i]._bMeetWith = false;
	}

	for (int i = 0; i < NUM_OTHER_PARTYS; i++) {
		g_stOtherPartys[i]._bMeetWith = false;
	}
}

void SrafComputer::initCurrentEMail() {
}

void SrafComputer::initAudioSettings() {
}

void SrafComputer::initRoboButler() {
}

void SrafComputer::onListMainScreen() {
	switch (_nSelection) {
	case DEALSUMMARY:
		activateDealSummary();
		break;

	case BUYERBIDS:
		activateBuyerBids();
		break;

	case DEALBACKGROUND:
		activateDealBackground();
		break;

	case SELLERBIOS:
		activateSellerBios();
		break;

	case OTHERBIOS:
		activateOtherBios();
		break;

	case STAFFBIOS:
		activateStaffBios();
		break;

	case DISPATCHTEAM:
		initDispatchTeam();
		activateDispatchTeam();
		break;

	case CURRENTEMAIL:
		activateCurrentEMail();
		break;

	case AUDIOSETTINGS:
		activateAudioSettings();
		break;

	case ROBOBUTLER:
		activateRoboButler();
		break;

	case CHECKTEAMS:
		activateCheckTeams();
		break;

	default:
		break;
	}
	_nSelection = -1;
}

void SrafComputer::onListDealSummary() {
}

void SrafComputer::onListBuyerBids() {
	CBofPoint   cPoint = getPrevMouseDown();    // Already in local coords
	int         index = _nSelection - 1;

	// Make sure that this is not out of range.
	if (index >= NUM_BUYERS) {
		return;
	}

	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	// Calculate the text width based on the attributes of the text
	// rather than guessing to where they are.
	sStr = " ACCEPT ";
	CBofRect        cAcceptBidRect = calculateTextRect(this, &sStr, kBuyerBidsPointSize, FONT_MONO);

	alignAtColumn(sStr, "", kFirstMineralColumn + NUM_MINERALS * kMineralColWidth);
	CBofRect cDummyRect = calculateTextRect(this, &sStr, kBuyerBidsPointSize, FONT_MONO);

	CBofPoint cStartPoint(cDummyRect.right, 0);
	cAcceptBidRect.offsetRect(cStartPoint);
	cAcceptBidRect.bottom = 1000;

	//  Based on the location of the mouse determine if we should check/uncheck
	//  the "accept bid" field or present a bio on he who was checked.
	if (cAcceptBidRect.ptInRect(cPoint)) {

		// Negate the current value
		g_stBuyerBids[index]._bAccept = !g_stBuyerBids[index]._bAccept;

		// Redraw with the new one checked/unchecked
		CBofString cStr = buildBidString(index);
		_pLBox->setText(_nSelection, cStr);
		if (g_stBuyerBids[index]._bAccept) {
			_pLBox->setTextLineColor(_nSelection, _cTextLineColor);
		} else {
			_pLBox->setTextLineColor(_nSelection, (COLORREF) - 1); // Restore default
		}

		_pLBox->repaintItem(_nSelection);

	} else {
		szLocalBuff[0] = '\0';
		CBofString sStr2(szLocalBuff, 256);

		if (index >= 0) {
			sStr2 = buildSrafDir(g_stBuyerBids[index]._pszBuyerBio);
			displayTextScreen(sStr2);
		}
	}
}

void SrafComputer::onListDealBackground() {
}

void SrafComputer::onListSellerBios() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (_nSelection >= 2) {
		sStr = buildSrafDir(g_stSellerNames[_nSelection - 2]._pszSellerBio);
		displayTextScreen(sStr);
	}
}

void SrafComputer::onListOtherBios() {
	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (_nSelection >= 2) {
		sStr = buildSrafDir(g_stOtherPartys[_nSelection - 2]._pszOtherPartyBio);
		displayTextScreen(sStr);
	}
}


void SrafComputer::onListStaffBios() {
	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (_nSelection >= 2) {
		sStr = buildSrafDir(g_staffers[_nSelection - 2]._pszStafferBio);
		displayTextScreen(sStr);
	}
}

void SrafComputer::onListDispatchTeam() {
	CBofPoint   cPoint = getPrevMouseDown();    // Already in local coords
	CBofRect    cMeetWithRect(0,
	                          0,
	                          (kTeamMemberColumn - kStandardIndentation) * kBuyerBidsPointWidth,
	                          gCompDisplay.height());
	CBofRect    cTeamMembersRect((kTeamMemberColumn - kStandardIndentation) * kBuyerBidsPointWidth,
	                             0,
	                             gCompDisplay.width(),
	                             gCompDisplay.height());
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);
	int nElementIndex;

	// First two entries in list mean nothing to us...
	if (_nSelection < 2)
		return;

	// Check if we're in the left hand column, if we are, then determine
	// if we're left of the list or in the list itself.
	if (cMeetWithRect.ptInRect(cPoint)) {
		nElementIndex = _nSelection - 3;   // three header lines before data starts

		bool        bDeleteAll = false;
		int         nrecalcVal = 0;
		CBofRect    cMeetBio(cMeetWithRect.left + kStandardDoubleIndentation * kBuyerBidsPointWidth,
		                     cMeetWithRect.top, cMeetWithRect.right, cMeetWithRect.bottom);
		CBofRect    cMeetMember(0, 0, cMeetWithRect.left + kStandardDoubleIndentation * kBuyerBidsPointWidth, cMeetWithRect.bottom);
		int         nMeetMember = -1;
		bool        bInMeetMemberColumn = false;
		int         nPreceedingHeaders = 0;
		int         nListToCheck = 0;

		switch (gDispatchCurState) {
		case 0:
			bDeleteAll = true;
			nrecalcVal = (nElementIndex == -1 ? mSellersExpanded :
			              (nElementIndex == 0 ? mBuyersExpanded :
			               (nElementIndex == 1 ? mOthersExpanded : 0)));
			break;
		case mOthersExpanded:
			nListToCheck = kOthersList;             // List to check
			nPreceedingHeaders = 2;                 // Buyers and sellers come before this guy...

			switch (nElementIndex) {

			case -1:
				bDeleteAll = true;
				nrecalcVal = mSellersExpanded;      // Collapse list, expand sellers
				break;
			case 0:
				bDeleteAll = true;
				nrecalcVal = mBuyersExpanded;       // Collapse list, expand buyers
				break;
			case 1:
				bDeleteAll = true;
				nrecalcVal = 0;                     // Collapse list
				break;
			default:
				nElementIndex -= nPreceedingHeaders;    // Account for headers
				// Account for those guys out on meetings that we have not displayed
				// recalc first...
				nElementIndex = getAdjustedIndex(nListToCheck, nElementIndex, true);

				if (nElementIndex >= NUM_OTHER_PARTYS)
					break;

				if (cMeetBio.ptInRect(cPoint)) {        // if so, bring up biography.
					sStr = buildSrafDir(g_stOtherPartys[nElementIndex]._pszOtherPartyBio);
					displayTextScreen(sStr);
				} else if (cMeetMember.ptInRect(cPoint)) {         // if so, put a checkmark in that column.
					// Uncheck any member we already have checked, this is a singular operation
					nMeetMember = getMeetMember(nListToCheck);
					if (nMeetMember != -1 && nMeetMember < NUM_OTHER_PARTYS) {
						g_stOtherPartys[nMeetMember]._bMeetWith = false;
					}

					// Now put the check mark in the column for the new guy to meet
					if (nMeetMember != nElementIndex && nMeetMember < NUM_OTHER_PARTYS) {
						g_stOtherPartys[nElementIndex]._bMeetWith = true;
						bInMeetMemberColumn = true;
					}
				}
				break;
			}
			break;

		case mSellersExpanded:
			nListToCheck = kSellersList;
			// No headers above this guy
			nPreceedingHeaders = 0;

			if (nElementIndex == -1) {
				bDeleteAll = true;
				// Collapse list
				nrecalcVal = 0;
			} else {
				nElementIndex -= nPreceedingHeaders; // Account for headers

				// Account for those guys out on meetings that we have not displayed
				// recalc first...
				nElementIndex = getAdjustedIndex(nListToCheck, nElementIndex, true);

				if (nElementIndex >= NUM_SELLERS) {
					if (nElementIndex == (NUM_SELLERS - nPreceedingHeaders)) {
						bDeleteAll = true;
						// collapse list, expand buyers
						nrecalcVal = mBuyersExpanded;
					}

					if (nElementIndex == (NUM_SELLERS + 1 - nPreceedingHeaders)) {
						bDeleteAll = true;
						// collapse list, expand others
						nrecalcVal = mOthersExpanded;
					}
				} else if (cMeetBio.ptInRect(cPoint)) {
					// If so, bring up biography.
					sStr = buildSrafDir(g_stSellerNames[nElementIndex]._pszSellerBio);
					displayTextScreen(sStr);
				} else if (cMeetMember.ptInRect(cPoint)) {
					// If so, put a check mark in that column.

					// Uncheck any member we already have checked, this is a singular operation
					nMeetMember = getMeetMember(nListToCheck);
					if (nMeetMember != -1 && nMeetMember < NUM_SELLERS) {
						g_stSellerNames[nMeetMember]._bMeetWith = false;
					}

					// Now put the check mark in the column for the new guy to meet
					if (nMeetMember != nElementIndex && nMeetMember < NUM_SELLERS) {
						g_stSellerNames[nElementIndex]._bMeetWith = true;
						bInMeetMemberColumn = true;
					}
				}
			}
			break;

		case mBuyersExpanded:
			// recalc first...
			nListToCheck = kBuyersList;
			nPreceedingHeaders = 1;

			switch (nElementIndex) {
			case -1:
				bDeleteAll = true;
				nrecalcVal = mSellersExpanded;      // Collapse list, expand sellers
				break;

			case 0:
				bDeleteAll = true;
				nrecalcVal = 0;                     // Collapse list
				break;

			default:
				nElementIndex -= nPreceedingHeaders;    // Correct for other headers
				nElementIndex = getAdjustedIndex(nListToCheck, nElementIndex, true);

				if (nElementIndex >= (NUM_BUYERS - MIN_MART_ENTRIES)) {
					if (nElementIndex == (NUM_BUYERS - MIN_MART_ENTRIES + 1 - nPreceedingHeaders)) {
						bDeleteAll = true;
						nrecalcVal = mOthersExpanded;       // Collapse list, expand others
					}
					break;
				}

				// Account for those guys out on meetings that we have not displayed

				if (cMeetBio.ptInRect(cPoint)) {
					// if so, bring up biography.
					sStr = buildSrafDir(g_stBuyerBids[nElementIndex]._pszBuyerBio);
					displayTextScreen(sStr);
				} else if (cMeetMember.ptInRect(cPoint)) {
					// if so, put a checkmark in that column.
					// Uncheck any member we already have checked, this is a singular operation
					nMeetMember = getMeetMember(nListToCheck);
					if (nMeetMember != -1) {
						g_stBuyerBids[nMeetMember]._bMeetWith = false;
					}

					// Now put the check mark in the column for the new guy to meet
					if (nMeetMember != nElementIndex) {
						g_stBuyerBids[nElementIndex]._bMeetWith = true;
						bInMeetMemberColumn = true;
					}
				}
				break;
			}
			break;
		}

		// If we have to uncheck a column, do that here.
		if (nMeetMember != -1) {
			int nThisItemAt = getAdjustedIndex(nListToCheck, nMeetMember, false) + 3 + nPreceedingHeaders;
			sStr = _pLBox->getText(nThisItemAt);
			sStr.replaceCharAt(kStandardIndentation + 1, ' ');
			_pLBox->setText(nThisItemAt, sStr);
			_pLBox->repaintItem(nThisItemAt);
		}

		// If we have a new column to check, do that here.
		if (bInMeetMemberColumn) {
			sStr = _pLBox->getText(_nSelection);
			sStr.replaceCharAt(kStandardIndentation + 1, kCheckMark);       // ??? works fine on mac, not sure what check mark is for pc
			_pLBox->setText(_nSelection, sStr);
			_pLBox->repaintItem(_nSelection);
		}

		if (bDeleteAll) {
			_pLBox->deleteAll();
			recalcDispatchList(nrecalcVal);
			_pLBox->show();
		}
	}

	// Handle if we're in the right hand side of the screen.  We only care about
	// two columns, the staff member names and the guys check boxes on the right.

	if (cTeamMembersRect.ptInRect(cPoint)) {
		nElementIndex = _nSelection - 2; // two header lines before data starts
		if (nElementIndex >= 0 && nElementIndex < NUM_STAFFERS) {
			CBofRect cStaffNames(cTeamMembersRect.left + kStandardIndentation * kBuyerBidsPointWidth,
			                     cTeamMembersRect.top,
			                     cTeamMembersRect.left + (kStandardIndentation + kMaxStafferNameLen) * kBuyerBidsPointWidth + 60,
			                     cTeamMembersRect.bottom);
			CBofRect cStaffInclude((kTeamIncludeColumn - kStandardIndentation) * kBuyerBidsPointWidth,
			                       cTeamMembersRect.top,
			                       cTeamMembersRect.right,
			                       cTeamMembersRect.bottom);

			//  If in the staff names column, then show the biography
			if (cStaffNames.ptInRect(cPoint)) {
				sStr = buildSrafDir(g_staffers[nElementIndex]._pszStafferBio);
				displayTextScreen(sStr);
			} else {
				// if in the "include on team" column, then handle appropriately
				if (cStaffInclude.ptInRect(cPoint)) {
					// Make sure that this dude is available
					if (g_staffers[nElementIndex]._bAvailable) {
						char cNewChar = ' ';
						if (g_staffers[nElementIndex]._bOnCurrentTeam == false) {
							cNewChar = kCheckMark;
						}

						// Negate
						g_staffers[nElementIndex]._bOnCurrentTeam = !g_staffers[nElementIndex]._bOnCurrentTeam;

						sStr = _pLBox->getText(_nSelection);
						sStr.replaceCharAt(kTeamIncludeColumn, cNewChar);
						_pLBox->setText(_nSelection, sStr);
						_pLBox->repaintItem(_nSelection);
					}
				}
			}
		}
	}

}

void SrafComputer::onListCurrentEMail() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (_nSelection >= 2) {
		sStr = buildSrafDir(g_stEmailMessages[_nSelection - 2]._pszMessageFile);
		displayTextScreen(sStr);
	}
}

void SrafComputer::onListAudioSettings() {
	int nTrackSelection = _nSelection - 2;

	// Reject out of range selections
	if (nTrackSelection < 0 || nTrackSelection >= NUM_MUSICAL_SCORES) {
		return;
	}

	// Start the new track (will stop any track playing)
	//
	// Add a selection for random play.
	if (g_stAudioSetting[nTrackSelection]->_pszAudioFile == nullptr) {
		_bRandomAudio = true;
		CBagVar *pVar = g_VarManager->getVariable("SRATURNCOUNT");
		_nRandomTime = pVar->getNumValue();
		nTrackSelection = g_engine->getRandomNumber() % (NUM_MUSICAL_SCORES - 1);
	} else {
		_bRandomAudio = false;
	}

	// Now start playing...
	if (g_stAudioSetting[nTrackSelection]->_pMidiTrack != nullptr) {
		g_stAudioSetting[nTrackSelection]->_pMidiTrack->play();
	}

	// If state changes, then change the button also
	bool bAnythingPlaying = CBofSound::midiSoundPlaying();
	if (bAnythingPlaying) {
		_pButtons[NO_MUSIC_BUTTON]->show();
	} else {
		_pButtons[NO_MUSIC_BUTTON]->hide();
	}
}

void SrafComputer::onListRoboButler() {
	int nSelection = _nSelection - 2;

	if (nSelection < 0 || nSelection >= NUM_OFFERINGS) {
		return;
	}

	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString  sStr(szLocalBuff, 256);

	sStr = buildSrafDir(g_stOfferings[nSelection]._pszFile);
	displayTextScreen(sStr);
}

// Here are the constants that define how strong a team must be in order
// to succeed with any given buyer, seller or other interested party
//
// Here's the negotiating constant associated with each...
//
//  Norg-72     1
//  Pnurth-81   3
//  Zig-64      6
//  Lentil-24   5       (double this when sent alone)
//  Vargas-20   10
//  Churg-53    5

#define YEEF8_REQUIRED_TEAM_STRENGTH        10
#define SINJIN11_REQUIRED_TEAM_STRENGTH     12
#define GUNG14_REQUIRED_TEAM_STRENGTH       13
#define GILD13_REQUIRED_TEAM_STRENGTH       14
#define DORK44_REQUIRED_TEAM_STRENGTH       11

bool SrafComputer::reportMeetingStatus(int nTeamNumber) {
	const char *pszFailureFile = nullptr;
	const char *pszSuccessFile = nullptr;
	bool                    bDone = false;
	int                     nTeamCaptain = 0;
	char                    szLocalResponse[256];
	szLocalResponse[0] = '\0';
	CBofString              sResponse(szLocalResponse, 256);
	bool                    bTimeElapsed = true;
	bool                    bNeedRedraw = false;

	// If the text screen is frontmost, then don't report meeting status
	if (gTextScreenFrontmost == true) {
		return bNeedRedraw;
	}

	// Could possibly have no list established.
	if (_pTeamList == nullptr) {
		return bNeedRedraw;
	}

	int numItems = _pTeamList->getCount();

	// Handle out of range case first.
	if (nTeamNumber < 0 || nTeamNumber >= numItems) {
		return bNeedRedraw;
	}

	DispatchedTeamItem teamListItem = _pTeamList->getNodeItem(nTeamNumber);

	// Start by counting the number of team members.  Count up how powerful this team
	// is in terms of negotiating strength.
	int nTeamStrength = 0;
	int nTeamMembers = 0;
	for (int i = 0; i < NUM_STAFFERS; i++) {
		if ((1 << (i + 3)) & teamListItem._nFlags) {
			nTeamMembers++;
			nTeamStrength += g_staffers[i]._nNegotiatingStrength;
		}
	}

	// Get the current time...
	CBagVar *pVar = g_VarManager->getVariable("SRATURNCOUNT");
	assert(pVar != nullptr);
	int nCurTime = pVar->getNumValue();

	// If less then 5 turns have elapsed, then we're still in the driving stage...
	if ((teamListItem._nDispatchTime + kDrivingTime) >= nCurTime) {
		bDone = true;
		bTimeElapsed = false;
		if (teamListItem._nFlags & mStafferMale) {
			if (nTeamMembers == 1) {
				pszFailureFile = kGSM21SraMaleStr;
			} else {
				pszFailureFile = kGSM22SraMaleStr;
			}
		} else if (nTeamMembers == 1) {
			pszFailureFile = kGSM21SraFemStr;
		} else {
			pszFailureFile = kGSM22SraFemStr;
		}
	}

	// Meeting not done yet, let 'em know it.
	if (bDone == false) {
		if (teamListItem._nDispatchTime + teamListItem._nMeetingTime > pVar->getNumValue()) {
			bDone = true;
			bTimeElapsed = false;

			if (teamListItem._nFlags & mStafferMale) {
				pszFailureFile = kGSM19SraMaleStr;
			} else {
				pszFailureFile = kGSM19SraFemStr;
			}
		}
	}

	// If Swonza has already been enlightened to the plans for the planet,
	// then reject this meeting right away.
	if (bDone == false) {
		if (_bSwonzaEnlightened &&
		        (teamListItem._nFlags & mOtherParty) &&
		        teamListItem._nMeetWithID == SWONZA5) {
			bDone = true;
			if (teamListItem._nFlags & mStafferMale) {
				pszFailureFile = kGSM18SraMaleStr;
			} else {
				pszFailureFile = kGSM18SraFemStr;
			}
		}
	}

	// Need to have a driver, {Norg, Zig and Lentil}
	if (bDone == false) {
		if (!((teamListItem._nFlags & mNorg72) || (teamListItem._nFlags & mZig64) || (teamListItem._nFlags & mLentil24))) {
			bDone = true;
			if (teamListItem._nFlags & mStafferMale) {
				pszFailureFile = kGSM6SraMaleStr;
			} else {
				pszFailureFile = kGSM6SraFemStr;
			}
		}
	}

	// Now handle all the resultant cases, get rid of the easy ones first.
	//
	// - Norg and Lentil can't work together, automatic failure
	// - Need to have a driver, {Norg, Zig and Lentil}
	// - Need to have someone who can hold their voltage {Norg, Lentil and Churg}
	// - Pnurth must be teamed up with Lentil, Vargas or Churg (still learning)
	// - Need a good negotiator Norg (1), Pnurth (3), Zig (6), Lentil (8), (4)
	//   when with someone else, Vargas (10), Churg (5)
	//
	// There are many more cases, which we'll handle as they come up.

	// Norg and Lentil can't work together, automatic failure
	if ((bDone == false)  && (teamListItem._nFlags & mNorg72) && (teamListItem._nFlags & mLentil24)) {
		bDone = true;
		if (teamListItem._nFlags & mStafferMale) {
			pszFailureFile = kGSM8SraMaleStr;
		} else {
			pszFailureFile = kGSM8SraFemStr;
		}
	}

	// Need to have someone who can hold their voltage {Norg, Lentil and Churg}
	if ((bDone == false) && !((teamListItem._nFlags & mNorg72) || (teamListItem._nFlags & mLentil24) || (teamListItem._nFlags & mChurg53))) {
		bDone = true;
		if (teamListItem._nFlags & mStafferMale) {
			pszFailureFile = kGSM7SraMaleStr;
		} else {
			pszFailureFile = kGSM7SraFemStr;
		}
	}

	// - Pnurth must be teamed up with Lentil, Vargas or Churg (still learning)
	if ((bDone == false)  && (teamListItem._nFlags & mPnurth81) &&
	        !((teamListItem._nFlags & mLentil24) || (teamListItem._nFlags & mVargas20) || (teamListItem._nFlags & mChurg53))) {
		bDone = true;
		if (teamListItem._nFlags & mStafferMale) {
			if (nTeamMembers == 1) {
				pszFailureFile = kGSM20SraMaleStr;
			} else {
				pszFailureFile = kGSM15SraMaleStr;
			}
		} else if (nTeamMembers == 1) {
			pszFailureFile = kGSM20SraFemStr;
		} else {
			pszFailureFile = kGSM15SraFemStr;
		}
	}

	if (!bDone) {
		// Special rule, if lentil is on her own, then she is twice as effective.
		if (nTeamMembers == 1 && (teamListItem._nFlags & mLentil24)) {
			nTeamStrength *= 2;
		}

		if (teamListItem._nFlags & mOtherParty) {
			switch (teamListItem._nMeetWithID) {
			case SWONZA5:
				// Swonza-5 is a sucker, but Norg is not trustworthy, so if he tells, then
				// the flashback is over, well, not really, but it can't be won without
				// Swonza-5 on board.
				if (teamListItem._nFlags & mNorg72) {
					_bSwonzaEnlightened = true;
					if (nTeamMembers == 1) {
						pszFailureFile = kGSM17SraMaleStr;
					} else if (teamListItem._nFlags & mStafferMale) {
						pszFailureFile = kGSM16SraMaleStr;
					} else {
						pszFailureFile = kGSM16SraFemStr;
					}
				} else {
					// Update the other party bids...
					g_stOtherPartys[SWONZA5]._nPaymentAmount = 20;
					if (teamListItem._nFlags & mStafferMale) {
						pszSuccessFile = kGSM9SraMaleStr;
					} else {
						pszSuccessFile = kGSM9SraFemStr;
					}
				}
				break;
			case POLITICIANS:
				// Need to have either vargas or pnurth on this team
				if ((teamListItem._nFlags & mVargas20) || (teamListItem._nFlags & mPnurth81)) {
					if (teamListItem._nFlags & mStafferMale) {
						pszSuccessFile = kGSM11SraMaleStr;
					} else {
						pszSuccessFile = kGSM11SraFemStr;
					}

					// Update the other party bids...
					g_stOtherPartys[POLITICIANS]._nPaymentAmount = -15;
				} else if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;

			case ENVIRONMENTALISTS:
				// Need to have either lentil or pnurth on this team
				if ((teamListItem._nFlags & mLentil24) || (teamListItem._nFlags & mPnurth81)) {
					if (teamListItem._nFlags & mStafferMale) {
						pszSuccessFile = kGSM12SraMaleStr;
					} else {
						pszSuccessFile = kGSM12SraFemStr;
					}

					// Update the other party bids...
					g_stOtherPartys[ENVIRONMENTALISTS]._nPaymentAmount = -2;

				} else if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;

			default:
				assert(false);
			}
		}

		if (pszFailureFile == nullptr && (teamListItem._nFlags & mBuyer)) {
			int nRequiredStrength = 0;
			int nTalkedUpAmount1 = 0, nTalkedUpAmount2 = 0;
			int nMineralID1 = 0, nMineralID2 = 0;
			switch (teamListItem._nMeetWithID) {
			case SINJIN11:
				// Can up structures from 5 to (7gZ)
				nRequiredStrength = SINJIN11_REQUIRED_TEAM_STRENGTH;
				nTalkedUpAmount1 = kSinjinTalkedUpAmount;
				nMineralID1 = kSinjinMineralID;
			// fallthrough

			case GUNG14:
				// This guy can be talked up by a good negotiating team to (8gZ)
				if (nRequiredStrength == 0) {
					nRequiredStrength = GUNG14_REQUIRED_TEAM_STRENGTH;
					nTalkedUpAmount1 = kGungTalkedUpAmount;
					nMineralID1 = kGungMineralID;
				}
			// fallthrough

			case GILD13:
				// Gild can be talked up on zinc from 6 to 11 and on
				// platinum from 4 to 6
				if (nRequiredStrength == 0) {
					nRequiredStrength = GILD13_REQUIRED_TEAM_STRENGTH;
					nTalkedUpAmount1 = kGildTalkedUpAmount1;
					nMineralID1 = kGildMineralID1;
					nTalkedUpAmount2 = kGildTalkedUpAmount2;
					nMineralID2 = kGildMineralID2;
				}
			// fallthrough

			case DORK44: {
				// Negotiating team must be at least half female.
				if (teamListItem._nMeetWithID == DORK44) {
					int nMale = 0;
					int nFemale = 0;

					for (int k = 0; k < NUM_STAFFERS; k++) {
						switch (k) {
						case 0:
							nMale += (teamListItem._nFlags & mNorg72) ? 1 : 0;
							break;
						case 1:
							nFemale += (teamListItem._nFlags & mPnurth81) ? 1 : 0;
							break;
						case 2:
							nFemale += (teamListItem._nFlags & mZig64) ? 1 : 0;
							break;
						case 3:
							nFemale += (teamListItem._nFlags & mLentil24) ? 1 : 0;
							break;
						case 4:
							nMale += (teamListItem._nFlags & mVargas20) ? 1 : 0;
							break;
						case 5:
							nMale += (teamListItem._nFlags & mChurg53) ? 1 : 0;
							break;
						}
					}
					if (nFemale < nMale) {
						if (teamListItem._nFlags & mStafferMale) {
							pszFailureFile = kGSM23SraMaleStr;
						} else {
							pszFailureFile = kGSM23SraFemStr;
						}
						bDone = true;
						break;
					}
				}

				// Dork's uptite bid can be upped from 4 to 8 if a negotiating
				// team visits him.
				if (nRequiredStrength == 0) {
					nRequiredStrength = DORK44_REQUIRED_TEAM_STRENGTH;
					nTalkedUpAmount1 = kDorkTalkedUpAmount;
					nMineralID1 = kDorkMineralID;
				}

				// These 4 guys can all be talked up
				//
				// If they've already been talked up, then fail.
				if (nTeamStrength >= nRequiredStrength &&
				        g_stBuyerBids[teamListItem._nMeetWithID]._nMineralVal[nMineralID1] != nTalkedUpAmount1) {
					int nDiff1 = nTalkedUpAmount1 - g_stBuyerBids[teamListItem._nMeetWithID]._nMineralVal[nMineralID1];
					int nDiff2 = nTalkedUpAmount2 - g_stBuyerBids[teamListItem._nMeetWithID]._nMineralVal[nMineralID2];

					// Set new values in the buyer bids static
					g_stBuyerBids[teamListItem._nMeetWithID]._nMineralVal[nMineralID1] = (int16)nTalkedUpAmount1;
					g_stBuyerBids[teamListItem._nMeetWithID]._nBidSum += nDiff1;
					if (nTalkedUpAmount2) {
						g_stBuyerBids[teamListItem._nMeetWithID]._nMineralVal[nMineralID2] = (int16)nTalkedUpAmount2;
						g_stBuyerBids[teamListItem._nMeetWithID]._nBidSum += nDiff2;
					}

					// Determine the voice file to give back...
					if (teamListItem._nFlags & mStafferMale) {
						pszSuccessFile = kGSM4SraMaleStr;
					} else {
						pszSuccessFile = kGSM4SraFemStr;
					}
				} else if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;
			}

			case RAWLEY23:
			// 25% chance that medicine bid will be upped
			case CLANG2:
			// 40% chance of increasing Clang's dumping bid of 9 to ...
			case VEBBIL18:
				// Design says vebbil can be talked up from 7 for te, but doesn't
				// tell by how much.

				// Just talked to sm, he says that the bios say they can possibly
				// be talked up, but they' can't no matter how hard we try...
				if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;

			case PYLON3:
			case DIPPIK10:
			case REYES24:
			case HEM20:
			case JELLA37:
			case HUNDEY42:
			case CHANDRA15:
				// These guys are happy to be part of the deal, but won't budge any on their
				// offer.
				if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;

			case MINMARTZN:
			case MINMARTBA:
			case MINMARTRG:
			case MINMARTUT:
			case MINMARTPN:
			case MINMARTSZ:
				// Mineral mart doesn't budge.  Don't even bother negotiating with them,
				// tell the user they wouldn't budge.
				if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;
			default:
				assert(false);
			}
		}

		if (pszFailureFile == nullptr && (teamListItem._nFlags & mSeller)) {
			switch (teamListItem._nMeetWithID) {
			case IRK4:
				// Irk can't be talked down and will only talk to Deven in person
				if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM14SraMaleStr;
				} else {
					pszFailureFile = kGSM14SraFemStr;
				}
				break;
			case QUOSH23:
				// Quosh can't be talked down and will only talk to Deven in person
				if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM13SraMaleStr;
				} else {
					pszFailureFile = kGSM13SraFemStr;
				}
				break;
			case YEEF8:
				// Yeef can be talked down but requires a really sharp negotiating
				// team.
				//
				// if they've already talked Yeef down, then reply with
				// a fail message.
				if (nTeamStrength >= YEEF8_REQUIRED_TEAM_STRENGTH && g_stSellerNames[YEEF8]._nAmount != kYeefTalkedDownAmount) {
					g_stSellerNames[YEEF8]._nAmount = kYeefTalkedDownAmount;
					if (teamListItem._nFlags & mStafferMale) {
						pszSuccessFile = kGSM3SraMaleStr;
					} else {
						pszSuccessFile = kGSM3SraFemStr;
					}
				} else if (teamListItem._nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;
			default:
				assert(false);
			}
		}
	}

	// If we failed or succeeded, remove it from the list of meetings
	if (pszFailureFile || pszSuccessFile) {
		setMeetingResult(teamListItem._nFlags, teamListItem._nMeetWithID, (pszSuccessFile != nullptr));
		if (bTimeElapsed) {
			for (int i = 0; i < NUM_STAFFERS; i++) {
				if (teamListItem._nFlags & (1 << (i + 3))) {
					g_staffers[i]._bAvailable = true;
				}
			}
		}

		// Pick the team captain and build the response file.
		nTeamCaptain = teamListItem._nTeamCaptain;
		if (teamListItem._nFlags & mStafferMale) {
			sResponse = buildMaleSrafDir(pszFailureFile == nullptr ? pszSuccessFile : pszFailureFile);
		} else {
			sResponse = buildFemaleSrafDir(pszFailureFile == nullptr ? pszSuccessFile : pszFailureFile);
		}

		if (bTimeElapsed) {
			// Mark the 'meetee' as available.
			if (teamListItem._nFlags & mOtherParty) {
				g_stOtherPartys[teamListItem._nMeetWithID]._bAvailable = true;
			} else if (teamListItem._nFlags & mSeller) {
				g_stSellerNames[teamListItem._nMeetWithID]._bAvailable = true;
			} else if (teamListItem._nFlags & mBuyer) {
				g_stBuyerBids[teamListItem._nMeetWithID]._bAvailable = true;
			}

			_pTeamList->remove(nTeamNumber);
			bNeedRedraw = true;
		}

		// Failure file, a text file for now.
		// We'll want to play a sound file, for now, just put the text to the screen
		notifyBoss(sResponse, nTeamCaptain);
	}

	return bNeedRedraw;
}

void SrafComputer::onListCheckTeams() {
	int nTeamNumber = _nSelection - 3;

	if (reportMeetingStatus(nTeamNumber)) {
		activateCheckTeams();
	}
}

void SrafComputer::onListCodeWords() {
	int nCodeWordLine = _nSelection - 4;
	int nLastLine = NUM_CODE_WORDS + 5;

	// Reject out of range selections
	if (nCodeWordLine < 0 || nCodeWordLine >= NUM_CODE_WORDS) {
		return;
	}

	CBofString sStr;
	// Calculate the text width based on the attributes of the text
	// rather than guessing to where they are.
	alignAtColumn(sStr, "", kGroup1Col2);
	CBofRect        cCol1Rect = calculateTextRect(this, &sStr, kOtherPointSize, FONT_MONO);
	CBofRect        cCol2Rect = cCol1Rect;
	CBofPoint       cStartPoint(cCol1Rect.right, 0);

	cCol2Rect.offsetRect(cStartPoint);

	// Second bunch of code words start at column 45, create a dummy string and then
	// start offsetting from there.
	alignAtColumn(sStr, "", kGroup2Col1);
	CBofRect        cDummyRect = calculateTextRect(this, &sStr, kOtherPointSize, FONT_MONO);
	cStartPoint.x = cDummyRect.right;

	CBofRect        cCol3Rect = cCol1Rect;
	cCol3Rect.offsetRect(cStartPoint);

	CBofRect        cCol4Rect = cCol1Rect;
	cStartPoint.x = cCol3Rect.right;
	cCol4Rect.offsetRect(cStartPoint);

	// Extend each rectangle to the bottom of the screen.
	cCol1Rect.bottom = cCol2Rect.bottom = cCol3Rect.bottom = cCol4Rect.bottom = 1000;

	// Figure out which words were picked

	const char *pszWord = nullptr;
	int         nWordGroup = 0;
	CBofPoint   cPoint = getPrevMouseDown();        // already in local coords

	// Find the rect that it was in.
	if (cCol1Rect.ptInRect(cPoint)) {
		nWordGroup = 1;
		pszWord = g_stCodeWords[nCodeWordLine]._pszCodeWord1;
	} else if (cCol2Rect.ptInRect(cPoint)) {
		nWordGroup = 2;
		pszWord = g_stCodeWords[nCodeWordLine]._pszCodeWord2;
	} else if (cCol3Rect.ptInRect(cPoint)) {
		nWordGroup = 3;
		pszWord = g_stCodeWords[nCodeWordLine]._pszCodeWord3;
	} else if (cCol4Rect.ptInRect(cPoint)) {
		nWordGroup = 4;
		pszWord = g_stCodeWords[nCodeWordLine]._pszCodeWord4;
	}

	// Find out if a new word was picked.
	if (pszWord) {
		if (nWordGroup == 1 || nWordGroup == 2) {
			*_pszGroup1Word = pszWord;
		} else {
			*_pszGroup2Word = pszWord;
		}

		sStr = "CODE uint16 PAIR: (";
		if (_pszGroup1Word) {
			sStr += *_pszGroup1Word;
		}

		sStr += ",";

		if (_pszGroup2Word) {
			sStr += *_pszGroup2Word;
		}

		sStr += ")";
		_pLBox->setText(nLastLine, sStr);
		_pLBox->repaintItem(nLastLine);
	}
}

void SrafComputer::deactivateMainScreen() {
	deleteListBox();

	//  Hide the on/off button
	if (_eMode == SC_ON) {
		_pButtons[ON_BUTTON]->hide();
	} else {
		_pButtons[OFF_BUTTON]->hide();
	}
}

void SrafComputer::activateMainScreen() {
	char szLocalStr[256];
	szLocalStr[0] = '\0';
	CBofString sStr(szLocalStr, 256);

	// Current screen is now the MAIN screen.
	_eCurScreen = SC_MAIN;

	// Delete the list box
	deleteListBox();

	// Hide the return to main if it's been created

	show();

	if (getBackdrop()) {
		paintBackdrop();
	}

	hideAllButtons();
	_pButtons[QUIT_BUTTON]->show();

	// initialize point size and item height
	_nListPointSize = kOtherPointSize;
	_nListItemHeight = kLineItemHeight;

	ErrorCode errorCode = createListBox();
	assert(errorCode == ERR_NONE);

	int numItems = _pMainList->getCount();

	// Populate listbox
	for (int i = 0; i < numItems; i++) {
		SrafCompItem compItem = _pMainList->getNodeItem(i);
		_pLBox->addToTail(CBofString(compItem._pItem), false);
	}

	// Add instructions to the bottom of the list...
	sStr = "";
	_pLBox->addToTail(sStr, false);

	sStr = kActivateFooterStr1;
	_pLBox->addToTail(sStr, false);

	sStr = kActivateFooterStr2;
	_pLBox->addToTail(sStr, false);

	sStr = kActivateFooterStr3;
	_pLBox->addToTail(sStr, false);

	// Display the current time...

	sStr = " ";
	_pLBox->addToTail(sStr, false);

	// Add a dummy line so we can overwrite it with the
	// current turncount.
	sStr = " ";
	_pLBox->addToTail(sStr, false);

	int nLineNo = _pLBox->getNumItems();
	displayTurnCount(nLineNo - 1);

	// Show list box
	_pLBox->show();

	updateWindow();
}


void SrafComputer::alignAtColumn(CBofString &sStr, const char *szRightText, int nAlignAt) {
	int nAppendLen = strlen(szRightText);

	while (sStr.getLength() < nAlignAt) {
		sStr += " ";
	}

	// Now right justify and get rid of any zeros

	if (nAppendLen == 2 && *szRightText == '0') {
		if (szRightText[1] == '0') {
			sStr += '-';
			sStr += szRightText + 2;
		} else {
			sStr += szRightText + 1;
		}
	} else {
		sStr += szRightText;
	}
}

void SrafComputer::displayTextScreen(CBofString &sStr) {
	// Use a global to determine if we can give meeting reports or not.
	gTextScreenFrontmost = true;

	_pTextOnlyScreen = new SrafTextScreen(sStr);
	_pTextOnlyScreen->createTextScreen(this);
	_pTextOnlyScreen->doModal();

	delete _pTextOnlyScreen;
	_pTextOnlyScreen = nullptr;

	// if we have a list, then return focus to it.
	if (_pLBox) {
		_pLBox->setFocus();
	}

	gTextScreenFrontmost = false;
}

void SrafComputer::onButtonMainScreen(CBofButton *pButton, int nState) {
	if (nState != BUTTON_CLICKED)
		return;

	switch (pButton->getControlID()) {
	case ON_BUTTON:
		setOn();
		break;
	case OFF_BUTTON:
		setOff();
		break;
	case QUIT_BUTTON:
		setQuit();
		break;
	default:
		break;
	}
}

void SrafComputer::onButtonDealSummary(CBofButton *pButton, int nState) {
	if (nState != BUTTON_CLICKED)
		return;

	switch (pButton->getControlID()) {
	case RETURN_TO_MAIN_BUTTON:
		_pButtons[SUBMIT_BUTTON]->hide();
		activateMainScreen();
		break;

	case SUBMIT_BUTTON:
		if (onButtonSubmitOffer()) {
			_pButtons[SUBMIT_BUTTON]->hide();
			_pButtons[RETURN_TO_MAIN_BUTTON]->hide();
			activateCodeWords();
		}
		break;

	default:
		break;
	}
}

bool SrafComputer::onButtonSubmitOffer() {
	bool bOfferAccepted = true;
	char szFailureReason[256];
	int nTotalOffer = 0;
	int i = 0;

	//  First, make sure that we know the terms of the sellers.  If they're
	//  not all resolved, then we can't continue.

	int nAskingPrice = 0;
	while (i < NUM_SELLERS) {
		if (g_stSellerNames[i]._nAmount == -1) {
			Common::sprintf_s(szFailureReason, kszUnresolvedSeller, g_stSellerNames[i]._pszName);
			bOfferAccepted = false;
			break;
		}

		nAskingPrice += g_stSellerNames[i]._nAmount;

		i++;
	}

	// Check that all the terms have been resolved for the buyers... all we really
	// need to check here is the environmentalists.

	if (bOfferAccepted && (g_stOtherPartys[ENVIRONMENTALISTS]._nPaymentAmount == -1)) {
		Common::strcpy_s(szFailureReason, kszUnresolvedEnriro);
		bOfferAccepted = false;
	}

	// If we didn't fail because of the seller, let's now check the buyer
	// grid.  First check is that we didn't give the same mining right away
	// to more than a single party.

	if (bOfferAccepted) {
		int bMinerals[NUM_MINERALS];

		// Clear out the mineral accepted array.  If a mining right is given
		// away to more than one party, then the array will have a non-zero
		// value for that party.
		for (i = 0; i < NUM_MINERALS; i++) {
			bMinerals[i] = -1;
		}

		nTotalOffer = 0;
		for (i = 0; i < NUM_BUYERS; i++) {
			if (g_stBuyerBids[i]._bAccept) {
				nTotalOffer += g_stBuyerBids[i]._nBidSum;

				// We've chose to accept this bid, so make sure that we have not
				// already promised these mining rights to somebody else.
				for (int j = 0; j < NUM_MINERALS; j++) {
					if (g_stBuyerBids[i]._nMineralVal[j] != 0) {
						if (bMinerals[j] != -1) {
							Common::sprintf_s(szFailureReason, kszMiningConflict,
							                  g_stMinerals[j]._pszMineralName, g_stBuyerBids[i]._pszName, g_stBuyerBids[bMinerals[j]]._pszName);
							bOfferAccepted = false;
							break;
						} else {
							bMinerals[j] = i;
						}
					}
				}

				// If we found a dup, then get out of here.
				if (bOfferAccepted == false) {
					break;
				}
			}
		}
	}

	// Now check and make sure that they came up with enough gigaZoks.
	if (bOfferAccepted) {
		// Add in the conditions of the other parties... we know that by now the
		// environmentatlists have been resolved.
		for (i = 0; i < NUM_OTHER_PARTYS; i++) {
			nTotalOffer += (g_stOtherPartys[i]._nPaymentAmount == -1 ? 0 : g_stOtherPartys[i]._nPaymentAmount);
		}

		if (nAskingPrice > nTotalOffer) {
			Common::sprintf_s(szFailureReason, kszOfferNotEnough, nAskingPrice - nTotalOffer);
			bOfferAccepted = false;
		}
	}

	// Now check that the weirdness of each of the buyers is met, here is a
	// list of them:
	//
	// Gung-14 won't go into a deal with anyone with an '8' in their name
	// Hem-20 won't go in on a deal with Dippik
	// Dork-44 insists backer group is 50% female.
	// Jella-37's lumber bid includes crop mining rights
	// Sinjin and Dork won't participate in the same deal.

	if (bOfferAccepted) {
		for (i = 0; i < NUM_BUYERS; i++) {
			switch (g_stBuyerBids[i]._nBuyerID) {
			case GUNG14:
				// Make sure that there are no backers with the number '8'
				// in their name.
				if (g_stBuyerBids[GUNG14]._bAccept &&
				        g_stBuyerBids[VEBBIL18]._bAccept) {
					Common::sprintf_s(szFailureReason, kszGungNotHappy, g_stBuyerBids[VEBBIL18]._pszName);
					bOfferAccepted = false;
					break;
				}
				break;

			case HEM20:
				// Hem-20 won't do business with dippik.
				if (g_stBuyerBids[HEM20]._bAccept &&
				        g_stBuyerBids[DIPPIK10]._bAccept) {
					Common::strcpy_s(szFailureReason, kszHemNotHappy);
					bOfferAccepted = false;
					break;
				}
				break;

			case DORK44: {
				// Dork-44 insists that at least half the backers are female.
				int16 nMale = 0;
				int16 nFemale = 0;

				if (g_stBuyerBids[DORK44]._bAccept) {
					for (int j = 0; j < NUM_BUYERS; j++) {
						if (g_stBuyerBids[j]._bAccept) {
							if (g_stBuyerBids[j]._nFlags & mBuyerMale) {
								nMale++;
							} else if (g_stBuyerBids[j]._nFlags & mBuyerFemale) {
								nFemale++;
							}
						}
					}

					if (nMale > nFemale) {
						Common::strcpy_s(szFailureReason, kszDorkNotHappy);
						bOfferAccepted = false;
						break;
					}
				}
				break;
			}

			case JELLA37:
				// If we accepted Jella-37's lumber bid, then we also have to reserve
				// crop harvesting rights for her.
				if (g_stBuyerBids[JELLA37]._bAccept) {
					for (int j = 0; j < NUM_BUYERS; j++) {
						if (g_stBuyerBids[j]._bAccept && g_stBuyerBids[j]._nMineralVal[CROP] != 0) {
							Common::sprintf_s(szFailureReason, kszJellaNotHappy, g_stBuyerBids[j]._pszName);
							bOfferAccepted = false;
							break;
						}
					}
				}
				break;

			case SINJIN11:
				// Sinjin-11 won't do business with Dork-44.
				if (g_stBuyerBids[SINJIN11]._bAccept &&
				        g_stBuyerBids[DORK44]._bAccept) {
					Common::strcpy_s(szFailureReason, kszSinjinNotHappy);
					bOfferAccepted = false;
					break;
				}
				break;
			case PYLON3:
			case DIPPIK10:
			case VEBBIL18:
			case REYES24:
			case GILD13:
			case RAWLEY23:
			case HUNDEY42:
			case CHANDRA15:
			case CLANG2:
			case MINMARTZN:
			case MINMARTBA:
			case MINMARTRG:
			case MINMARTUT:
			case MINMARTPN:
			case MINMARTSZ:
				break;
			}
			if (bOfferAccepted == false) {
				break;
			}
		}
	}

	if (bOfferAccepted == false) {
		displayMessage(szFailureReason);
	}

	return bOfferAccepted;
}

void SrafComputer::onButtonBuyerBids(CBofButton *pButton, int nState) {
	if (nState != BUTTON_CLICKED)
		return;

	switch (pButton->getControlID()) {
	case RETURN_TO_MAIN_BUTTON:
		activateMainScreen();
		break;

	case DISPLAY_KEY_BUTTON: {
		char szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString sStr(szLocalBuff, 256);

		sStr = buildSrafDir("KEYINFO.TXT");
		displayTextScreen(sStr);
		break;
	}

	default:
		break;
	}
}

void SrafComputer::onButtonDealBackground(CBofButton *pButton, int nState) {
	if ((nState == BUTTON_CLICKED) && (pButton->getControlID() == RETURN_TO_MAIN_BUTTON))
		activateMainScreen();
}

void SrafComputer::onButtonSellerBios(CBofButton *pButton, int nState) {
	if ((nState == BUTTON_CLICKED) && (pButton->getControlID() == RETURN_TO_MAIN_BUTTON))
		activateMainScreen();
}

void SrafComputer::onButtonOtherBios(CBofButton *pButton, int nState) {
	if ((nState == BUTTON_CLICKED) && (pButton->getControlID() == RETURN_TO_MAIN_BUTTON))
		activateMainScreen();
}

void SrafComputer::onButtonStaffBios(CBofButton *pButton, int nState) {
	if ((nState == BUTTON_CLICKED) && (pButton->getControlID() == RETURN_TO_MAIN_BUTTON))
		activateMainScreen();
}

void SrafComputer::onButtonDispatchTeam(CBofButton *pButton, int nState) {
	if (nState != BUTTON_CLICKED)
		return;

	switch (pButton->getControlID()) {
	case RETURN_TO_MAIN_BUTTON:
		activateMainScreen();
		break;
	case DISPATCH_TEAM_BUTTON:
		verifyDispatchTeam();
	default:
		break;
	}
}

// Apply staffer meeting rules to figure out how long a meeting should take.
int SrafComputer::calculateMeetingTime(int nFlags) {
	int mFlag = 0;
	int nTeamMembers = 0;
	int nTotalTime = 0;

	// Handle the failure case first... that is, if no driver was sent {Norg,
	// Zig and Lentil} Try seeing if 5 is a good thing.
	if (!((nFlags & mNorg72) || (nFlags & mZig64) || (nFlags & mLentil24))) {
		return kDrivingTime;
	}

	// Add up the total time.
	for (int i = 0; i < NUM_STAFFERS; i++) {
		switch (i) {
		case 0:
			mFlag = mNorg72;
			break;
		case 1:
			mFlag = mPnurth81;
			break;
		case 2:
			mFlag = mZig64;
			break;
		case 3:
			mFlag = mLentil24;
			break;
		case 4:
			mFlag = mVargas20;
			break;
		case 5:
			mFlag = mChurg53;
			break;
		}

		if (nFlags & mFlag) {
			nTeamMembers++;
			nTotalTime += g_staffers[i]._nMeetingTime;
		}
	}

	// Done adding up, now apply the following special rules.
	//
	// If Lentil not on her own, then double her time.
	// If Pnurth on her own, double her time.

	if ((nFlags & mLentil24) && nTeamMembers > 1) {
		nTotalTime += g_staffers[LENTIL24]._nMeetingTime;
	}

	if ((nFlags & mPnurth81) &&
	        ((nFlags & mChurg53) || (nFlags & mVargas20) || (nFlags & mLentil24))) {
		nTotalTime -= (g_staffers[PNURTH81]._nMeetingTime / 2);
	}

	return nTotalTime / nTeamMembers;
}

void SrafComputer::notifyBoss(CBofString &sSoundFile, int nStafferID) {         // Must be a full file spec
	CBofBitmap *pSaveBackground = nullptr;

	// Allow for no staffer screen
	if (nStafferID != -1) {
		pSaveBackground = new CBofBitmap(gTextWindow.width(), gTextWindow.height(), (CBofPalette *)nullptr, false);
		pSaveBackground->captureScreen(this, &gTextWindow);

		if (_pStafferBmp[nStafferID] == nullptr) {
			char        szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString  sStr(szLocalBuff, 256);

			sStr = buildSrafDir(g_staffers[nStafferID]._pszStafferBmp);

			_pStafferBmp[nStafferID] = new CBofBitmap(szLocalBuff);
		}

		// Guaranteed to have it now.  Paste it to the screen.

		_pStafferBmp[nStafferID]->paint(this, &gStafferDisplay);

	}

	// Play the voice file... Depends on if we have a voice file or a text file...
	// the last three will tell us.

	if (sSoundFile.find(".WAV") != -1 || sSoundFile.find(".wav") != -1) {
		CBofCursor::hide();
		BofPlaySound(sSoundFile.getBuffer(), SOUND_WAVE);
		CBofCursor::show();
	} else if (sSoundFile.find(".TXT") || sSoundFile.find(".txt")) {
		// Make sure the file is there, read it in to our own buffer.
		CBofFile fTxtFile(sSoundFile, CBF_BINARY | CBF_READONLY);
		uint32 nLength = fTxtFile.getLength();

		if (nLength == 0) {
			reportError(ERR_FREAD, "Unexpected empty file %s", sSoundFile.getBuffer());
		} else {
			char *pszBuf = (char *)bofCleanAlloc(nLength + 1);
			fTxtFile.read(pszBuf, nLength);

			// Put it up on the screen
			displayMessage(pszBuf);
			bofFree(pszBuf);
		}
		fTxtFile.close();
	}

	// allow for no staffer screen
	if (nStafferID != -1) {
		pSaveBackground->paint(this, &gTextWindow);

		delete pSaveBackground;
	}
}

// Most experienced person is captain.
int SrafComputer::getTeamCaptain(int nFlags) {
	int nStaffCaptain = NORG72;

	if (nFlags & mVargas20) {
		nStaffCaptain = VARGAS20;
	} else if (nFlags & mLentil24) {
		nStaffCaptain = LENTIL24;
	} else if (nFlags & mChurg53) {
		nStaffCaptain = CHURG53;
	} else if (nFlags & mZig64) {
		nStaffCaptain = ZIG64;
	} else if (nFlags & mNorg72) {
		nStaffCaptain = NORG72;
	} else if (nFlags & mPnurth81) {
		nStaffCaptain = PNURTH81;
	}

	return nStaffCaptain;
}

// Most experienced person determines sex of team.
int SrafComputer::getTeamGender(int nFlags) {

	int     nStaffGender = 0;

	if (nFlags & mVargas20) {
		nStaffGender = mStafferMale;
	} else if (nFlags & mLentil24) {
		nStaffGender = mStafferFemale;
	} else if (nFlags & mChurg53) {
		nStaffGender = mStafferMale;
	} else if (nFlags & mZig64) {
		nStaffGender = mStafferFemale;
	} else if (nFlags & mNorg72) {
		nStaffGender = mStafferMale;
	} else if (nFlags & mPnurth81) {
		nStaffGender = mStafferFemale;
	}

	return nStaffGender;
}

void SrafComputer::onButtonCurrentEMail(CBofButton *pButton, int nState) {
	if ((nState == BUTTON_CLICKED) && (pButton->getControlID() == RETURN_TO_MAIN_BUTTON))
		activateMainScreen();
}

void SrafComputer::onButtonAudioSettings(CBofButton *pButton, int nState) {
	if (nState != BUTTON_CLICKED)
		return;

	switch (pButton->getControlID()) {
	case RETURN_TO_MAIN_BUTTON:
		activateMainScreen();
		break;
	case NO_MUSIC_BUTTON:
		CBofSound::stopSounds();
		_pButtons[NO_MUSIC_BUTTON]->hide();
		activateMainScreen();
		break;
	default:
		break;
	}
}

void SrafComputer::onButtonRoboButler(CBofButton *pButton, int nState) {
	if (nState != BUTTON_CLICKED)
		return;

	switch (pButton->getControlID()) {
	case RETURN_TO_MAIN_BUTTON:
		activateMainScreen();
		break;
	case ORDER_BEVERAGE_BUTTON:
		doOrderBeverage();
		break;
	case ORDER_SNACK_BUTTON:
		doOrderSnack();
		break;
	default:
		break;
	}
}

void SrafComputer::doOrderBeverage() {
	char    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	g_SDevManager->addObject("INV_WLD", "SZTB");
	g_SDevManager->addObject("INVZ_WLD", "SZTB");

	// Build the string to indicate that the beverage has been stashed
	sStr = buildSrafDir(g_stOfferings[0]._pszRcvOfferingFile);
	notifyBoss(sStr, -1);

	// state of buttons has changed
	activateRoboButler();
}

void SrafComputer::doOrderSnack() {
	char    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	g_SDevManager->addObject("INV_WLD", "SZTA");
	g_SDevManager->addObject("INVZ_WLD", "SZTA");

	// Build the string to indicate that the beverage has been stashed
	sStr = buildSrafDir(g_stOfferings[1]._pszRcvOfferingFile);
	notifyBoss(sStr, -1);

	// state of buttons has changed
	activateRoboButler();
}

void SrafComputer::onButtonCheckTeams(CBofButton *pButton, int nState) {
	if ((nState == BUTTON_CLICKED) && (pButton->getControlID() == RETURN_TO_MAIN_BUTTON))
		activateMainScreen();
}

void SrafComputer::onButtonCodeWords(CBofButton *pButton, int nState) {
	if ((nState == BUTTON_CLICKED) && (pButton->getControlID() == DONE_BUTTON)) {
		// Normally we'd return to the main screen, but in this instance, we're
		// done with the flashback... that is, as long as they've picked two
		// code words.
		onButtonFinished(true);
	}
}

void SrafComputer::onButtonFinished(bool bVictorious) {
	CBagVar *pVar;

	// Make sure the user selected two code words.
	if (bVictorious == true) {
		if (_pszGroup1Word->getLength() == 0 ||
		        _pszGroup2Word->getLength() == 0) {
			// Put up an error...
			displayMessage(kszFinishCodeWords);
			return;
		}

		// Pass our codewords back to the bar...
		pVar = g_VarManager->getVariable("DEVENCODE1");
		assert(pVar != nullptr);

		if (pVar)
			pVar->setValue(_pszGroup1Word->getBuffer());

		pVar = g_VarManager->getVariable("DEVENCODE2");
		assert(pVar != nullptr);
		if (pVar)
			pVar->setValue(_pszGroup2Word->getBuffer());
	}

	// Setting the flashback variable will trigger the
	// event world condition which gets us back to the bar.
	pVar = g_VarManager->getVariable("RFLASHBACK");
	assert(pVar != nullptr);

	if (pVar) {
		if (bVictorious == true) {
			pVar->setValue(2);
		} else {
			pVar->setValue(1);
		}
	}

	close();
}

// Display's a message at the bottom of the screen.

void SrafComputer::displayMessage(const char *szMsg) {
	// Use a global to determine if we can give meeting reports or not.
	gTextScreenFrontmost = true;

	_pTextOnlyScreen = new SrafTextScreen(szMsg, true);
	_pTextOnlyScreen->createTextScreen(this);
	_pTextOnlyScreen->doModal();

	delete _pTextOnlyScreen;
	_pTextOnlyScreen = nullptr;

	// if we have a list, then return focus to it.
	if (_pLBox) {
		_pLBox->setFocus();
	}

	gTextScreenFrontmost = false;

	updateWindow();
}


void SrafComputer::setMainScreen() {
	if (_pHead == nullptr) {
		return;
	}

	SrafComputer *srafComp = _pHead;
	if (srafComp->_eCurScreen == SC_BACKGROUND_DATA) {
		srafComp->_eCurScreen = SC_MAIN;
	}
}

void SrafComputer::incrementTurnCount() {
	CBagVar *pVar = g_VarManager->getVariable("SRATURNCOUNT");
	int nTurncount = 0;

	assert(pVar != nullptr);

	// If the puzzle is already solved, then don't increment.
	if (_eCurScreen == SC_CODE_WORDS) {
		return;
	}


	if (pVar != nullptr) {
		nTurncount = pVar->getNumValue();
		pVar->setValue(++nTurncount);
	}

	// If we're in a screen that has a time count, then update it here...
	if (_eCurScreen == SC_CHECK_TEAMS || _eCurScreen == SC_MAIN) {
		displayTurnCount(gTurncountLineNo);
	}

	// Automatically give the user the status of the meeting if the
	// meeting time has expired...
	if (_pTeamList) {
		int nTeams = _pTeamList->getCount();
		for (int i = 0; i < nTeams; i++) {
			DispatchedTeamItem teamListItem = _pTeamList->getNodeItem(i);

			if (nTurncount > (teamListItem._nDispatchTime + teamListItem._nMeetingTime)) {
				if (reportMeetingStatus(i)) {
					// If current screen is the check teams screen, then make sure we update
					// it.
					switch (_eCurScreen) {
					case SC_CHECK_TEAMS:
						activateCheckTeams();
						break;

					// update the buyer bids screen if that's our underlying
					// guy.
					case SC_BIDS:
						activateBuyerBids();
						break;

					case SC_DEAL:
						activateDealSummary();
						break;

					case SC_DISPATCH:
						activateDispatchTeam();
						break;

					default:
						break;
					}
				}

				// Only report one meeting per turn...
				break;
			}
		}
	}

	// If we got to our max, then put durteen up and snap out of the flashback...
	//
	// hack around the problem where the user is notified twice
	// about failure.

	if (_bFailureNotified == false) {
		if (nTurncount > (_nStartingTime + kSrafMaxTurns)) {
			_bFailureNotified = true;

			char        szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString  sResponse(szLocalBuff, 256);

			sResponse = buildMaleSrafDir(SRAF_TIMEOUT);

			notifyBoss(sResponse, DURTEEN97);
			onButtonFinished(false);
		}
	}

	// add random play function (5676)
	if (_bRandomAudio == true && (nTurncount > _nRandomTime + kRandomPlayTime)) {
		int nTrackSelection = g_engine->getRandomNumber() % (NUM_MUSICAL_SCORES - 1);

		// Now start playing...
		if (g_stAudioSetting[nTrackSelection]->_pMidiTrack != nullptr) {
			g_stAudioSetting[nTrackSelection]->_pMidiTrack->play();
		}
		_nRandomTime += kRandomPlayTime;
	}
}

void SrafComputer::displayTurnCount(int nLineNo) {
	CBagVar *pVar = g_VarManager->getVariable("SRATURNCOUNT");
	assert(pVar != nullptr);

	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString  sStr(szLocalBuff, 256);
	int         nCurrentTime = pVar->getNumValue();

	Common::sprintf_s(szLocalBuff,
	                  "CURRENT TIME: %02d:%02d",
	                  nCurrentTime / 100,
	                  nCurrentTime % 100);

	// Get whatever is on that line...
	if (_pLBox) {
		CBofString cStr = _pLBox->getText(nLineNo);

		// reset the value on that line.
		if (cStr.getLength() != 0) {
			_pLBox->setText(nLineNo, sStr);
		}
	}

	// Keep track of the line number that it's stored on... might need this again.
	gTurncountLineNo = nLineNo;
}

CBofBitmap *SrafComputer::getComputerBackdrop() {
	if (_pHead == nullptr) {
		return nullptr;
	}

	SrafComputer *srafComp = _pHead;
	return srafComp->getBackdrop();
}


// Sraffin text screen classes.  This class is used to read a text file in
// and post it to the full screen with a "done" button in the lower right
// hand corner of the screen.

SrafTextScreen::SrafTextScreen(const CBofString &str, bool isText) :
	gCompDisplay(40, 40, 600, 440), gTextWindow(0, 0, 640 - 1, 480 - 1),
	gStafferDisplay(0, 0, 640 - 1, 480 - 1), gSrafTextWindow(0, 440, 640 - 1, 480 - 1) {
	if (isText) {
		_text = str;

	} else {
		CBofFile file(str, CBF_BINARY | CBF_READONLY);

		size_t len = file.getLength();
		char *tmp = new char[len + 1];
		file.read(tmp, len);
		tmp[len] = '\0';

		_text = CBofString(tmp);
		delete[] tmp;
	}

	// save the currently active window
	_pSaveActiveWin = getActiveWindow();
}

int SrafTextScreen::createTextScreen(CBofWindow *pParent) {
	CBofRect cRect;
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString  sStr(szLocalBuff, 256);

	// Create our parent window
	create("Sraffin Text", &gTextWindow, pParent, 0);

	// Needs the computer bitmap for a backdrop
	setBackdrop(SrafComputer::getComputerBackdrop(), false);

	assert(_pBackdrop != nullptr);
	CBofPalette *pPal = _pBackdrop->getPalette();
	// Create our OK button

	_pOKButton = new CBofBmpButton;

	CBofBitmap *pUp = loadBitmap(buildSrafDir(g_stButtons[DONE_BUTTON]._pszUp), pPal);
	CBofBitmap *pDown = loadBitmap(buildSrafDir(g_stButtons[DONE_BUTTON]._pszDown), pPal);
	CBofBitmap *pFocus = loadBitmap(buildSrafDir(g_stButtons[DONE_BUTTON]._pszFocus), pPal);
	CBofBitmap *pDis = loadBitmap(buildSrafDir(g_stButtons[DONE_BUTTON]._pszDisabled), pPal);

	_pOKButton->loadBitmaps(pUp, pDown, pFocus, pDis);

	_pOKButton->create(g_stButtons[DONE_BUTTON]._pszName,
	                   g_stButtons[DONE_BUTTON]._nLeft,
	                   g_stButtons[DONE_BUTTON]._nTop,
	                   g_stButtons[DONE_BUTTON]._nWidth,
	                   g_stButtons[DONE_BUTTON]._nHeight,
	                   this,
	                   g_stButtons[DONE_BUTTON]._nID);

	//  Create our text box.
	cRect.setRect(gCompDisplay.left, gCompDisplay.top, gCompDisplay.right, gCompDisplay.bottom);

	_pTextBox = new CBofTextBox(this, &cRect, _text);
	_pTextBox->setPageLength(24);
	_pTextBox->setColor(CTEXT_WHITE);
	_pTextBox->setFont(FONT_MONO);
	_pTextBox->setPointSize(FONT_14POINT);

	return ERR_NONE;
}

void SrafTextScreen::displayTextScreen() {
	show();

	if (getBackdrop())
		paintBackdrop();

	setFocus();

	_pOKButton->show();

	_pTextBox->display();
}

SrafTextScreen::~SrafTextScreen() {
	// Trash everything!
	if (_pTextBox) {
		delete _pTextBox;
		_pTextBox = nullptr;
	}

	if (_pOKButton) {
		delete _pOKButton;
		_pOKButton = nullptr;
	}

	// Make sure the underlying window gets focus back
	if (_pSaveActiveWin) {
		_pSaveActiveWin->setFocus();
	}
}

// Called to delete our text object

void SrafTextScreen::onClose() {
	CBofDialog::onClose();

	// Set the backdrop to nullptr so that it doesn't get trashed (we borrowed it from
	// the sraf computer.
	clearBackdrop();
}

void SrafTextScreen::onPaint(CBofRect * /*pRect*/) {
	displayTextScreen();

	validateAnscestors();
}

void SrafTextScreen::onBofButton(CBofObject *pObject, int nState) {
	assert(isValidObject(this));
	assert(pObject != nullptr);

	CBofButton *pButton = (CBofButton *)pObject;

	if (nState == BUTTON_CLICKED && pButton->getControlID() == DONE_BUTTON) {
		close();
		SrafComputer::setMainScreen();
	}
}


const char *buildAudioDir(const char *pszFile) {
	return formPath(SRAF_AUDIO_DIR, pszFile);
}

const char *buildSrafDir(const char *pszFile) {
	return formPath(SRAF_DIR, pszFile);
}

const char *buildMaleSrafDir(const char *pszFile) {
	return formPath(SRAF_MALE_DIR, pszFile);
}

const char *buildFemaleSrafDir(const char *pszFile) {
	return formPath(SRAF_FEMALE_DIR, pszFile);
}

void SrafComputer::restoreSraffanVars() {
	// Retrieve sellers terms
	const char *pVarName = nullptr;

	// Save sellers terms
	SELLERS nSellerID = IRK4;
	int nNumToRestore = 3;

	CBagVar *pVar;
	for (int i = 0; i < nNumToRestore; i++) {
		switch (i) {
		case 0:
			pVarName = "IRKSTERMS";
			nSellerID = IRK4;
			break;
		case 1:
			pVarName = "YEEFSTERMS";
			nSellerID = YEEF8;
			break;
		case 2:
			pVarName = "QUOSHSTERMS";
			nSellerID = QUOSH23;
			break;
		}

		pVar = g_VarManager->getVariable(pVarName);
		if (pVar != nullptr) {
			g_stSellerNames[nSellerID]._nAmount = pVar->getNumValue();
		}
	}

	// Retrieve buyer terms

	nNumToRestore = 5;
	BUYERS nBuyerID = PYLON3;
	int nMineralID = 0;

	for (int i = 0; i < nNumToRestore; i++) {
		switch (i) {
		case 0:
			pVarName = "SINJINSTTERMS";
			nBuyerID = SINJIN11;
			nMineralID = kSinjinMineralID;
			break;
		case 1:
			pVarName = "GUNGBATERMS";
			nBuyerID = GUNG14;
			nMineralID = kGungMineralID;
			break;
		case 2:
			pVarName = "GILDZITERMS";
			nBuyerID = GILD13;
			nMineralID = kGildMineralID1;
			break;
		case 3:
			pVarName = "GILDPLTERMS";
			nBuyerID = GILD13;
			nMineralID = kGildMineralID2;
			break;
		case 4:
			pVarName = "DORKUPTERMS";
			nBuyerID = DORK44;
			nMineralID = kDorkMineralID;
			break;
		}

		pVar = g_VarManager->getVariable(pVarName);
		if (pVar != nullptr) {
			g_stBuyerBids[nBuyerID]._nMineralVal[nMineralID] = pVar->getNumValue();

			// total each one...
			int nBidSum = 0;
			for (int j = 0; j < NUM_MINERALS; j++) {
				nBidSum += g_stBuyerBids[nBuyerID]._nMineralVal[j];
			}

			g_stBuyerBids[nBuyerID]._nBidSum = nBidSum;
		}
	}

	// Save the list of who is in on this deal... use a bit to indicate if they are in or not.
	uint32 nBuyersMask = 0;

	pVar = g_VarManager->getVariable("BUYERSMASK");
	if (pVar != nullptr) {
		nBuyersMask = pVar->getNumValue();
	}

	for (int i = 0; i < NUM_BUYERS; i++) {
		if (nBuyersMask & (1 << i)) {
			g_stBuyerBids[i]._bAccept = true;
		} else {
			g_stBuyerBids[i]._bAccept = false;
		}
	}

	// Restore the list of who is available and who is not.
	// Do this using a mask.
	uint32 nAvailMask = 0;
	uint32 nIndex = 0;

	pVar = g_VarManager->getVariable("AVAILABLEMASK");
	if (pVar != nullptr) {
		nAvailMask = pVar->getNumValue();
	}

	for (int i = 0; i < NUM_BUYERS; i++, nIndex++) {
		if (nAvailMask & (1 << nIndex)) {
			g_stBuyerBids[i]._bAvailable = false;
		} else {
			g_stBuyerBids[i]._bAvailable = true;
		}
	}

	for (int i = 0; i < NUM_SELLERS; i++, nIndex++) {
		if (nAvailMask & (1 << nIndex)) {
			g_stSellerNames[i]._bAvailable = false;
		} else {
			g_stSellerNames[i]._bAvailable = true;
		}
	}

	for (int i = 0; i < NUM_OTHER_PARTYS; i++, nIndex++) {
		if (nAvailMask & (1 << nIndex)) {
			g_stOtherPartys[i]._bAvailable = false;
		} else {
			g_stOtherPartys[i]._bAvailable = true;
		}
	}

	assert(nIndex < 32);

	// Restore other party's info
	OTHERPARTYS nOtherID = SWONZA5;

	nNumToRestore = 3;
	for (int i = 0; i < nNumToRestore; i++) {
		switch (i) {
		case 0:
			pVarName = "SWONZATERMS";
			nOtherID = SWONZA5;
			break;
		case 1:
			pVarName = "POLITTERMS";
			nOtherID = POLITICIANS;
			break;
		case 2:
			pVarName = "ENVIROTERMS";
			nOtherID = ENVIRONMENTALISTS;
			break;
		}

		pVar = g_VarManager->getVariable(pVarName);
		if (pVar != nullptr) {
			g_stOtherPartys[nOtherID]._nPaymentAmount = pVar->getNumValue();
		}
	}

	// Retrieve swonza's state
	pVar = g_VarManager->getVariable("SWONZAENLIGHTENED");
	if (pVar != nullptr) {
		_bSwonzaEnlightened = pVar->getNumValue();
	}

	// Mark each of our staffers as available before we start.
	for (int i = 0; i < NUM_STAFFERS; i++) {
		g_staffers[i]._bAvailable = true;
	}

	// Now the real pain in the ass... restoring team information...
	nNumToRestore = 6;

	DispatchedTeamItem teamListItem;
	char szFLAGS[20];
	char szMEETWITH[20];
	char szDISPATCHTIME[20];
	char szCAPTAIN[20];
	char szMEETINGTIME[20];

	for (int i = 0; i < nNumToRestore; i++) {
		// Build the variable names with default values.
		Common::sprintf_s(szFLAGS, "%s%d%s", "TEAM", i + 1, "FLAGS");
		Common::sprintf_s(szMEETWITH, "%s%d%s", "TEAM", i + 1, "MEETWITH");
		Common::sprintf_s(szDISPATCHTIME, "%s%d%s", "TEAM", i + 1, "DISPATCHTIME");
		Common::sprintf_s(szCAPTAIN, "%s%d%s", "TEAM", i + 1, "CAPTAIN");
		Common::sprintf_s(szMEETINGTIME, "%s%d%s", "TEAM", i + 1, "MEETINGTIME");

		// Restore the whole block...
		pVar = g_VarManager->getVariable(szFLAGS);
		if (pVar != nullptr) {
			teamListItem._nFlags = pVar->getNumValue();

			// If we have a meeting going on, then mark that
			// sraffan staffer as unavailable.
			if (teamListItem._nFlags != 0) {
				for (int j = 0; j < NUM_STAFFERS; j++) {
					if ((1 << (j + 3)) & teamListItem._nFlags) {
						g_staffers[j]._bAvailable = false;
					}
				}
			}
		}
		pVar = g_VarManager->getVariable(szMEETWITH);
		if (pVar != nullptr) {
			teamListItem._nMeetWithID = pVar->getNumValue();
		}
		pVar = g_VarManager->getVariable(szDISPATCHTIME);
		if (pVar != nullptr) {
			teamListItem._nDispatchTime = pVar->getNumValue();
		}
		pVar = g_VarManager->getVariable(szCAPTAIN);
		if (pVar != nullptr) {
			teamListItem._nTeamCaptain = pVar->getNumValue();
		}
		pVar = g_VarManager->getVariable(szMEETINGTIME);
		if (pVar != nullptr) {
			teamListItem._nMeetingTime = pVar->getNumValue();
		}

		// If we have a list established, then add this item to it if there's anything
		// there.
		if (_pTeamList == nullptr) {
			_pTeamList = new CBofList<DispatchedTeamItem>;
		}

		if (teamListItem._nFlags != 0) {
			_pTeamList->addToTail(teamListItem);
		}
	}


	// Save the successful and unsuccessful meeting status.  Here's how we're gonna do
	// this.  For each negotiating target, we will save a bit in "METWITH", 0 => no
	// meeting occurred, 1 => a meeting happened with this dude.  There will be a
	// second variable, "MEETINGRESULTS", there will be a bit that is synonymous with
	// the "METWITH" var indicating the result of the meeting, i.e. 0=>meeting was a
	// success and 1=> meeting was a failure.
	//
	// Now get the variables
	int nMetWithVal = 0;
	int nMeetingResultVal = 0;

	pVar = g_VarManager->getVariable("METWITH");
	if (pVar != nullptr) {
		nMetWithVal = pVar->getNumValue();
	}

	pVar = g_VarManager->getVariable("MEETINGRESULTS");
	if (pVar != nullptr) {
		nMeetingResultVal = pVar->getNumValue();
	}

	int nBitNo = 0;
	// Restore seller meeting history
	for (int i = 0; i < NUM_SELLERS; i++, nBitNo++) {
		if (nMetWithVal & (1 << nBitNo)) {
			g_stSellerNames[i]._nMeetingResult = (((1 << nBitNo) & nMeetingResultVal) ? SRAF_GOOD_MEETING : SRAF_BAD_MEETING);
		} else {
			g_stSellerNames[i]._nMeetingResult = 0;
		}
	}

	// Save buyer meeting history
	for (int i = 0; i < NUM_BUYERS; i++, nBitNo++) {
		if (nMetWithVal & (1 << nBitNo)) {
			g_stBuyerBids[i]._nMeetingResult = (((1 << nBitNo) & nMeetingResultVal) ? SRAF_GOOD_MEETING : SRAF_BAD_MEETING);
		} else {
			g_stBuyerBids[i]._nMeetingResult = 0;
		}
	}

	// Save other party meeting history
	for (int i = 0; i < NUM_OTHER_PARTYS; i++, nBitNo++) {
		if (nMetWithVal & (1 << nBitNo)) {
			g_stOtherPartys[i]._nMeetingResult = (((1 << nBitNo) & nMeetingResultVal) ? SRAF_GOOD_MEETING : SRAF_BAD_MEETING);
		} else {
			g_stOtherPartys[i]._nMeetingResult = 0;
		}
	}

	// All done!
}

void SrafComputer::saveSraffanVars() {
	const char *pVarName = nullptr;

	// Save sellers terms
	SELLERS nSellerID = IRK4;
	int nNumToSave = 3;

	for (int i = 0; i < nNumToSave; i++) {
		switch (i) {
		case 0:
			pVarName = "IRKSTERMS";
			nSellerID = IRK4;
			break;
		case 1:
			pVarName = "YEEFSTERMS";
			nSellerID = YEEF8;
			break;
		case 2:
			pVarName = "QUOSHSTERMS";
			nSellerID = QUOSH23;
			break;
		}

		CBagVar *pVar = g_VarManager->getVariable(pVarName);
		if (pVar != nullptr) {
			pVar->setValue(g_stSellerNames[nSellerID]._nAmount);
		}
	}

	// Retrieve buyer terms
	nNumToSave = 5;
	BUYERS nBuyerID = PYLON3;
	int nMineralID = 0;
	CBagVar *pVar;

	for (int i = 0; i < nNumToSave; i++) {
		switch (i) {
		case 0:
			pVarName = "SINJINSTTERMS";
			nBuyerID = SINJIN11;
			nMineralID = kSinjinMineralID;
			break;
		case 1:
			pVarName = "GUNGBATERMS";
			nBuyerID = GUNG14;
			nMineralID = kGungMineralID;
			break;
		case 2:
			pVarName = "GILDZITERMS";
			nBuyerID = GILD13;
			nMineralID = kGildMineralID1;
			break;
		case 3:
			pVarName = "GILDPLTERMS";
			nBuyerID = GILD13;
			nMineralID = kGildMineralID2;
			break;
		case 4:
			pVarName = "DORKUPTERMS";
			nBuyerID = DORK44;
			nMineralID = kDorkMineralID;
			break;
		}

		pVar = g_VarManager->getVariable(pVarName);
		if (pVar != nullptr) {
			pVar->setValue(g_stBuyerBids[nBuyerID]._nMineralVal[nMineralID]);
		}
	}

	// Save the list of who is in on this deal... use a bit to indicate if they are in or not.
	uint32 nBuyersMask = 0;
	for (int i = 0; i < NUM_BUYERS; i++) {
		if (g_stBuyerBids[i]._bAccept == true) {
			nBuyersMask |= (1 << i);
		}
	}

	pVar = g_VarManager->getVariable("BUYERSMASK");
	if (pVar != nullptr) {
		pVar->setValue(nBuyersMask);
	}

	// Save the list of who is available and who is not.
	// Do this using a mask.
	uint32       nAvailMask = 0;
	uint32       nIndex = 0;
	for (int i = 0; i < NUM_BUYERS; i++, nIndex++) {
		if (g_stBuyerBids[i]._bAvailable == false) {
			nAvailMask |= (1 << nIndex);
		}
	}

	for (int i = 0; i < NUM_SELLERS; i++, nIndex++) {
		if (g_stSellerNames[i]._bAvailable == false) {
			nAvailMask |= (1 << nIndex);
		}
	}

	for (int i = 0; i < NUM_OTHER_PARTYS; i++, nIndex++) {
		if (g_stOtherPartys[i]._bAvailable == false) {
			nAvailMask |= (1 << nIndex);
		}
	}

	assert(nIndex < 32);

	pVar = g_VarManager->getVariable("AVAILABLEMASK");
	if (pVar != nullptr) {
		pVar->setValue(nAvailMask);
	}

	// Save other party's info
	OTHERPARTYS nOtherID = SWONZA5;

	nNumToSave = 3;
	for (int i = 0; i < nNumToSave; i++) {
		switch (i) {
		case 0:
			pVarName = "SWONZATERMS";
			nOtherID = SWONZA5;
			break;
		case 1:
			pVarName = "POLITTERMS";
			nOtherID = POLITICIANS;
			break;
		case 2:
			pVarName = "ENVIROTERMS";
			nOtherID = ENVIRONMENTALISTS;
			break;
		}

		pVar = g_VarManager->getVariable(pVarName);
		if (pVar != nullptr) {
			pVar->setValue(g_stOtherPartys[nOtherID]._nPaymentAmount);
		}
	}

	// Save swonza's state
	pVar = g_VarManager->getVariable("SWONZAENLIGHTENED");
	if (pVar != nullptr) {
		pVar->setValue(_bSwonzaEnlightened);
	}

	// Now the real pain in the ass... saving team information...
	nNumToSave = 6;

	DispatchedTeamItem  teamListItem;
	int nOutstandingTeams = 0;
	char szFLAGS[20];
	char szMEETWITH[20];
	char szDISPATCHTIME[20];
	char szCAPTAIN[20];
	char szMEETINGTIME[20];

	// Get the number of outstanding teams, we don't want to overkill
	if (_pTeamList) {
		nOutstandingTeams = _pTeamList->getCount();
	}

	for (int i = 0; i < nNumToSave; i++) {
		// Build the variable names with default values.
		Common::sprintf_s(szFLAGS, "%s%d%s", "TEAM", i + 1, "FLAGS");
		Common::sprintf_s(szMEETWITH, "%s%d%s", "TEAM", i + 1, "MEETWITH");
		Common::sprintf_s(szDISPATCHTIME, "%s%d%s", "TEAM", i + 1, "DISPATCHTIME");
		Common::sprintf_s(szCAPTAIN, "%s%d%s", "TEAM", i + 1, "CAPTAIN");
		Common::sprintf_s(szMEETINGTIME, "%s%d%s", "TEAM", i + 1, "MEETINGTIME");

		if (i < nOutstandingTeams) {
			teamListItem = _pTeamList->getNodeItem(i);
		} else {
			teamListItem._nFlags = 0;
			teamListItem._nMeetWithID = 0;
			teamListItem._nDispatchTime = 0;
			teamListItem._nTeamCaptain = 0;
			teamListItem._nMeetingTime = 0;
		}

		pVar = g_VarManager->getVariable(szFLAGS);
		if (pVar != nullptr) {
			pVar->setValue(teamListItem._nFlags);
		}

		pVar = g_VarManager->getVariable(szMEETWITH);
		if (pVar != nullptr) {
			pVar->setValue(teamListItem._nMeetWithID);
		}

		pVar = g_VarManager->getVariable(szDISPATCHTIME);
		if (pVar != nullptr) {
			pVar->setValue(teamListItem._nDispatchTime);
		}

		pVar = g_VarManager->getVariable(szCAPTAIN);
		if (pVar != nullptr) {
			pVar->setValue(teamListItem._nTeamCaptain);
		}

		pVar = g_VarManager->getVariable(szMEETINGTIME);
		if (pVar != nullptr) {
			pVar->setValue(teamListItem._nMeetingTime);
		}
	}

	// Save the successful and unsuccessful meeting status.  Here's how we're gonna do
	// this.  For each negotiating target, we will save a bit in "METWITH", 0 => no
	// meeting occurred, 1 => a meeting happened with this dude.  There will be a
	// second variable, "MEETINGRESULTS", there will be a bit that is synonymous with
	// the "METWITH" var indicating the result of the meeting, i.e. 0=>meeting was a
	// success and 1=> meeting was a failure.
	//
	// I am writing this code as a response to bug number 7619 which just requested
	// that the results of each meeting be displayed on the negotiations screen, I
	// am willing to bet a months paycheck that someone comes back and requests that
	// the attendees of each meeting be listed with the success/failure of each.

	int nBitNo = 0;
	int nMetWithVal = 0;
	int nMeetingResultVal = 0;

	// Save seller meeting history
	for (int i = 0; i < NUM_SELLERS; i++, nBitNo++) {
		if (g_stSellerNames[i]._nMeetingResult != SRAF_NO_MEETING) {
			nMetWithVal |= (1 << nBitNo);
			nMeetingResultVal |= (g_stSellerNames[i]._nMeetingResult == SRAF_GOOD_MEETING ? 1 << nBitNo : 0);
		}
	}

	// Save buyer meeting history
	for (int i = 0; i < NUM_BUYERS; i++, nBitNo++) {
		if (g_stBuyerBids[i]._nMeetingResult != SRAF_NO_MEETING) {
			nMetWithVal |= (1 << nBitNo);
			nMeetingResultVal |= (g_stBuyerBids[i]._nMeetingResult == SRAF_GOOD_MEETING ? 1 << nBitNo : 0);
		}
	}

	// Save other party meeting history
	for (int i = 0; i < NUM_OTHER_PARTYS; i++, nBitNo++) {
		if (g_stOtherPartys[i]._nMeetingResult != SRAF_NO_MEETING) {
			nMetWithVal |= (1 << nBitNo);
			nMeetingResultVal |= (g_stOtherPartys[i]._nMeetingResult == SRAF_GOOD_MEETING ? 1 << nBitNo : 0);
		}
	}

	// Now save the variables
	pVar = g_VarManager->getVariable("METWITH");
	if (pVar != nullptr) {
		pVar->setValue(nMetWithVal);
	}

	pVar = g_VarManager->getVariable("MEETINGRESULTS");
	if (pVar != nullptr) {
		pVar->setValue(nMeetingResultVal);
	}

	// All done!
}

void SrafComputer::setMeetingResult(int nFlags, int nMetWith, bool bSucceeded) {
	if ((nFlags & mSeller) && nMetWith < NUM_SELLERS) {
		g_stSellerNames[nMetWith]._nMeetingResult = (bSucceeded ? SRAF_GOOD_MEETING : SRAF_BAD_MEETING);
	}

	if ((nFlags & mBuyer) && nMetWith < NUM_BUYERS) {
		g_stBuyerBids[nMetWith]._nMeetingResult = (bSucceeded ? SRAF_GOOD_MEETING : SRAF_BAD_MEETING);
	}

	if ((nFlags & mOtherParty) && nMetWith < NUM_OTHER_PARTYS) {
		g_stOtherPartys[nMetWith]._nMeetingResult = (bSucceeded ? SRAF_GOOD_MEETING : SRAF_BAD_MEETING);
	}
}

} // namespace SpaceBar
} // namespace Bagel
