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
#include "bagel/boflib/app.h"
#include "bagel/baglib/bagel.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/link_object.h"
#include "bagel/bagel.h"

namespace Bagel {
namespace SpaceBar {

#define SRAFDIR         "$SBARDIR\\SRAFFA\\CLOSEUP\\COMPUTER\\"
#define SRAFMALEDIR     "$SBARDIR\\SRAFFA\\CHAR\\GMALE\\"
#define SRAFFEMALEDIR   "$SBARDIR\\SRAFFA\\CHAR\\GFEMALE\\"
#define SRAFAUDIODIR    "$SBARDIR\\SRAFFA\\AUDIO\\EVENTS\\"
#define SRAFTIMEOUT     "SFTIMOUT.WAV"

#define USETEXTWIDTHS       TRUE

// Local constants
//
// deal summary constants
#define kUnknownTermStr     "(Unknown)"
#define kRightColumnAlign   25

// Sraffan flashback max turns...
#define kSrafMaxTurns       200

// Amount of time to figure out who is driving...
#define kDrivingTime        5

// Constants not linked to any one screen
#define kCheckmark 'x'

#define kStandardIndentation        4
#define kStandardDoubleIndentation  8

// buyer bids summary
#define kFirstMineralColumn 12

#if BOF_MAC
#define kBuyerBidsPointSize     10
#define kOtherPointSize         10
#define kMineralColWidth        5
#else
#define kBuyerBidsPointSize     14
#define kOtherPointSize         13
#define kMineralColWidth        4
#endif

#define kBuyerBidsPointWidth                6
#define kLineItemHeight kBuyerBidsPointSize + 4

// add instructions to the main screen

#define kActivateFooterStr1  "NOTE: On any subsequent screens where the information displayed does"
#define kActivateFooterStr2  "not fit on the screen, scroll up or down one line using up-arrow and"
#define kActivateFooterStr3  "down-arrow.  Scroll up or down a page using page-up or page-down."

#if BOF_MAC
#define kBuyerBidsHeaderStr  "BUYER       Zn   Ba   Rg   Ut   Pn   Sz    0   H20  LH   CH   ME   TE   AS   PD   ACCEPT"
#else
#define kBuyerBidsHeaderStr  "BUYER      Zn  Ba  Rg  Ut  Pn  Sz   0  H20 LH  CH  ME  TE  AS  PD   ACCEPT"
#endif
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

// code word constants
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

// main menu items

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
	const char *m_pszName;
	const char *m_pszUp;
	const char *m_pszDown;
	const char *m_pszFocus;
	const char *m_pszDisabled;
	int m_nLeft;
	int m_nTop;
	int m_nWidth;
	int m_nHeight;
	int m_nID;

};

#define NOMEETING 0
#define GOODMEETING 1
#define BADMEETING 2

struct SELLERITEM {
	const char *m_pszName;
	int16       m_nAmount;
	SELLERS     m_nSellerID;
	const char *m_pszSellerBio;
	bool        m_bMeetWith;
	bool        m_bAvailable;
	int16       m_nMeetingResult;
};

// struct for email messages

struct EMAILITEM {
	const char *m_pszMessageSubj;
	const char *m_pszMessageFrom;
	const char *m_pszMessageTo;
	const char *m_pszMessageFile;
};

// struct for buyer bids grid
struct BUYERBIDSREC {
	const char *m_pszName;
	int16       m_nMineralVal[NUM_MINERALS];
	int16       m_nBidSum;
	bool        m_bAccept;
	BUYERS      m_nBuyerID;
	const char *m_pszBuyerBio;
	bool        m_bMeetWith;
	bool        m_bAvailable;
	int16       m_nFlags;
	int16       m_nMeetingResult;
};

// Mineral information
struct MINERAL_NAMES {
	const char *m_pszMineralName;
	const char *m_pszMineralAbbrev;
};

// staffer bio information
struct STAFFERITEM {
	const char *m_pszStafferName;
	const char *m_pszStafferBio;
	const char *m_pszStafferBmp;
	bool     m_bAvailable;
	bool     m_bOnCurrentTeam;
	int      m_nFlags;
	int      m_nNegotiatingStrength;
	int      m_nMeetingTime;
};

// staffer bio information
struct OTHERITEM {
	const char *m_pszName;
	const char *m_pszOtherPartyBio;
	bool     m_bMeetWith;
	bool     m_bAvailable;
	int16    m_nPaymentAmount;
	int16    m_nMeetingResult;
};

struct AUDIOITEM {
	const char *m_pszTitle;
	const char *m_pszAuthor;
	const char *m_pszPlanet;
	const char *m_pszAudioFile;
	CBofSound *m_pMidiTrack;
};

#define kRandomPlayTime 15

struct OFFERINGITEM {
	const char *m_pszType;
	const char *m_pszOffering;
	const char *m_pszFile;
	const char *m_pszRcvOfferingFile;
};

// codewords
struct CODEWORDITEM {
	const char *m_pszCodeWord1;
	const char *m_pszCodeWord2;
	const char *m_pszCodeWord3;
	const char *m_pszCodeWord4;
};


// Local globals
//
// there are 6 regular staffers, but durteen, the 7th, is the one that notifies deven that
// time has run out... we need room in the structure for him also, although he won't be
// doing any negotiating or meetings for us.
static STAFFERITEM g_staffers[NUM_STAFFERS + 1] = {
	{ "Norg-72",   "nor72bio.txt", "sanenorg.bmp",   TRUE, FALSE, mStafferMale,    1,  30 },
	{ "Pnurth-81", "Pnu81bio.txt", "sanepnur.bmp",   TRUE, FALSE, mStafferFemale,  3,  30 },
	{ "Zig-64",    "Zig64bio.txt", "sanezig.bmp",    TRUE, FALSE, mStafferFemale,  6,  18 },
	{ "Lentil-24", "Len24bio.txt", "sanelent.bmp",   TRUE, FALSE, mStafferFemale,  5,  15 },
	{ "Vargas-20", "Var20bio.txt", "sanevarg.bmp",   TRUE, FALSE, mStafferMale,   10,  25 },
	{ "Churg-53",  "Chu53bio.txt", "sanechur.bmp",   TRUE, FALSE, mStafferMale,    5,  22 },
	{ "Durteen-97", "DUR97bio.txt", "sanedurt.bmp",   TRUE, FALSE, mStafferMale,    0,  0 },
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
	{ "Irk-4", -1, IRK4, "IRK4BIO.TXT", FALSE, TRUE, NOMEETING },
	{ "Yeef-8", 38, YEEF8, "YEE8BIO.TXT", FALSE, TRUE, NOMEETING },
	{ "Quosh-23", -1, QUOSH23, "QUO11BIO.TXT", FALSE, TRUE, NOMEETING },
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

#define MINMARTENTRIES              6

// Buyer bids grid, dork, gung sinjin and gild can be talked up, but the
// rest of them are non-negotiable. Only dork and gung are part of the
// final solution.
static BUYERBIDSREC g_stBuyerBids[NUM_BUYERS] = {
	{ "Pylon-3",    { 0,  0, 0, 0, 0, 0,  7,  9, 2, 3, 4, 8, 0, 0 }, 33, FALSE, PYLON3, "PYL3BIO.TXT", FALSE, TRUE, mBuyerFemale, NOMEETING },
	{ "Dippik-10",  { 0,  0, 0, 0, 0, 0,  5,  4, 0, 0, 0, 0, 6, 8 }, 23, FALSE, DIPPIK10, "DIP10BIO.TXT", FALSE, TRUE, mBuyerMale, NOMEETING },
	{ "Vebbil-18",  { 0,  0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 7, 0, 0 }, 7, FALSE, VEBBIL18, "VEB18BIO.TXT", FALSE, TRUE, mBuyerFemale, NOMEETING },
	{ "Gung-14",    { 0,  4, 2, 0, 0, 10, 0,  0, 0, 0, 0, 0, 0, 0 }, 16, FALSE, GUNG14, "GUN14BIO.TXT", FALSE, TRUE, mBuyerMale, NOMEETING },
	{ "Reyes-24",   { 10, 0, 0, 7, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 17, FALSE, REYES24, "REY24BIO.TXT", FALSE, TRUE, mBuyerMale, NOMEETING },
	{ "Gild-13",    { 6,  0, 0, 0, 4, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 10, FALSE, GILD13, "GIL13BIO.TXT", FALSE, TRUE, mBuyerFemale, NOMEETING },
	{ "Hem-20",     { 0,  0, 3, 0, 5, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 8, FALSE, HEM20, "HEM20BIO.TXT", FALSE, TRUE, mBuyerFemale, NOMEETING },
	{ "Dork-44",    { 0,  6, 0, 4, 0, 11, 0,  0, 0, 0, 0, 0, 0, 0 }, 21, FALSE, DORK44, "DOR44BIO.TXT", FALSE, TRUE, mBuyerMale, NOMEETING },
	{ "Rawley-23",  { 0,  0, 0, 0, 0, 0,  0,  0, 0, 0, 8, 0, 0, 0 }, 8, FALSE, RAWLEY23, "RAW23BIO.TXT", FALSE, TRUE, mBuyerFemale, NOMEETING },
	{ "Jella-37",   { 0,  0, 0, 0, 0, 0,  0,  0, 4, 0, 0, 0, 0, 0 }, 4, FALSE, JELLA37, "JEL37BIO.TXT", FALSE, TRUE, mBuyerFemale, NOMEETING },
	{ "Sinjin-11",  { 0,  0, 0, 0, 0, 0,  12, 0, 0, 0, 0, 0, 5, 6 }, 23, FALSE, SINJIN11, "SIN11BIO.TXT", FALSE, TRUE, mBuyerMale, NOMEETING },
	{ "Hundey-42",  { 0,  0, 4, 0, 0, 0,  0,  0, 3, 0, 0, 0, 0, 6 }, 13, FALSE, HUNDEY42, "HUN42BIO.TXT", FALSE, TRUE, mBuyerMale, NOMEETING },
	{ "Chandra-15", { 0,  0, 0, 0, 0, 0,  0, 13, 0, 2, 4, 0, 0, 0 }, 19, FALSE, CHANDRA15, "CHA15BIO.TXT", FALSE, TRUE, mBuyerFemale, NOMEETING },
	{ "Clang-2",    { 0,  0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 9 }, 9, FALSE, CLANG2, "CLA2BIO.TXT", FALSE, TRUE, mBuyerMale, NOMEETING },
	{ "Min. Mart",  { 4,  0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 4, FALSE, MINMARTZN, "MINBIO.TXT", FALSE, TRUE, 0, NOMEETING },
	{ "Min. Mart",  { 0,  3, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 3, FALSE, MINMARTBA, "MINBIO.TXT", FALSE, TRUE, 0, NOMEETING },
	{ "Min. Mart",  { 0,  0, 1, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 1, FALSE, MINMARTRG, "MINBIO.TXT", FALSE, TRUE, 0, NOMEETING },
	{ "Min. Mart",  { 0,  0, 0, 3, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 3, FALSE, MINMARTUT, "MINBIO.TXT", FALSE, TRUE, 0, NOMEETING },
	{ "Min. Mart",  { 0,  0, 0, 0, 3, 0,  0,  0, 0, 0, 0, 0, 0, 0 }, 3, FALSE, MINMARTPN, "MINBIO.TXT", FALSE, TRUE, 0, NOMEETING },
	{ "Min. Mart",  { 0,  0, 0, 0, 0, 7,  0,  0, 0, 0, 0, 0, 0, 0 }, 7, FALSE, MINMARTSZ,  "MINBIO.TXT", FALSE, TRUE, 0, NOMEETING },
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
	{ "Swonza-5", "SWO5BIO.TXT", FALSE, TRUE, -1, NOMEETING },
	{ "Politicians", "POLITBIO.TXT", FALSE, TRUE, -30, NOMEETING },
	{ "Environmentalists", "ENVIRBIO.TXT", FALSE, TRUE, -1, NOMEETING },
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

static AUDIOITEM g_stAudioSetting[NUM_MUSICAL_SCORES] = {
	{ "Harpsichord Invention #1", "J. S. Bach",     "Earth",      "SRAFFAN1.MID", nullptr },
	{ "Jazz Theme #44981",        "Urgon-Thmo",     "Thelbia",    "SRAFFAN2.MID", nullptr },
	{ "Bamboo Breeze",            "H. Fugimachi",   "Earth",      "SRAFFAN3.MID", nullptr },
	{ "Power of Crystal, OP. 12", "Brak-44",        "H'poctyl",   "SRAFFAN4.MID", nullptr },
	{ "String Quartet",           "J. Salesin",     "Earth",      "SRAFFAN5.MID", nullptr },
#if BOF_MAC
	{ "Chicken Dance",            "Unknown Origin", "Mars",       "CHIKDANC.MID", nullptr },
#else
	{ "The Womp Song",            "The Womps",      "Armpit IV",  "SRAFFAN6.MID", nullptr },
#endif
	{ "Mixed Selections",         " ",    " ",       nullptr,        nullptr },
};

static OFFERINGITEM g_stOfferings[NUM_OFFERINGS] = {
	{ "BEVERAGE:", "Distilled Fleeblunx Sap", "BEVDESC.TXT", "DRINKRCV.TXT" },
	{ "LIGHT SNACK:", "Br'thl'gian Centipede Legs", "SNKDESC.TXT", "SNACKRCV.TXT" }
};

// codewords

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

static CBofRect gCompDisplay(40, 40, 600, 440);
static CBofRect gTextWindow(0, 0, 640, 480);            // Window to place text box
static CBofRect gStafferDisplay(0, 0, 640, 480);        // staffer (SANE*.BMP) bitmaps cover whole screen
static CBofRect gSrafTextWindow(0, 440, 640 - 1, 480 - 1);

static bool gTextScreenFrontmost = FALSE;
static CBofString  gBidStr;
static int gDispatchCurState = 0;       // Retain current state of dispatch negotiating team screen
static int gTurncountLineNo = 0;    // keep track of turncount line number
// member static initializations

bool          SrafComputer::m_bSwonzaEnlightened = FALSE;
SrafComputer *SrafComputer::m_pHead = nullptr;           // only sraffan computer
int           SrafComputer::m_nStartingTime = 0;
bool          SrafComputer::m_bRandomAudio = FALSE;
int           SrafComputer::m_nRandomTime = 0;
bool          SrafComputer::m_bFailureNotified = FALSE;

// states that the dispatch negotiating team can take on
#define mSellersExpanded 0x01
#define mBuyersExpanded  0x02
#define mOthersExpanded  0x04

// Local prototype functions
const char *BuildSrafDir(const char *pszFile);
const char *BuildAudioDir(const char *pszFile);
const char *BuildMaleSrafDir(const char *pszFile);
const char *BuildFemaleSrafDir(const char *pszFile);

// Local prototypes

SrafComputer::SrafComputer() {
	int i;

	m_cTextColor = RGB(255, 255, 255);              // start out with white as our text color
	m_cTextHiliteColor = RGB(255, 255, 255);                // and some other color as our hilite

#if BOF_MAC
	m_cTextLineColor = 0x80;
#else
	m_cTextLineColor = RGB(0xFF, 0xFF, 0x00);
#endif

	// computer starts as off and at the main screen
	m_eMode = SCOFF;
	m_eCurScreen = SCMAIN;

	// Initialize our three buttons
	for (i = 0; i < NUM_SRAFCOMPBUTT; i++) {
		m_pButtons[i] = nullptr;
	}

	// Initialize bitmaps (one extra staffer, durteen)

	for (i = 0; i < (NUM_STAFFERS + 1); i++) {
		m_pStafferBmp[i] = nullptr;
	}

	// List and text box
	m_pLBox = nullptr;
	m_pMainList = nullptr;
	m_nSelection = -1;
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = 20;

	// Subscreen initializations

	m_pSellerSummaryList = nullptr;
	m_pBuyerSummaryList = nullptr;
	m_pTeamList = nullptr;

	// text only screen initializations

	m_pTextOnlyScreen = nullptr;

	m_pszGroup1Word = nullptr;
	m_pszGroup2Word = nullptr;
	m_bSrafAttached = FALSE;

	m_pHead = this;
}

//

// fyi - we only use the seller summary list to build the onscreen
// list, we use the global to store intermediate data.
void SrafComputer::RecalcSellerSummaryList() {
	DealSummarySellerItem   sellerItem;
	int                     i = 0;

	// Initialize the initial state of the seller summary
	//
	// Ok all the following is due to VC2.0 Templates the compiler needs to see an instance before the destructor
	// othewise it will not include the code for the destructor.
	if (m_pSellerSummaryList == nullptr) {
		m_pSellerSummaryList = new CBofList<DealSummarySellerItem>;
		Assert(m_pSellerSummaryList != nullptr);
	} else {
		delete m_pSellerSummaryList;
		m_pSellerSummaryList = nullptr;
	}
	// lets verify that its all set before we go to use it
	if (m_pSellerSummaryList == nullptr) {
		m_pSellerSummaryList = new CBofList<DealSummarySellerItem>;
		Assert(m_pSellerSummaryList != nullptr);
	}

	while (i < NUM_SELLERS) {
		sellerItem.m_eSellerID = g_stSellerNames[i].m_nSellerID;
		sellerItem.m_nSellerOffer = g_stSellerNames[i].m_nAmount;
		sellerItem.m_pSellerName = g_stSellerNames[i].m_pszName;

		m_pSellerSummaryList->AddToTail(sellerItem);
		i++;
	}
}


void SrafComputer::RecalcBuyerSummaryList() {
	DealSummaryBuyerItem *pBuyerItem;
	int                     i = 0;

	// Initialize the initial state of the deal summary

	// Ok all the following is due to VC2.0 Templates the compiler needs to see an instance before the destructor
	// othewise it will not include the code for the destructor.
	if (m_pBuyerSummaryList == nullptr) {
		m_pBuyerSummaryList = new CBofList<DealSummaryBuyerItem>;
		Assert(m_pBuyerSummaryList != nullptr);
	} else {
		delete m_pBuyerSummaryList;
		m_pBuyerSummaryList = nullptr;
	}
	// lets verify that its all set before we go to use it
	if (m_pBuyerSummaryList == nullptr) {
		m_pBuyerSummaryList = new CBofList<DealSummaryBuyerItem>;
		Assert(m_pBuyerSummaryList != nullptr);
	}

	i = 0;
	while (i < NUM_BUYERS) {
		if (g_stBuyerBids[i].m_bAccept) {
			pBuyerItem = new DealSummaryBuyerItem();
			Assert(pBuyerItem != nullptr);

			pBuyerItem->m_eBuyerID = g_stBuyerBids[i].m_nBuyerID;
			pBuyerItem->m_nBuyerOffer = g_stBuyerBids[i].m_nBidSum;
			pBuyerItem->m_pBuyerName = g_stBuyerBids[i].m_pszName;

			m_pBuyerSummaryList->AddToTail(*pBuyerItem);
			delete pBuyerItem;
		}

		i++;
	}
}

// Pretty crappy routine that verifies whether or not the dispatch team is cool
// the way they are.
bool SrafComputer::VerifyDispatchTeam() {
	bool bValidTeam = TRUE;
	char szFailureReason[256];
	int nMeetOthers, nMeetSellers, nMeetBuyers;
	int nTeam = 0;
	int nDispatchFlags;
	int nMeetingWith;

	// Make sure that we have someone to meet
	nMeetOthers = GetMeetMember(kOthersList);
	nMeetSellers = GetMeetMember(kSellersList);
	nMeetBuyers = GetMeetMember(kBuyersList);

	if (nMeetOthers == -1 && nMeetSellers == -1 && nMeetBuyers == -1) {
		bool        bStafferSelected = FALSE;
		// nobody to meet with, see if we have any team members
		// selected.  This changes the error message.
		for (int i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i].m_bOnCurrentTeam) {
				bStafferSelected = TRUE;
				break;
			}
		}

		if (bStafferSelected == FALSE) {
			Common::strcpy_s(szFailureReason, kszNotEvenClose);
		} else {
			Common::strcpy_s(szFailureReason, kszNoNegotiators);
		}
		bValidTeam = FALSE;
	}

	if (bValidTeam) {
		if ((nMeetOthers != -1 && nMeetSellers != -1 && nMeetBuyers != -1)) {
			Common::sprintf_s(szFailureReason, ksz3MeetingTargets,
			                  g_stSellerNames[nMeetSellers].m_pszName,
			                  g_stOtherPartys[nMeetOthers].m_pszName,
			                  g_stBuyerBids[nMeetBuyers].m_pszName);
			bValidTeam = FALSE;
		}
	}

	if (bValidTeam) {
		if ((nMeetOthers != -1 && nMeetSellers != -1)) {
			Common::sprintf_s(szFailureReason, ksz2MeetingTargets,
			                  g_stSellerNames[nMeetSellers].m_pszName,
			                  g_stOtherPartys[nMeetOthers].m_pszName);
			bValidTeam = FALSE;
		}
	}

	if (bValidTeam) {
		if ((nMeetSellers != -1 && nMeetBuyers != -1)) {
			Common::sprintf_s(szFailureReason, ksz2MeetingTargets,
			                  g_stSellerNames[nMeetSellers].m_pszName,
			                  g_stBuyerBids[nMeetBuyers].m_pszName);
			bValidTeam = FALSE;
		}
	}

	if (bValidTeam) {
		if ((nMeetOthers != -1 && nMeetBuyers != -1)) {
			Common::sprintf_s(szFailureReason, ksz2MeetingTargets,
			                  g_stOtherPartys[nMeetOthers].m_pszName,
			                  g_stBuyerBids[nMeetBuyers].m_pszName);
			bValidTeam = FALSE;
		}
	}

	// Make sure at least one staff member is sent on this mission

	int nMaleMembers = 0, nFemaleMembers = 0;
	if (bValidTeam) {
		for (int i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i].m_bOnCurrentTeam) {
				nTeam |= (1 << (i + 3));
				if (g_staffers[i].m_nFlags & mStafferMale)
					nMaleMembers++;
				else
					nFemaleMembers++;
			}
		}

		if (nTeam == 0) {
			Common::strcpy_s(szFailureReason, kszNoTeamMembers);
			bValidTeam = FALSE;
		}
	}

	// No meetings with irk or quosh allowed.
	if (bValidTeam) {
		if (nMeetSellers == IRK4) {
			Common::strcpy_s(szFailureReason, kszCantMeetIrk);
			bValidTeam = FALSE;
		}

		if (nMeetSellers == QUOSH23) {
			Common::strcpy_s(szFailureReason, kszCantMeetQuosh);
			bValidTeam = FALSE;
		}
	}

	// don't allow repeat negotiations with someone that
	// has already been successfully negotiated with.
	if (bValidTeam) {
		if (nMeetSellers != -1) {
			if (g_stSellerNames[nMeetSellers].m_nMeetingResult == GOODMEETING) {
				Common::strcpy_s(szFailureReason, kszCantRenegSeller);
				bValidTeam = FALSE;
			}
		}

		if (nMeetOthers != -1) {
			if (g_stOtherPartys[nMeetOthers].m_nMeetingResult == GOODMEETING) {
				Common::strcpy_s(szFailureReason, kszCantRenegOther);
				bValidTeam = FALSE;
			}
		}

		if (nMeetBuyers != -1) {
			if (g_stBuyerBids[nMeetBuyers].m_nMeetingResult == GOODMEETING) {
				Common::strcpy_s(szFailureReason, kszCantRenegBuyer);
				bValidTeam = FALSE;
			}
		}
	}

	// Build a valid meeting list element.
	DispatchedTeamItem *pTeamItem = nullptr;
	if (bValidTeam) {

		nDispatchFlags = (nMeetOthers != -1 ? mOtherParty : (nMeetSellers != -1 ? mSeller : mBuyer));
		nMeetingWith = (nMeetOthers != -1 ? nMeetOthers : (nMeetSellers != -1 ? nMeetSellers : nMeetBuyers));

		if (m_pTeamList == nullptr) {
			m_pTeamList = new CBofList<DispatchedTeamItem>;
			Assert(m_pTeamList != nullptr);
		}

		pTeamItem = new DispatchedTeamItem();
		Assert(pTeamItem != nullptr);

		pTeamItem->m_nFlags = nTeam | nDispatchFlags;
		pTeamItem->m_nMeetWithID = nMeetingWith;
		CBagVar *pVar = VARMNGR->GetVariable("SRATURNCOUNT");
		Assert(pVar != nullptr);

		pTeamItem->m_nDispatchTime = pVar->GetNumValue();
		pTeamItem->m_nMeetingTime = CalculateMeetingTime(pTeamItem->m_nFlags);

		// The spokesperson will be the majority of the sexes
		// If same number of each, then randomize it.

		if (nMaleMembers == nFemaleMembers) {
			int     nRand = 0;
			nRand = pTeamItem->m_nDispatchTime; // ?? Brian - need to get a random number here to decide
			// spokesperson of even team
			nMaleMembers += (nRand & 1 ? 1 : -1);
		}

		// Pick a team captain, must be same sex as the announcer.
		pTeamItem->m_nTeamCaptain = GetTeamCaptain(pTeamItem->m_nFlags);

		// team gender is based on sex of captain.
		pTeamItem->m_nFlags |= GetTeamGender(pTeamItem->m_nFlags);

		m_pTeamList->AddToTail(*pTeamItem);

		// Finally, since we have a valid team, set those team members who are attending to
		// be unavailable for any other staff meetings, also reset them to FALSE for
		// on current team

		for (int i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i].m_bOnCurrentTeam) {
				g_staffers[i].m_bOnCurrentTeam = FALSE;
				g_staffers[i].m_bAvailable = FALSE;
			}
		}

	}

	// There are all kinds of ways to screw up a meeting, but we address those in some kind of
	// staff meeting grid (search StaffMeeting)
	if (bValidTeam == FALSE) {
		DisplayMessage(szFailureReason);
	} else {
		char        szLocalBuff[256];
		szLocalBuff[0] = '\0';
		CBofString  sStr(szLocalBuff, 256);
		//char      *pszResponse;

		// We're very excited, we notified the user, now clear up the globals and reset.

		if (nMeetOthers != -1) {
			g_stOtherPartys[nMeetOthers].m_bMeetWith = FALSE;
			g_stOtherPartys[nMeetOthers].m_bAvailable = FALSE;
		} else {
			if (nMeetSellers != -1) {
				g_stSellerNames[nMeetSellers].m_bMeetWith = FALSE;
				g_stSellerNames[nMeetSellers].m_bAvailable = FALSE;
			} else {
				g_stBuyerBids[nMeetBuyers].m_bMeetWith = FALSE;
				g_stBuyerBids[nMeetBuyers].m_bAvailable = FALSE;
			}
		}

		// Redraw the screen with the meeting with column colapsed and
		// the checkmarks out of the staffer columns.  This assures that
		// any screen capture will come back to us ready for display to the
		// user.

		InitDispatchTeam();
		ActivateDispatchTeam();
		//char *pszBmpFile = nullptr;
		//int           nStaffCaptain;

		// Also have to give the boss a sound or text file to play.
		if (pTeamItem->m_nFlags & mStafferMale) {
			sStr = BuildMaleSrafDir(kGSM1SraMaleStr);
		} else {
			sStr = BuildFemaleSrafDir(kGSM1SraFemStr);
		}

		// Notify Deven...
		NotifyBoss(sStr, pTeamItem->m_nTeamCaptain);

		// All done with our team item, trash it.
		if (bValidTeam && pTeamItem != nullptr) {
			delete pTeamItem;
		}
	}

	return bValidTeam;
}


SrafComputer::~SrafComputer() {
	// If we haven't detached yet, do that here.  The reason we do the
	// detach here is to avoid a palette shift... if we do it at the time
	// that we hit the "return to main" button then we get a really ugly
	// palette shift while we return to the bar.
#if BOF_MAC
	Detach();
#endif

	// These lists are persistent across turning the computer on and off, so
	// delete them only at the end of the game, not when you turn on/off the
	// computer (attach/detach)
	if (m_pSellerSummaryList != nullptr) {
		delete m_pSellerSummaryList;
		m_pSellerSummaryList = nullptr;
	}

	if (m_pBuyerSummaryList != nullptr) {
		delete m_pBuyerSummaryList;
		m_pBuyerSummaryList = nullptr;
	}

	if (m_pTeamList != nullptr) {
		delete m_pTeamList;
		m_pTeamList = nullptr;
	}

	// we grab these bad babies in the attach sequence, but since
	// we need them to live past having the computer on, we need to
	// destruct them in the destructor.

	for (int i = 0; i < (NUM_MUSICAL_SCORES - 1); i++) {
		if (g_stAudioSetting[i].m_pMidiTrack != nullptr) {
			delete g_stAudioSetting[i].m_pMidiTrack;
			g_stAudioSetting[i].m_pMidiTrack = nullptr;
		}
	}

	m_pTextOnlyScreen = nullptr;
	m_pHead = nullptr;
}

//

void SrafComputer::OnMainLoop() {
	if (m_bFirstPaint) {
		m_bFirstPaint = FALSE;
		AttachActiveObjects();
	}
}

//

void SrafComputer::OnPaint(CBofRect *pRect) {
	if (GetBackdrop()) {
		Assert(GetWorkBmp() != nullptr);

		// erase everything from the background
		GetWorkBmp()->Paint(GetBackdrop(), pRect, pRect);

		// paint all the objects to the background
		PaintStorageDevice(nullptr, GetBackdrop(), pRect);
	}

	// Paint the backdrop
	if (GetBackdrop()) {
		PaintBackdrop();
	}

#if 0 //BOF_WINDOWS
	if (m_pLBox != nullptr) {
		m_pLBox->RepaintAll();
	}
#endif

#if 0 //BOF_WINDOWS
	if (m_eMode == SCON) {
		m_pButtons[QUIT_BUTTON]->Paint(nullptr);
	}
#else

	if (m_eMode == SCOFF) {
		m_pButtons[ON_BUTTON]->Paint(nullptr);
	} else {
		m_pButtons[OFF_BUTTON]->Paint(nullptr);
	}
#endif
}

//

ERROR_CODE  SrafComputer::Update(CBofBitmap * /*pBmp*/, CBofPoint /*pt*/, CBofRect * /*pSrcRect*/, int /*nMaskColor*/) {
	return ERR_NONE;
}

//

ERROR_CODE SrafComputer::Attach() {
	CBofPalette *pPal;
	ERROR_CODE          rc = ERR_NONE;
	CBofRect cRect;
	int i;

	if ((rc = CBagStorageDevWnd::Attach()) == ERR_NONE) {

		// build our main menu list
		Assert(m_pMainList == nullptr);

		m_pMainList = new CBofList<SrafCompItem>;
		Assert(m_pMainList != nullptr);
		FillMain();

		// Bring in all our audio tracks

		for (i = 0; i < (NUM_MUSICAL_SCORES - 1); i++) {
			if (g_stAudioSetting[i].m_pMidiTrack == nullptr) {
				g_stAudioSetting[i].m_pMidiTrack = new CBofSound(this, BuildAudioDir(g_stAudioSetting[i].m_pszAudioFile), SOUND_MIDI | SOUND_ASYNCH | SOUND_LOOP, 32000);
				Assert(g_stAudioSetting[i].m_pMidiTrack != nullptr);
			}
		}

		// Must have a valid backdrop by now

		Assert(m_pBackdrop != nullptr);
		pPal = m_pBackdrop->GetPalette();

		for (i = 0; i < NUM_SRAFCOMPBUTT; i++) {
			if ((m_pButtons[i] = new CBofBmpButton) != nullptr) {

				CBofBitmap *pUp, *pDown, *pFocus, *pDis;

				pUp = LoadBitmap(BuildSrafDir(g_stButtons[i].m_pszUp), pPal);
				pDown = LoadBitmap(BuildSrafDir(g_stButtons[i].m_pszDown), pPal);
				pFocus = LoadBitmap(BuildSrafDir(g_stButtons[i].m_pszFocus), pPal);
				pDis = LoadBitmap(BuildSrafDir(g_stButtons[i].m_pszDisabled), pPal);

				m_pButtons[i]->LoadBitmaps(pUp, pDown, pFocus, pDis);

#if BOF_MAC
				// make this our own custom window such that no frame is drawn
				// around the window/button
				m_pButtons[i]->SetCustomWindow(TRUE);
#endif
				m_pButtons[i]->Create(g_stButtons[i].m_pszName, g_stButtons[i].m_nLeft, g_stButtons[i].m_nTop, g_stButtons[i].m_nWidth, g_stButtons[i].m_nHeight, this, g_stButtons[i].m_nID);
			}
		}

		// Get our code words

		m_pszGroup1Word = new CBofString();
		m_pszGroup2Word = new CBofString();

		// Set the starting time...

		if (m_nStartingTime == 0) {
			CBagVar *pVar = VARMNGR->GetVariable("SRATURNCOUNT");
			Assert(pVar != nullptr);
			m_nStartingTime = pVar->GetNumValue();
		}

		Assert(m_pszGroup1Word != nullptr);
		Assert(m_pszGroup2Word != nullptr);

#if !BOF_MAC
		m_pButtons[QUIT_BUTTON]->Show();
#endif
		Show();

#if BOF_MAC
		m_pButtons[ON_BUTTON]->Show();
#endif

		UpdateWindow();

#if BOF_WINDOWS
		SetOn(); //ActivateMainScreen();
#endif

		// set our window to be the active window so we can receive key
		// down events and things like that...

		m_pLastActive = GetActiveWindow();
		SetActive();

		// Finally, use our regular system cursor, not the custom ones
		CBagCursor::ShowSystemCursor();

		// bring in all the external variables
		RestoreSraffanVars();

		// finally, if we're hallucinating, turn off the hallucination
		// filter.
		CBagVar *pVar = VARMNGR->GetVariable("HALLUCINATE");
		if (pVar && pVar->GetNumValue() > 0) {
			pVar->SetValue(0);
		}
	}
	m_bSrafAttached = TRUE;
	m_bFailureNotified = FALSE;

	// no start state for dispatch screen
	gDispatchCurState = 0;

	return ERR_NONE;
}

//

ERROR_CODE SrafComputer::Detach() {
	int i;
	// Computer gets turned off
	m_eMode = SCOFF;

#if BOF_MAC
	::HideCursor();
#else
	// Get rid of the system cursor
	if (m_bSrafAttached) {
		CBagCursor::HideSystemCursor();
		m_bSrafAttached = FALSE;
	}
#endif

	// save all the external variables
	SaveSraffanVars();

	// Delete our computer buttons
	for (i = 0; i < NUM_SRAFCOMPBUTT; i++) {
		if (m_pButtons[i] != nullptr) {
			delete m_pButtons[i];
			m_pButtons[i] = nullptr;
		}
	}

	if (m_pLBox) {
		delete m_pLBox;
		m_pLBox = nullptr;
	}

	//  (one extra staffer, durteen)
	for (i = 0; i < (NUM_STAFFERS + 1); i++) {
		if (m_pStafferBmp[i] != nullptr) {
			delete m_pStafferBmp[i];
			m_pStafferBmp[i] = nullptr;
		}
	}

	// Get rid of our codewords...

	if (m_pszGroup1Word) {
		delete m_pszGroup1Word;
		m_pszGroup1Word = nullptr;
	}

	if (m_pszGroup2Word) {
		delete m_pszGroup2Word;
		m_pszGroup2Word = nullptr;
	}

#if BOF_MAC
	RemoveFromActiveList();
#endif
	// restore previous active window
	m_pLastActive->SetActive();

	if (m_pMainList != nullptr) {
		delete m_pMainList;
		m_pMainList = nullptr;
	}

	// The attach method will restore the state of the dispatched teams...,
	// we can trash that info here.
	if (m_pTeamList) {
		delete m_pTeamList;
		m_pTeamList = nullptr;
	}

	CBagStorageDevWnd::Detach();

	return ERR_NONE;
}


void SrafComputer::OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void *) {
	// If we don't want to allow exiting from the left hand column,
	// then remove this call.
	//CBagStorageDevWnd::OnLButtonUp(nFlags, xPoint);
}

void SrafComputer::OnLButtonDown(uint32 /*nFlags*/, CBofPoint * /*xPoint*/, void *) {
}
//

void SrafComputer::OnKeyHit(uint32 lKey, uint32 nRepCount) {
	switch (lKey) {
	case BKEY_BACK:                 // temporary, go back to main screen.
		DeleteListBox();
		m_eCurScreen = SCMAIN;
		break;

	case BKEY_ALT_q:
	case BKEY_ALT_Q:
		// don't allow a quit from the sraf computer.
		//Quit();
		break;

	case ' ':
		// increment turn count each time screen is updated.
		IncrementTurnCount();
		break;

	default:
		CBagStorageDevWnd::OnKeyHit(lKey, nRepCount);
		break;
	}
}


void SrafComputer::OnBofButton(CBofObject *pObject, int nState) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	// count a turn if we have been clicked
	//
	// mouse downs on button clicks were causing too many weird
	// problems with meeting results... so get rid of them.
	//
	//if (nState == BUTTON_CLICKED) {
	//  IncrementTurnCount ();
	//}

	CBofButton *pButton;

	pButton = (CBofButton *)pObject;

	switch (m_eCurScreen) {
	case SCMAIN:
		OnButtonMainScreen(pButton, nState);
		break;
	case SCDEAL:
		OnButtonDealSummary(pButton, nState);
		break;
	case SCBIDS:
		OnButtonBuyerBids(pButton, nState);
		break;
	case SCBACKGROUNDDATA:
		OnButtonDealBackground(pButton, nState);
		break;
	case SCSELLER_BIOS:
		OnButtonSellerBios(pButton, nState);
		break;
	case SCOTHER_BIOS:
		OnButtonOtherBios(pButton, nState);
		break;
	case SCSTAFF_BIOS:
		OnButtonStaffBios(pButton, nState);
		break;
	case SCDISPATCH:
		OnButtonDispatchTeam(pButton, nState);
		break;
	case SCEMAIL:
		OnButtonCurrentEMail(pButton, nState);
		break;
	case SCAUDIO:
		OnButtonAudioSettings(pButton, nState);
		break;
	case SCORDER:
		OnButtonRoboButler(pButton, nState);
		break;
	case SCCHECK_TEAMS:
		OnButtonCheckTeams(pButton, nState);
		break;
	case SCCODE_WORDS:
		OnButtonCodeWords(pButton, nState);
		break;
	default:
		break;
	}
}


void SrafComputer::SetOn() {

	m_eMode = SCON;

	ActivateMainScreen();
}

void SrafComputer::SetOff() {
	if (m_eMode != SCOFF) {
		m_eMode = SCOFF;

		m_pButtons[QUIT_BUTTON]->Hide();

#if !BOF_WINDOWS
		m_pButtons[OFF_BUTTON]->Hide();
		m_pButtons[ON_BUTTON]->Show();
#endif

		SetFocus();
#if BOF_MAC
		// whenever dealing with the on/off button, make sure to
		// paint the new bitmap.
		m_pButtons[ON_BUTTON]->Paint(nullptr);
#endif
	}
	DeleteListBox();

	InvalidateRect(&gCompDisplay);
	UpdateWindow();
}

void SrafComputer::SetQuit() {
	Close();
}

ERROR_CODE SrafComputer::CreateListBox() {
	ERROR_CODE error = ERR_NONE;
	//CBofPalette       *pPal;

	if (m_pLBox == nullptr) { // We need to create one

		if ((m_pLBox = new CBofListBox) != nullptr) {
#if BOF_MAC
			// make this our own custom window such that no frame is drawn
			// around the window/button
			m_pLBox->SetCustomWindow(TRUE);
#endif
			error = m_pLBox->Create("ListBox", &gCompDisplay, this);
			if (error != ERR_NONE) {
				return error;
			}
		}
	}

	if (m_pLBox == nullptr)
		return ERR_MEMORY;

	m_pLBox->SetPointSize(m_nListPointSize);
	m_pLBox->SetItemHeight(m_nListItemHeight);
	m_pLBox->SetTextColor(m_cTextColor);
	m_pLBox->SetHighlightColor(m_cTextHiliteColor);
	m_pLBox->SetFont(FONT_MONO);
	m_pLBox->SetFocus();

	return error;
}


void SrafComputer::DeleteListBox() {
	if (m_pLBox) {
		m_pLBox->DeleteAll();       // clears all in the text box
	}
}

void SrafComputer::FillMain() {
	SrafCompItem *pCompItem;
	int i = 0;

	while (i < NUM_MAIN_ITEMS) {
		pCompItem = new SrafCompItem();
		Assert(pCompItem != nullptr);

		pCompItem->m_pItem = g_stMainItems[i];

		m_pMainList->AddToTail(*pCompItem);
		delete pCompItem;

		i++;
	}
}

void SrafComputer::OnBofListBox(CBofObject * /*pListBox*/, int nItemIndex) {
	m_nSelection = nItemIndex;

	// do all kinds of neet things based on our current screen.

	switch (m_eCurScreen) {
	case SCMAIN:
		OnListMainScreen();
		break;
	case SCDEAL:
		OnListDealSummary();
		break;
	case SCBIDS:
		OnListBuyerBids();
		break;
	case SCBACKGROUNDDATA:
		OnListDealBackground();
		break;
	case SCSELLER_BIOS:
		OnListSellerBios();
		break;
	case SCOTHER_BIOS:
		OnListOtherBios();
		break;
	case SCSTAFF_BIOS:
		OnListStaffBios();
		break;
	case SCDISPATCH:
		OnListDispatchTeam();
		break;
	case SCEMAIL:
		OnListCurrentEMail();
		break;
	case SCAUDIO:
		OnListAudioSettings();
		break;
	case SCORDER:
		OnListRoboButler();
		break;
	case SCCHECK_TEAMS:
		OnListCheckTeams();
		break;
	case SCCODE_WORDS:
		OnListCodeWords();
		break;
	default:
		break;
	}

	IncrementTurnCount();
}

void SrafComputer::ActivateDealSummary() {

	ERROR_CODE error = ERR_NONE;
	DealSummaryBuyerItem    buyerItem;
	DealSummarySellerItem   sellerItem;

	char                    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString              sStr(szLocalBuff, 256);

	char                    szRightCol[128];
	int                     totalAsking;
	int                     numItems;
	bool                    bAskKnown;
	int i;

	// current screen is now the DEAL screen.
	m_eCurScreen = SCDEAL;

	// Initialize point size and item height
	m_nListPointSize = kBuyerBidsPointSize;
	m_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"

	DeactivateMainScreen();

	// Make sure we start with the correct computer background

	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// Retrieve IRK's and QUOSH's terms

	CBagVar *pVar;
	if ((pVar = VARMNGR->GetVariable("IRKSTERMS")) != nullptr) {
		g_stSellerNames[IRK4].m_nAmount = pVar->GetNumValue();
	}

	if ((pVar = VARMNGR->GetVariable("QUOSHSTERMS")) != nullptr) {
		g_stSellerNames[QUOSH23].m_nAmount = pVar->GetNumValue();
	}

	RecalcSellerSummaryList();
	RecalcBuyerSummaryList();

	Assert(m_pSellerSummaryList != nullptr);
	Assert(m_pBuyerSummaryList != nullptr);

	// load up the main list now with our deal summary data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	//  Populate first line of list with header

	sStr = "DEAL SUMMARY: Armpit III";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// Align text with others in column

	sStr = "COSTS";
	AlignAtColumn(sStr, "CURRENT TERMS", kRightColumnAlign);
	m_pLBox->AddToTail(sStr, FALSE);

	// populate the listbox with the sellers
	numItems = m_pSellerSummaryList->GetCount();
	totalAsking = 0;

	bAskKnown = TRUE;
	for (i = 0; i < numItems; i++) {
		sellerItem = m_pSellerSummaryList->GetNodeItem(i);
		sStr = sellerItem.m_pSellerName;

		//  Put the appropriate info in the right hand column

		if (sellerItem.m_nSellerOffer == -1) {
			Common::strcpy_s(szRightCol, kUnknownTermStr);
			bAskKnown = FALSE;
		} else {

			Common::sprintf_s(szRightCol, "%2dgZ", sellerItem.m_nSellerOffer);

			totalAsking += sellerItem.m_nSellerOffer;
		}
		AlignAtColumn(sStr, szRightCol, kRightColumnAlign);
		m_pLBox->AddToTail(sStr, FALSE);
	}

	for (i = 0; i < NUM_OTHER_PARTYS; i++) {
		switch (i) {
		case POLITICIANS:
		case ENVIRONMENTALISTS:
			sStr = g_stOtherPartys[i].m_pszName;

			if (g_stOtherPartys[i].m_nPaymentAmount == -1) {
				Common::strcpy_s(szRightCol, kUnknownTermStr);
				bAskKnown = FALSE;
			} else {
				totalAsking += -g_stOtherPartys[i].m_nPaymentAmount;
				Common::sprintf_s(szRightCol, "%2dgZ", -g_stOtherPartys[i].m_nPaymentAmount);
			}

			AlignAtColumn(sStr, szRightCol, kRightColumnAlign);
			m_pLBox->AddToTail(sStr, FALSE);
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

	AlignAtColumn(sStr, szRightCol, kRightColumnAlign);
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = "BUYERS";
	AlignAtColumn(sStr, "CURRENT OFFER", kRightColumnAlign);

	m_pLBox->AddToTail(sStr, FALSE);

	// populate the listbox with the buyers
	numItems = m_pBuyerSummaryList->GetCount();
	totalAsking = 0;

	for (i = 0; i < numItems; i++) {
		buyerItem = m_pBuyerSummaryList->GetNodeItem(i);
		sStr = buyerItem.m_pBuyerName;
		//  Put the appropriate info in the right hand column, don't want it
		// if it's zero though.

		if (buyerItem.m_nBuyerOffer != 0) {
			Common::sprintf_s(szRightCol, "%2dgZ", buyerItem.m_nBuyerOffer);
			totalAsking += buyerItem.m_nBuyerOffer;
			AlignAtColumn(sStr, szRightCol, kRightColumnAlign);
			m_pLBox->AddToTail(sStr, FALSE);
		}
	}

	// just take care of swonza here.
	//
	// Don't even list swonza if we have not resolved his terms yet.
	if (g_stOtherPartys[SWONZA5].m_nPaymentAmount != -1) {

		sStr = g_stOtherPartys[SWONZA5].m_pszName;

		totalAsking += g_stOtherPartys[SWONZA5].m_nPaymentAmount;
		Common::sprintf_s(szRightCol, "%2dgZ", g_stOtherPartys[SWONZA5].m_nPaymentAmount);

		AlignAtColumn(sStr, szRightCol, kRightColumnAlign);
		m_pLBox->AddToTail(sStr, FALSE);
	}

	// update the asking with the bribes
	sStr = "TOTAL OFFER PRICE  ";

	Common::sprintf_s(szRightCol, "%2dgZ", totalAsking);
	AlignAtColumn(sStr, szRightCol, kRightColumnAlign);

	m_pLBox->AddToTail(sStr, FALSE);

	// show list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	//  Show the return to main button
	HideAllButtons();
#endif


	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[SUBMIT_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}

void SrafComputer::ActivateBuyerBids() {
	ERROR_CODE      error = ERR_NONE;
	char            szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString      sStr(szLocalBuff, 256);

	//char          szRightCol[256];

	// current screen is now the BIDS screen.
	m_eCurScreen = SCBIDS;

	// Initialize point size and item height
	m_nListPointSize = kBuyerBidsPointSize;
	m_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"

	DeactivateMainScreen();

	// load up the main list now with our bid data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	// Make sure we start with the correct computer background

	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// Put up the column header
	sStr = kBuyerBidsHeaderStr;
	m_pLBox->AddToTail(sStr, FALSE);

	// Walk through the grid and build individual strings and add to the list.

	for (int i = 0; i < NUM_BUYERS; i++) {
		sStr = BuildBidString(i);
		m_pLBox->AddToTail(sStr, FALSE);
		if (g_stBuyerBids[i].m_bAccept) {
			m_pLBox->SetTextLineColor(i + 1, m_cTextLineColor);
		}
	}

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kBuyerBidsMessage1;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kBuyerBidsMessage2;
	m_pLBox->AddToTail(sStr, FALSE);

	// show the list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	// Show our return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[DISPLAY_KEY_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}

CBofString &SrafComputer::BuildBidString(int index) {
	char        szRightCol[256];
	gBidStr = g_stBuyerBids[index].m_pszName;

	for (int j = 0; j < NUM_MINERALS; j++) {
		// don't display zeroes.
		if (g_stBuyerBids[index].m_nMineralVal[j] == 0) {
			Common::strcpy_s(szRightCol, "  ");
		} else {
			Common::sprintf_s(szRightCol, "%2d", g_stBuyerBids[index].m_nMineralVal[j]);
		}
		AlignAtColumn(gBidStr, szRightCol, kFirstMineralColumn + j * kMineralColWidth);
	}

	Common::strcpy_s(szRightCol, (g_stBuyerBids[index].m_bAccept ? "[*]" : "[ ]"));

	AlignAtColumn(gBidStr, szRightCol, kFirstMineralColumn + NUM_MINERALS * kMineralColWidth + 2);

	return gBidStr;
}

void SrafComputer::ActivateDealBackground() {
	char                    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString              sStr(szLocalBuff, 256);

	m_eCurScreen = SCBACKGROUNDDATA;

	sStr = BuildSrafDir("SRAFDEAL.TXT");
	DisplayTextScreen(sStr);
}


#if BOF_WINDOWS
void SrafComputer::HideAllButtons() {
	Assert(IsValidObject(this));

	int i;

	for (i = 0; i < NUM_SRAFCOMPBUTT; i++) {

		if (m_pButtons[i] != nullptr) {
			m_pButtons[i]->Hide();
		}
	}
}
#endif


void SrafComputer::ActivateSellerBios() {

	ERROR_CODE error = ERR_NONE;

	char                    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString              sStr(szLocalBuff, 256);
	//char                  szRightCol[128];

	// current screen is now the SELLER BIOS screen.
	m_eCurScreen = SCSELLER_BIOS;

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = 20;

	// Get rid of the main screen, who needs it anyway!!!
	DeactivateMainScreen();

	// Make sure we start with the correct computer background
	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// load up the main list now with our deal summary data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	//  Populate first line of list with header

	sStr = "SELLER BIOGRAPHIES";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// populate the listbox with the staff names

	for (int i = 0; i < NUM_SELLERS; i++) {
		sStr = g_stSellerNames[i].m_pszName;
		m_pLBox->AddToTail(sStr, FALSE);
	}

	// show list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	//  Show the return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}


void SrafComputer::ActivateOtherBios() {

	ERROR_CODE error = ERR_NONE;

	char                    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString              sStr(szLocalBuff, 256);
	//char                  szRightCol[128];

	// current screen is now the OTHER BIOS screen.
	m_eCurScreen = SCOTHER_BIOS;

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = 20;

	// Get rid of the main screen, who needs it anyway!!!
	DeactivateMainScreen();

	// Make sure we start with the correct computer background
	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// load up the main list now with our deal summary data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	//  Populate first line of list with header

	sStr = "OTHER BIOGRAPHIES";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// populate the listbox with the other names

	for (int i = 0; i < NUM_OTHER_PARTYS; i++) {
		sStr = g_stOtherPartys[i].m_pszName;
		m_pLBox->AddToTail(sStr, FALSE);
	}

	// show list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	//  Show the return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}

void SrafComputer::ActivateStaffBios() {

	ERROR_CODE error = ERR_NONE;

	char                    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString              sStr(szLocalBuff, 256);
	//char                  szRightCol[128];

	// current screen is now the STAFF BIOS screen.
	m_eCurScreen = SCSTAFF_BIOS;

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = 20;

	// Get rid of the main screen, who needs it anyway!!!
	DeactivateMainScreen();

	// Make sure we start with the correct computer background
	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// load up the main list now with our deal summary data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	//  Populate first line of list with header

	sStr = "STAFF BIOGRAPHIES";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// populate the listbox with the staff names

	for (int i = 0; i < NUM_STAFFERS; i++) {
		sStr = g_staffers[i].m_pszStafferName;
		m_pLBox->AddToTail(sStr, FALSE);
	}

	// show list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	//  Show the return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}

// This one's gonna be kind of ugly because we need to align several of the
// columns to reflect what is expanded and what is not.

void SrafComputer::ActivateDispatchTeam() {
	ERROR_CODE      error = ERR_NONE;

	//char          szLocalBuff[256];
	//szLocalBuff[0] = '\0';
	//CBofString        sStr(szLocalBuff, 256);
	//char          szRightCol[256];

	// current screen is now the DISPATCH screen.
	m_eCurScreen = SCDISPATCH;

	// Initialize point size and item height
	m_nListPointSize = kBuyerBidsPointSize;
	m_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.
	DeactivateMainScreen();

	// load up the main list now with our bid data.
	error = CreateListBox();
	Assert(error == ERR_NONE);

	// Make sure we start with the correct computer background

	if (GetBackdrop()) {
		PaintBackdrop();
	}

	RecalcDispatchList(gDispatchCurState);

	// show the list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	// Show our return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[DISPATCH_TEAM_BUTTON]->Show();

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Paint();
	m_pButtons[DISPATCH_TEAM_BUTTON]->Paint();

	m_pButtons[QUIT_BUTTON]->Hide();
}


void SrafComputer::RecalcDispatchList(int mExpansionFlag) {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);
	char szRightCol[256];
	bool bDone = FALSE;
	int nListEntries = 0;
	const char *pMeetWith[3] = { "A Seller", "A Buyer", "Other Interested Party" };
	int nMeetWithEntries = 3;
	bool bAddToTail = TRUE;
	int nSkipped = 0;

	// Put up the column header and a blank line
	sStr = kDispatchHeaderStr;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// keep track of the currently expanded state
	gDispatchCurState = mExpansionFlag;

	// Figure out how big our list will be
	switch (mExpansionFlag) {
	case mSellersExpanded:
	case mOthersExpanded:
	case 0:
		nListEntries = NUM_STAFFERS;
		break;

	case mBuyersExpanded:
		nListEntries = NUM_BUYERS - MINMARTENTRIES + 3;
		break;
	}

	// Walk through the grid and build individual strings and add to the list.
	int     i = 0;
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
					// only include those that are not being met with already.
					if (g_stSellerNames[i - 1].m_bAvailable == TRUE) {
						sStr += "[ ] ";
						sStr += g_stSellerNames[i - 1].m_pszName;
						if (g_stSellerNames[i - 1].m_bMeetWith) {
							sStr.ReplaceCharAt(kStandardIndentation + 1, kCheckmark);
						}
					} else {
						bAddToTail = FALSE;
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

			case NUM_BUYERS - MINMARTENTRIES + 2:
				sStr += pMeetWith[2];
				break;

			default:
				// Don't display the min mart entries, there's really no reason to
				// show them.
				// щnly include those that are not being met with already.
				if (g_stBuyerBids[i - 2].m_bAvailable == TRUE) {
					if (i <= (NUM_BUYERS - MINMARTENTRIES + 1)) {
						sStr += "[ ] ";
						sStr += g_stBuyerBids[i - 2].m_pszName;
						if (g_stBuyerBids[i - 2].m_bMeetWith) {
							sStr.ReplaceCharAt(kStandardIndentation + 1, kCheckmark);
						}
					}
				} else {
					bAddToTail = FALSE;
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
					if (g_stOtherPartys[i - 3].m_bAvailable == TRUE) {
						sStr += "[ ] ";
						sStr += g_stOtherPartys[i - 3].m_pszName;
						if (g_stOtherPartys[i - 3].m_bMeetWith) {
							sStr.ReplaceCharAt(kStandardIndentation + 1, kCheckmark);
						}
					} else {
						bAddToTail = FALSE;
					}
				}
			}
			break;
		}

		// Add the right hand column
		if (bAddToTail == TRUE) {
			if ((i - nSkipped) < NUM_STAFFERS) {
				Common::strcpy_s(szRightCol, g_staffers[i - nSkipped].m_pszStafferName);
				AlignAtColumn(sStr, szRightCol, kTeamMemberColumn);

				Common::strcpy_s(szRightCol, (g_staffers[i - nSkipped].m_bAvailable ? "YES" : "NO "));
				AlignAtColumn(sStr, szRightCol, kTeamAvailableColumn);

				Common::strcpy_s(szRightCol, (g_staffers[i - nSkipped].m_bOnCurrentTeam ? "[*]" : "[ ]"));

				AlignAtColumn(sStr, szRightCol, kTeamIncludeColumn - 1);
			}
			m_pLBox->AddToTail(sStr, FALSE);
		} else {
			nSkipped++;
		}

		bAddToTail = TRUE;

		// find out if we're done
		i++;
		if ((i - nSkipped) >= nListEntries) {
			bDone = TRUE;
		}
	}

	// Include our footer in this list

	sStr = "";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kDispatchFooterStr1;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kDispatchFooterStr2;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kDispatchFooterStr3;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kDispatchFooterStr4;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kDispatchFooterStr5;
	m_pLBox->AddToTail(sStr, FALSE);
}


void SrafComputer::ActivateCurrentEMail() {
	ERROR_CODE      error = ERR_NONE;
	char            szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString      sStr(szLocalBuff, 256);

	char            szRightCol[256];

	// current screen is now the EMAIL screen.
	m_eCurScreen = SCEMAIL;

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"

	DeactivateMainScreen();

	// load up the main list now with our bid data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	// Make sure we start with the correct computer background

	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kEmailHeaderStr;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// Walk through the grid and build individual strings and add to the list.

	for (int i = 0; i < NUM_MAIL_MESSAGES; i++) {
		sStr = g_stEmailMessages[i].m_pszMessageSubj;

		Common::strcpy_s(szRightCol, g_stEmailMessages[i].m_pszMessageFrom);
		AlignAtColumn(sStr, szRightCol, kMessageFromColumn);

		Common::strcpy_s(szRightCol, g_stEmailMessages[i].m_pszMessageTo);
		AlignAtColumn(sStr, szRightCol, kMessageToColumn);

		m_pLBox->AddToTail(sStr, FALSE);
	}

	// show the list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	// Show our return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}


void SrafComputer::ActivateAudioSettings() {
	ERROR_CODE      error = ERR_NONE;
	char            szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString      sStr(szLocalBuff, 256);

	char            szRightCol[256];

	// current screen is now the AUDIO screen.
	m_eCurScreen = SCAUDIO;

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"

	DeactivateMainScreen();

	// load up the main list now with our bid data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	// Make sure we start with the correct computer background

	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kAudioHeaderStr;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// Walk through the titles and build the list

	for (int i = 0; i < NUM_MUSICAL_SCORES; i++) {
		sStr = g_stAudioSetting[i].m_pszTitle;

		Common::strcpy_s(szRightCol, g_stAudioSetting[i].m_pszAuthor);
		AlignAtColumn(sStr, szRightCol, kAudioAuthorCol);

		Common::strcpy_s(szRightCol, g_stAudioSetting[i].m_pszPlanet);
		AlignAtColumn(sStr, szRightCol, kAudioPlanetCol);

		m_pLBox->AddToTail(sStr, FALSE);
	}

	// right now audio volume control is a mac specific feature
#if BOF_MAC
	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = "TO ADJUST VOLUME, DEPRESS COMMAND-ARROW-UP OR COMMAND-ARROW-DOWN";
	m_pLBox->AddToTail(sStr, FALSE);
#endif

	// show the list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	// Show our return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();

	// only show the no music button if music is playing.
	bool    bAnythingPlaying = CBofSound::MidiSoundPlaying();
	if (bAnythingPlaying) {
		m_pButtons[NO_MUSIC_BUTTON]->Show();
	}
	m_pButtons[QUIT_BUTTON]->Hide();
}


void SrafComputer::ActivateRoboButler() {
	ERROR_CODE      error = ERR_NONE;
	char            szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString      sStr(szLocalBuff, 256);

	char            szRightCol[256];

	// current screen is now the robobutler screen.
	m_eCurScreen = SCORDER;

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"

	DeactivateMainScreen();

	// load up the main list now with our bid data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	// Make sure we start with the correct computer background
	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kRobobutlerHeaderStr;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// Walk through the titles and build the list

	for (int i = 0; i < NUM_OFFERINGS; i++) {
		sStr = g_stOfferings[i].m_pszType;

		Common::strcpy_s(szRightCol, g_stOfferings[i].m_pszOffering);
		AlignAtColumn(sStr, szRightCol, kRobobutlerCol);

		m_pLBox->AddToTail(sStr, FALSE);
	}

	// show the list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	// Show our return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	DoShowChowButtons();
	m_pButtons[QUIT_BUTTON]->Hide();
}

void SrafComputer::DoShowChowButtons() {
	// Only list this stuff if it has not already been ordered.
	bool    bAllowBeverage = TRUE;
	bool    bAllowSnack = TRUE;

	CBagStorageDev *pSDev = SDEVMNGR->GetStorageDevice("INV_WLD");
	if (pSDev) {
		CBagObject *pBevObj = pSDev->GetObject("SZTB", TRUE);
		CBagObject *pSnackObj = pSDev->GetObject("SZTA", TRUE);

		if (pSnackObj) {
			bAllowSnack = FALSE;
		}

		if (pBevObj) {
			bAllowBeverage = FALSE;
		}
	}

	if (bAllowBeverage) {
		m_pButtons[ORDER_BEVERAGE_BUTTON]->Show();
	} else {
		m_pButtons[ORDER_BEVERAGE_BUTTON]->Hide();
	}

	if (bAllowSnack) {
		m_pButtons[ORDER_SNACK_BUTTON]->Show();
	} else {
		m_pButtons[ORDER_SNACK_BUTTON]->Hide();
	}

	// added test for PALETTESHIFTFIX
#if BOF_MAC && PALETTESHIFTFIX
	CBofWindow::CheckPaletteShiftList();
#endif
}

void SrafComputer::ActivateCheckTeams() {
	ERROR_CODE      error = ERR_NONE;
	char            szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString      sStr(szLocalBuff, 256);
	int             numItems;
	char            szAttendeesBuff[256];
	szAttendeesBuff[0] = '\0';
	CBofString      aStr(szAttendeesBuff, 256);
	char            szRightCol[256];

	// current screen is now the Check teams screen.
	m_eCurScreen = SCCHECK_TEAMS;

	// Initialize point size and item height
	//
	// set this stuff to a tiny point size so that it won't flow over the
	// right hand side of the screen.
	m_nListPointSize = kBuyerBidsPointSize;
	m_nListItemHeight = kLineItemHeight;

	//  This screen, like most of the subscreens, will be implemented
	//  as a text box.  The only event that we care about on this screen
	//  is a mouse down on our "Make current offer to sellers"

	DeactivateMainScreen();

	// load up the main list now with our bid data.

	error = CreateListBox();
	Assert(error == ERR_NONE);

	// Make sure we start with the correct computer background

	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// Put up the column header and a blank line
	sStr = kCheckTeamHeaderStr1;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kCheckTeamHeaderStr2;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	numItems = (m_pTeamList == nullptr ? 0 : m_pTeamList->GetCount());

	// Walk through the list and build individual strings and add to the list.

	int  nNumAttendees = 0;
	int  nMaxNumAttendees = 0;
	int i;

	for (i = 0; i < numItems; i++) {
		DispatchedTeamItem      teamListItem;

		teamListItem = m_pTeamList->GetNodeItem(i);

		// Figure out who they are meeting with...

		if (teamListItem.m_nFlags & mOtherParty) {
			sStr = g_stOtherPartys[teamListItem.m_nMeetWithID].m_pszName;
		} else {
			if (teamListItem.m_nFlags & mBuyer) {
				sStr = g_stBuyerBids[teamListItem.m_nMeetWithID].m_pszName;
			} else {
				sStr = g_stSellerNames[teamListItem.m_nMeetWithID].m_pszName;
			}
		}

		// List the dispatch time
		Common::sprintf_s(szRightCol,
		                  "%02d:%02d",
		                  teamListItem.m_nDispatchTime / 100,
		                  teamListItem.m_nDispatchTime % 100);
		AlignAtColumn(sStr, szRightCol, kCheckTeamTimeCol);

		// Now build the list of attendees.

		bool bFirstAttendee = TRUE;
		int mFlag = 0;
		int j;

		// count number of attendees first, this will help us format our list
		for (j = 0; j < NUM_STAFFERS; j++) {
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

			if (teamListItem.m_nFlags & mFlag) {
				nNumAttendees++;
			}
		}

		for (j = 0; j < NUM_STAFFERS; j++) {
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

			if (teamListItem.m_nFlags & mFlag) {
				if (bFirstAttendee) {
					aStr = g_staffers[j].m_pszStafferName;
					bFirstAttendee = FALSE;
				} else {
					aStr += ", ";
					aStr += g_staffers[j].m_pszStafferName;
				}
			}
		}

		// save high water mark.
		if (nNumAttendees > nMaxNumAttendees) {
			nMaxNumAttendees = nNumAttendees;
		}
		nNumAttendees = 0;

		AlignAtColumn(sStr, szAttendeesBuff, kCheckTeamStaffCol);
		m_pLBox->AddToTail(sStr, FALSE);
	}

	// decrease point size if we got too much for one screen
	if (nMaxNumAttendees > 4) {
		m_nListPointSize = kOtherPointSize;
		m_nListItemHeight = kLineItemHeight;
		m_pLBox->SetPointSize(m_nListPointSize);
		m_pLBox->SetItemHeight(m_nListItemHeight);
	}

	// Display the current time...

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// add a dummy line so we can overwrite it with the
	// current turncount.
	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	int nLineNo = m_pLBox->GetNumItems();
	DisplayTurnCount(nLineNo - 1);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

#define kCheckSuccessfulNegotiations "SUCCESSFUL NEGOTIATIONS: "
#define kCheckUnsuccessfulNegotiations "UNSUCCESSFUL NEGOTIATIONS: "
	// add a few lines indicating who has succeeded and who has failed.
	char        szGoodMeetings[256];
	char        szBadMeetings[256];
	CBofString  sGoodMeetings(szGoodMeetings, 256);
	CBofString  sBadMeetings(szBadMeetings, 256);
	bool        bSFirstTime = TRUE;
	bool        bUFirstTime = TRUE;

	sGoodMeetings = "";
	sBadMeetings = "";

	for (i = 0; i < NUM_SELLERS; i++) {
		if (g_stSellerNames[i].m_nMeetingResult != NOMEETING) {
			if (g_stSellerNames[i].m_nMeetingResult == GOODMEETING) {
				if (bSFirstTime == TRUE) {
					bSFirstTime = FALSE;
				} else {
					sGoodMeetings += ", ";
				}
				sGoodMeetings += g_stSellerNames[i].m_pszName;
			} else {
				if (bUFirstTime == TRUE) {
					bUFirstTime = FALSE;
				} else {
					sBadMeetings += ", ";
				}
				sBadMeetings += g_stSellerNames[i].m_pszName;
			}
		}
	}

	// scan the buyers for meetings
	for (i = 0; i < NUM_BUYERS; i++) {
		if (g_stBuyerBids[i].m_nMeetingResult != NOMEETING) {
			if (g_stBuyerBids[i].m_nMeetingResult == GOODMEETING) {
				if (bSFirstTime == TRUE) {
					bSFirstTime = FALSE;
				} else {
					sGoodMeetings += ", ";
				}
				sGoodMeetings += g_stBuyerBids[i].m_pszName;
			} else {
				if (bUFirstTime == TRUE) {
					bUFirstTime = FALSE;
				} else {
					sBadMeetings += ", ";
				}
				sBadMeetings += g_stBuyerBids[i].m_pszName;
			}
		}
	}

	// scan the other partys for meetings
	for (i = 0; i < NUM_OTHER_PARTYS; i++) {
		if (g_stOtherPartys[i].m_nMeetingResult != NOMEETING) {
			if (g_stOtherPartys[i].m_nMeetingResult == GOODMEETING) {
				if (bSFirstTime == TRUE) {
					bSFirstTime = FALSE;
				} else {
					sGoodMeetings += ", ";
				}
				sGoodMeetings += g_stOtherPartys[i].m_pszName;
			} else {
				if (bUFirstTime == TRUE) {
					bUFirstTime = FALSE;
				} else {
					sBadMeetings += ", ";
				}
				sBadMeetings += g_stOtherPartys[i].m_pszName;
			}
		}
	}

	// Display all this info we just calculated...
	sStr = kCheckSuccessfulNegotiations;
	m_pLBox->AddToTail(sStr, FALSE);

	m_pLBox->AddToTail(sGoodMeetings, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kCheckUnsuccessfulNegotiations;
	m_pLBox->AddToTail(sStr, FALSE);

	m_pLBox->AddToTail(sBadMeetings, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kCheckDispatchTeamFooter1;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kCheckDispatchTeamFooter2;
	m_pLBox->AddToTail(sStr, FALSE);

	// show the list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	// Show our return to main button
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}


void SrafComputer::ActivateCodeWords() {
	ERROR_CODE error = ERR_NONE;
	char                    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString              sStr(szLocalBuff, 256);
	char                    szRightCol[128];

	// current screen is now the CODE WORDS screen.
	m_eCurScreen = SCCODE_WORDS;

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = kLineItemHeight;

	// Get rid of the main screen, who needs it anyway!!!
	DeactivateMainScreen();

	// Make sure we start with the correct computer background
	if (GetBackdrop()) {
		PaintBackdrop();
	}

	// load up the main list now with our deal summary data.
	error = CreateListBox();
	Assert(error == ERR_NONE);

	//  Populate first line of list with header
	sStr = kCodeWordsHeader1;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kCodeWordsHeader2;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kCodeWordsHeader3;
	m_pLBox->AddToTail(sStr, FALSE);

	// populate the listbox with the staff names
	for (int i = 0; i < NUM_CODE_WORDS; i++) {
		sStr = g_stCodeWords[i].m_pszCodeWord1;

		Common::strcpy_s(szRightCol, g_stCodeWords[i].m_pszCodeWord2);
		AlignAtColumn(sStr, szRightCol, kGroup1Col2);

		Common::strcpy_s(szRightCol, g_stCodeWords[i].m_pszCodeWord3);
		AlignAtColumn(sStr, szRightCol, kGroup2Col1);

		Common::strcpy_s(szRightCol, g_stCodeWords[i].m_pszCodeWord4);
		AlignAtColumn(sStr, szRightCol, kGroup2Col2);

		m_pLBox->AddToTail(sStr, FALSE);
	}

	// blank line and add to end

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = "CODE WORD PAIR: (,)";
	m_pLBox->AddToTail(sStr, FALSE);

	// show list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->RepaintAll();

	//  Show the return to main button
	HideAllButtons();
#endif

	// use the done button instead of return to main.
	m_pButtons[DONE_BUTTON]->Show();
	m_pButtons[QUIT_BUTTON]->Hide();
}


void SrafComputer::InitDealSummary() {
	// Initialize the initial state of the deal summary

	RecalcSellerSummaryList();
	RecalcBuyerSummaryList();
}

//	Allocate everything we need here...
void SrafComputer::InitBuyerBids() {
}

void SrafComputer::InitDealBackground() {
}

void SrafComputer::InitSellerBios() {
}

void SrafComputer::InitOtherBios() {
}

void SrafComputer::InitStaffBios() {
}


// Search the specified list and return the index of a TRUE member, if there
// is one, return the element in the list that has "meet with" set to TRUE.
int SrafComputer::GetMeetMember(int nListToSearch) {
	int         i;
	switch (nListToSearch) {
	case 0:
		break;

	case kOthersList:
		for (i = 0; i < NUM_OTHER_PARTYS; i++) {
			if (g_stOtherPartys[i].m_bMeetWith)
				return i;
		}
		break;

	case kSellersList:
		for (i = 0; i < NUM_SELLERS; i++) {
			if (g_stSellerNames[i].m_bMeetWith)
				return i;
		}
		break;


	case kBuyersList:
		for (i = 0; i < NUM_BUYERS; i++) {
			if (g_stBuyerBids[i].m_bMeetWith)
				return i;
		}
		break;


	case kStaffersList:
		for (i = 0; i < NUM_STAFFERS; i++) {
			if (g_staffers[i].m_bOnCurrentTeam)
				return i;
		}
		break;
	}
	return -1;
}

// Search the specified list and return the index of a TRUE member, if there
// is one, return the element in the list that has "meet with" set to TRUE.
int SrafComputer::GetAdjustedIndex(int nListToSearch, int nElementIndex, bool bScreenToIndex) {
	int         i = 0;
	int         nTotalNotAvail = 0;
	int         nOrigElementIndex = nElementIndex;
//	int         nUpperBound = 0;

	switch (nListToSearch) {
	case 0:
		break;

	case kOthersList:
		// Adjust for out of range.
		nElementIndex = (nElementIndex >= NUM_OTHER_PARTYS ? (NUM_OTHER_PARTYS - 1) : nElementIndex);

		for (i = 0; i <= NUM_OTHER_PARTYS; i++) {
			// get all consecutive that are not available.
			while (g_stOtherPartys[i].m_bAvailable == FALSE && i <= NUM_OTHER_PARTYS) {
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

		for (i = 0; i <= NUM_BUYERS; i++) {
			// get all consecutive that are not available.
			while (g_stSellerNames[i].m_bAvailable == FALSE && i <= NUM_SELLERS) {
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

		for (i = 0; i <= NUM_BUYERS; i++) {
			// get all consecutive that are not available.
			while (g_stBuyerBids[i].m_bAvailable == FALSE && i <= NUM_BUYERS) {
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

		for (i = 0; i <= NUM_BUYERS; i++) {
			// get all consecutive that are not available.
			while (g_staffers[i].m_bAvailable == FALSE && i <= NUM_STAFFERS) {
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
	} else {
		return nOrigElementIndex - nTotalNotAvail;
	}
}

void SrafComputer::InitDispatchTeam() {
	// Go through the entire list and make sure that they are all not on the
	// currrent team.
	int i;

	for (i = 0; i < NUM_STAFFERS; i++) {
		g_staffers[i].m_bOnCurrentTeam = FALSE;
	}

	for (i = 0; i < NUM_SELLERS; i++) {
		g_stSellerNames[i].m_bMeetWith = FALSE;
	}

	for (i = 0; i < NUM_BUYERS; i++) {
		g_stBuyerBids[i].m_bMeetWith = FALSE;
	}

	for (i = 0; i < NUM_OTHER_PARTYS; i++) {
		g_stOtherPartys[i].m_bMeetWith = FALSE;
	}

}

void SrafComputer::InitCurrentEMail() {
}

void SrafComputer::InitAudioSettings() {
}

void SrafComputer::InitRoboButler() {
}

void SrafComputer::OnListMainScreen() {

	switch (m_nSelection) {
	case DEALSUMMARY:
		ActivateDealSummary();
		break;

	case BUYERBIDS:
		ActivateBuyerBids();
		break;

	case DEALBACKGROUND:
		ActivateDealBackground();
		break;

	case SELLERBIOS:
		ActivateSellerBios();
		break;

	case OTHERBIOS:
		ActivateOtherBios();
		break;

	case STAFFBIOS:
		ActivateStaffBios();
		break;

	case DISPATCHTEAM:
		InitDispatchTeam();
		ActivateDispatchTeam();
		break;

	case CURRENTEMAIL:
		ActivateCurrentEMail();
		break;

	case AUDIOSETTINGS:
		ActivateAudioSettings();
		break;

	case ROBOBUTLER:
		ActivateRoboButler();
		break;

	case CHECKTEAMS:
		ActivateCheckTeams();
		break;

	default:
		break;
	}
	m_nSelection = -1;
}

void SrafComputer::OnListDealSummary() {
}

void SrafComputer::OnListBuyerBids() {
	CBofPoint   cPoint = GetPrevMouseDown();        // already in local coords
	int         index = m_nSelection - 1;

	// I can't believe this isn't in there... make sure that this is not out
	// of range.

	if (index >= NUM_BUYERS) {
		return;
	}

#if USETEXTWIDTHS
	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	// calculate the text width based on the attributes of the text
	// rather than guessing to where they are.
	sStr = " ACCEPT ";
	CBofRect        cAcceptBidRect = CalculateTextRect(this, &sStr, kBuyerBidsPointSize, FONT_MONO);

	AlignAtColumn(sStr, "", kFirstMineralColumn + NUM_MINERALS * kMineralColWidth);
	CBofRect        cDummyRect = CalculateTextRect(this, &sStr, kBuyerBidsPointSize, FONT_MONO);

	CBofPoint       cStartPoint(cDummyRect.right, 0);
	cAcceptBidRect.OffsetRect(cStartPoint);
	cAcceptBidRect.bottom = 1000;
#else
	CBofRect    cAcceptBidRect(kFirstMineralColumn * kBuyerBidsPointWidth + NUM_MINERALS * kBuyerBidsPointWidth * kMineralColWidth,
	                           kLineItemHeight,
	                           gCompDisplay.right,
	                           (kLineItemHeight + 1) * NUM_BUYERS);
#endif

	//  Based on the location of the mouse determine if we should check/uncheck
	//  the "accept bid" field or present a bio on he who was checked.
	if (cAcceptBidRect.PtInRect(cPoint)) {

		// negate the current value
		g_stBuyerBids[index].m_bAccept = !g_stBuyerBids[index].m_bAccept;

		//  Redraw with the new one checked/unchecked
		CBofString cStr = m_pLBox->GetText(m_nSelection);
		cStr = BuildBidString(index);
		m_pLBox->SetText(m_nSelection, cStr);
		if (g_stBuyerBids[index].m_bAccept) {
			m_pLBox->SetTextLineColor(m_nSelection, m_cTextLineColor);
		} else {
			m_pLBox->SetTextLineColor(m_nSelection, (RGBCOLOR) - 1); // Restore default
		}

		m_pLBox->RepaintItem(m_nSelection);

	} else {
		szLocalBuff[0] = '\0';
		CBofString sStr2(szLocalBuff, 256);

		if (index >= 0 && index < NUM_BUYERS) {
			sStr2 = BuildSrafDir(g_stBuyerBids[index].m_pszBuyerBio);
			DisplayTextScreen(sStr2);
		}
	}
}

void SrafComputer::OnListDealBackground() {
}

void SrafComputer::OnListSellerBios() {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (m_nSelection >= 2) {
		sStr = BuildSrafDir(g_stSellerNames[m_nSelection - 2].m_pszSellerBio);
		DisplayTextScreen(sStr);
	}
}

void SrafComputer::OnListOtherBios() {
	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (m_nSelection >= 2) {
		sStr = BuildSrafDir(g_stOtherPartys[m_nSelection - 2].m_pszOtherPartyBio);
		DisplayTextScreen(sStr);
	}
}


void SrafComputer::OnListStaffBios() {
	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (m_nSelection >= 2) {
		sStr = BuildSrafDir(g_staffers[m_nSelection - 2].m_pszStafferBio);
		DisplayTextScreen(sStr);
	}
}


// Please don't look too closely at this code.  It works, for the most part
// at least, but it's really gross.

void SrafComputer::OnListDispatchTeam() {
	CBofPoint   cPoint = GetPrevMouseDown();    // already in local coords
	CBofRect    cMeetWithRect(0,
	                          0,
	                          (kTeamMemberColumn - kStandardIndentation) * kBuyerBidsPointWidth,
	                          gCompDisplay.Height());
	CBofRect    cTeamMembersRect((kTeamMemberColumn - kStandardIndentation) * kBuyerBidsPointWidth,
	                             0,
	                             gCompDisplay.Width(),
	                             gCompDisplay.Height());
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);
	int nElementIndex;
	int nPreceedingHeaders = 0;
	int nListToCheck = 0;

	//  First two entries in list mean nothing to us...
	if (m_nSelection < 2)
		return;

	// Check if we're in the left hand column, if we are, then determine
	// if we're left of the list or in the list itself.
	if (cMeetWithRect.PtInRect(cPoint)) {
		nElementIndex = m_nSelection - 3;   // three header lines before data starts

		bool        bDeleteAll = FALSE;
		int         nRecalcVal = 0;
		CBofRect    cMeetBio(cMeetWithRect.left + kStandardDoubleIndentation * kBuyerBidsPointWidth,
		                     cMeetWithRect.top, cMeetWithRect.right, cMeetWithRect.bottom);
		CBofRect    cMeetMember(0, 0, cMeetWithRect.left + kStandardDoubleIndentation * kBuyerBidsPointWidth, cMeetWithRect.bottom);
		int         nMeetMember = -1;
		bool        bInMeetMemberColumn = FALSE;

		switch (gDispatchCurState) {
		case 0:
			bDeleteAll = TRUE;
			nRecalcVal = (nElementIndex == -1 ? mSellersExpanded :
			              (nElementIndex == 0 ? mBuyersExpanded :
			               (nElementIndex == 1 ? mOthersExpanded : 0)));
			break;
		case mOthersExpanded:
			nListToCheck = kOthersList;             // List to check
			nPreceedingHeaders = 2;                 // buyers and sellers come before this guy...

			switch (nElementIndex) {

			case -1:
				bDeleteAll = TRUE;
				nRecalcVal = mSellersExpanded;      // collapse list, expand sellers
				break;
			case 0:
				bDeleteAll = TRUE;
				nRecalcVal = mBuyersExpanded;       // collapse list, expand buyers
				break;
			case 1:
				bDeleteAll = TRUE;
				nRecalcVal = 0;                     // collapse list
				break;
			default:
				nElementIndex -= nPreceedingHeaders;    // account for headers
				// account for those guys out on meetings that we have not displayed
				// recalc first...
				nElementIndex = GetAdjustedIndex(nListToCheck, nElementIndex, TRUE);

				if (nElementIndex >= NUM_OTHER_PARTYS)
					break;

				if (cMeetBio.PtInRect(cPoint)) {        // if so, bring up biography.
					sStr = BuildSrafDir(g_stOtherPartys[nElementIndex].m_pszOtherPartyBio);
					DisplayTextScreen(sStr);
				} else {
					if (cMeetMember.PtInRect(cPoint)) {         // if so, put a checkmark in that column.

						// uncheck any member we already have checked, this is a singular operation
						if ((nMeetMember = GetMeetMember(nListToCheck)) != -1) {
							g_stOtherPartys[nMeetMember].m_bMeetWith = FALSE;
						}

						// Now put the check mark in the column for the new guy to meet
						if (nMeetMember != nElementIndex) {
							g_stOtherPartys[nElementIndex].m_bMeetWith = TRUE;
							bInMeetMemberColumn = TRUE;
						}
					}
				}
				break;
			}
			break;

		case mSellersExpanded:
			nListToCheck = kSellersList;            // List to check
			nPreceedingHeaders = 0;                 // no headers above this guy

			switch (nElementIndex) {
			case -1:
				bDeleteAll = TRUE;
				nRecalcVal = 0;         // collapse list
				break;

			default:
				nElementIndex -= nPreceedingHeaders;    // account for headers

				// account for those guys out on meetings that we have not displayed
				// recalc first...
				nElementIndex = GetAdjustedIndex(nListToCheck, nElementIndex, TRUE);

				if (nElementIndex >= NUM_SELLERS) {
					if (nElementIndex == (NUM_SELLERS - nPreceedingHeaders)) {
						bDeleteAll = TRUE;
						nRecalcVal = mBuyersExpanded;       // collapse list, expand buyers
					}

					if (nElementIndex == (NUM_SELLERS + 1 - nPreceedingHeaders)) {
						bDeleteAll = TRUE;
						nRecalcVal = mOthersExpanded;       // collapse list, expand others
					}
					break;
				}

				if (cMeetBio.PtInRect(cPoint)) {        // if so, bring up biography.
					sStr = BuildSrafDir(g_stSellerNames[nElementIndex].m_pszSellerBio);
					DisplayTextScreen(sStr);
				} else {
					if (cMeetMember.PtInRect(cPoint)) {         // if so, put a checkmark in that column.

						// uncheck any member we already have checked, this is a singular operation
						if ((nMeetMember = GetMeetMember(nListToCheck)) != -1) {
							g_stSellerNames[nMeetMember].m_bMeetWith = FALSE;
						}

						// Now put the check mark in the column for the new guy to meet
						if (nMeetMember != nElementIndex) {
							g_stSellerNames[nElementIndex].m_bMeetWith = TRUE;
							bInMeetMemberColumn = TRUE;
						}
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
				bDeleteAll = TRUE;
				nRecalcVal = mSellersExpanded;      // collapse list, expand sellers
				break;

			case 0:
				bDeleteAll = TRUE;
				nRecalcVal = 0;                     // collapse list
				break;

			default:
				nElementIndex -= nPreceedingHeaders;    // correct for other headers
				nElementIndex = GetAdjustedIndex(nListToCheck, nElementIndex, TRUE);

				if (nElementIndex >= (NUM_BUYERS - MINMARTENTRIES)) {
					if (nElementIndex == (NUM_BUYERS - MINMARTENTRIES + 1 - nPreceedingHeaders)) {
						bDeleteAll = TRUE;
						nRecalcVal = mOthersExpanded;       // collapse list, expand others
					}
					break;
				}

				// account for those guys out on meetings that we have not displayed
				// nElementIndex = GetAdjustedIndex (nListToCheck, nElementIndex, TRUE);

				if (cMeetBio.PtInRect(cPoint)) {        // if so, bring up biography.
					sStr = BuildSrafDir(g_stBuyerBids[nElementIndex].m_pszBuyerBio);
					DisplayTextScreen(sStr);
				} else {
					if (cMeetMember.PtInRect(cPoint)) {         // if so, put a checkmark in that column.

						// uncheck any member we already have checked, this is a singular operation
						if ((nMeetMember = GetMeetMember(nListToCheck)) != -1) {
							g_stBuyerBids[nMeetMember].m_bMeetWith = FALSE;
						}

						// Now put the check mark in the column for the new guy to meet
						if (nMeetMember != nElementIndex) {
							g_stBuyerBids[nElementIndex].m_bMeetWith = TRUE;
							bInMeetMemberColumn = TRUE;
						}
					}
				}
				break;
			}
			break;
		}

		// If we have to uncheck a column, do that here.
		if (nMeetMember != -1) {
			int     nThisItemAt = GetAdjustedIndex(nListToCheck, nMeetMember, FALSE) + 3 + nPreceedingHeaders;
			sStr = m_pLBox->GetText(nThisItemAt);
			sStr.ReplaceCharAt(kStandardIndentation + 1, ' ');
			m_pLBox->SetText(nThisItemAt, sStr);
			m_pLBox->RepaintItem(nThisItemAt);
		}

		// If we have a new column to check, do that here.
		//
		if (bInMeetMemberColumn) {
			sStr = m_pLBox->GetText(m_nSelection);
			sStr.ReplaceCharAt(kStandardIndentation + 1, kCheckmark);       // ??? works fine on mac, not sure what check mark is for pc
			m_pLBox->SetText(m_nSelection, sStr);
			m_pLBox->RepaintItem(m_nSelection);
		}

		if (bDeleteAll) {
			m_pLBox->DeleteAll();
			RecalcDispatchList(nRecalcVal);
			m_pLBox->Show();

#if BOF_WINDOWS
			m_pLBox->RepaintAll();
#endif
		}
	}

	// Handle if we're in the right hand side of the screen.  We only care about
	// two columns, the staff member names and the guys check boxes on the right.

	if (cTeamMembersRect.PtInRect(cPoint)) {
		nElementIndex = m_nSelection - 2; // two header lines before data starts
		if (nElementIndex >= 0 && nElementIndex < NUM_STAFFERS) {
			CBofRect    cStaffNames(cTeamMembersRect.left + kStandardIndentation * kBuyerBidsPointWidth,
			                        cTeamMembersRect.top,
#if BOF_MAC
			                        cTeamMembersRect.left + (kStandardIndentation + kMaxStafferNameLen) * kBuyerBidsPointWidth,
#else
			                        cTeamMembersRect.left + (kStandardIndentation + kMaxStafferNameLen) * kBuyerBidsPointWidth + 60,
#endif
			                        cTeamMembersRect.bottom);
			CBofRect    cStaffInclude((kTeamIncludeColumn - kStandardIndentation) * kBuyerBidsPointWidth,
			                          cTeamMembersRect.top,
#if BOF_MAC
			                          (kTeamIncludeColumn + kStandardIndentation) * kBuyerBidsPointWidth,
#else
			                          cTeamMembersRect.right,
#endif
			                          cTeamMembersRect.bottom);

			//  If in the staff names column, then show the biography
			if (cStaffNames.PtInRect(cPoint)) {
				sStr = BuildSrafDir(g_staffers[nElementIndex].m_pszStafferBio);
				DisplayTextScreen(sStr);
			} else {
				// if in the "include on team" column, then handle appropriately
				if (cStaffInclude.PtInRect(cPoint)) {

					// Make sure that this dude is available

					if (g_staffers[nElementIndex].m_bAvailable) {
						char        cNewChar = ' ';
						if (g_staffers[nElementIndex].m_bOnCurrentTeam == FALSE) {
							cNewChar = kCheckmark;
						}

						// negate
						g_staffers[nElementIndex].m_bOnCurrentTeam = !g_staffers[nElementIndex].m_bOnCurrentTeam;

						sStr = m_pLBox->GetText(m_nSelection);
						sStr.ReplaceCharAt(kTeamIncludeColumn, cNewChar);
						m_pLBox->SetText(m_nSelection, sStr);
						m_pLBox->RepaintItem(m_nSelection);
					}
				}
			}
		}
	}

}

void SrafComputer::OnListCurrentEMail() {
	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	if (m_nSelection >= 2) {
		sStr = BuildSrafDir(g_stEmailMessages[m_nSelection - 2].m_pszMessageFile);
		DisplayTextScreen(sStr);
	}
}

void SrafComputer::OnListAudioSettings() {

	int     nTrackSelection = m_nSelection - 2;

	// reject out of range selections
	if (nTrackSelection < 0 || nTrackSelection >= NUM_MUSICAL_SCORES) {
		return;
	}

	// Start the new track (will stop any track playing)
	//
	// add a selection for random play.
	if (g_stAudioSetting[nTrackSelection].m_pszAudioFile == nullptr) {
		m_bRandomAudio = TRUE;
		CBagVar *pVar = VARMNGR->GetVariable("SRATURNCOUNT");
		m_nRandomTime = pVar->GetNumValue();
		nTrackSelection = g_engine->getRandomNumber() % (NUM_MUSICAL_SCORES - 1);
	} else {
		m_bRandomAudio = FALSE;
	}

	// Now start playing...
	if (g_stAudioSetting[nTrackSelection].m_pMidiTrack != nullptr) {
		g_stAudioSetting[nTrackSelection].m_pMidiTrack->Play();
	}

	// if state changes, then change the button also
	bool    bAnythingPlaying = CBofSound::MidiSoundPlaying();
	if (bAnythingPlaying) {
		m_pButtons[NO_MUSIC_BUTTON]->Show();
	} else {
		m_pButtons[NO_MUSIC_BUTTON]->Hide();
	}
}

void SrafComputer::OnListRoboButler() {
	int     nSelection = m_nSelection - 2;

	if (nSelection < 0 || nSelection > NUM_OFFERINGS) {
		return;
	}

	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString  sStr(szLocalBuff, 256);

	sStr = BuildSrafDir(g_stOfferings[nSelection].m_pszFile);
	DisplayTextScreen(sStr);
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

bool SrafComputer::ReportMeetingStatus(int nTeamNumber) {
	int                     numItems;
	DispatchedTeamItem      teamListItem;
	const char *pszFailureFile = nullptr;
	const char *pszSuccessFile = nullptr;
	bool                    bDone = FALSE;
	int                     i;
	int                     nTeamCaptain = 0;
	char                    szLocalResponse[256];
	szLocalResponse[0] = '\0';
	CBofString              sResponse(szLocalResponse, 256);
	bool                    bTimeElapsed = TRUE;
	bool                    bNeedRedraw = FALSE;

	// if the text screen is frontmost, then don't report meeting
	// status
	if (gTextScreenFrontmost == TRUE) {
		return bNeedRedraw;
	}

	// could possibly have no list established.
	if (m_pTeamList == nullptr) {
		return bNeedRedraw;
	}

	numItems = m_pTeamList->GetCount();

	// Handle out of range case first.
	if (nTeamNumber < 0 || nTeamNumber >= numItems) {
		return bNeedRedraw;
	}

	teamListItem = m_pTeamList->GetNodeItem(nTeamNumber);

	// Start by counting the number of team members.  Count up how powerful this team
	// is in terms of negotiating strength.
	int     nTeamStrength = 0;
	int     nTeamMembers = 0;
	for (i = 0; i < NUM_STAFFERS; i++) {
		if ((1 << (i + 3)) & teamListItem.m_nFlags) {
			nTeamMembers++;
			nTeamStrength += g_staffers[i].m_nNegotiatingStrength;
		}
	}

	// Get the current time...
	CBagVar *pVar = VARMNGR->GetVariable("SRATURNCOUNT");
	Assert(pVar != nullptr);
	int nCurTime = pVar->GetNumValue();

	// If less then 5 turns have elapsed, then we're still in the driving stage...
	if ((teamListItem.m_nDispatchTime + kDrivingTime) >= nCurTime) {
		bDone = TRUE;
		bTimeElapsed = FALSE;
		if (teamListItem.m_nFlags & mStafferMale) {
			if (nTeamMembers == 1) {
				pszFailureFile = kGSM21SraMaleStr;
			} else {
				pszFailureFile = kGSM22SraMaleStr;
			}
		} else {
			if (nTeamMembers == 1) {
				pszFailureFile = kGSM21SraFemStr;
			} else {
				pszFailureFile = kGSM22SraFemStr;
			}
		}
	}

	// meeting not done yet, let 'em know it.
	if (bDone == FALSE) {
		if ((teamListItem.m_nDispatchTime + teamListItem.m_nMeetingTime) > pVar->GetNumValue()) {
			bDone = TRUE;
			bTimeElapsed = FALSE;

			if (teamListItem.m_nFlags & mStafferMale) {
				pszFailureFile = kGSM19SraMaleStr;
			} else {
				pszFailureFile = kGSM19SraFemStr;
			}
		}
	}

	// If swonza has already been enlightened to the plans for the planet,
	// then reject this meeting right away.
	if (bDone == FALSE) {
		if (m_bSwonzaEnlightened &&
		        (teamListItem.m_nFlags & mOtherParty) &&
		        teamListItem.m_nMeetWithID == SWONZA5) {
			bDone = TRUE;
			if (teamListItem.m_nFlags & mStafferMale) {
				pszFailureFile = kGSM18SraMaleStr;
			} else {
				pszFailureFile = kGSM18SraFemStr;
			}
		}
	}

	// Need to have a driver, {Norg, Zig and Lentil}
	if (bDone == FALSE) {
		if (!((teamListItem.m_nFlags & mNorg72) || (teamListItem.m_nFlags & mZig64) || (teamListItem.m_nFlags & mLentil24))) {
			bDone = TRUE;
			if (teamListItem.m_nFlags & mStafferMale) {
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
	if (bDone == FALSE) {
		if ((teamListItem.m_nFlags & mNorg72) && (teamListItem.m_nFlags & mLentil24)) {
			bDone = TRUE;
			if (teamListItem.m_nFlags & mStafferMale) {
				pszFailureFile = kGSM8SraMaleStr;
			} else {
				pszFailureFile = kGSM8SraFemStr;
			}
		}
	}

	// Need to have someone who can hold their voltage {Norg, Lentil and Churg}
	if (bDone == FALSE) {
		if (!((teamListItem.m_nFlags & mNorg72) || (teamListItem.m_nFlags & mLentil24) || (teamListItem.m_nFlags & mChurg53))) {
			bDone = TRUE;
			if (teamListItem.m_nFlags & mStafferMale) {
				pszFailureFile = kGSM7SraMaleStr;
			} else {
				pszFailureFile = kGSM7SraFemStr;
			}
		}
	}

	// - Pnurth must be teamed up with Lentil, Vargas or Churg (still learning)
	if (bDone == FALSE) {
		if ((teamListItem.m_nFlags & mPnurth81) &&
		        !((teamListItem.m_nFlags & mLentil24) ||
		          (teamListItem.m_nFlags & mVargas20) ||
		          (teamListItem.m_nFlags & mChurg53))) {
			bDone = TRUE;
			if (teamListItem.m_nFlags & mStafferMale) {
				if (nTeamMembers == 1) {
					pszFailureFile = kGSM20SraMaleStr;
				} else {
					pszFailureFile = kGSM15SraMaleStr;
				}
			} else {
				if (nTeamMembers == 1) {
					pszFailureFile = kGSM20SraFemStr;
				} else {
					pszFailureFile = kGSM15SraFemStr;
				}
			}
		}
	}

	if (!bDone) {
		// special rule, if lentil is on her own, then she is twice as effective.

		if (nTeamMembers == 1 && (teamListItem.m_nFlags & mLentil24)) {
			nTeamStrength *= 2;
		}

		if (teamListItem.m_nFlags & mOtherParty) {
			switch (teamListItem.m_nMeetWithID) {
			case SWONZA5:
				// swonza-5 is a sucker, but norg is not trustworthy, so if he tells, then
				// the flashback is over, well, not really, but it can't be won without
				// swonza-5 on board.
				if (teamListItem.m_nFlags & mNorg72) {
					m_bSwonzaEnlightened = TRUE;
					if (nTeamMembers == 1) {
						pszFailureFile = kGSM17SraMaleStr;
					} else {
						if (teamListItem.m_nFlags & mStafferMale) {
							pszFailureFile = kGSM16SraMaleStr;
						} else {
							pszFailureFile = kGSM16SraFemStr;
						}
					}
				} else {
					// Update the other party bids...
					g_stOtherPartys[SWONZA5].m_nPaymentAmount = 20;
					if (teamListItem.m_nFlags & mStafferMale) {
						pszSuccessFile = kGSM9SraMaleStr;
					} else {
						pszSuccessFile = kGSM9SraFemStr;
					}
				}
				break;
			case POLITICIANS:
				// Need to have either vargas or pnurth on this team
				if ((teamListItem.m_nFlags & mVargas20) || (teamListItem.m_nFlags & mPnurth81)) {
					if (teamListItem.m_nFlags & mStafferMale) {
						pszSuccessFile = kGSM11SraMaleStr;
					} else {
						pszSuccessFile = kGSM11SraFemStr;
					}

					// Update the other party bids...
					g_stOtherPartys[POLITICIANS].m_nPaymentAmount = -15;
				} else {
					if (teamListItem.m_nFlags & mStafferMale) {
						pszFailureFile = kGSM5SraMaleStr;
					} else {
						pszFailureFile = kGSM5SraFemStr;
					}
				}
				break;

			case ENVIRONMENTALISTS:
				// Need to have either lentil or pnurth on this team
				if ((teamListItem.m_nFlags & mLentil24) || (teamListItem.m_nFlags & mPnurth81)) {
					if (teamListItem.m_nFlags & mStafferMale) {
						pszSuccessFile = kGSM12SraMaleStr;
					} else {
						pszSuccessFile = kGSM12SraFemStr;
					}

					// Update the other party bids...
					g_stOtherPartys[ENVIRONMENTALISTS].m_nPaymentAmount = -2;

				} else {
					if (teamListItem.m_nFlags & mStafferMale) {
						pszFailureFile = kGSM5SraMaleStr;
					} else {
						pszFailureFile = kGSM5SraFemStr;
					}
				}
				break;

			default:
				Assert(FALSE);
			}
		}

		if (pszFailureFile == nullptr && (teamListItem.m_nFlags & mBuyer)) {
			int nRequiredStrength = 0;
			int nTalkedUpAmount1 = 0, nTalkedUpAmount2 = 0;
			int nMineralID1 = 0, nMineralID2 = 0;
			switch (teamListItem.m_nMeetWithID) {
			case SINJIN11:
				// Can up structures from 5 to (7gZ)
				nRequiredStrength = SINJIN11_REQUIRED_TEAM_STRENGTH;
				nTalkedUpAmount1 = kSinjinTalkedUpAmount;
				nMineralID1 = kSinjinMineralID;

			case GUNG14:
				// This guy can be talked up by a good negotiating team to (8gZ)
				if (nRequiredStrength == 0) {
					nRequiredStrength = GUNG14_REQUIRED_TEAM_STRENGTH;
					nTalkedUpAmount1 = kGungTalkedUpAmount;
					nMineralID1 = kGungMineralID;
				}

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
			case DORK44: {
				// negotiating team must be at least half female.
				int     nMale = 0;
				int     nFemale = 0;
				if (teamListItem.m_nMeetWithID == DORK44) {
					for (int k = 0; k < NUM_STAFFERS; k++) {
						switch (k) {
						case 0:
							nMale += (teamListItem.m_nFlags & mNorg72 ? 1 : 0);
							break;
						case 1:
							nFemale += (teamListItem.m_nFlags & mPnurth81 ? 1 : 0);
							break;
						case 2:
							nFemale += (teamListItem.m_nFlags & mZig64 ? 1 : 0);
							break;
						case 3:
							nFemale += (teamListItem.m_nFlags & mLentil24 ? 1 : 0);
							break;
						case 4:
							nMale += (teamListItem.m_nFlags & mVargas20 ? 1 : 0);
							break;
						case 5:
							nMale += (teamListItem.m_nFlags & mChurg53 ? 1 : 0);
							break;
						}
					}
					if (nFemale < nMale) {
						if (teamListItem.m_nFlags & mStafferMale) {
							pszFailureFile = kGSM23SraMaleStr;
						} else {
							pszFailureFile = kGSM23SraFemStr;
						}
						bDone = TRUE;
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
				        g_stBuyerBids[teamListItem.m_nMeetWithID].m_nMineralVal[nMineralID1] != nTalkedUpAmount1) {
					int nDiff1 = nTalkedUpAmount1 - g_stBuyerBids[teamListItem.m_nMeetWithID].m_nMineralVal[nMineralID1];
					int nDiff2 = nTalkedUpAmount2 - g_stBuyerBids[teamListItem.m_nMeetWithID].m_nMineralVal[nMineralID2];

					// Set new values in the buyer bids static
					g_stBuyerBids[teamListItem.m_nMeetWithID].m_nMineralVal[nMineralID1] = (int16)nTalkedUpAmount1;
					g_stBuyerBids[teamListItem.m_nMeetWithID].m_nBidSum += nDiff1;
					if (nTalkedUpAmount2) {
						g_stBuyerBids[teamListItem.m_nMeetWithID].m_nMineralVal[nMineralID2] = (int16)nTalkedUpAmount2;
						g_stBuyerBids[teamListItem.m_nMeetWithID].m_nBidSum += nDiff2;
					}

					// determine the voice file to give back...
					if (teamListItem.m_nFlags & mStafferMale) {
						pszSuccessFile = kGSM4SraMaleStr;
					} else {
						pszSuccessFile = kGSM4SraFemStr;
					}
				} else {
					if (teamListItem.m_nFlags & mStafferMale) {
						pszFailureFile = kGSM5SraMaleStr;
					} else {
						pszFailureFile = kGSM5SraFemStr;
					}
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

				// just talked to sm, he says that the bios say they can possibly
				// be talked up, but they' can't no matter how hard we try...
				if (teamListItem.m_nFlags & mStafferMale) {
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
				if (teamListItem.m_nFlags & mStafferMale) {
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
				if (teamListItem.m_nFlags & mStafferMale) {
					pszFailureFile = kGSM5SraMaleStr;
				} else {
					pszFailureFile = kGSM5SraFemStr;
				}
				break;
			default:
				Assert(FALSE);
			}
		}

		if (pszFailureFile == nullptr && (teamListItem.m_nFlags & mSeller)) {
			switch (teamListItem.m_nMeetWithID) {
			case IRK4:
				// Irk can't be talked down and will only talk to deven in person
				if (teamListItem.m_nFlags & mStafferMale) {
					pszFailureFile = kGSM14SraMaleStr;
				} else {
					pszFailureFile = kGSM14SraFemStr;
				}
				break;
			case QUOSH23:
				// Quosh can't be talked down and will only talk to deven in person
				if (teamListItem.m_nFlags & mStafferMale) {
					pszFailureFile = kGSM13SraMaleStr;
				} else {
					pszFailureFile = kGSM13SraFemStr;
				}
				break;
			case YEEF8:
				// Yeef can be talked down but requires a really sharp negotiating
				// team.
				//
				// if they've already talked yeef down, then reply with
				// a fail message.
				if (nTeamStrength >= YEEF8_REQUIRED_TEAM_STRENGTH && g_stSellerNames[YEEF8].m_nAmount != kYeefTalkedDownAmount) {
					g_stSellerNames[YEEF8].m_nAmount = kYeefTalkedDownAmount;
					if (teamListItem.m_nFlags & mStafferMale) {
						pszSuccessFile = kGSM3SraMaleStr;
					} else {
						pszSuccessFile = kGSM3SraFemStr;
					}
				} else {
					if (teamListItem.m_nFlags & mStafferMale) {
						pszFailureFile = kGSM5SraMaleStr;
					} else {
						pszFailureFile = kGSM5SraFemStr;
					}
				}
				break;
			default:
				Assert(FALSE);
			}
		}
	}

	// if we failed or succeeded, remove it from the list of meetings
	if (pszFailureFile || pszSuccessFile) {
		SetMeetingResult(teamListItem.m_nFlags, teamListItem.m_nMeetWithID, (pszSuccessFile != nullptr));
		if (bTimeElapsed) {
			for (i = 0; i < NUM_STAFFERS; i++) {
				if (teamListItem.m_nFlags & (1 << (i + 3))) {
					g_staffers[i].m_bAvailable = TRUE;
				}
			}
		}

		// Pick the team captain and build the response file.
		nTeamCaptain = teamListItem.m_nTeamCaptain;
		if (teamListItem.m_nFlags & mStafferMale) {
			sResponse = BuildMaleSrafDir(pszFailureFile == nullptr ? pszSuccessFile : pszFailureFile);
		} else {
			sResponse = BuildFemaleSrafDir(pszFailureFile == nullptr ? pszSuccessFile : pszFailureFile);
		}

		if (bTimeElapsed) {
			// 12.11.96 mark the 'meetee' as available.
			if (teamListItem.m_nFlags & mOtherParty) {
				g_stOtherPartys[teamListItem.m_nMeetWithID].m_bAvailable = TRUE;
			} else {
				if (teamListItem.m_nFlags & mSeller) {
					g_stSellerNames[teamListItem.m_nMeetWithID].m_bAvailable = TRUE;
				} else {
					if (teamListItem.m_nFlags & mBuyer) {
						g_stBuyerBids[teamListItem.m_nMeetWithID].m_bAvailable = TRUE;
					}
				}
			}

			m_pTeamList->Remove(nTeamNumber);
			bNeedRedraw = TRUE;
		}
	}

	// failure file, a text file for now.
	// we'll want to play a sound file, for now, just put the text to the screen
	if (pszFailureFile || pszSuccessFile) {
		NotifyBoss(sResponse, nTeamCaptain);
	}

	return bNeedRedraw;
}

void SrafComputer::OnListCheckTeams() {
	int nTeamNumber = m_nSelection - 3;

	if (ReportMeetingStatus(nTeamNumber)) {
		ActivateCheckTeams();
	}
}

void SrafComputer::OnListCodeWords() {

	int         nCodeWordLine = m_nSelection - 4;
//	int          nTextWidth = 8;
	int         nLastLine = NUM_CODE_WORDS + 5;
	char        szLocalBuff[256];
	CBofString  sStr(szLocalBuff, 256);

	// reject out of range selections
	if (nCodeWordLine < 0 || nCodeWordLine >= NUM_CODE_WORDS) {
		return;
	}

#if USETEXTWIDTHS

	// calculate the text width based on the attributes of the text
	// rather than guessing to where they are.
	AlignAtColumn(sStr, "", kGroup1Col2);
	CBofRect        cCol1Rect = CalculateTextRect(this, &sStr, kOtherPointSize, FONT_MONO);
	CBofRect        cCol2Rect = cCol1Rect;
	CBofPoint       cStartPoint(cCol1Rect.right, 0);

	cCol2Rect.OffsetRect(cStartPoint);

	// Second bunch of code words start at column 45, create a dummy string and then
	// start offsetting from there.
	AlignAtColumn(sStr, "", kGroup2Col1);
	CBofRect        cDummyRect = CalculateTextRect(this, &sStr, kOtherPointSize, FONT_MONO);
	cStartPoint.x = cDummyRect.right;

	CBofRect        cCol3Rect = cCol1Rect;
	cCol3Rect.OffsetRect(cStartPoint);

	CBofRect        cCol4Rect = cCol1Rect;
	cStartPoint.x = cCol3Rect.right;
	cCol4Rect.OffsetRect(cStartPoint);

	// Extend each rectangle to the bottom of the screen.
	cCol1Rect.bottom = cCol2Rect.bottom = cCol3Rect.bottom = cCol4Rect.bottom = 1000;
#else

#if BOF_MAC
	nTextWidth = ::CharWidth('a');
#else
	nTextWidth = 8;         // ??? brian, just guessing at the width of a
	// monowidth font would be
#endif
	CBofRect        cCol1Rect(0,
	                          gCompDisplay.top,
	                          (kGroup1Col2 - 5) * nTextWidth,
	                          gCompDisplay.bottom);

	CBofRect        cCol2Rect(kGroup1Col2 * nTextWidth,
	                          gCompDisplay.top,
	                          (kGroup1Col2 + 10) * nTextWidth,
	                          999);

	CBofRect        cCol3Rect(kGroup2Col1 * nTextWidth,
	                          gCompDisplay.top,
	                          (kGroup2Col1 + 10) * nTextWidth,
	                          999);

	CBofRect        cCol4Rect(kGroup2Col2 * nTextWidth,
	                          gCompDisplay.top,
	                          (kGroup2Col2 + 10) * nTextWidth,
	                          999);
#endif

	// Figure out which words were picked

	const char *pszWord = nullptr;
	int         nWordGroup = 0;
	CBofPoint   cPoint = GetPrevMouseDown();        // already in local coords

	// Find the rect that it was in.
	if (cCol1Rect.PtInRect(cPoint)) {
		nWordGroup = 1;
		pszWord = g_stCodeWords[nCodeWordLine].m_pszCodeWord1;
	} else {
		if (cCol2Rect.PtInRect(cPoint)) {
			nWordGroup = 2;
			pszWord = g_stCodeWords[nCodeWordLine].m_pszCodeWord2;
		} else {
			if (cCol3Rect.PtInRect(cPoint)) {
				nWordGroup = 3;
				pszWord = g_stCodeWords[nCodeWordLine].m_pszCodeWord3;
			} else {
				if (cCol4Rect.PtInRect(cPoint)) {
					nWordGroup = 4;
					pszWord = g_stCodeWords[nCodeWordLine].m_pszCodeWord4;
				}
			}
		}
	}

	// Find out if a new word was picked.
	if (pszWord) {
		if (nWordGroup == 1 || nWordGroup == 2) {
			*m_pszGroup1Word = pszWord;
		} else {
			*m_pszGroup2Word = pszWord;
		}

		sStr = "CODE WORD PAIR: (";
		if (m_pszGroup1Word) {
			sStr += *m_pszGroup1Word;
		}

		sStr += ",";

		if (m_pszGroup2Word) {
			sStr += *m_pszGroup2Word;
		}

		sStr += ")";
		m_pLBox->SetText(nLastLine, sStr);
		m_pLBox->RepaintItem(nLastLine);
	}
}

void SrafComputer::DeactivateMainScreen() {
	DeleteListBox();

#if BOF_WINDOWS
	m_pButtons[QUIT_BUTTON]->Hide();
#endif

	//  Hide the on/off button
#if !BOF_WINDOWS
	if (m_eMode == SCON) {
		m_pButtons[ON_BUTTON]->Hide();
	} else {
		m_pButtons[OFF_BUTTON]->Hide();
	}
#endif
}

void SrafComputer::ActivateMainScreen() {
	int numItems;
	ERROR_CODE error = ERR_NONE;
	SrafCompItem    compItem;
	char            szLocalStr[256];
	szLocalStr[0] = '\0';
	CBofString      sStr(szLocalStr, 256);

	// current screen is now the MAIN screen.
	m_eCurScreen = SCMAIN;

	// Delete the list box
	DeleteListBox();

	// Hide the return to main if it's been created

	Show();

	if (GetBackdrop()) {
		PaintBackdrop();
	}

#if BOF_WINDOWS
	HideAllButtons();
#endif

	m_pButtons[RETURN_TO_MAIN_BUTTON]->Hide();

#if !BOF_WINDOWS
	m_pButtons[ON_BUTTON]->Hide();

	m_pButtons[OFF_BUTTON]->Show();
#endif
	m_pButtons[QUIT_BUTTON]->Show();

#if BOF_WINDOWS
	m_pButtons[QUIT_BUTTON]->Paint(nullptr);
#endif

#if BOF_MAC
	// whenever dealing with the on/off button, make sure to
	// paint the new bitmap.  Seems like this should be a windowz fix
	// also, but I'll let the next windows guy in here determine that.
	m_pButtons[OFF_BUTTON]->Paint(nullptr);
	m_pButtons[QUIT_BUTTON]->Paint(nullptr);
#endif

	// Initialize point size and item height
	m_nListPointSize = kOtherPointSize;
	m_nListItemHeight = kLineItemHeight;

	error = CreateListBox();
	Assert(error == ERR_NONE);

	numItems = m_pMainList->GetCount();

	// populate listbox

	for (int i = 0; i < numItems; i++) {
		compItem = m_pMainList->GetNodeItem(i);
		m_pLBox->AddToTail(CBofString(compItem.m_pItem), FALSE);
	}

	// add instructions to the bottom of the list...

	sStr = "";
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kActivateFooterStr1;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kActivateFooterStr2;
	m_pLBox->AddToTail(sStr, FALSE);

	sStr = kActivateFooterStr3;
	m_pLBox->AddToTail(sStr, FALSE);

	// Display the current time...

	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	// add a dummy line so we can overwrite it with the
	// current turncount.
	sStr = " ";
	m_pLBox->AddToTail(sStr, FALSE);

	int nLineNo = m_pLBox->GetNumItems();
	DisplayTurnCount(nLineNo - 1);

	// show list box
	m_pLBox->Show();

#if BOF_WINDOWS
	m_pLBox->UpdateWindow();
	m_pLBox->RepaintAll();

#endif

	//  Show the computer on/off button

	//m_pButtons[OFF_BUTTON]->Show();
}


void SrafComputer::AlignAtColumn(CBofString &sStr, const char *szRightText, int nAlignAt) {
//	int nStartLen = sStr.GetLength();
	int nAppendLen = strlen(szRightText);

	while (sStr.GetLength() < nAlignAt) {
		sStr += " ";
	}

	// now right justify and get rid of any zeros

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

void SrafComputer::DisplayTextScreen(CBofString &sStr) {
	// Use a global to determine if we can give meeting reports or not.
	gTextScreenFrontmost = TRUE;

	m_pTextOnlyScreen = new SrafTextScreen(sStr);

	if (m_pTextOnlyScreen != nullptr) {
		m_pTextOnlyScreen->CreateTextScreen(this);

#if BOF_WINDOWS
		m_pTextOnlyScreen->DoModal();
		delete m_pTextOnlyScreen;
		m_pTextOnlyScreen = nullptr;

		// if we have a list, then return focus to it.
		if (m_pLBox) {
			m_pLBox->SetFocus();
		}

		gTextScreenFrontmost = FALSE;
#else
		m_pTextOnlyScreen->DisplayTextScreen();

		// Mac doesn't have to delete this as the OnClose method will do it
		// for us (because it inherits from CBofWindow).
#endif
	}

}

void SrafComputer::OnButtonMainScreen(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {

		switch (pButton->GetControlID()) {
		case ON_BUTTON:
			SetOn();
			break;
		case OFF_BUTTON:
			SetOff();
			break;
		case QUIT_BUTTON:
			SetQuit();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonDealSummary(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			m_pButtons[SUBMIT_BUTTON]->Hide();
			ActivateMainScreen();
			break;

		case SUBMIT_BUTTON:
			if (OnButtonSubmitOffer()) {
				ActivateCodeWords();
			}
			break;

		default:
			break;
		}
	}
}

bool SrafComputer::OnButtonSubmitOffer() {

	bool bOfferAccepted = TRUE;
	//const char *szAcceptedOfferp = kszOfferAccepted;
	char szFailureReason[256];
	int nAskingPrice, nTotalOffer = 0;
	int i = 0, j;

	//  First, make sure that we know the terms of the sellers.  If they're
	//  not all resolved, then we can't continue.

	nAskingPrice = 0;
	while (i < NUM_SELLERS) {
		if (g_stSellerNames[i].m_nAmount == -1) {
			Common::sprintf_s(szFailureReason, kszUnresolvedSeller, g_stSellerNames[i].m_pszName);
			bOfferAccepted = FALSE;
			break;
		}

		nAskingPrice += g_stSellerNames[i].m_nAmount;

		i++;
	}

	// Check that all the terms have been resolved for the buyers... all we really
	// need to check here is the environmentalists.

	if (bOfferAccepted) {
		if (g_stOtherPartys[ENVIRONMENTALISTS].m_nPaymentAmount == -1) {
			Common::strcpy_s(szFailureReason, kszUnresolvedEnriro);
			bOfferAccepted = FALSE;
		}
	}

	// If we didn't fail because of the seller, let's now check the buyer
	// grid.  First check is that we didn't give the same mining right away
	// to more than a single party.

	if (bOfferAccepted) {
		int     bMinerals[NUM_MINERALS];

		// clear out the mineral accepted array.  If a mining right is given
		// away to more than one party, then the array will have a non-zero
		// value for that party.
		for (i = 0; i < NUM_MINERALS; i++) {
			bMinerals[i] = -1;
		}

		nTotalOffer = 0;
		for (i = 0; i < NUM_BUYERS; i++) {
			if (g_stBuyerBids[i].m_bAccept) {

				nTotalOffer += g_stBuyerBids[i].m_nBidSum;
				// we've chose to accept this bid, so make sure that we have not
				// already promised these mining rights to somebody else.
				for (j = 0; j < NUM_MINERALS; j++) {
					if (g_stBuyerBids[i].m_nMineralVal[j] != 0) {
						if (bMinerals[j] != -1) {
							Common::sprintf_s(szFailureReason, kszMiningConflict,
							                  g_stMinerals[j].m_pszMineralName, g_stBuyerBids[i].m_pszName, g_stBuyerBids[bMinerals[j]].m_pszName);
							bOfferAccepted = FALSE;
							break;
						} else {
							bMinerals[j] = i;
						}
					}
				}

				//  if we found a dup, then get out of here.
				if (bOfferAccepted == FALSE) {
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
			nTotalOffer += (g_stOtherPartys[i].m_nPaymentAmount == -1 ? 0 : g_stOtherPartys[i].m_nPaymentAmount);
		}
		if (nAskingPrice > nTotalOffer) {
			Common::sprintf_s(szFailureReason, kszOfferNotEnough, nAskingPrice - nTotalOffer);
			bOfferAccepted = FALSE;
		}
	}

	// Now check that the weirdnesses of each of the buyers is met, here is a
	// list of them:
	//
	// Gung-14 won't go into a deal with anyone with an '8' in their name
	// Hem-20 won't go in on a deal with Dippik
	// Dork-44 insists backer group is 50% female.
	// Jella-37's lumber bid includes crop mining rights
	// Sinjin and Dork won't participate in the same deal.

	if (bOfferAccepted) {
		for (i = 0; i < NUM_BUYERS; i++) {
			switch (g_stBuyerBids[i].m_nBuyerID) {
			case GUNG14:
				// Make sure that there are no backers with the number '8'
				// in their name.
				if (g_stBuyerBids[GUNG14].m_bAccept &&
				        g_stBuyerBids[VEBBIL18].m_bAccept) {
					Common::sprintf_s(szFailureReason, kszGungNotHappy, g_stBuyerBids[VEBBIL18].m_pszName);
					bOfferAccepted = FALSE;
					break;
				}
				break;

			case HEM20:
				// Hem-20 won't do business with dippik.
				if (g_stBuyerBids[HEM20].m_bAccept &&
				        g_stBuyerBids[DIPPIK10].m_bAccept) {
					Common::strcpy_s(szFailureReason, kszHemNotHappy);
					bOfferAccepted = FALSE;
					break;
				}
				break;

			case DORK44: {
				// Dork-44 insists that at least half the backers are female.
				int16       nMale = 0;
				int16       nFemale = 0;

				if (g_stBuyerBids[DORK44].m_bAccept) {
					for (j = 0; j < NUM_BUYERS; j++) {
						if (g_stBuyerBids[j].m_bAccept) {
							if (g_stBuyerBids[j].m_nFlags & mBuyerMale) {
								nMale++;
							} else {
								if (g_stBuyerBids[j].m_nFlags & mBuyerFemale) {
									nFemale++;
								}
							}
						}
					}

					if (nMale > nFemale) {
						Common::strcpy_s(szFailureReason, kszDorkNotHappy);
						bOfferAccepted = FALSE;
						break;
					}
				}
				break;
			}

			case JELLA37:
				// if we accepted Jella-37's lumber bid, then we also have to reserve
				// crop harvesting rights for her.

				if (g_stBuyerBids[JELLA37].m_bAccept) {
					for (j = 0; j < NUM_BUYERS; j++) {
						if (g_stBuyerBids[j].m_bAccept && g_stBuyerBids[j].m_nMineralVal[CROP] != 0) {
							Common::sprintf_s(szFailureReason, kszJellaNotHappy, g_stBuyerBids[j].m_pszName);
							bOfferAccepted = FALSE;
							break;
						}
					}
				}
				break;

			case SINJIN11:
				// Sinjin-11 won't do business with Dork-44.
				if (g_stBuyerBids[SINJIN11].m_bAccept &&
				        g_stBuyerBids[DORK44].m_bAccept) {
					Common::strcpy_s(szFailureReason, kszSinjinNotHappy);
					bOfferAccepted = FALSE;
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
			if (bOfferAccepted == FALSE) {
				break;
			}
		}
	}

	if (bOfferAccepted == FALSE) {
		DisplayMessage(szFailureReason);
	}

	return bOfferAccepted;
}

void SrafComputer::OnButtonBuyerBids(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;

		case DISPLAY_KEY_BUTTON: {
			char szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString sStr(szLocalBuff, 256);

			sStr = BuildSrafDir("KEYINFO.TXT");
			DisplayTextScreen(sStr);
			break;
		}

		default:
			break;
		}
	}
}

void SrafComputer::OnButtonDealBackground(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonSellerBios(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonOtherBios(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonStaffBios(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonDispatchTeam(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		case DISPATCH_TEAM_BUTTON:
			VerifyDispatchTeam();
		default:
			break;
		}
	}
}

// Apply staffer meeting rules to figure out how long a meeting should take.
int SrafComputer::CalculateMeetingTime(int nFlags) {
	int mFlag = 0;
	int nTeamMembers = 0;
	int nTotalTime = 0;
	int i;

	// Handle the failure case first... that is, if no driver was sent {Norg,
	// Zig and Lentil} Try seeing if 5 is a good thing.
	if (!((nFlags & mNorg72) || (nFlags & mZig64) || (nFlags & mLentil24))) {
		return kDrivingTime;
	}

	// Add up the total time.

	for (i = 0; i < NUM_STAFFERS; i++) {
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
			nTotalTime += g_staffers[i].m_nMeetingTime;
		}
	}

	// Done adding up, now apply the following special rules.
	//
	// If Lentil not on her own, then double her time.
	// If pnurth on her own, double her time.

	if ((nFlags & mLentil24) && nTeamMembers > 1) {
		nTotalTime += g_staffers[LENTIL24].m_nMeetingTime;
	}

	if ((nFlags & mPnurth81) &&
	        ((nFlags & mChurg53) || (nFlags & mVargas20) || (nFlags & mLentil24))) {
		nTotalTime -= (g_staffers[PNURTH81].m_nMeetingTime / 2);
	}

	return nTotalTime / nTeamMembers;
}

void SrafComputer::NotifyBoss(CBofString &sSoundFile, int nStafferID) {         // Must be a full file spec
	// This routine is perfectly synchronous, so we can do whatever we want.

	CBofBitmap *pSaveBackground = nullptr;

	// allow for no staffer screen
	if (nStafferID != -1) {
		pSaveBackground = new CBofBitmap(gTextWindow.Width(), gTextWindow.Height(), (CBofPalette *)nullptr, FALSE);
		Assert(pSaveBackground != nullptr);

		if (pSaveBackground == nullptr) {
			return;
		}

		pSaveBackground->CaptureScreen(this, &gTextWindow);

		if (m_pStafferBmp[nStafferID] == nullptr) {
			char        szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString  sStr(szLocalBuff, 256);

			sStr = BuildSrafDir(g_staffers[nStafferID].m_pszStafferBmp);

			m_pStafferBmp[nStafferID] = new CBofBitmap(szLocalBuff);
			Assert(m_pStafferBmp[nStafferID] != nullptr);
			if (m_pStafferBmp[nStafferID] == nullptr) {
				return;
			}
		}

		// Guaranteed to have it now.  Paste it to the screen.

		m_pStafferBmp[nStafferID]->Paint(this, &gStafferDisplay);

	}

	CBofCursor::Hide();

	// Play the voice file... Depends on if we have a voice file or a text file...
	// the last three will tell us.

	if (sSoundFile.Find(".WAV") != -1 ||
	        sSoundFile.Find(".wav") != -1) {
		BofPlaySound(sSoundFile.GetBuffer(), SOUND_WAVE);
	} else {
		if (sSoundFile.Find(".TXT") ||
		        sSoundFile.Find(".txt")) {

			// Make sure the file is there, read it in to our own buffer.
			CBofFile        fTxtFile(sSoundFile, CBF_BINARY | CBF_READONLY);
			char *pszBuf;
			int             nLength = fTxtFile.GetLength();

			if (nLength != 0 && (pszBuf = (char *)BofAlloc(nLength + 1)) != nullptr) {
				BofMemSet(pszBuf, 0, nLength + 1);
				fTxtFile.Read(pszBuf, nLength);
#if BOF_MAC
				StrReplaceChar(pszBuf, '\n', ' ');
#endif
				// Put it up on the screen
				DisplayMessage(pszBuf);
				BofFree(pszBuf);
				fTxtFile.Close();
			} else {
				ReportError(ERR_MEMORY, "Could not read %s into memory", sSoundFile.GetBuffer());
			}
		}
	}

	// allow for no staffer screen
	if (nStafferID != -1) {
		pSaveBackground->Paint(this, &gTextWindow);

		delete pSaveBackground;
	}

	CBofCursor::Show();
}

// most experienced person is captain.
int SrafComputer::GetTeamCaptain(int nFlags) {

	int     nStaffCaptain = 0;

	if (nFlags & mVargas20) {
		nStaffCaptain = VARGAS20;
	} else {
		if (nFlags & mLentil24) {
			nStaffCaptain = LENTIL24;
		} else {
			if (nFlags & mChurg53) {
				nStaffCaptain = CHURG53;
			} else {
				if (nFlags & mZig64) {
					nStaffCaptain = ZIG64;
				} else {
					if (nFlags & mNorg72) {
						nStaffCaptain = NORG72;
					} else {
						if (nFlags & mPnurth81) {
							nStaffCaptain = PNURTH81;
						}
					}
				}
			}
		}
	}

	return nStaffCaptain;
}

// most experienced person determines sex of team.
int SrafComputer::GetTeamGender(int nFlags) {

	int     nStaffGender = 0;

	if (nFlags & mVargas20) {
		nStaffGender = mStafferMale;
	} else {
		if (nFlags & mLentil24) {
			nStaffGender = mStafferFemale;
		} else {
			if (nFlags & mChurg53) {
				nStaffGender = mStafferMale;
			} else {
				if (nFlags & mZig64) {
					nStaffGender = mStafferFemale;
				} else {
					if (nFlags & mNorg72) {
						nStaffGender = mStafferMale;
					} else {
						if (nFlags & mPnurth81) {
							nStaffGender = mStafferFemale;
						}
					}
				}
			}
		}
	}

	return nStaffGender;
}

void SrafComputer::OnButtonCurrentEMail(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonAudioSettings(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		case NO_MUSIC_BUTTON:
			CBofSound::StopSounds();
			m_pButtons[NO_MUSIC_BUTTON]->Hide();
			ActivateMainScreen();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonRoboButler(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		case ORDER_BEVERAGE_BUTTON:
			DoOrderBeverage();
			break;
		case ORDER_SNACK_BUTTON:
			DoOrderSnack();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::DoOrderBeverage() {
	char    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	SDEVMNGR->AddObject("INV_WLD", "SZTB");
	SDEVMNGR->AddObject("INVZ_WLD", "SZTB");

	// Build the string to indicate that the beverage has been stashed
	sStr = BuildSrafDir(g_stOfferings[0].m_pszRcvOfferingFile);
	NotifyBoss(sStr, -1);

	// state of buttons has changed
	ActivateRoboButler();
}

void SrafComputer::DoOrderSnack() {
	char    szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sStr(szLocalBuff, 256);

	SDEVMNGR->AddObject("INV_WLD", "SZTA");
	SDEVMNGR->AddObject("INVZ_WLD", "SZTA");

	// Build the string to indicate that the beverage has been stashed
	sStr = BuildSrafDir(g_stOfferings[1].m_pszRcvOfferingFile);
	NotifyBoss(sStr, -1);

	// state of buttons has changed
	ActivateRoboButler();
}

void SrafComputer::OnButtonCheckTeams(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case RETURN_TO_MAIN_BUTTON:
			ActivateMainScreen();
			break;
		default:
			break;
		}
	}
}

void SrafComputer::OnButtonCodeWords(CBofButton *pButton, int nState) {
	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case DONE_BUTTON:
			// Normally we'd return to the main screen, but in this instance, we're
			// done with the flashback... that is, as long as they've picked two
			// code words.
			OnButtonFinished(TRUE);
			break;

		default:
			break;
		}
	}
}

void SrafComputer::OnButtonFinished(bool bVictorious) {
//	CBagLinkObject *pLink = nullptr;
	CBagVar *pVar = nullptr;

	// Make sure the user selected two code words.

	if (bVictorious == TRUE) {
		if (m_pszGroup1Word->GetLength() == 0 ||
		        m_pszGroup2Word->GetLength() == 0) {
			// Put up an error...
			DisplayMessage(kszFinishCodeWords);
			return;
		}

		// Pass our codewords back to the bar...
		pVar = VARMNGR->GetVariable("DEVENCODE1");
		Assert(pVar != nullptr);

		if (pVar)
			pVar->SetValue(m_pszGroup1Word->GetBuffer());

		pVar = VARMNGR->GetVariable("DEVENCODE2");
		Assert(pVar != nullptr);
		if (pVar)
			pVar->SetValue(m_pszGroup2Word->GetBuffer());
	}

	// Setting the flashback variable will trigger the
	// event world condition which gets us back to the bar.
	pVar = VARMNGR->GetVariable("RFLASHBACK");
	Assert(pVar != nullptr);

	if (pVar) {
		if (bVictorious == TRUE) {
			pVar->SetValue(2);
		} else {
			pVar->SetValue(1);
		}
	}

	// Need to kill current window before we go back to the bar
//#if BOF_WINDOWS
//    Close();
//#endif

	return;
}

// Display's a message at the bottom of the screen.

void SrafComputer::DisplayMessage(const char *szMsg) {
	CBofBitmap saveBackground(gSrafTextWindow.Width(), gSrafTextWindow.Height(),
	                          (CBofPalette *)nullptr, FALSE);
	saveBackground.CaptureScreen(this, &gSrafTextWindow);

#if 1
	//CBofRect cRect(0, 0, gSrafTextWindow.Width(), gSrafTextWindow.Height());

	PaintBeveledText(this, &gSrafTextWindow, szMsg, FONT_15POINT, TEXT_NORMAL, m_cTextColor, JUSTIFY_WRAP, FORMAT_TOP_LEFT);

#else

	CBofBitmap messageScreen(gSrafTextWindow.Width(), gSrafTextWindow.Height(), nullptr, FALSE);

	messageScreen.Lock();

#if BOF_WINDOWS
	CBofApp *pApp;
	CBofPalette *pPal;

	if ((pApp = CBofApp::GetApp()) != nullptr) {
		pPal = pApp->GetPalette();
	}

	if (pPal != nullptr) {
		CBofRect cRect;

		cRect = messageScreen.GetRect();
		messageScreen.FillRect(nullptr, pPal->GetNearestIndex(RGB(92, 92, 92)));

		messageScreen.DrawRect(&cRect, pPal->GetNearestIndex(RGB(0, 0, 0)));
	} else {
		messageScreen.FillRect(nullptr, COLOR_BLACK);
	}

	CBofRect cBevel, r;
	int i, left, top, right, bottom;
	byte c1, c2;

	c1 = 3;
	c2 = 9;
	r = messageScreen.GetRect();
	cBevel = r;

	left = cBevel.left;
	top = cBevel.top;
	right = cBevel.right;
	bottom = cBevel.bottom;

	r.left += 6;
	r.top += 3;
	r.right -= 5;
	r.bottom -= 5;

	for (i = 1; i <= 3; i++) {
		messageScreen.Line(left + i, bottom - i, right - i, bottom - i, c1);
		messageScreen.Line(right - i, bottom - i, right - i, top + i - 1, c1);
	}

	for (i = 1; i <= 3; i++) {
		messageScreen.Line(left + i, bottom - i, left + i, top + i - 1, c2);
		messageScreen.Line(left + i, top + i - 1, right - i, top + i - 1, c2);
	}

	PaintText(&messageScreen,
	          &r,
	          szMsg,
	          FONT_14POINT,
	          TEXT_NORMAL,
	          m_cTextColor,
	          JUSTIFY_WRAP,
	          FORMAT_TOP_LEFT);

#else

	// Now paint the text to this rect.
	CBofRect        cRect(0, 0, gSrafTextWindow.Width(), gSrafTextWindow.Height());

	PaintText(&messageScreen,
	          &cRect,
	          szMsg,
	          FONT_14POINT,
	          TEXT_NORMAL,
	          m_cTextColor,
	          JUSTIFY_WRAP,
	          FORMAT_TOP_LEFT);
#endif

	messageScreen.Paint(this, &gSrafTextWindow);
	messageScreen.UnLock();

#endif

#if BOF_MAC
	while (!::Button())
		;
	::FlushEvents(everyEvent, 0);       // swallow the mousedown, don't want it processed
#else
	// ?? Brian, you'll need a statement here to cause the screen to wait for a mouse
	// down to occur

	WaitForInput();
#endif

	saveBackground.Paint(this, &gSrafTextWindow);
}


void SrafComputer::SetMainScreen() {
	if (m_pHead == nullptr) {
		return;
	}

	SrafComputer *srafComp = m_pHead;
	if (srafComp->m_eCurScreen == SCBACKGROUNDDATA) {
		srafComp->m_eCurScreen = SCMAIN;
	}
}

void SrafComputer::IncrementTurnCount() {

	CBagVar *pVar = VARMNGR->GetVariable("SRATURNCOUNT");
	int nTurncount = 0;

	Assert(pVar != nullptr);

	// if the puzzle is already solved, then don't increment.
	if (m_eCurScreen == SCCODE_WORDS) {
		return;
	}


	if (pVar != nullptr) {
		nTurncount = pVar->GetNumValue();
		pVar->SetValue(++nTurncount);
	}

	// If we're in a screen that has a time count, then update it here...
	if (m_eCurScreen == SCCHECK_TEAMS || m_eCurScreen == SCMAIN) {
		DisplayTurnCount(gTurncountLineNo);
	}

	// automatically give the user the status of the meeting if the
	// meeting time has expired...
	if (m_pTeamList) {
		DispatchedTeamItem      teamListItem;
		int nTeams = m_pTeamList->GetCount();
		for (int i = 0; i < nTeams; i++) {
			teamListItem = m_pTeamList->GetNodeItem(i);

			if (nTurncount > (teamListItem.m_nDispatchTime + teamListItem.m_nMeetingTime)) {
				if (ReportMeetingStatus(i)) {
					// If current screen is the check teams screen, then make sure we update
					// it.
					switch (m_eCurScreen) {
					case SCCHECK_TEAMS:
						ActivateCheckTeams();
						break;

					// update the buyer bids screen if that's our underlying
					// guy.
					case SCBIDS:
						ActivateBuyerBids();
						break;

					case SCDEAL:
						ActivateDealSummary();
						break;

					case SCDISPATCH:
						ActivateDispatchTeam();
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

	if (m_bFailureNotified == FALSE) {
		if (nTurncount > (m_nStartingTime + kSrafMaxTurns)) {
			m_bFailureNotified = TRUE;

			char        szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString  sResponse(szLocalBuff, 256);

			sResponse = BuildMaleSrafDir(SRAFTIMEOUT);

			NotifyBoss(sResponse, DURTEEN97);
			OnButtonFinished(FALSE);
		}
	}

	// add random play function (5676)
	if (m_bRandomAudio == TRUE && (nTurncount > m_nRandomTime + kRandomPlayTime)) {
		int nTrackSelection = g_engine->getRandomNumber() % (NUM_MUSICAL_SCORES - 1);

		// Now start playing...
		if (g_stAudioSetting[nTrackSelection].m_pMidiTrack != nullptr) {
			g_stAudioSetting[nTrackSelection].m_pMidiTrack->Play();
		}
		m_nRandomTime += kRandomPlayTime;
	}
}

// this dynamically updates the turncount on the screen, must
// know which line to update though.
void SrafComputer::DisplayTurnCount(int nLineNo) {
	CBagVar *pVar = VARMNGR->GetVariable("SRATURNCOUNT");
	Assert(pVar != nullptr);

	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString  sStr(szLocalBuff, 256);
	int         nCurrentTime = pVar->GetNumValue();

	Common::sprintf_s(szLocalBuff,
	                  "CURRENT TIME: %02d:%02d",
	                  nCurrentTime / 100,
	                  nCurrentTime % 100);

	// Get whatever is on that line...
	if (m_pLBox) {
		CBofString cStr = m_pLBox->GetText(nLineNo);

		// reset the value on that line.
		if (cStr.GetLength() != 0) {
			m_pLBox->SetText(nLineNo, sStr);

#if BOF_WINDOWS
			m_pLBox->RepaintItem(nLineNo);
#endif
		}
	}

	// Keep track of the line number that it's stored on... might need this again.
	gTurncountLineNo = nLineNo;
}

CBofBitmap *SrafComputer::GetComputerBackdrop() {
	if (m_pHead == nullptr) {
		return nullptr;
	}

	SrafComputer *srafComp = m_pHead;
	return srafComp->GetBackdrop();
}

// Sraffin text screen classes.  This class is used to read a text file in
// and post it to the full screen with a "done" button in the lower right
// hand corner of the screen.

CBofWindow *SrafTextScreen::m_pSaveActiveWin = nullptr;

SrafTextScreen::SrafTextScreen(CBofString &sStr) {

	// Initializations (definitely in the constructor)

	m_pTextBox = nullptr;
	m_pOKButton = nullptr;
	m_pTextFile = new CBofFile(sStr, CBF_BINARY | CBF_READONLY);
	Assert(m_pTextFile != nullptr);

	m_pszFileName = new CBofString(sStr.GetBuffer());
	Assert(m_pszFileName != nullptr);

	m_pSaveBackground = new CBofBitmap(gTextWindow.Width(), gTextWindow.Height(), (CBofPalette *)nullptr, FALSE);
	Assert(m_pSaveBackground != nullptr);

	// save the currently active window
	m_pSaveActiveWin = GetActiveWindow();
}

int SrafTextScreen::CreateTextScreen(CBofWindow *pParent) {
	CBofRect    cRect;
	int         nLength;
	char *pszBuf = nullptr;
	CBofString  cStr;
	//CBofBitmap  *pBmp;

	char        szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString  sStr(szLocalBuff, 256);

#if BOF_MAC
	::GetPort(&m_pSavePort);
#endif
	// Create our parent window

#if BOF_MAC
	Create("Sraffin Text", &gTextWindow, nullptr, 0);
#else
	Create("Sraffin Text", &gTextWindow, pParent, 0);
#endif

	// Save the background
	if (m_pSaveBackground) {
		m_pSaveBackground->CaptureScreen(this, &gTextWindow);
	}

	// Needs the computer bitmap for a backdrop
	SetBackdrop(SrafComputer::GetComputerBackdrop(), FALSE);

	Assert(m_pBackdrop != nullptr);
	CBofPalette *pPal = m_pBackdrop->GetPalette();
	// Create our OK button

	if ((m_pOKButton = new CBofBmpButton) != nullptr) {

		CBofBitmap *pUp, *pDown, *pFocus, *pDis;

		pUp = LoadBitmap(BuildSrafDir(g_stButtons[DONE_BUTTON].m_pszUp), pPal);
		pDown = LoadBitmap(BuildSrafDir(g_stButtons[DONE_BUTTON].m_pszDown), pPal);
		pFocus = LoadBitmap(BuildSrafDir(g_stButtons[DONE_BUTTON].m_pszFocus), pPal);
		pDis = LoadBitmap(BuildSrafDir(g_stButtons[DONE_BUTTON].m_pszDisabled), pPal);

		m_pOKButton->LoadBitmaps(pUp, pDown, pFocus, pDis);

#if BOF_MAC
		// make this our own custom window such that no frame is drawn
		// around the window/button
		m_pOKButton->SetCustomWindow(TRUE);
#endif
		m_pOKButton->Create(g_stButtons[DONE_BUTTON].m_pszName,
		                    g_stButtons[DONE_BUTTON].m_nLeft,
		                    g_stButtons[DONE_BUTTON].m_nTop,
		                    g_stButtons[DONE_BUTTON].m_nWidth,
		                    g_stButtons[DONE_BUTTON].m_nHeight,
		                    this,
		                    g_stButtons[DONE_BUTTON].m_nID);
	}

	Assert(m_pOKButton != nullptr);

	// Read the text file into memory...
	nLength = m_pTextFile->GetLength();

	if ((pszBuf = (char *)BofAlloc(nLength + 1)) != nullptr) {
		BofMemSet(pszBuf, 0, nLength + 1);
		m_pTextFile->Read(pszBuf, nLength);
#if BOF_MAC
		StrReplaceChar(pszBuf, '\r', ' ');
#endif
		cStr = pszBuf;
		BofFree(pszBuf);
		m_pTextFile->Close();
	} else {
		ReportError(ERR_MEMORY, "Could not read %s into memory", m_pszFileName);
	}

	Assert(cStr.GetLength() != 0);

	//  Create our text box.
	cRect.SetRect(gCompDisplay.left, gCompDisplay.top, gCompDisplay.right, gCompDisplay.bottom);

	if ((m_pTextBox = new CBofTextBox(this, &cRect, cStr)) != nullptr) {
		m_pTextBox->SetPageLength(24);
		m_pTextBox->SetColor(RGB(255, 255, 255));
		m_pTextBox->SetFont(FONT_MONO);
		m_pTextBox->SetPointSize(FONT_14POINT);
	} else {
		ReportError(ERR_MEMORY, "Unable to allocate a CBofTextBox");
	}

	return ERR_NONE;
}

void SrafTextScreen::DisplayTextScreen() {
	Show();

	if (GetBackdrop())
		PaintBackdrop();

	SetFocus();

	m_pOKButton->Show();

#if BOF_MAC
	//m_pOKButton->Paint ();
#endif

	m_pTextBox->Display();
}

SrafTextScreen::~SrafTextScreen() {
	// Restore the background if we have one.
#if BOF_MAC
	::SetPort(m_pSavePort);

	if (m_pSaveBackground) {
		m_pSaveBackground->Paint(this, &gTextWindow);
	}
#endif

	// Trash everything!
	if (m_pTextBox) {
		delete m_pTextBox;
		m_pTextBox = nullptr;
	}

	if (m_pOKButton) {
		delete m_pOKButton;
		m_pOKButton = nullptr;
	}

	if (m_pTextFile) {
		delete m_pTextFile;
		m_pTextFile = nullptr;
	}

	if (m_pszFileName) {
		delete m_pszFileName;
		m_pszFileName = nullptr;
	}

	if (m_pSaveBackground) {
		delete m_pSaveBackground;
		m_pSaveBackground = nullptr;
	}

	// Make sure the underlying window gets focus back
	if (m_pSaveActiveWin) {
		m_pSaveActiveWin->SetFocus();
	}

	// display text screen is gone by-bye...
#if BOF_MAC
	gTextScreenFrontmost = FALSE;
#endif
}

// Called to delete our text object

void SrafTextScreen::OnClose() {
#if BOF_WINDOWS
	CBofDialog::OnClose();
#endif

	// Set the backdrop to nullptr so that it doesn't get trashed (we borrowed it from
	// the sraf computer.
	ClearBackdrop();

#if BOF_MAC
	delete this;
#endif
}


#if BOF_WINDOWS
void SrafTextScreen::OnPaint(CBofRect * /*pRect*/) {
	DisplayTextScreen();

	ValidateAnscestors();
}
#endif


void SrafTextScreen::OnBofButton(CBofObject *pObject, int nState) {
	Assert(IsValidObject(this));
	Assert(pObject != nullptr);

	CBofButton *pButton;

	pButton = (CBofButton *)pObject;

	if (nState == BUTTON_CLICKED) {
		switch (pButton->GetControlID()) {
		case DONE_BUTTON:
			Close();
			SrafComputer::SetMainScreen();
			break;
		default:
			break;
		}
	}
}

// Had to fix how MACROREPLACE is used - The value of $SBARDIR can not be
// saved because of install issues.

//
// Where are the SRAFFAN assets?
//

const char *BuildAudioDir(const char *pszFile) {
	return formPath(SRAFAUDIODIR, pszFile);
}

const char *BuildSrafDir(const char *pszFile) {
	return formPath(SRAFDIR, pszFile);
}

const char *BuildMaleSrafDir(const char *pszFile) {
	Assert(pszFile != nullptr);

	static char szBuf[MAX_DIRPATH];

	Common::sprintf_s(szBuf, "%s%s%s", SRAFMALEDIR, PATH_DELIMETER, pszFile);

	// Male sraffan voices

	CBofString sSrafDir(szBuf, MAX_DIRPATH);
	MACROREPLACE(sSrafDir);

	return &szBuf[0];
}

const char *BuildFemaleSrafDir(const char *pszFile) {
	Assert(pszFile != nullptr);

	static char szBuf[MAX_DIRPATH];

	Common::sprintf_s(szBuf, "%s%s%s", SRAFFEMALEDIR, PATH_DELIMETER, pszFile);

	// Female sraffan voices

	CBofString sSrafDir(szBuf, MAX_DIRPATH);
	MACROREPLACE(sSrafDir);

	return &szBuf[0];
}

void SrafComputer::RestoreSraffanVars() {
	// Retrieve sellers terms
	CBagVar *pVar;
	int nNumToRestore;
	const char *pVarName = nullptr;
	int i, j;

	// Save sellers terms
	SELLERS nSellerID = IRK4;
	nNumToRestore = 3;

	for (i = 0; i < nNumToRestore; i++) {
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

		if ((pVar = VARMNGR->GetVariable(pVarName)) != nullptr) {
			g_stSellerNames[nSellerID].m_nAmount = pVar->GetNumValue();
		}
	}

	// Retrieve buyer terms

	nNumToRestore = 5;
	BUYERS nBuyerID = PYLON3;
	int nMineralID = 0;

	for (i = 0; i < nNumToRestore; i++) {
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

		if ((pVar = VARMNGR->GetVariable(pVarName)) != nullptr) {
			g_stBuyerBids[nBuyerID].m_nMineralVal[nMineralID] = pVar->GetNumValue();

			// total each one...
			int nBidSum = 0;
			for (j = 0; j < NUM_MINERALS; j++) {
				nBidSum += g_stBuyerBids[nBuyerID].m_nMineralVal[j];
			}

			g_stBuyerBids[nBuyerID].m_nBidSum = nBidSum;
		}
	}

	// save the list of who is in on this deal... use a bit to indicate if they are in or not.
	uint32       nBuyersMask = 0;
	if ((pVar = VARMNGR->GetVariable("BUYERSMASK")) != nullptr) {
		nBuyersMask = pVar->GetNumValue();
	}

	for (i = 0; i < NUM_BUYERS; i++) {
		if (nBuyersMask & (1 << i)) {
			g_stBuyerBids[i].m_bAccept = TRUE;
		} else {
			g_stBuyerBids[i].m_bAccept = FALSE;
		}
	}

	// Restore the list of who is available and who is not.
	// Do this using a mask.
	uint32 nAvailMask = 0;
	uint32 nIndex = 0;

	if ((pVar = VARMNGR->GetVariable("AVAILABLEMASK")) != nullptr) {
		nAvailMask = pVar->GetNumValue();
	}

	for (i = 0; i < NUM_BUYERS; i++, nIndex++) {
		if (nAvailMask & (1 << nIndex)) {
			g_stBuyerBids[i].m_bAvailable = FALSE;
		} else {
			g_stBuyerBids[i].m_bAvailable = TRUE;
		}
	}

	for (i = 0; i < NUM_SELLERS; i++, nIndex++) {
		if (nAvailMask & (1 << nIndex)) {
			g_stSellerNames[i].m_bAvailable = FALSE;
		} else {
			g_stSellerNames[i].m_bAvailable = TRUE;
		}
	}

	for (i = 0; i < NUM_OTHER_PARTYS; i++, nIndex++) {
		if (nAvailMask & (1 << nIndex)) {
			g_stOtherPartys[i].m_bAvailable = FALSE;
		} else {
			g_stOtherPartys[i].m_bAvailable = TRUE;
		}
	}

	Assert(nIndex < 32);

	// Restore other party's info
	OTHERPARTYS nOtherID = SWONZA5;

	nNumToRestore = 3;
	for (i = 0; i < nNumToRestore; i++) {
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

		if ((pVar = VARMNGR->GetVariable(pVarName)) != nullptr) {
			g_stOtherPartys[nOtherID].m_nPaymentAmount = pVar->GetNumValue();
		}
	}

	// Retrieve swonza's state
	if ((pVar = VARMNGR->GetVariable("SWONZAENLIGHTENED")) != nullptr) {
		m_bSwonzaEnlightened = pVar->GetNumValue();
	}

	// Mark each of our staffers as available before we start.
	for (i = 0; i < NUM_STAFFERS; i++) {
		g_staffers[i].m_bAvailable = TRUE;
	}

	// Now the real pain in the ass... restoring team information...
	nNumToRestore = 6;

	DispatchedTeamItem teamListItem;
	char szFLAGS[20];
	char szMEETWITH[20];
	char szDISPATCHTIME[20];
	char szCAPTAIN[20];
	char szMEETINGTIME[20];

	for (i = 0; i < nNumToRestore; i++) {
		// Build the variable names with default values.
		Common::sprintf_s(szFLAGS, "%s%d%s", "TEAM", i + 1, "FLAGS");
		Common::sprintf_s(szMEETWITH, "%s%d%s", "TEAM", i + 1, "MEETWITH");
		Common::sprintf_s(szDISPATCHTIME, "%s%d%s", "TEAM", i + 1, "DISPATCHTIME");
		Common::sprintf_s(szCAPTAIN, "%s%d%s", "TEAM", i + 1, "CAPTAIN");
		Common::sprintf_s(szMEETINGTIME, "%s%d%s", "TEAM", i + 1, "MEETINGTIME");

		// Restore the whole block...

		if ((pVar = VARMNGR->GetVariable(szFLAGS)) != nullptr) {
			teamListItem.m_nFlags = pVar->GetNumValue();

			// If we have a meeting going on, then mark that
			// sraffan staffer as unavailable.
			if (teamListItem.m_nFlags != 0) {
				for (j = 0; j < NUM_STAFFERS; j++) {
					if ((1 << (j + 3)) & teamListItem.m_nFlags) {
						g_staffers[j].m_bAvailable = FALSE;
					}
				}
			}
		}
		if ((pVar = VARMNGR->GetVariable(szMEETWITH)) != nullptr) {
			teamListItem.m_nMeetWithID = pVar->GetNumValue();
		}
		if ((pVar = VARMNGR->GetVariable(szDISPATCHTIME)) != nullptr) {
			teamListItem.m_nDispatchTime = pVar->GetNumValue();
		}
		if ((pVar = VARMNGR->GetVariable(szCAPTAIN)) != nullptr) {
			teamListItem.m_nTeamCaptain = pVar->GetNumValue();
		}
		if ((pVar = VARMNGR->GetVariable(szMEETINGTIME)) != nullptr) {
			teamListItem.m_nMeetingTime = pVar->GetNumValue();
		}

		// If we have a list established, then add this item to it if there's anything
		// there.
		if (m_pTeamList == nullptr) {
			m_pTeamList = new CBofList<DispatchedTeamItem>;
			Assert(m_pTeamList != nullptr);
		}

		if (teamListItem.m_nFlags != 0) {
			m_pTeamList->AddToTail(teamListItem);
		}
	}


	// Save the successful and unsuccesful meeting status.  Here's how we're gonna do
	// this.  For each negotiating target, we will save a bit in "METWITH", 0 => no
	// meeting occurred, 1 => a meeting happened with this dude.  There will be a
	// second variable, "MEETINGRESULTS", there will be a bit that is synonamous with
	// the "METWITH" var indicating the result of the meeting, i.e. 0=>meeting was a
	// success and 1=> meeting was a failure.
	//
	// Now get the variables
	int nMetWithVal = 0;
	int nMeetingResultVal = 0;

	if ((pVar = VARMNGR->GetVariable("METWITH")) != nullptr) {
		nMetWithVal = pVar->GetNumValue();
	}

	if ((pVar = VARMNGR->GetVariable("MEETINGRESULTS")) != nullptr) {
		nMeetingResultVal = pVar->GetNumValue();
	}


	int nBitNo = 0;
	// Restore seller meeting history
	for (i = 0; i < NUM_SELLERS; i++, nBitNo++) {
		if (nMetWithVal & (1 << nBitNo)) {
			g_stSellerNames[i].m_nMeetingResult = (((1 << nBitNo) & nMeetingResultVal) ? GOODMEETING : BADMEETING);
		} else {
			g_stSellerNames[i].m_nMeetingResult = 0;
		}
	}

	// Save buyer meeting history
	for (i = 0; i < NUM_BUYERS; i++, nBitNo++) {
		if (nMetWithVal & (1 << nBitNo)) {
			g_stBuyerBids[i].m_nMeetingResult = (((1 << nBitNo) & nMeetingResultVal) ? GOODMEETING : BADMEETING);
		} else {
			g_stBuyerBids[i].m_nMeetingResult = 0;
		}
	}

	// Save other party meeting history
	for (i = 0; i < NUM_OTHER_PARTYS; i++, nBitNo++) {
		if (nMetWithVal & (1 << nBitNo)) {
			g_stOtherPartys[i].m_nMeetingResult = (((1 << nBitNo) & nMeetingResultVal) ? GOODMEETING : BADMEETING);
		} else {
			g_stOtherPartys[i].m_nMeetingResult = 0;
		}
	}

	// all done!
}

void SrafComputer::SaveSraffanVars() {
	int nNumToSave;
	const char *pVarName = nullptr;
	CBagVar *pVar;
	int i;

	// Save sellers terms
	SELLERS nSellerID = IRK4;
	nNumToSave = 3;

	for (i = 0; i < nNumToSave; i++) {
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

		if ((pVar = VARMNGR->GetVariable(pVarName)) != nullptr) {
			pVar->SetValue(g_stSellerNames[nSellerID].m_nAmount);
		}
	}

	// Retrieve buyer terms
	nNumToSave = 5;
	BUYERS nBuyerID = PYLON3;
	int nMineralID = 0;

	for (i = 0; i < nNumToSave; i++) {
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

		if ((pVar = VARMNGR->GetVariable(pVarName)) != nullptr) {
			pVar->SetValue(g_stBuyerBids[nBuyerID].m_nMineralVal[nMineralID]);
		}
	}

	// save the list of who is in on this deal... use a bit to indicate if they are in or not.
	uint32       nBuyersMask = 0;
	for (i = 0; i < NUM_BUYERS; i++) {
		if (g_stBuyerBids[i].m_bAccept == TRUE) {
			nBuyersMask |= (1 << i);
		}
	}

	if ((pVar = VARMNGR->GetVariable("BUYERSMASK")) != nullptr) {
		pVar->SetValue(nBuyersMask);
	}

	// Save the list of who is available and who is not.
	// Do this using a mask.
	uint32       nAvailMask = 0;
	uint32       nIndex = 0;
	for (i = 0; i < NUM_BUYERS; i++, nIndex++) {
		if (g_stBuyerBids[i].m_bAvailable == FALSE) {
			nAvailMask |= (1 << nIndex);
		}
	}

	for (i = 0; i < NUM_SELLERS; i++, nIndex++) {
		if (g_stSellerNames[i].m_bAvailable == FALSE) {
			nAvailMask |= (1 << nIndex);
		}
	}

	for (i = 0; i < NUM_OTHER_PARTYS; i++, nIndex++) {
		if (g_stOtherPartys[i].m_bAvailable == FALSE) {
			nAvailMask |= (1 << nIndex);
		}
	}

	Assert(nIndex < 32);

	if ((pVar = VARMNGR->GetVariable("AVAILABLEMASK")) != nullptr) {
		pVar->SetValue(nAvailMask);
	}

	// Save other party's info

	OTHERPARTYS nOtherID = SWONZA5;

	nNumToSave = 3;
	for (i = 0; i < nNumToSave; i++) {
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

		if ((pVar = VARMNGR->GetVariable(pVarName)) != nullptr) {
			pVar->SetValue(g_stOtherPartys[nOtherID].m_nPaymentAmount);
		}
	}

	// Save swonza's state
	if ((pVar = VARMNGR->GetVariable("SWONZAENLIGHTENED")) != nullptr) {
		pVar->SetValue(m_bSwonzaEnlightened);
	}

	// Now the real pain in the ass... saving team information...

	nNumToSave = 6;

	DispatchedTeamItem  teamListItem;
	int                 nOutstandingTeams = 0;
	char                szFLAGS[20];
	char                szMEETWITH[20];
	char                szDISPATCHTIME[20];
	char                szCAPTAIN[20];
	char                szMEETINGTIME[20];

	// Get the number of outstanding teams, we don't want to overkill
	if (m_pTeamList) {
		nOutstandingTeams = m_pTeamList->GetCount();
	}

	for (i = 0; i < nNumToSave; i++) {
		// Build the variable names with default values.
		Common::sprintf_s(szFLAGS, "%s%d%s", "TEAM", i + 1, "FLAGS");
		Common::sprintf_s(szMEETWITH, "%s%d%s", "TEAM", i + 1, "MEETWITH");
		Common::sprintf_s(szDISPATCHTIME, "%s%d%s", "TEAM", i + 1, "DISPATCHTIME");
		Common::sprintf_s(szCAPTAIN, "%s%d%s", "TEAM", i + 1, "CAPTAIN");
		Common::sprintf_s(szMEETINGTIME, "%s%d%s", "TEAM", i + 1, "MEETINGTIME");

		if (i < nOutstandingTeams) {
			teamListItem = m_pTeamList->GetNodeItem(i);
		} else {
			teamListItem.m_nFlags = 0;
			teamListItem.m_nMeetWithID = 0;
			teamListItem.m_nDispatchTime = 0;
			teamListItem.m_nTeamCaptain = 0;
			teamListItem.m_nMeetingTime = 0;
		}

		if ((pVar = VARMNGR->GetVariable(szFLAGS)) != nullptr) {
			pVar->SetValue(teamListItem.m_nFlags);
		}
		if ((pVar = VARMNGR->GetVariable(szMEETWITH)) != nullptr) {
			pVar->SetValue(teamListItem.m_nMeetWithID);
		}
		if ((pVar = VARMNGR->GetVariable(szDISPATCHTIME)) != nullptr) {
			pVar->SetValue(teamListItem.m_nDispatchTime);
		}
		if ((pVar = VARMNGR->GetVariable(szCAPTAIN)) != nullptr) {
			pVar->SetValue(teamListItem.m_nTeamCaptain);
		}
		if ((pVar = VARMNGR->GetVariable(szMEETINGTIME)) != nullptr) {
			pVar->SetValue(teamListItem.m_nMeetingTime);
		}
	}

	// Save the successful and unsuccesful meeting status.  Here's how we're gonna do
	// this.  For each negotiating target, we will save a bit in "METWITH", 0 => no
	// meeting occurred, 1 => a meeting happened with this dude.  There will be a
	// second variable, "MEETINGRESULTS", there will be a bit that is synonamous with
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
	for (i = 0; i < NUM_SELLERS; i++, nBitNo++) {
		if (g_stSellerNames[i].m_nMeetingResult != NOMEETING) {
			nMetWithVal |= (1 << nBitNo);
			nMeetingResultVal |= (g_stSellerNames[i].m_nMeetingResult == GOODMEETING ? 1 << nBitNo : 0);
		}
	}

	// Save buyer meeting history
	for (i = 0; i < NUM_BUYERS; i++, nBitNo++) {
		if (g_stBuyerBids[i].m_nMeetingResult != NOMEETING) {
			nMetWithVal |= (1 << nBitNo);
			nMeetingResultVal |= (g_stBuyerBids[i].m_nMeetingResult == GOODMEETING ? 1 << nBitNo : 0);
		}
	}

	// Save other party meeting history
	for (i = 0; i < NUM_OTHER_PARTYS; i++, nBitNo++) {
		if (g_stOtherPartys[i].m_nMeetingResult != NOMEETING) {
			nMetWithVal |= (1 << nBitNo);
			nMeetingResultVal |= (g_stOtherPartys[i].m_nMeetingResult == GOODMEETING ? 1 << nBitNo : 0);
		}
	}

	// Now save the variables
	if ((pVar = VARMNGR->GetVariable("METWITH")) != nullptr) {
		pVar->SetValue(nMetWithVal);
	}

	if ((pVar = VARMNGR->GetVariable("MEETINGRESULTS")) != nullptr) {
		pVar->SetValue(nMeetingResultVal);
	}

	// all done!
}

void SrafComputer::SetMeetingResult(int nFlags, int nMetWith, bool bSucceeded) {
	if (nFlags & mSeller) {
		g_stSellerNames[nMetWith].m_nMeetingResult = (bSucceeded ? GOODMEETING : BADMEETING);
	}

	if (nFlags & mBuyer) {
		g_stBuyerBids[nMetWith].m_nMeetingResult = (bSucceeded ? GOODMEETING : BADMEETING);
	}

	if (nFlags & mOtherParty) {
		g_stOtherPartys[nMetWith].m_nMeetingResult = (bSucceeded ? GOODMEETING : BADMEETING);
	}
}

} // namespace SpaceBar
} // namespace Bagel
