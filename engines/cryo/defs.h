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

#include "cryo/cryolib.h"

#ifndef CRYO_DEFS_H
#define CRYO_DEFS_H

namespace Cryo {

#define getElem(array, idx) \
	( (char *)(array) + PLE16((idx) * 2 + (char *)(array)) )
/*
static inline void* AGetElem(byte *arr, int16 index)
{
    byte *p = arr + num * 2;
    byte o0 = *p++;
    byte o1 = *p++;
    uint16 ofs = (o1 << 8) | o0;
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
	uint16  _roomNum;    // room this person currently in
	uint16  _actionId;   // TODO: checkme
	uint16  _partyMask;      // party bit mask
	byte   _id;         // character
	byte   _flags;      // flags and kind
	byte   _roomBankId;// index in kPersoRoomBankTable for specific room banks
	byte   _spriteBank;       // sprite bank
	uint16  _items;      // inventory
	uint16  _powers;     // obj of power bitmask
	byte   _targetLoc;  // For party member this is mini sprite index
	byte   _lastLoc;    // For party member this is mini sprite x offset
	byte   _speed;      // num ticks per step
	byte   _steps;      // current ticks
};
typedef struct perso_t perso_t;

class EdenGame;

struct phase_t {
	int16           _id;
	void (EdenGame::*disp)();
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
	byte   _id;
	byte   _flags;
	int    _locations;      // index in kObjectLocations
	uint16  _itemMask;
	uint16  _powerMask;          // object of power bitmask
	int16  _count;
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
	char        _flags;          // 0-3 - action index, 4 - highest bit of contidion index, rest is DialogFlags
	char        _condNumLow;     // condition index low bits
	char        _textCondHiMask; // 0-1 text index hi bits, 2-5 - perso mask num, 6-7 condition index hi bits
	char        _textNumLow;     // text line index low bits
};
typedef struct dial_t dial_t;

struct tape_t {
	int16       _textNum;
	perso_t     *_perso;
	int16       _party;
	int16       _roomNum;
	int16       _bgBankNum;
	dial_t      *_dialog;
};
typedef struct tape_t tape_t;

struct Follower {      // Characters on Mirror screen
	char        _id;         // character
	char        _spriteNum;      // sprite number
	int16       sx;
	int16       sy;
	int16       ex;
	int16       ey;
	int16       _spriteBank;
	int16       ff_C;
	int16       ff_E;
};
typedef struct Follower Follower;

struct icon_t {
	int16       sx;
	int16       sy;
	int16       ex;
	int16       ey;
	uint16  cursor_id;          // & 0x8000 - inactive/hidden
	unsigned int    action_id;
	unsigned int    object_id;
};
typedef struct icon_t icon_t;

struct goto_t {
	byte   areaNum;    // target area
	byte   curAreaNum; // current area
	byte   departVid;
	byte   travelTime;     // time to skip while in travel
	byte   arriveVid;
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
	byte   ff_0;
	byte   exits[4];   //TODO: signed?
	byte   flags;
	uint16  bank;
	uint16  party;
	byte   level;      // Citadel level
	byte   video;
	byte   location;
	byte   background; // bg/mirror image number (relative)
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
	byte   num;
	byte   type;
	uint16  flags;
	uint16  firstRoomIndex;
	byte   citadelLevel;
	byte   salNum;
	room_t          *citadelRoom;
	int16           visitCount;
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
	dfFrescoes = 0x10,
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
	byte   areaNum;
	byte   areaVisitCount;
	byte   menuItemIdLo;
	byte   menuItemIdHi;   //TODO: pad?
	uint16  randomNumber;   //TODO: this is randomized in pc ver and used by some conds. always zero on mac
	uint16  gameTime;
	uint16  gameDays;
	uint16  chrono;
	uint16  eloiDepartureDay;
	uint16  roomNum;        // current room number
	uint16  newRoomNum;     // target room number selected on world map
	uint16  phaseNum;
	uint16  metPersonsMask1;
	uint16  party;
	uint16  partyOutside;
	uint16  metPersonsMask2;
	uint16  __UNUSED_1C;    //TODO: write-only?
	uint16  phaseActionsCount;
	uint16  curAreaFlags;
	uint16  curItemsMask;
	uint16  curPowersMask;
	uint16  curPersoItems;
	uint16  curPersoPowers;
	uint16  wonItemsMask;
	uint16  wonPowersMask;
	uint16  stepsToFindAppleFast;
	uint16  stepsToFindAppleNormal;
	uint16  roomPersoItems; //TODO: write-only?
	uint16  roomPersoPowers;    //TODO: write-only?
	uint16  gameFlags;
	uint16  morkusSpyVideoNum1; //TODO: pad?
	uint16  morkusSpyVideoNum2; //TODO: pad?
	uint16  morkusSpyVideoNum3; //TODO: pad?
	uint16  morkusSpyVideoNum4; //TODO: pad?
	byte   newMusicType;
	byte   ff_43;
	byte   videoSubtitleIndex;
	byte   partyInstruments;   // &1 - Bell for Monk, &2 - Drum for Thugg
	byte   monkGotRing;
	byte   chrono_on;
	byte   curRoomFlags;
	byte   endGameFlag;
	byte   last_info;
	bool   autoDialog;
	byte   worldTyrannSighted;
	byte   ff_4D;
	byte   ff_4E;
	byte   worldGaveGold;
	byte   worldHasTriceraptors;
	byte   worldHasVelociraptors;
	byte   worldHasTyrann;
	byte   ff_53;
	byte   ff_54;
	byte   ff_55;  //TODO: pad?
	byte   ff_56;
	byte   textToken1;
	byte   textToken2; //TODO: pad?
	byte   eloiHaveNews;
	byte   dialogFlags;
	byte   curAreaType;
	byte   curCitadelLevel;
	byte   newLocation;
	byte   prevLocation;
	byte   curPersoFlags;
	bool ff_60;
	byte   eventType;
	byte   ff_62;  //TODO: pad?
	byte   curObjectId;
	byte   curObjectFlags;
	byte   ff_65;  //TODO: pad?
	byte   roomPersoType;
	byte   roomPersoFlags;
	byte   narratorSequence;
	byte   ff_69;
	byte   ff_6A;
	byte   frescoNumber;
	byte   ff_6C;  //TODO: pad?
	byte   ff_6D;  //TODO: pad?
	byte   labyrinthDirections;
	byte   labyrinthRoom;
	dial_t          *_dialogPtr;
	tape_t          *tape_ptr;
	dial_t          *_nextDialogPtr;
	dial_t          *narrator_dialog_ptr;
	dial_t          *last_dialog_ptr;
	icon_t          *nextRoomIcon;
	byte   *phraseBufferPtr;
	room_t          *room_ptr;
	area_t          *area_ptr;
	area_t          *last_area_ptr;
	area_t          *cur_area_ptr;
	room_t          *cita_area_firstRoom;
	perso_t         *perso_ptr;
	perso_t         *room_perso;
	byte   last_info_idx;
	byte   next_info_idx;
	byte   *persoSpritePtr;
	byte   *persoSpritePtr2;
	byte   *curPersoAnimPtr;
	byte   *ff_C2; //TODO: image desc arr
	int16           iconsIndex;
	int16           curObjectCursor;    // TODO: useless?
	int16           ff_CA;
	int16           __UNUSED_CC;        //TODO: unused/pad
	int16           perso_img_bank; //TODO: unsigned?
	uint16  roomImgBank;
	uint16  persoBackgroundBankIdx;
	uint16  ff_D4;  //TODO: unsigned?
	uint16  fresqWidth;
	uint16  fresqImgBank;
	uint16  ff_DA;  //TODO: pad?
	uint16  ff_DC;  //TODO: pad?
	uint16  room_x_base;
	uint16  ff_E0;  //TODO: pad?
	uint16  dialogType;
	uint16  ff_E4;  //TODO: pad?
	uint16  currentMusicNum;
	int16           textNum;
	uint16  travelTime;
	uint16  ff_EC;  //TODO: pad?
	byte   displayFlags;
	byte   oldDisplayFlags;
	byte   drawFlags;
	byte   ff_F1;
	byte   ff_F2;
	byte   menuFlags;
	byte   ff_F4;  //TODO: write-only?
	byte   ff_F5;
	byte   ff_F6;
	byte   ff_F7;
	byte   ff_F8;  //TODO: pad?
	byte   ff_F9;  //TODO: pad?
	byte   ff_FA;  //TODO: pad?
	byte   animationFlags;
	byte   giveobj1;
	byte   giveobj2;
	byte   giveobj3;
	byte   ff_100;
	byte   roomVidNum;
	byte   ff_102;
	byte   ff_103;
	byte   roomBgBankNum;
	byte   valleyVidNum;
	byte   updatePaletteFlag;
	byte   inventoryScrollPos;
	byte   obj_count;
	byte   textBankIndex;
	byte   pref_language;
	byte   pref_10C[2];    //TODO: volume
	byte   pref_10E[2];    // -//-
	byte   pref_110[2];    // -//-
	byte   cita_area_num;
	byte   ff_113;
	byte   lastSalNum;
	byte   save_end;
	int16           textWidthLimit;
	byte   numGiveObjs;
	byte   ff_119;     // unused
};
typedef struct global_t global_t;

struct pakfile_t {
	char            name[16];
	int32            size;
	int32            offs;
	char            flag;
};
typedef struct pakfile_t pakfile_t;

struct pak_t {
	uint16  count;
	pakfile_t       files[10];
};
typedef struct pak_t pak_t;
#pragma pack(pop)

struct cita_t {
	int16   ff_0;
	int16   ff_2[8 * 2];
};
typedef struct cita_t cita_t;

/////////////// vars

extern Follower followerList[];


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

extern byte kLabyrinthPath[];

extern char kDinoSpeedForCitaLevel[16];

extern char kTabletView[];

// special character backgrounds for specific rooms
extern char kPersoRoomBankTable[];

// area transition descriptors
extern goto_t gotos[];
extern uint16 kFramesVid170[];
extern uint16 kFramesVid83[];
extern uint16 kFramesVid88[];
extern uint16 kFramesVid89[];
extern uint16 kFramesVid94[];
extern object_t objects[];
extern uint16 kObjectLocations[100];
extern perso_t kPersons[];
extern cita_t cita_list[];
extern int16 tab_2CB16[];
extern char tab_2CB1E[8][4];

struct prect_t {
	int16   sx, sy, ex, ey;
};
typedef struct prect_t prect_t;

extern prect_t perso_rects[];
extern byte tab_persxx[][5];
extern area_t kAreasTable[];
extern int16 tab_2CEF0[64];
extern int16 tab_2CF70[64];
extern int16 kActionCursors[299];

struct cubeface_t {
	int             tri;
	char            ff_4;
	char            ff_5;

	byte   *texptr;
	uint16  *indices;
	int16           *uv;
};
typedef struct cubeface_t cubeface_t;

struct cube_t {
	int             num;
	cubeface_t      **faces;
	int16           *projection;    // projected XYZ coords
	int16           *vertices;
};
typedef struct cube_t cube_t;

extern float flt_2DF7C;
extern float flt_2DF80;
extern float flt_2DF84;

// Cube faces to texture coords mapping
// each entry is num_polys(6) * num_faces_per_poly(2) * vertex_per_face(3) * uv(2)

extern int16 cube_texcoords[3][6 * 2 * 3 * 2];
extern char tab_2E138[4 * 3];

//

struct XYZ {
	signed short x, y, z;
};

struct cubeCursor {
	unsigned char	sides[6];	// spr idx for each side
	unsigned char	kind;
	signed char	speed;
};

} // End of namespace Cryo

#endif
