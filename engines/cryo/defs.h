#include <stddef.h>
#include <stdlib.h>
#include "cryolib.h"

#define GetElem(array, idx) \
	( (char*)(array) + PLE16((idx) * 2 + (char*)(array)) )
/*
static inline void* AGetElem(unsigned char *arr, short index)
{
    unsigned char *p = arr + num * 2;
    unsigned char o0 = *p++;
    unsigned char o1 = *p++;
    unsigned short ofs = (o1 << 8) | o0;
    return arr + ofs;
}
*/

///////////////// Game defs

#define FONT_HEIGHT 9


/*
Glossary
  room      - a single game world's screen. referenced by 16-bit number 0xAALL, where AA - area# and LL - location#
  area      - geographic area - Mo, Chamaar, etc
  location  - coordinates of particular room in an area. usually in form of 0xXY, where X - map row, Y - map column
  character - an unique character (human or dino.) Has their own voice/dialog lines
  person    - instance of a character. Usually tied to specific room, but some may travel with you
  party     - a group of characters that travel with you
  object    - inventory item
  icon      - clickable rectangle with some action tied to it
  dialog    - a set of of dialog lines for character. further divided by categories and each entry may have associated
              condition to be validated
  global    - game-wide storage area. must be preserved when saving/loading
  phase     - current story progress. Incremented by 1 for minor events, by 0x10 for major advancements
*/

enum Phases {
	phNewGame = 0
};

namespace Areas {
enum Areas {
	arMo = 1,
	arTausCave,
	arChamaar,
	arUluru,
	arKoto,
	arTamara,
	arCantura,
	arShandovra,
	arNarimsCave,
	arEmbalmersCave,
	arWhiteArch,
	arMoorkusLair
};
}

#define MKRM(a,l) (((a) << 8) | (l))

enum OBJECT {
	OBJ_0,
	OBJ_1,
	OBJ_2,
	OBJ_3,
	OBJ_4,
	OBJ_PRISME, // 5
	OBJ_6,
	OBJ_7,
	OBJ_OEUF,   // 8
	OBJ_9,
	OBJ_10,
	OBJ_CHAMPB, // 11
	OBJ_CHAMPM, // 12
	OBJ_COUTEAU,    // 13
	OBJ_NIDV,   // 14
	OBJ_NIDO,   // 15
	OBJ_OR,     // 16
	OBJ_17,
	OBJ_18,
	OBJ_SOLEIL, // 19
	OBJ_CORNE,  // 20
	OBJ_21,
	OBJ_22,
	OBJ_23,
	OBJ_24,
	OBJ_25,
	OBJ_26,
	OBJ_27,
	OBJ_28,
	OBJ_29,
	OBJ_30,
	OBJ_31,
	OBJ_32,
	OBJ_33,
	OBJ_34,
	OBJ_35,
	OBJ_36,     // 36 is 1st plaque, 6 total
	OBJ_37,
	OBJ_PLAQUE, // 38
	OBJ_39,
	OBJ_40,
	OBJ_41
};

namespace Objects {
enum Objects {
	obNone,
	obWayStone,
	obShell,
	obTalisman,
	obTooth,
	obPrism,    // 5
	obFlute,
	obApple,
	obEgg,      // 8
	obRoot,
	obUnused10,
	obShroom,   // 11
	obBadShroom,    // 12
	obKnife,    // 13
	obNest, // 14
	obFullNest, // 15
	obGold,     // 16
	obMoonStone,
	obBag,
	obSunStone, // 19
	obHorn, // 20
	obSword,

	obMaskOfDeath,
	obMaskOfBonding,
	obMaskOfBirth,

	obEyeInTheStorm,    // 25
	obSkyHammer,
	obFireInTheClouds,
	obWithinAndWithout,
	obEyeInTheCyclone,
	obRiverThatWinds,

	obTrumpet,      // 31
	obUnused32,
	obDrum,
	obUnused34,
	obUnused35,
	obRing,

	obTablet1,      // 37 is 1st plaque, 6 total
	obTablet2,
	obTablet3,  // 39
	obTablet4,
	obTablet5,
	obTablet6
};
}

enum PERSO {
	PER_ROI = 0,
	PER_DINA,       // 0x12
	PER_THOO,       // 0x24
	PER_MONK,       // 0x36
	PER_BOURREAU,   // 0x48
	PER_MESSAGER,   // 0x5A
	PER_MANGO,      // 0x6C
	PER_EVE,        // 0x7E
	PER_AZIA,       // 0x90
	PER_MAMMI,      // 0xA2
	PER_MAMMI_1,    // 0xB4
	PER_MAMMI_2,    // 0xC6
	PER_MAMMI_3,    // 0xD8
	PER_MAMMI_4,    // 0xEA
	PER_MAMMI_5,    // 0xFC
	PER_MAMMI_6,    // 0x10E
	PER_BAMBOO,     // 0x120
	PER_KABUKA,     // 0x132
	PER_GARDES,     // 0x144
	PER_UNKN_156,   // 0x156
	PER_FISHER,     // 0x168
	PER_MORKUS,     // 0x17A
	PER_UNKN_18C,   // 0x18C
	PER_UNKN_19E,   // 0x19E
	PER_UNKN_1B0,   // 0x1B0
	PER_UNKN_1C2,   // 0x1C2
	PER_UNKN_1D4,   // 0x1D4
	PER_UNKN_1E6,   // 0x1E6
	PER_UNKN_1F8,   // 0x1F8
	PER_UNKN_20A,   // 0x20A
	PER_UNKN_21C,   // 0x21C
	PER_UNKN_22E,   // 0x22E
	PER_UNKN_240,   // 0x240
	PER_UNKN_252,   // 0x252
	PER_UNKN_264,   // 0x264
	PER_UNKN_276,   // 0x276
	PER_UNKN_288,   // 0x288
	PER_UNKN_29A,   // 0x29A
	PER_UNKN_2AC,   // 0x2AC
	PER_UNKN_2BE,   // 0x2BE
	PER_UNKN_2D0,   // 0x2D0
	PER_UNKN_2E2,   // 0x2E2
	PER_UNKN_2F4,   // 0x2F4
	PER_UNKN_306,   // 0x306
	PER_UNKN_318,   // 0x318
	PER_UNKN_32A,   // 0x32A
	PER_UNKN_33C,   // 0x33C
	PER_UNKN_34E,   // 0x34E
	PER_UNKN_360,   // 0x360
	PER_UNKN_372,   // 0x372
	PER_UNKN_384,   // 0x384
	PER_UNKN_396,   // 0x396
	PER_UNKN_3A8,   // 0x3A8
	PER_UNKN_3BA,   // 0x3BA
	PER_UNKN_3CC,   // 0x3CC
	PER_UNKN_3DE,   // 0x3DE
	PER_UNKN_3F0,   // 0x3F0
	PER_UNKN_402    // 0x402
};

