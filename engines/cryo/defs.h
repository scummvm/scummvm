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
	( (char *)(array) + READ_LE_UINT16((idx) * 2 + (char *)(array)) )

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
	PER_KING = 0,
	PER_DINA,       // 0x12
	PER_TAU,        // 0x24
	PER_MONK,       // 0x36
	PER_JABBER,     // 0x48
	PER_ELOI,       // 0x5A
	PER_MUNGO,      // 0x6C
	PER_EVE,        // 0x7E
	PER_SHAZIA,     // 0x90
	PER_MAMMI,      // 0xA2
	PER_MAMMI_1,    // 0xB4
	PER_MAMMI_2,    // 0xC6
	PER_MAMMI_3,    // 0xD8
	PER_MAMMI_4,    // 0xEA
	PER_MAMMI_5,    // 0xFC
	PER_MAMMI_6,    // 0x10E
	PER_BAMBOO,     // 0x120
	PER_KABUKA,     // 0x132
	PER_GUARDS,     // 0x144
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

#include "common/pack-start.h"

struct perso_t {
	uint16  _roomNum;    // room this person currently in
	uint16  _actionId;   // TODO: checkme
	uint16  _partyMask;      // party bit mask
	byte    _id;         // character
	byte    _flags;      // flags and kind
	byte    _roomBankId;// index in _personRoomBankTable for specific room banks
	byte    _spriteBank;       // sprite bank
	uint16  _items;      // inventory
	uint16  _powers;     // obj of power bitmask
	byte    _targetLoc;  // For party member this is mini sprite index
	byte    _lastLoc;    // For party member this is mini sprite x offset
	byte    _speed;      // num ticks per step
	byte    _steps;      // current ticks
};

class EdenGame;

struct phase_t {
	int16 _id;
	void (EdenGame::*disp)();
};

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
	int    _locations;      // index in _objectLocations
	uint16  _itemMask;
	uint16  _powerMask;          // object of power bitmask
	int16  _count;
};

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

struct Dialog {
	int8        _flags;          // 0-3 - action index, 4 - highest bit of contidion index, rest is DialogFlags
	int8        _condNumLow;     // condition index low bits
	int8        _textCondHiMask; // 0-1 text index hi bits, 2-5 - perso mask num, 6-7 condition index hi bits
	int8        _textNumLow;     // text line index low bits
};

struct tape_t {
	int16       _textNum;
	perso_t     *_perso;
	int16       _party;
	int16       _roomNum;
	int16       _backgroundBankNum;
	Dialog      *_dialog;
};

struct Follower {      // Characters on Mirror screen
	int8        _id;         // character
	int8        _spriteNum;      // sprite number
	int16       sx;
	int16       sy;
	int16       ex;
	int16       ey;
	int16       _spriteBank;
	int16       ff_C;
	int16       ff_E;
};

struct Icon {
	int16  sx;
	int16  sy;
	int16  ex;
	int16  ey;
	uint16 _cursorId;          // & 0x8000 - inactive/hidden
	uint32 _actionId;
	uint32 _objectId;
};

struct Goto {
	byte   _areaNum;    // target area
	byte   _curAreaNum; // current area
	byte   _enterVideoNum;
	byte   _travelTime;     // time to skip while in travel
	byte   _arriveVideoNum;
};

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

struct Room {
	byte    _id;
	byte    _exits[4];   //TODO: signed?
	byte    _flags;
	uint16  _bank;
	uint16  _party;
	byte    _level;      // Citadel level
	byte    _video;
	byte    _location;
	byte    _backgroundBankNum; // bg/mirror image number (relative)
};

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

