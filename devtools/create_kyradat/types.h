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

#ifndef CREATE_KYRADAT_TYPES_H
#define CREATE_KYRADAT_TYPES_H

#include "create_kyradat.h"

struct Room {
	byte index;
	uint16 north, east, south, west;
};

struct Shape {
	byte imageIndex;
	byte x, y, w, h;
	int8 xOffset, yOffset;
};

struct AmigaSfxTable {
	byte note;
	byte patch;
	uint16 duration;
	byte volume;
	byte pan;
};

struct HoFSequence {
	uint16 flags;
	char wsaFile[14];
	char cpsFile[14];
	byte fadeInTransitionType;
	byte fadeOutTransitionType;
	uint16 stringIndex1;
	uint16 stringIndex2;
	uint16 startFrame;
	uint16 numFrames;
	uint16 duration;
	uint16 xPos;
	uint16 yPos;
	uint16 timeout;
};

struct FrameControl {
	uint16 index;
	uint16 delay;
};

struct HoFNestedSequence {
	uint16 flags;
	char wsaFile[14];
	uint16 startFrame;
	uint16 endFrame;
	uint16 frameDelay;
	uint16 x;
	uint16 y;
	uint16 numControls;
	const FrameControl *wsaControl;
	uint16 fadeInTransitionType;
	uint16 fadeOutTransitionType;
};

struct HoFSeqItemAnimData {
	int16 itemIndex;
	uint16 y;
	uint16 frames[20];
};

struct ItemAnimDefinition {
	int16 itemIndex;
	byte numFrames;
	const FrameControl *frames;
};

struct LoLCharacter {
	uint16 flags;
	char name[11];
	byte raceClassSex;
	int16 id;
	byte curFaceFrame;
	byte tempFaceFrame;
	byte screamSfx;
	/*uint32 unused;*/
	uint16 itemsMight[8];
	uint16 protectionAgainstItems[8];
	uint16 itemProtection;
	int16 hitPointsCur;
	uint16 hitPointsMax;
	int16 magicPointsCur;
	uint16 magicPointsMax;
	byte field_41;
	uint16 damageSuffered;
	uint16 weaponHit;
	uint16 totalMightModifier;
	uint16 totalProtectionModifier;
	uint16 might;
	uint16 protection;
	int16 nextAnimUpdateCountdown;
	uint16 items[11];
	byte skillLevels[3];
	byte skillModifiers[3];
	uint32 experiencePts[3];
	byte characterUpdateEvents[5];
	byte characterUpdateDelay[5];
};

struct SpellProperty {
	uint16 spellNameCode;
	uint16 mpRequired[4];
	uint16 field_a;
	uint16 field_c;
	uint16 hpRequired[4];
	uint16 field_16;
	uint16 field_18;
	uint16 flags;
};

struct CompassDef {
	byte shapeIndex;
	int8 x, y;
	byte flags;
};

struct FlyingObjectShape {
	byte shapeFront;
	byte shapeBack;
	byte shapeLeft;
	byte drawFlags;
	byte flipFlags;
};

struct LoLButtonDef {
	uint16 buttonFlags;
	uint16 keyCode;
	uint16 keyCode2;
	int16 x, y;
	uint16 w, h;
	uint16 index;
	uint16 screenDim;
};

struct DarkMoonAnimCommand {
	byte command;
	byte obj;
	int16 x1;
	byte y1;
	byte delay;
	byte pal;
	byte x2, y2;
	byte w, h;
};

struct DarkMoonShapeDef {
	int16 index;
	byte x, y, w, h;
};

struct EoBCharacter {
	byte id;
	byte flags;
	//char name[11];
	int8 strengthCur, strengthMax;
	int8 strengthExtCur, strengthExtMax;
	int8 intelligenceCur, intelligenceMax;
	int8 wisdomCur, wisdomMax;
	int8 dexterityCur, dexterityMax;
	int8 constitutionCur, constitutionMax;
	int8 charismaCur, charismaMax;
	int16 hitPointsCur, hitPointsMax;
	int8 armorClass;
	byte disabledSlots;
	byte raceSex;
	byte cClass;
	byte alignment;
	int8 portrait;
	byte food;
	byte level[3];
	uint32 experience[3];
	uint32 mageSpellsAvailableFlags;
	uint16 inventory[27];
};

template<typename Type>
struct ArrayProvider {
	uint numEntries;
	const Type *data;
};

typedef ArrayProvider<const char *> StringListProvider;
typedef ArrayProvider<byte> ByteProvider;
typedef ArrayProvider<Room> RoomProvider;
typedef ArrayProvider<Shape> ShapeProvider;
typedef ArrayProvider<AmigaSfxTable> AmigaSfxTableProvider;

struct HoFSequenceProvider {
	uint numSequences;
	const HoFSequence *sequences;

	uint numNestedSequences;
	const HoFNestedSequence *nestedSequences;
};

typedef ArrayProvider<HoFSeqItemAnimData> HoFSeqItemAnimDataProvider;
typedef ArrayProvider<ItemAnimDefinition> ItemAnimDefinitionProvider;
typedef ArrayProvider<LoLCharacter> LoLCharacterProvider;
typedef ArrayProvider<SpellProperty> SpellPropertyProvider;
typedef ArrayProvider<CompassDef> CompassDefProvider;
typedef ArrayProvider<FlyingObjectShape> FlyingObjectShapeProvider;
typedef ArrayProvider<LoLButtonDef> LoLButtonDefProvider;
typedef ArrayProvider<uint16> Uint16Provider;
typedef ArrayProvider<uint32> Uint32Provider;
typedef ArrayProvider<DarkMoonAnimCommand> DarkMoonAnimCommandProvider;
typedef ArrayProvider<DarkMoonShapeDef> DarkMoonShapeDefProvider;
typedef ArrayProvider<EoBCharacter> EoBCharacterProvider;

class PAKFile;
void writeResource(PAKFile &out, const char *filename, ResTypes type, const void *provider);

#endif