namespace PersonId {
enum PersonId {
	pidGregor = 0,                          // The King
	pidDina,                                // Pink dino
	pidTau,                                 // Late grandpa
	pidMonk,                                // Old wizard
	pidJabber,                              // Executioner
	pidEloi,                                // Evergreen ptero
	pidMungo,                               // Dina's husband
	pidEve,                                 // Blonde girl
	pidShazia,                              // Big boobs sis
	pidLeadersBegin,                        // 9
	pidChongOfChamaar = pidLeadersBegin,    // Dogface
	pidKommalaOfKoto,                       // Clones
	pidUlanOfUlele,                         // Shaman
	pidCabukaOfCantura,                     // Stone people
	pidMarindaOfEmbalmers,                  // Gods
	pidFuggOfTamara,                        // Boar-like
	pidThugg,                               // Bodyguard
	pidNarrator,                            // 16, Old Eloi, also BGM
	pidNarrim,                              // Sea snake
	pidMorkus,                              // Vicious tyran
	pidDinosaur,                            // different species of friendly dino
	pidEnemy                                // different species of enemy dino
};
}

// person in room mask bits
namespace PersonMask {
enum PersonMask {
	pmGregor = 1,
	pmDina   = 2,
	pmTau    = 4,
	pmMonk   = 8,
	pmJabber = 0x10,
	pmEloi   = 0x20,
	pmMungo  = 0x40,
	pmEve    = 0x80,
	pmShazia = 0x100,
	pmLeader = 0x200,   // valley tribe leader
	pmThugg  = 0x400,
	pmQuest  = 0x800,   // special quest person
	pmDino   = 0x1000,
	pmEnemy  = 0x2000,
	pmMorkus = 0x4000
};
}

namespace PersonFlags {
enum PersonFlags {
	pfType0 = 0,
	pftTyrann,
	pfType2,
	pfType3,
	pfType4,
	pfType5,
	pfType6,
	pfType7,
	pfType8,
	pftMosasaurus,
	pftTriceraptor,
	pftVelociraptor,
	pfType12,
	pfType13,
	pfType14,
	pfType15,
	pfTypeMask = 0xF,
	pf10 = 0x10,
	pf20 = 0x20,
	pfInParty = 0x40,
	pf80 = 0x80
};
}

#pragma pack(push, 1)
struct perso_t {
	unsigned short  roomNum;    // room this person currently in
	unsigned short  actionId;   // TODO: checkme
	unsigned short  party;      // party bit mask
	unsigned char   id;         // character
	unsigned char   flags;      // flags and kind
	unsigned char   roomBankIdx;// index in kPersoRoomBankTable for specific room banks
	unsigned char   bank;       // sprite bank
	unsigned short  items;      // inventory
	unsigned short  powers;     // obj of power bitmask
	unsigned char   targetLoc;  // For party member this is mini sprite index
	unsigned char   lastLoc;    // For party member this is mini sprite x offset
	unsigned char   speed;      // num ticks per step
	unsigned char   steps;      // current ticks
};
typedef struct perso_t perso_t;

struct phase_t {
	short           id;
	void (EdenGameImpl::*disp)();
};
typedef struct phase_t phase_t;

namespace ObjectFlags {
enum ObjectFlags {
	ofFlag1 = 1,
	ofInHands = 2       // Currently holding this object in hands
};
}

#define MAX_OBJECTS 42
struct object_t {
	unsigned char   id;
	unsigned char   flags;
	int             locations;      // index in kObjectLocations
	short           itemMask;
	short           powerMask;          // object of power bitmask
	short           count;
};
typedef struct object_t object_t;

namespace DialogFlags {
enum DialogFlags {
	df20 = 0x20,
	dfRepeatable = 0x40,
	dfSpoken = 0x80
};
}

namespace DialogType {
enum DialogType {
	dtTalk = 0,
	dtDinoAction,
	dtDinoItem,
	dtItem,
	dtEvent,
	dtInspect,
	dtHint
};
}

struct dial_t {
	char        flags;          // 0-3 - action index, 4 - highest bit of contidion index, rest is DialogFlags
	char        condNumLow;     // condition index low bits
	char        textCondHiMask; // 0-1 text index hi bits, 2-5 - perso mask num, 6-7 condition index hi bits
	char        textNumLow;     // text line index low bits
};
typedef struct dial_t dial_t;

struct tape_t {
	short       textNum;
	perso_t     *perso;
	short       party;
	short       roomNum;
	short       bgBankNum;
	dial_t      *dialog;
};
typedef struct tape_t tape_t;

struct suiveur_t {      // Characters on Mirror screen
	char        id;         // character
	char        image;      // sprite number
	short       sx;
	short       sy;
	short       ex;
	short       ey;
	short       bank;
	short       ff_C;
	short       ff_E;
};
typedef struct suiveur_t suiveur_t;

struct icon_t {
	short       sx;
	short       sy;
	short       ex;
	short       ey;
	unsigned short  cursor_id;          // & 0x8000 - inactive/hidden
	unsigned int    action_id;
	unsigned int    object_id;
};
typedef struct icon_t icon_t;

struct goto_t {
	unsigned char   areaNum;    // target area
	unsigned char   curAreaNum; // current area
	unsigned char   departVid;
	unsigned char   travelTime;     // time to skip while in travel
	unsigned char   arriveVid;
};
typedef struct goto_t goto_t;

namespace RoomFlags {
enum RoomFlags {
	rf01 = 1,
	rf02 = 2,
	rf04 = 4,
	rf08 = 8,
	rfPanable = 0x10,
	rfHasCitadel = 0x20,
	rf40 = 0x40,
	rf80 = 0x80
};
}

struct room_t {
	unsigned char   ff_0;
	unsigned char   exits[4];   //TODO: signed?
	unsigned char   flags;
	unsigned short  bank;
	unsigned short  party;
	unsigned char   level;      // Citadel level
	unsigned char   video;
	unsigned char   location;
	unsigned char   background; // bg/mirror image number (relative)
};
typedef struct room_t room_t;

namespace AreaFlags {
enum AreaFlags {
	afFlag1 = 1,
	afFlag2 = 2,
	afFlag4 = 4,
	afFlag8 = 8,
	afGaveGold = 0x10,
	afFlag20 = 0x20,

	HasTriceraptors = 0x100,
	HasVelociraptors = 0x200,
	HasTyrann = 0x400,

	TyrannSighted = 0x4000,
	afFlag8000 = 0x8000
};
}

namespace AreaType {
enum AreaType {
	atCitadel = 1,
	atValley = 2,
	atCave = 3
};
}

