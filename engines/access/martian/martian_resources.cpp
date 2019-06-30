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

#include "access/martian/martian_resources.h"
#include "access/access.h"

namespace Access {

namespace Martian {

MartianResources::~MartianResources() {
	delete _font6x6;
	delete _font3x5;
}

void MartianResources::load(Common::SeekableReadStream &s) {
	Resources::load(s);
	uint count;

	// Get the offset of the general shared data for the game
	uint entryOffset = findEntry(_vm->getGameID(), 2, 0, (Common::Language)0);
	s.seek(entryOffset);

	// Read in the cursor list
	count = s.readUint16LE();
	CURSORS.resize(count);
	for (uint idx = 0; idx < count; ++idx) {
		uint count2 = s.readUint16LE();
		CURSORS[idx].resize(count2);
		s.read(&CURSORS[idx][0], count2);
	}

	// Load font data
	_font6x6 = new MartianFont(6, s);
	_font3x5 = new MartianFont(5, s);
}

/*------------------------------------------------------------------------*/

const int SIDEOFFR[] = {  4, 0, 7, 10,  3, 1, 2, 13, 0, 0, 0, 0 };
const int SIDEOFFL[] = { 11, 6, 1,  4, 10, 6, 1,  4, 0, 0, 0, 0 };
const int SIDEOFFU[] = {  1, 2, 0,  2,  2, 1, 1,  0, 0, 0, 0, 0 };
const int SIDEOFFD[] = {  2, 0, 1,  1,  0, 1, 1,  1, 2, 0, 0, 0 };

const byte CREDIT_DATA[] = {
	0x1F, 0x00, 0x49, 0x00, 0x00, 0x00, 0xB7, 0x00, 0x49, 0x00,
	0x01, 0x00, 0x79, 0x00, 0x6F, 0x00, 0x02, 0x00, 0xFF, 0xFF,
	0xEA, 0x01, 0x75, 0x00, 0x46, 0x00, 0x03, 0x00, 0x46, 0x00,
	0x5E, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xEA, 0x01, 0x72, 0x00,
	0x3E, 0x00, 0x05, 0x00, 0x46, 0x00, 0x57, 0x00, 0x04, 0x00,
	0x5C, 0x00, 0x6E, 0x00, 0x06, 0x00, 0xFF, 0xFF, 0xEA, 0x01,
	0x63, 0x00, 0x48, 0x00, 0x07, 0x00, 0x2A, 0x00, 0x65, 0x00,
	0x08, 0x00, 0xFF, 0xFF, 0xEA, 0x01, 0x7E, 0x00, 0x39, 0x00,
	0x09, 0x00, 0x5C, 0x00, 0x57, 0x00, 0x06, 0x00, 0x45, 0x00,
	0x6B, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xEA, 0x01, 0x5F, 0x00,
	0x46, 0x00, 0x0A, 0x00, 0x67, 0x00, 0x62, 0x00, 0x0B, 0x00,
	0x47, 0x00, 0x76, 0x00, 0x0C, 0x00, 0xFF, 0xFF, 0xEA, 0x01,
	0x62, 0x00, 0x38, 0x00, 0x0D, 0x00, 0x47, 0x00, 0x55, 0x00,
	0x0E, 0x00, 0x49, 0x00, 0x6A, 0x00, 0x0F, 0x00, 0xFF, 0xFF,
	0xEA, 0x01, 0x18, 0x00, 0x22, 0x00, 0x10, 0x00, 0x17, 0x00,
	0x3E, 0x00, 0x11, 0x00, 0x16, 0x00, 0x52, 0x00, 0x12, 0x00,
	0xEE, 0x00, 0x7B, 0x00, 0x13, 0x00, 0xB5, 0x00, 0x93, 0x00,
	0x0B, 0x00, 0xFF, 0xFF, 0xF4, 0x01, 0xFF, 0xFF, 0xFF, 0xFF
};

const byte ICON_PALETTE[] = {
	0x3F, 0x3F, 0x00, 0x00, 0x07, 0x16,
	0x00, 0x0A, 0x1A, 0x00, 0x0D, 0x1F,
	0x00, 0x11, 0x28, 0x00, 0x15, 0x30,
	0x00, 0x19, 0x39, 0x00, 0x1B, 0x3F,
	0x00, 0x2D, 0x3A
};

const int RMOUSE[10][2] = {
	{ 7, 36 },    { 38, 68 },   { 70, 99 },   { 102, 125 }, { 128, 152 },
	{ 155, 185 }, { 188, 216 }, { 219, 260 }, { 263, 293 }, { 295, 314 }
};

const char *const TRAVDATA[] = {
	"GALACTIC PICTURES", "TERRAFORM", "WASHROOM", "MR. BIG", "ALEXIS' HOME",
	"JOHNNY FEDORA", "JUNKYARD IN", "TEX'S OFFICE", "MURDER SCENE", "PLAZA HOTEL",
	"RESTAURANT", "GIFT SHOP", "LOVE SCENE", "RICK LOGAN", "HUT",
	"SMUGGLERS BASE", "PYRAMID", "CASINO", "CAS LOBBY", "BAR",
	"DUCTWORK", "RESTROOM", "OFFICE", "SAFE", "ALLEY",
	"POWER PLANT", "PLANT OFFICE", "PLANT ROOM", "TEMPLE", "IN TEMPLE",
	"JANE MANSFIELD'S HOME", "AEROBICS ACADEMY", "DR. LAWRENCE BARKLEY", "COLONISTS CAMP", "IN SLUM",
	"REMOTE OUTPOST", "WALK", "CAVE", "PRISON", "ORACLE",
	"JOCQUES SPARROW", "MAC MALDEN", "CHANTAL VARGAS", "GUY CALLABERO", "ROCKWELL BACHE",
	"FERRIS COLLETTE", "NORA DESMOND ALEXANDER", "LOWELL PERCIVAL", "MICHELE BLOODWORTH", "BRADLEY ERICSON",
	"COOPER BRADBURY", nullptr
};

const char *const ASK_TBL[] = {
	"NONE", "MARSHALL ALEXANDER", "TERRAFORM CORP.", "COLLIER STANTON", "ROCKWELL BACHE",
	"JOCQUES SPARROW", "NORA DESMOND ALEXANDER", "GALACTIC PICTURES", "LAWRENCE BARKLEY", "TMS",
	"MAC MALDEN", "STANTON EXPEDITION", "LOWELL PERCIVAL", "CHANTAL VARGAS", "RICK LOGAN",
	"ALEXIS ALEXANDER", "FERRIS COLLETT", "GUY CALLABERO", "ORACLE STONE", "THOMAS DANGERFIELD",
	"JANE MANSFIELD", "STACY CRAWFORD", "DICK CASTRO", "ROCKY BULLWINKEL", "DEACON HAWKE",
	"NATHAN BLOODWORTH", "MICHELLE BLOODWORTH", "BRADLEY ERICSON", "COOPER BRADBURY", "MARTIAN MEMORANDUM",
	"JOHNNY FEDORA", "RHONDA FOXWORTH", "ANGELO ANDRETTI", "TEX MURPHY", "ROBERT BLOODWORTH",
	"LARRRY HAMMOND", nullptr
};

byte HELP[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const char *const SPEC7MESSAGE = {
	"THOMAS DANGERFIELD'S MAD EXPERIMENT OF ATTEMPTING TO HARNESS THE STONE'S POWER, ENDED HIS LIFE. DANGERFIELD WAS A DECENT HUMAN " \
	"BEING ONCE, BUT WAS DRIVEN INSANE BY HIS QUEST FOR THE ULTIMATE POWER. ALEXIS AND I DECIDE THAT DEACON HAWKE IS THE ONLY " \
	"LOGICAL CHOICE FOR THE STONE. WE ARRIVE AT THE TEMPLE WHERE SHE IS WAITING FOR US. SHE TURNS AND WHISPERS; 'YOU HAVE RETURNED " \
	"THE STONE TO THE MISTRESS OF THE LIGHT. YOU HAVE SURELY SAVED THE WORLD FROM ANNIHILATION BUT YOU MUST CONTINUE TO BE DILIGENT. " \
	"MANKIND MAY YET PROVE TO BE THE AUTHOR OF HIS OWN DEMISE. REVERENCE LIFE. PROTECT THE LIVING THINGS AND RECYCLE. AND NOW FOR " \
	"THE SAFETY OF MANKIND, I MUST TAKE THE STONE. PERHAPS SOMEDAY, WHEN THE HUMAN RACE IS READY, IT WILL BE RETURNED. UNTIL THEN "\
	"FAREWELL...'"
};

const byte _byte1EEB5[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 1, 0, 1, 1, 1,
	1
};

const int PICTURERANGE[][2] = {
// { min X, max X}, {min Y, max Y}
	{ 20, 30 },  { 82, 87 },
	{ 20, 30 },  { 105, 110 },
	{ 0, 8 },    { 92, 100 },
	{ 42, 46 },  { 92, 100 },
	{ 9, 41 },   { 88, 104 },
	{ 9, 41 },   { 117, 133 },
	{ -1, -1 }
};

} // End of namespace Martian
} // End of namespace Access