struct Area {
	byte   _num;
	byte   _type;
	uint16 _flags;
	uint16 _firstRoomIdx;
	byte   _citadelLevel;
	byte   _placeNum;
	Room  *_citadelRoomPtr;
	int16  _visitCount;
};

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
	gfNone = 0,
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
	byte   _areaNum;
	byte   _areaVisitCount;
	byte   _menuItemIdLo;
	byte   _menuItemIdHi;   //TODO: pad?
	uint16  _randomNumber;   //TODO: this is randomized in pc ver and used by some conds. always zero on mac
	uint16  _gameTime;
	uint16  _gameDays;
	uint16  _chrono;
	uint16  _eloiDepartureDay;
	uint16  _roomNum;        // current room number
	uint16  _newRoomNum;     // target room number selected on world map
	uint16  _phaseNum;
	uint16  _metPersonsMask1;
	uint16  _party;
	uint16  _partyOutside;
	uint16  _metPersonsMask2;
	uint16  _var1C;    //TODO: write-only?
	uint16  _phaseActionsCount;
	uint16  _curAreaFlags;
	uint16  _curItemsMask;
	uint16  _curPowersMask;
	uint16  _curPersoItems;
	uint16  _curCharacterPowers;
	uint16  _wonItemsMask;
	uint16  _wonPowersMask;
	uint16  _stepsToFindAppleFast;
	uint16  _stepsToFindAppleNormal;
	uint16  _roomPersoItems; //TODO: write-only?
	uint16  _roomCharacterPowers;    //TODO: write-only?
	uint16  _gameFlags;
	uint16  _curVideoNum;
	uint16  _morkusSpyVideoNum1; //TODO: pad?
	uint16  _morkusSpyVideoNum2; //TODO: pad?
	uint16  _morkusSpyVideoNum3; //TODO: pad?
	uint16  _morkusSpyVideoNum4; //TODO: pad?
	byte   _newMusicType;
	byte   _var43;
	byte   _videoSubtitleIndex;
	byte   _partyInstruments;   // &1 - Bell for Monk, &2 - Drum for Thugg
	byte   _monkGotRing;
	byte   _chronoFlag;
	byte   _curRoomFlags;
	byte   _endGameFlag;
	byte   _lastInfo;
	bool   _autoDialog;
	byte   _worldTyranSighted;
	byte   _var4D;
	byte   _var4E;
	byte   _worldGaveGold;
	byte   _worldHasTriceraptors;
	byte   _worldHasVelociraptors;
	byte   _worldHasTyran;
	byte   _var53;
	byte   _var54;  //CHEKME: Used?
	byte   _var55;  //TODO: pad?
	byte   _gameHours;
	byte   _textToken1;
	byte   _textToken2; //TODO: pad?
	byte   _eloiHaveNews;
	byte   _dialogFlags;
	byte   _curAreaType;
	byte   _curCitadelLevel;
	byte   _newLocation;
	byte   _prevLocation;
	byte   _curPersoFlags;
	byte   _var60;
	byte   _eventType;
	byte   _var62;  //TODO: pad?
	byte   _curObjectId;
	byte   _curObjectFlags;
	byte   _var65;  //TODO: pad?
	byte   _roomCharacterType;
	byte   _roomCharacterFlags;
	byte   _narratorSequence;
	byte   _var69;
	byte   _var6A;
	byte   _frescoNumber;
	byte   _var6C;  //TODO: pad?
	byte   _var6D;  //TODO: pad?
	byte   _labyrinthDirections;
	byte   _labyrinthRoom;
	Dialog *_dialogPtr;
	tape_t *_tapePtr;
	Dialog *_nextDialogPtr;
	Dialog *_narratorDialogPtr;
	Dialog *_lastDialogPtr;
	Icon *_nextRoomIcon;
	byte   *_sentenceBufferPtr;
	Room *_roomPtr;
	Area *_areaPtr;
	Area *_lastAreaPtr;
	Area *_curAreaPtr;
	Room *_citaAreaFirstRoom;
	perso_t *_characterPtr;
	perso_t *_roomCharacterPtr;
	byte   _lastInfoIdx;
	byte   _nextInfoIdx;
	byte   *_persoSpritePtr;
	byte   *_persoSpritePtr2;
	byte   *_curCharacterAnimPtr;
	byte   *_varC2; //TODO: image desc arr
	int16  _iconsIndex;
	int16  _curObjectCursor;    // TODO: useless?
	int16  _varCA;
	int16  _varCC;        //TODO: unused/pad
	int16  _characterImageBank; //TODO: unsigned?
	uint16  _roomImgBank;
	uint16  _characterBackgroundBankIdx;
	uint16  _varD4;  //TODO: unsigned?
	uint16  _frescoeWidth;
	uint16  _frescoeImgBank;
	uint16  _varDA;  //TODO: pad?
	uint16  _varDC;  //TODO: pad?
	uint16  _roomBaseX;
	uint16  _varE0;  //TODO: pad?
	uint16  _dialogType;
	uint16  _varE4;  //TODO: pad?
	uint16  _currMusicNum;
	int16   _textNum;
	uint16  _travelTime;
	uint16  _varEC;  //TODO: pad?
	byte   _displayFlags;
	byte   _oldDisplayFlags;
	byte   _drawFlags;
	byte   _varF1;
	byte   _varF2;
	byte   _menuFlags;
	byte   _varF4;  //TODO: write-only?
	byte   _varF5;
	byte   _varF6;
	byte   _varF7;
	byte   _varF8;  //TODO: pad?
	byte   _varF9;  //TODO: pad?
	byte   _varFA;  //TODO: pad?
	byte   _animationFlags;
	byte   _giveObj1;
	byte   _giveObj2;
	byte   _giveObj3;
	byte   _var100;
	byte   _roomVidNum;
	byte   _mirrorEffect;
	byte   _var103;
	byte   _roomBackgroundBankNum;
	byte   _valleyVidNum;
	byte   _updatePaletteFlag;
	byte   _inventoryScrollPos;
	byte   _objCount;
	byte   _textBankIndex;
	byte   _prefLanguage;
	byte   _prefMusicVol[2];
	byte   _prefVoiceVol[2];
	byte   _prefSoundVolume[2];
	byte   _citadelAreaNum;
	byte   _var113;
	byte   _lastPlaceNum;
	byte   _saveEnd; // TODO: This has to be removed
	int16  _textWidthLimit;
	byte   _numGiveObjs;
	byte   _var119;     // unused
};

struct PakHeaderItem {
	Common::String _name; //[16];
	int32 _size;
	int32 _offs;
	char  _flag;
};

class PakHeaderNode {
public:
	PakHeaderNode(int count);
	~PakHeaderNode();

	uint16    _count;
	PakHeaderItem* _files;
};

#include "common/pack-end.h"

struct Citadel {
	int16 _id;
	int16 _bank[8];
	int16 _video[8];
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

struct CubeFace {
	int    tri;
	char   ff_4;
	char   ff_5;

	byte   *_texturePtr;
	uint16 *_indices;
	int16  *_uv;
};

struct Point3D {
	int16	x;
	int16	y;
	int16	z;
};

struct Cube {
	int       _num;
	CubeFace **_faces;
	Point3D   *_projection;    // projected XYZ coords
	Point3D   *_vertices;
};

struct XYZ {
	signed short x, y, z;
};

struct CubeCursor {
	uint8 _sides[6];	// spr idx for each side
	uint8 _kind;
	int8  _speed;
};

} // End of namespace Cryo

#endif