struct area_t {
	unsigned char   num;
	unsigned char   type;
	unsigned short  flags;
	unsigned short  firstRoomIndex;
	unsigned char   citadelLevel;
	unsigned char   salNum;
	room_t          *citadelRoom;
	short           visitCount;
};
typedef struct area_t area_t;

namespace ValleyNews {
enum ValleyNews {
	vnAreaMask = 0xF,

	vnTriceraptorsIn = 0x10,
	vnVelociraptorsIn = 0x20,
	vnTyrannIn = 0x30,
	vnTyrannLost = 0x40,
	vnCitadelLost = 0x50,
	vnVelociraptorsLost = 0x60,

	vnFree = 0,
	vnHidden = 0x80,
	vnEnd = 0xFF
};
}

namespace DisplayFlags {
enum DisplayFlags {
	dfFlag1 = 1,
	dfFlag2 = 2,
	dfMirror = 4,
	dfPerson = 8,
	dfFresques = 0x10,
	dfPanable = 0x20,
	dfFlag40 = 0x40,
	dfFlag80 = 0x80
};
}

namespace DrawFlags {
enum DrawFlags {
	drDrawInventory = 1,
	drDrawFlag2 = 2,
	drDrawTopScreen = 4,
	drDrawFlag8 = 8,
	drDrawMenu = 0x10,
	drDrawFlag20 = 0x20
};
}

namespace MenuFlags {
enum MenuFlags {
	mfFlag1 = 1,
	mfFlag2 = 2,
	mfFlag4 = 4,
	mfFlag8 = 8,
	mfFlag10 = 0x10
};
}

namespace MusicType {
enum MusicType {  //TODO: same as DialogType?
	mtDontChange = 0,
	mtNormal = 1,
	mt2 = 2,
	mtEvent = 4,
	mtFF = 0xFF
};
}

namespace EventType {
enum EventType {
	etEvent1 = 1,
	etEvent2 = 2,
	etEvent3 = 3,
	etEvent4 = 4,
	etEvent5 = 5,
	etEvent6 = 6,
	etEvent7 = 7,
	etEvent8 = 8,
	etEvent9 = 9,
	etEventB = 11,
	etEventC = 12,
	etEventD = 13,
	etEventE = 14,
	etEventF = 15,
	etEvent10 = 16,
	etEvent12 = 18,
	etGotoArea = 0x80   // + area id
};
}

namespace GameFlags {
enum GameFlags {
	gfMummyOpened = 1,
	gfFlag2 = 2,
	gfFlag4 = 4,
	gfFlag8 = 8,
	gfFlag10 = 0x10,
	gfFlag20 = 0x20,
	gfFlag40 = 0x40,
	gfFlag80 = 0x80,
	gfFlag100 = 0x100,
	gfFlag200 = 0x200,
	gfFlag400 = 0x400,
	gfPrismAndMonk = 0x800,
	gfFlag1000 = 0x1000,
	gfFlag2000 = 0x2000,
	gfFlag4000 = 0x4000,
	gfFlag8000 = 0x8000
};
}

struct global_t {
	unsigned char   areaNum;
	unsigned char   areaVisitCount;
	unsigned char   menuItemIdLo;
	unsigned char   menuItemIdHi;   //TODO: pad?
	unsigned short  randomNumber;   //TODO: this is randomized in pc ver and used by some conds. always zero on mac
	unsigned short  gameTime;
	unsigned short  gameDays;
	unsigned short  chrono;
	unsigned short  eloiDepartureDay;
	unsigned short  roomNum;        // current room number
	unsigned short  newRoomNum;     // target room number selected on world map
	unsigned short  phaseNum;
	unsigned short  metPersonsMask1;
	unsigned short  party;
	unsigned short  partyOutside;
	unsigned short  metPersonsMask2;
	unsigned short  __UNUSED_1C;    //TODO: write-only?
	unsigned short  phaseActionsCount;
	unsigned short  curAreaFlags;
	unsigned short  curItemsMask;
	unsigned short  curPowersMask;
	unsigned short  curPersoItems;
	unsigned short  curPersoPowers;
	unsigned short  wonItemsMask;
	unsigned short  wonPowersMask;
	unsigned short  stepsToFindAppleFast;
	unsigned short  stepsToFindAppleNormal;
	unsigned short  roomPersoItems; //TODO: write-only?
	unsigned short  roomPersoPowers;    //TODO: write-only?
	unsigned short  gameFlags;
	unsigned short  curVideoNum;
	unsigned short  morkusSpyVideoNum1; //TODO: pad?
	unsigned short  morkusSpyVideoNum2; //TODO: pad?
	unsigned short  morkusSpyVideoNum3; //TODO: pad?
	unsigned short  morkusSpyVideoNum4; //TODO: pad?
	unsigned char   newMusicType;
	unsigned char   ff_43;
	unsigned char   videoSubtitleIndex;
	unsigned char   partyInstruments;   // &1 - Bell for Monk, &2 - Drum for Thugg
	unsigned char   monkGotRing;
	unsigned char   chrono_on;
	unsigned char   curRoomFlags;
	unsigned char   endGameFlag;
	unsigned char   last_info;
	unsigned char   autoDialog;
	unsigned char   worldTyrannSighted;
	unsigned char   ff_4D;
	unsigned char   ff_4E;
	unsigned char   worldGaveGold;
	unsigned char   worldHasTriceraptors;
	unsigned char   worldHasVelociraptors;
	unsigned char   worldHasTyrann;
	unsigned char   ff_53;
	unsigned char   ff_54;
	unsigned char   ff_55;  //TODO: pad?
	unsigned char   ff_56;
	unsigned char   textToken1;
	unsigned char   textToken2; //TODO: pad?
	unsigned char   eloiHaveNews;
	unsigned char   dialogFlags;
	unsigned char   curAreaType;
	unsigned char   curCitadelLevel;
	unsigned char   newLocation;
	unsigned char   prevLocation;
	unsigned char   curPersoFlags;
	unsigned char   ff_60;
	unsigned char   eventType;
	unsigned char   ff_62;  //TODO: pad?
	unsigned char   curObjectId;
	unsigned char   curObjectFlags;
	unsigned char   ff_65;  //TODO: pad?
	unsigned char   roomPersoType;
	unsigned char   roomPersoFlags;
	unsigned char   narratorSequence;
	unsigned char   ff_69;
	unsigned char   ff_6A;
	unsigned char   fresqNumber;
	unsigned char   ff_6C;  //TODO: pad?
	unsigned char   ff_6D;  //TODO: pad?
	unsigned char   labyrinthDirections;
	unsigned char   labyrinthRoom;
	void            *__UNUSED_70;   //TODO: pad?
	dial_t          *dialog_ptr;
	tape_t          *tape_ptr;
	dial_t          *next_dialog_ptr;
	dial_t          *narrator_dialog_ptr;
	dial_t          *last_dialog_ptr;
	icon_t          *nextRoomIcon;
	unsigned char   *phraseBufferPtr;
	unsigned char   *__UNUSED_90;   //TODO: write-only?
	unsigned char   *__UNUSED_94;   //TODO: write-only?
	room_t          *room_ptr;
	area_t          *area_ptr;
	area_t          *last_area_ptr;
	area_t          *cur_area_ptr;
	room_t          *cita_area_firstRoom;
	perso_t         *perso_ptr;
	perso_t         *room_perso;
	unsigned char   last_info_idx;
	unsigned char   next_info_idx;
	unsigned char   *persoSpritePtr;
	unsigned char   *persoSpritePtr2;
	unsigned char   *curPersoAnimPtr;
	unsigned char   *ff_C2; //TODO: image desc arr
	short           iconsIndex;
	short           curObjectCursor;    // TODO: useless?
	short           ff_CA;
	short           __UNUSED_CC;        //TODO: unused/pad
	short           perso_img_bank; //TODO: unsigned?
	unsigned short  roomImgBank;
	unsigned short  persoBackgroundBankIdx;
	unsigned short  ff_D4;  //TODO: unsigned?
	unsigned short  fresqWidth;
	unsigned short  fresqImgBank;
	unsigned short  ff_DA;  //TODO: pad?
	unsigned short  ff_DC;  //TODO: pad?
	unsigned short  room_x_base;
	unsigned short  ff_E0;  //TODO: pad?
	unsigned short  dialogType;
	unsigned short  ff_E4;  //TODO: pad?
	unsigned short  currentMusicNum;
	short           textNum;
	unsigned short  travelTime;
	unsigned short  ff_EC;  //TODO: pad?
	unsigned char   displayFlags;
	unsigned char   oldDisplayFlags;
	unsigned char   drawFlags;
	unsigned char   ff_F1;
	unsigned char   ff_F2;
	unsigned char   menuFlags;
	unsigned char   ff_F4;  //TODO: write-only?
	unsigned char   ff_F5;
	unsigned char   ff_F6;
	unsigned char   ff_F7;
	unsigned char   ff_F8;  //TODO: pad?
	unsigned char   ff_F9;  //TODO: pad?
	unsigned char   ff_FA;  //TODO: pad?
	unsigned char   animationFlags;
	unsigned char   __UNUSED_FC;    //TODO: pad?
	unsigned char   giveobj1;
	unsigned char   giveobj2;
	unsigned char   giveobj3;
	unsigned char   ff_100;
	unsigned char   roomVidNum;
	unsigned char   ff_102;
	unsigned char   ff_103;
	unsigned char   roomBgBankNum;
	unsigned char   valleyVidNum;
	unsigned char   updatePaletteFlag;
	unsigned char   inventoryScrollPos;
	unsigned char   obj_count;
	unsigned char   ff_109; //TODO: write-only?
	unsigned char   textBankIndex;
	unsigned char   pref_language;
	unsigned char   pref_10C[2];    //TODO: volume
	unsigned char   pref_10E[2];    // -//-
	unsigned char   pref_110[2];    // -//-
	unsigned char   cita_area_num;
	unsigned char   ff_113;
	unsigned char   lastSalNum;
	unsigned char   save_end;
	short           textWidthLimit;
	unsigned char   numGiveObjs;
	unsigned char   ff_119;     // unused
};
typedef struct global_t global_t;

struct pakfile_t {
	char            name[16];
	long            size;
	long            offs;
	char            flag;
};
typedef struct pakfile_t pakfile_t;

struct pak_t {
	unsigned short  count;
	pakfile_t       files[10];
};
typedef struct pak_t pak_t;
#pragma pack(pop)

struct cita_t {
	short   ff_0;
	short   ff_2[8 * 2];
};
typedef struct cita_t cita_t;

/////////////// vars

suiveur_t suiveurs_list[] = {
//            char,                 X,  sx, sy,  ex,  ey,bank,
	{ PersonId::pidGregor,          5, 211,  9, 320, 176, 228,   0,  0 },
	{ PersonId::pidEloi,            4, 162, 47, 223, 176, 228, 112, 78 },
	{ PersonId::pidDina,            3,  55,  0, 172, 176, 228,  90, 16 },
	{ PersonId::pidChongOfChamaar,  4,   0,  5, 114, 176, 229,   0, 16 },
	{ PersonId::pidKommalaOfKoto,   3,   0, 15, 102, 176, 229,   0, 16 },
	{ PersonId::pidUlanOfUlele,     1,   0,  0, 129, 176, 230,   0, 16 },
	{ PersonId::pidCabukaOfCantura, 2,   0,  0, 142, 176, 230,   0, 16 },
	{ PersonId::pidFuggOfTamara,    0,   0, 17, 102, 176, 230,   0, 16 },
	{ PersonId::pidJabber,          2,   0,  6, 134, 176, 228,   0, 16 },
	{ PersonId::pidShazia,          1,  90, 17, 170, 176, 228,  50, 22 },
	{ PersonId::pidThugg,           0, 489,  8, 640, 176, 228, 160, 24 },
	{ PersonId::pidMungo,           5, 361,  0, 517, 176, 229,   0, 16 },
	{ PersonId::pidMonk,            0, 419, 22, 569, 176, 229, 100, 30 },
	{ PersonId::pidEve,             1, 300, 28, 428, 176, 229,   0, 38 },
	{ -1,                          -1,  -1, -1,  -1,  -1,  -1,  -1, -1 }
};


/*
  Labyrinth of Mo

  | | | | | | | |

*/

enum {
	LAB_N = 1,
	LAB_E,
	LAB_S,
	LAB_W
};

#define LAB(toCrypt, toThrone)  \
	(((LAB_##toCrypt) << 4) | (LAB_##toThrone))

unsigned char kLabyrinthPath[] = {
// each nibble tells wich direction to choose to exit the labyrinth
	0x11, 0x11, 0x11, 0x22, 0x33, 0x55, 0x25, 0x44, 0x25, 0x11, 0x11, 0x11,
	0x11, 0x35, 0x55, 0x45, 0x45, 0x44, 0x44, 0x34, 0x44, 0x34, 0x32, 0x52,
	0x33, 0x23, 0x24, 0x44, 0x24, 0x22, 0x54, 0x22, 0x54, 0x54, 0x44, 0x22,
	0x22, 0x42, 0x45, 0x22, 0x42, 0x45, 0x35, 0x11, 0x44, 0x34, 0x52, 0x11,
	0x44, 0x32, 0x55, 0x11, 0x11, 0x33, 0x11, 0x11, 0x53, 0x11, 0x11, 0x53,
	0x54, 0x24, 0x11, 0x22, 0x25, 0x33, 0x53, 0x54, 0x23, 0x44
};

#undef LAB

char kDinoSpeedForCitaLevel[16] = { 1, 2, 3, 4, 4, 5, 6, 7, 8, 9 };

char kTabletView[] = {          //TODO: make as struct?
	// opposite tablet id, video id
	Objects::obUnused10, 83,
	Objects::obUnused10, 84,
	Objects::obTablet4, 85,
	Objects::obTablet3, 86,
	Objects::obTablet6, 87,
	Objects::obTablet5, 85
};

// special character backgrounds for specific rooms
char kPersoRoomBankTable[] = {
	// first entry is default bank, then pairs of [roomNum, bankNum], terminated by -1
	0,  3, 33, -1,
	21, 17, 35, -1,
	0,  2, 36, -1,
	22,  9, 38,  3, 39, -1,
	23,  8, 40, -1,
	0,  3, 41,  7, 42, -1,
	25, -1,
	27, 17, 45, -1,
	28, 26, 46, -1,
	29, 51, 48, -1,
	30, 53, 49, -1,
	0, 27, 50, -1,
	32, 17, 51, -1,
	52,  2, 52, -1,
	-3,  3, -3, -1,
	31, -1,
	24,  6, 43, -1,
	47, -1,
	0,  2, 64, -1,
	54,  3, 54, -1,
	27, -1,
	26, 17, 45, -1
};

// area transition descriptors
goto_t gotos[] = {
// area, oldarea, vid, time, valleyVid
	{  0,  1,   0,  2,  20 },
	{  0,  1, 162,  3, 168 },
	{  0,  2,   0,  2,  21 },
	{  0,  6,   0,  3, 108 },
	{  0,  9, 151,  3,   0 },
	{  0,  7, 106,  2, 101 },
	{  0, 10,  79,  3, 102 },
	{  0, 12,   0,  3,   0 },
	{ -1, -1,  -1, -1,  -1 },
	{  1,  3,  58,  2, 104 },
	{  1,  4, 100,  4, 104 },
	{  1,  5, 107,  6, 104 },
	{  1,  6, 155,  8, 104 },
	{  1,  7, 165,  6, 104 },
	{  1,  8, 169,  6, 104 },
	{  1, 10, 111,  2, 104 },
	{  1, 11, 164,  4, 104 },
	{ -1, -1,  -1, -1,  -1 },
	{  1,  3, 161,  3, 102 },
	{  1,  4, 163,  6, 102 },
	{  1,  5, 157,  9, 102 },
	{  1,  9, 160,  9, 102 },
	{  1, 10,  79,  3, 102 },
	{ -1, -1,  -1, -1,  -1 },
	{  1,  3,   0,  3, 153 },
	{ -1, -1,  -1, -1,  -1 },
	{  3,  1, 154,  2, 103 },
	{  3,  4, 100,  2, 103 },
	{  3,  5, 107,  4, 103 },
	{  3,  6, 155,  6, 103 },
	{  3,  7, 165,  8, 103 },
	{  3,  8, 169,  6, 103 },
	{  3, 10, 111,  4, 103 },
	{  3, 11, 164,  6, 103 },
	{ -1, -1,  -1, -1,  -1 },
	{  3,  1, 162,  3,  22 },
	{  3,  4, 163,  6,  22 },
	{  3,  5, 157,  9,  22 },
	{  3,  9, 160,  9,  22 },
	{ -1, -1,  -1, -1,  -1 },
	{  3,  1,   0,  3, 166 },
	{ -1, -1,  -1, -1,  -1 },
	{  4,  1, 154,  4,  51 },
	{  4,  3,  58,  2,  51 },
	{  4,  5, 107,  2,  51 },
	{  4,  6, 155,  4,  51 },
	{  4,  7, 165,  6,  51 },
	{  4,  8, 169,  8,  51 },
	{  4, 10, 111,  6,  51 },
	{  4, 11, 164,  8,  51 },
	{ -1, -1,  -1, -1,  -1 },
	{  4,  1, 162,  3, 109 },
	{  4,  3, 161,  6, 109 },
	{  4,  5, 157,  9, 109 },
	{  4,  9, 160,  9, 109 },
	{ -1, -1,  -1, -1,  -1 },
	{  5,  1, 154,  6,  33 },
	{  5,  3,  58,  4,  33 },
	{  5,  4, 100,  2,  33 },
	{  5,  6, 155,  2,  33 },
	{  5,  7, 165,  4,  33 },
	{  5,  8, 169,  8,  33 },
	{  5, 10, 111,  8,  33 },
	{  5, 11, 164,  8,  33 },
	{ -1, -1,  -1, -1,  -1 },
	{  5,  1, 162,  3,  99 },
	{  5,  3, 161,  6,  99 },
	{  5,  4, 163,  9,  99 },
	{  5,  9, 160,  9,  99 },
	{ -1, -1,  -1, -1,  -1 },
	{  9,  1, 162,  3, 167 },
	{  9,  3, 161,  6, 167 },
	{  9,  4, 163,  9, 167 },
	{  9,  5, 157,  9, 167 },
	{ -1, -1,  -1, -1,  -1 },
	{  6,  1, 154,  8, 105 },
	{  6,  3,  58,  6, 105 },
	{  6,  4, 100,  4, 105 },
	{  6,  5, 107,  2, 105 },
	{  6,  7, 165,  2, 105 },
	{  6,  8, 169, 10, 105 },
	{  6, 10, 111,  6, 105 },
	{  6, 11, 164,  8, 105 },
	{ -1, -1,  -1, -1,  -1 },
	{  7,  1, 154,  4, 158 },
	{  7,  3,  58,  6, 158 },
	{  7,  4, 100,  6, 158 },
	{  7,  5, 107,  4, 158 },
	{  7,  6, 155,  2, 158 },
	{  7,  8, 169,  8, 158 },
	{  7, 10, 111,  4, 158 },
	{  7, 11, 164,  6, 158 },
	{ -1, -1,  -1, -1,  -1 },
	{  8,  1, 154,  2, 159 },
	{  8,  3,  58,  4, 159 },
	{  8,  4, 100,  6, 159 },
	{  8,  5, 107,  8, 159 },
	{  8,  6, 155, 10, 159 },
	{  8,  7, 165,  8, 159 },
	{  8, 10, 111,  6, 159 },
	{  8, 11, 164,  4, 159 },
	{ -1, -1,  -1, -1,  -1 },
	{ 10,  1, 154,  2,  77 },
	{ 10,  3,  58,  4,  77 },
	{ 10,  4, 100,  6,  77 },
	{ 10,  5, 107,  8,  77 },
	{ 10,  6, 155,  6,  77 },
	{ 10,  7, 165,  4,  77 },
	{ 10,  8, 169,  6,  77 },
	{ 10, 11, 164,  4,  77 },
	{ -1, -1,  -1, -1,  -1 },
	{ 11,  1, 154,  2,  80 },
	{ 11,  3,  58,  4,  80 },
	{ 11,  4, 100,  6,  80 },
	{ 11,  5, 107,  8,  80 },
	{ 11,  6, 155,  8,  80 },
	{ 11,  7, 165,  6,  80 },
	{ 11,  8, 169,  2,  80 },
	{ 11, 10, 111,  4,  80 },
	{ -1, -1,  -1, -1,  -1 },
	{ 12,  1, 154,  8,  56 },
	{ 12,  3,  58,  4,  56 },
	{ 12,  4, 100,  4,  56 },
	{ 12,  5, 107,  6,  56 },
	{ 12,  6, 155,  8,  56 },
	{ 12,  7, 165, 10,  56 },
	{ 12,  8, 169,  4,  56 },
	{ 12, 10, 111, 10,  56 },
	{ 12, 11, 164,  6,  56 },
	{ -1, -1,  -1, -1,  -1 }
};

#define SUB_LINE(start, end) \
	(start), (end) | 0x8000

short tab_2D24C[] = {
	SUB_LINE(68, 120),
	123, 32964,
	199, 33042,
	276, 33138,
	799, 33653,
	888, 33708,
	947, 33768,
	1319, 34146,
	1380, 34208,
	1854, 34666,
	1900, 34728,
	2116, 34952,
	2186, 35020,
	2254, 35088,
	3038, 35862,
	3096, 35928,
	-1
};

short tab_2D28E[] = {
	99, 32923,
	157, 33024,
	-1
};

short tab_2D298[] = {
	106, 32941,
	175, 33012,
	246, 33118,
	352, 33235,
	-1
};

short tab_2D2AA[] = {
	126, 32944,
	178, 33035,
	269, 33110,
	344, 33166,
	400, 33226,
	460, 33326,
	-1
};

short tab_2D2C4[] = {
	101, 32981,
	215, 33121,
	355, 33223,
	457, 33286,
	520, 33428,
	662, 33536,
	-1
};
#undef SUB_LINE

object_t objects[] = {
	//id,fl,loc,masklow,maskhi,ct
	{  1, 0,  3,      1,     0, 0},     // Eve's Way Stone
	{  2, 0,  3,      2,     0, 0},     // Thau's Seashell
	{  3, 0,  3,      4,     0, 0},     // Talisman of bravery
	{  4, 0,  3,      8,     0, 0},     // An old tooth. Very old! Whoever lost it most certainly has no further use for it!
	{  5, 0,  0,   0x10,     0, 0},     // Prism
	{  6, 0,  3,      0,     0, 0},     // Flute
	{  7, 0,  3, 0x4000,     0, 0},     // Apple
	{  8, 0,  4, 0x1000,     0, 0},     // Egg of Destiny
	{  9, 0,  3,  0x800,     0, 0},     // Root
	{ 10, 0,  3,      0,     0, 0},     // ???
	{ 11, 0,  6,      0,     0, 0},     // Mushroom
	{ 12, 0, 13,      0,     0, 0},     // Poisonous Mushroom
	{ 13, 0,  2,  0x400,     0, 0},     // Graa's Knife
	{ 14, 0, 22,      0,     0, 0},     // Empty Nest
	{ 15, 0, 26,      0,     0, 0},     // Full Nest
	{ 16, 0, 33,   0x20,     0, 0},     // Gold
	{ 17, 0,  3,      0,     0, 0},     // Sign of Shadow Mistress  (moon stone)
	{ 18, 0,  3,      0,     0, 0},     // Sign of Mother of all    (bag of soil)
	{ 19, 0, 40,      0,     0, 0},     // Sign of the life-giving  (sun star)
	{ 20, 0, 20,  0x200,     0, 0},     // King's Horn
	{ 21, 0,  3,      0,     0, 0},     // Golden Sword of Mashaar
	// Masks
	{ 22, 0,  3,   0x40,     0, 0},     // Mask of Death
	{ 23, 0,  3,   0x80,     0, 0},     // Mask of Bonding
	{ 24, 0,  3,  0x100,     0, 0},     // Mask of Birth
	// Objects of power
	{ 25, 0,  3,      0,     1, 0},     // Eye in the Storm
	{ 26, 0,  3,      0,     2, 0},     // Sky Hammer
	{ 27, 0,  3,      0,     4, 0},     // Fire in the Clouds
	{ 28, 0,  3,      0,     8, 0},     // Within and Without
	{ 29, 0,  3,      0,  0x10, 0},     // Eye in the Cyclone
	{ 30, 0,  3,      0,  0x20, 0},     // River that Winds
	// Musical instruments
	{ 31, 0,  3,      0,  0x40, 0},     // Trumpet
	{ 32, 0,  3,      0,  0x80, 0},     // -- unused (but still has a dialog line)
	{ 33, 0,  3,      0, 0x100, 0},     // Drum
	{ 34, 0,  3,      0, 0x200, 0},     // -- unused (but still has a dialog line)
	{ 35, 0,  3,      0, 0x400, 0},     // -- unused (but still has a dialog line)
	{ 36, 0,  3,      0, 0x800, 0},     // Ring
	// Tablets
	{ 37, 0,  3,      0,     0, 0},     // Tablet #1 (Mo)
	{ 38, 0, 42, 0x2000,     0, 0},     // Tablet #2 (Morkus' Lair)
	{ 39, 0,  3,      0,     0, 0},     // Tablet #3 (White Arch?)
	{ 40, 0,  3,      0,     0, 0},     // Tablet #4
	{ 41, 0,  3,      0,     0, 0},     // Tablet #5
	{ 42, 0,  3, 0x8000,     0, 0}      // Tablet #6 (Castra)
};

short kObjectLocations[100] = {
	0x112, -1,
	0x202, -1,
	0x120, -1,
	0x340, 0x44B, 0x548, 0x640, 0x717, 0x830, -1,
	0x340, 0x44B, 0x548, 0x640, 0x717, 0x830, -1,
	0, -1,
	0x344, 0x53A, 0x831, -1,
	0x331, 0x420, 0x54B, 0x637, 0x716, 0x840, -1,
	0x834A, 0x8430, 0x8531, 0x644, 0x745, 0x838, -1,
	0x510, -1,
	0xC04, -1,
	-1
};

perso_t kPersons[] = {
	// room, aid, party mask,                            id,                                            flags,  X,bank,X, X,sprId,sprX,speed, X
	{ 0x103, 230, PersonMask::pmGregor, PersonId::pidGregor            ,                                                0,  0,  1, 0, 0,  0,   0, 0, 0 },
	{ 0x116, 231, PersonMask::pmDina  , PersonId::pidDina              ,                                                0,  4,  2, 0, 0,  3,   9, 0, 0 },
	{ 0x202, 232, PersonMask::pmTau   , PersonId::pidTau               ,                                                0,  8,  3, 0, 0,  0,   0, 0, 0 },
	{ 0x109, 233, PersonMask::pmMonk  , PersonId::pidMonk              ,                                                0, 12,  4, 0, 0,  6,  52, 0, 0 },
	{ 0x108, 234, PersonMask::pmJabber, PersonId::pidJabber            ,                                                0, 18,  5, 0, 0,  2,   0, 0, 0 },
	{ 0x103, 235, PersonMask::pmEloi  , PersonId::pidEloi              ,                                                0, 22,  6, 0, 0,  4,  20, 0, 0 },
	{ 0x301, 236, PersonMask::pmMungo , PersonId::pidMungo             ,                                                0, 28,  8, 0, 0, 11,  45, 0, 0 },
	{ 0x628, 237, PersonMask::pmEve   , PersonId::pidEve               ,                                                0, 30, 10, 0, 0,  7,  35, 0, 0 },
	{ 0x81A, 238, PersonMask::pmShazia, PersonId::pidShazia            ,                                                0, 34, 11, 0, 0,  1,  11, 0, 0 },
	{ 0x330, 239, PersonMask::pmLeader, PersonId::pidChongOfChamaar    ,                                                0, 38, 13, 0, 0, 10,   0, 0, 0 },
	{ 0x41B, 239, PersonMask::pmLeader, PersonId::pidUlanOfUlele       ,                                                0, 46, 15, 0, 0, 13,   0, 0, 0 },
	{ 0x53B, 239, PersonMask::pmLeader, PersonId::pidKommalaOfKoto     ,                                                0, 42, 14, 0, 0,  9,   0, 0, 0 },
	{ 0x711, 239, PersonMask::pmLeader, PersonId::pidCabukaOfCantura   ,                                                0, 50, 16, 0, 0, 14,   0, 0, 0 },
	{ 0xA02, 239, PersonMask::pmLeader, PersonId::pidMarindaOfEmbalmers,                                                0, 54, 17, 0, 0,  0,   0, 0, 0 },
	{ 0x628, 239, PersonMask::pmLeader, PersonId::pidFuggOfTamara      ,                                                0, 62, 18, 0, 0, 12,   0, 0, 0 },
	{ 0x801, 239, PersonMask::pmLeader, PersonId::pidChongOfChamaar    ,                                                0, 38, 13, 0, 0, 10,   0, 0, 0 },
	{ 0x41B,  10, PersonMask::pmQuest , PersonId::pidUlanOfUlele       , PersonFlags::pfType2                            , 46, 15, 0, 0, 13,   0, 0, 0 },
	{ 0x711,  11, PersonMask::pmQuest , PersonId::pidCabukaOfCantura   , PersonFlags::pfType2                            , 50, 16, 0, 0, 14,   0, 0, 0 },
	{ 0x106, 240, PersonMask::pmThugg , PersonId::pidThugg             ,                                                0, 64,  7, 0, 0,  0,  61, 0, 0 },
	{     0,  13,                    0, PersonId::pidNarrator          ,                                                0, 68, 12, 0, 0,  0,   0, 0, 0 },
	{ 0x902, 241, PersonMask::pmQuest , PersonId::pidNarrim            ,                                                0, 70, 19, 0, 0,  0,   0, 0, 0 },
	{ 0xC03, 244, PersonMask::pmMorkus, PersonId::pidMorkus            ,                                                0, 74, 20, 0, 0,  0,   0, 0, 0 },
	// dinos in each valley
	{ 0x332, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x329, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x33B, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x317, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x320, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType12                           ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x349, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x429, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x43B, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x422, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x432, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x522, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x534, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x515, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pftVelociraptor                    ,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x533, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x622, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x630, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x643, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x63A, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x737, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x739, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x74A, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftVelociraptor,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x726, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0x842, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pfType8        ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x822, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftTriceraptor ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x828, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pftVelociraptor                    ,  0,  0, 0, 0,  0,   0, 1, 0 },
	{ 0x84B, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pf80 | PersonFlags::pftMosasaurus  ,  0,  0, 0, 0,  0,   0, 0, 0 },

	{ 0xB03, 242, PersonMask::pmDino  , PersonId::pidDinosaur          , PersonFlags::pfType8                            , 58, 252, 0, 0,  0,   0, 0, 0 },
	// enemy dinos
	{ 0x311, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x410, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x51B, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x618, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x71B, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{ 0x81B, 243, PersonMask::pmEnemy , PersonId::pidEnemy             , PersonFlags::pf80 | PersonFlags::pftTyrann      ,  0,  0, 0, 0,  0,   0, 0, 0 },
	{    -1,  -1,                   -1,                              -1,                                               -1, -1, -1, -1, -1, -1,  -1, -1, -1 },
	{ 0x628, 237, PersonMask::pmEve   , PersonId::pidEve               ,                                                0, 80,  9, 0, 0,  8,  35, 0, 0 },
	{ 0x628, 237, PersonMask::pmEve   , PersonId::pidEve               ,                                                0, 78, 10, 0, 0,  7,  35, 0, 0 }
};

cita_t cita_list[] = {
	{
		1,
		{
			163, 182,   0, 0,
			124, 147, 193, 0,
			0,   0,   0, 0,
			0,   0,   0, 0
		}
	},
	{
		48,
		{
			285, 286,   0, 0,
			287, 288, 284, 0,
			114, 115,   0, 0,
			116, 117, 113, 0
		}
	},
	{
		63,
		{
			290, 291,   0, 0,
			292, 293, 289, 0,
			119, 120,   0, 0,
			121, 122, 118, 0
		}
	},
	{
		95,
		{
			295, 296,   0, 0,
			297, 298, 294, 0,
			124, 125,   0, 0,
			126, 127, 123, 0
		}
	},
	{
		127,
		{
			300, 301,   0, 0,
			302, 303, 299, 0,
			129, 130,   0, 0,
			131, 132, 128, 0
		}
	},
	{
		159,
		{
			305, 306,   0, 0,
			307, 308, 304, 0,
			134, 135,   0, 0,
			136, 137, 133, 0
		}
	},
	{
		255,
		{
			310, 311,   0, 0,
			312, 313, 309, 0,
			139, 140,   0, 0,
			141, 142, 138, 0
		}
	}
};


short tab_2CB16[] = { 2075, 2080, 2119, -1};

char tab_2CB1E[8][4] = {
	{ 0x10, 0x81,    1, 0x90},
	{ 0x90,    1, 0x81, 0x10},
	{    1, 0x90, 0x10, 0x81},
	{    1, 0x10, 0x90, 0x81},
	{    1, 0x90, 0x10, 0x81},
	{ 0x81, 0x10, 0x90,    1},
	{ 0x81, 0x10, 0x90,    1},
	{ 0x81, 0x90,    1, 0x10}
};

struct prect_t {
	short   sx, sy, ex, ey;
};
typedef struct prect_t prect_t;

prect_t perso_rects[] = {   //TODO: just an array of shorts?
	{  93,  69, 223, 176},
	{ 102,  86, 162, 126},
	{  88, 103, 168, 163},
	{ 116,  66, 192, 176},
	{ 129,  92, 202, 153},
	{  60,  95, 160, 176},
	{ 155,  97, 230, 145},
	{ 100,  77, 156, 145},
	{ 110,  78, 170, 156},
	{  84,  76, 166, 162},
	{  57,  77, 125, 114},
	{  93,  69, 223, 175},
	{  93,  69, 223, 176},
	{  93,  69, 223, 176},
	{ 154,  54, 245, 138},
	{ 200,  50, 261, 116},
	{  70,  84, 162, 176},
	{ 125, 101, 222, 172},
	{ 188,  83, 251, 158}
};

unsigned char tab_persxx[][5] = {   //TODO: struc?
	{  8, 15, 23, 25, -1},
	{  0,  9, -1        },
	{  0,  9, -1        },
	{  0,  9, -1        },
	{  0, 13, -1        },
	{ 16, 21, -1        },
	{ 11, 20, -1        },
	{  0, 12, -1        },
	{  0,  9, -1        },
	{  0,  9, -1        },
	{  5, 13, -1        },
	{ -1                },
	{  0,  8, -1        },
	{ -1                },
	{  0,  7, -1        },
	{  0,  8, -1        },
	{  8, 12, -1        },
	{  0,  5, -1        },
	{  0,  4, -1        },
	{ -1                }
};

area_t kAreasTable[] = {
	{ Areas::arMo           , AreaType::atCitadel,                           0,   0, 0,  1},
	{ Areas::arTausCave     , AreaType::atCave   ,                           0, 112, 0,  2},
	{ Areas::arChamaar      , AreaType::atValley ,                           0, 133, 0,  3},
	{ Areas::arUluru        , AreaType::atValley ,                           0, 187, 0,  4},
	{ Areas::arKoto         , AreaType::atValley , AreaFlags::HasVelociraptors, 236, 0,  5},
	{ Areas::arTamara       , AreaType::atValley ,                           0, 288, 0,  6},
	{ Areas::arCantura      , AreaType::atValley ,                           0, 334, 0,  7},
	{ Areas::arShandovra    , AreaType::atValley ,                           0, 371, 0,  8},
	{ Areas::arNarimsCave   , AreaType::atCave   ,                           0, 115, 0,  9},
	{ Areas::arEmbalmersCave, AreaType::atCave   ,                           0, 118, 0, 10},
	{ Areas::arWhiteArch    , AreaType::atCave   ,                           0, 122, 0, 11},
	{ Areas::arMoorkusLair  , AreaType::atCave   ,                           0, 127, 0, 12}
};

short tab_2CEF0[64] = {
	25, 257,  0,   0, 37, 258, 38, 259,  0,   0, 24, 260, 0, 0, 0, 0,
	0,   0, 53, 265,  0,   0,  0,   0,  0,   0,  0,   0, 0, 0, 0, 0,
	39, 261,  0,   0, 40, 262, 62, 263,  0,   0, 63, 264, 0, 0, 0, 0,
	18, 275,  0,   0, 35, 254, 36, 255, 19, 318, 23, 256, 0, 0, 0, 0
};

short tab_2CF70[64] = {
	65, 266,  0,   0, 66, 267, 67, 268,  0,   0, 68, 269, 0, 0, 0, 0,
	0,   0, 73, 274,  0,   0,  0,   0,  0,   0,  0,   0, 0, 0, 0, 0,
	69, 270,  0,   0, 70, 271, 71, 272,  0,   0, 72, 273, 0, 0, 0, 0,
	18, 275,  0,   0, 35, 254, 36, 255, 19, 318, 23, 256, 0, 0, 0, 0,
};

short kActionCursors[299] = {
	3, 1, 2, 4, 5, 5, 5, 0, 5, 5,
	5, 5, 5, 3, 2, 5, 5, 5, 3, 2,
	4, 5, 7, 7, 4, 5, 5, 0, 0, 0,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 0, 0, 0, 0, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 0, 0,
	0, 0, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 0, 5, 6,
	6, 1, 6, 6, 0, 0, 6, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 6, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 0, 0, 6, 6,
	53, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0
};


struct cubeface_t {
	int             tri;
	char            ff_4;
	char            ff_5;

	unsigned char   *texptr;
	unsigned short  *indices;
	short           *uv;
};
typedef struct cubeface_t cubeface_t;

struct cube_t {
	int             num;
	cubeface_t      **faces;
	short           *projection;    // projected XYZ coords
	short           *vertices;
};
typedef struct cube_t cube_t;

float flt_2DF7C = -3400;
float flt_2DF80 = -3400;
float flt_2DF84 =   200;

// Cube faces to texture coords mapping
// each entry is num_polys(6) * num_faces_per_poly(2) * vertex_per_face(3) * uv(2)

short cube_texcoords[3][6 * 2 * 3 * 2] = {
	{
		32, 32,  0, 32,  0,  0,
		32, 32,  0,  0, 32,  0,

		0, 32,  0,  0, 32,  0,
		0, 32, 32,  0, 32, 32,

		32, 32,  0, 32,  0,  0,
		32, 32,  0,  0, 32,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		0,  0, 32,  0, 32, 32,
		0,  0, 32, 32,  0, 32,

		0, 32,  0,  0, 32,  0,
		0, 32, 32,  0, 32, 32
	}, {
		32, 32,  0, 32,  0,  0,
		32, 32,  0,  0, 32,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		0, 32,  0,  0, 32,  0,
		0, 32, 32,  0, 32, 32,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0,

		32,  0, 32, 32,  0, 32,
		32,  0,  0, 32,  0,  0
	}, {
		30, 30,  2, 30,  2,  2,
		30, 30,  2,  2, 30,  2,

		2, 30,  2,  2, 30,  2,
		2, 30, 30,  2, 30, 30,

		30, 30,  2, 30,  2,  2,
		30, 30,  2,  2, 30,  2,

		30,  2, 30, 30,  2, 30,
		30,  2,  2, 30,  2,  2,

		2,  2, 30,  2, 30, 30,
		2,  2, 30, 30,  2, 30,

		2, 30,  2,  2, 30,  2,
		2, 30, 30,  2, 30, 30
	}
};

char tab_2E138[4 * 3] = {
	0, 0, 1, 1,
	0, 0, 0, 1,
	0, 0, 2, 0
};
