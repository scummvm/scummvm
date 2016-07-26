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

#include "martian_resources.h"

namespace Martian {


const byte MOUSE0[] = {
	// hotspot x and y, uint16 LE
	0, 0, 0, 0,
	// byte 1: number of skipped pixels
	// byte 2: number of plotted pixels
	// then, pixels
	0, 2, 0xF7, 5,
	0, 3, 0xF7, 0xF7, 5,
	0, 3, 0xF7, 0xF7, 5,
	0, 4, 0xF7, 0xF7, 0xF7, 5,
	0, 4, 0xF7, 0xF7, 0xF7, 5,
	0, 5, 0xF7, 0xF7, 0xF7, 0xF7, 5,
	0, 5, 0xF7, 0xF7, 0xF7, 0xF7, 5,
	0, 6, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 5,
	0, 6, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 5,
	0, 7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 5,
	0, 6, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 5,
	0, 5, 0xF7, 0xF7, 0xF7, 0xF7, 5,
	2, 3, 0xF7, 0xF7, 5,
	3, 3, 0xF7, 0xF7, 5,
	3, 3, 0xF7, 0xF7, 5,
	4, 2, 0xF7, 5
};
const byte MOUSE1[] = {
	// hotspot x and y, uint16 LE
	7, 0, 7, 0,
	// byte 1: number of skipped pixels
	// byte 2: number of plotted pixels
	// then, pixels
	6, 1, 0xF7,
	4, 5, 0xFF, 0xFF, 0, 0xFF, 0xFF,
	3, 7, 0xFF, 0, 0, 0, 0, 0, 0xFF,
	2, 9, 0xFF, 0, 0, 0, 0xF7, 0, 0, 0, 0xFF,
	1, 11, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF,
	1, 11, 0xFF, 0, 0, 0, 0, 0xF7, 0, 0, 0, 0, 0xFF,
	0, 13, 0xF7, 0, 0, 0xF7, 0, 0xF7, 0, 0xF7, 0, 0xF7, 0, 0, 0xF7,
	1, 11, 0xFF, 0, 0, 0, 0, 0xF7, 0, 0, 0, 0, 0xFF,
	1, 11, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF,
	2, 9, 0xFF, 0, 0, 0, 0xF7, 0, 0, 0, 0xFF,
	3, 7, 0xFF, 0, 0, 0, 0, 0, 0xFF,
	4, 5, 0xFF, 0xFF, 0, 0xFF, 0xFF,
	6, 1, 0xF7,
	0, 0,
	0, 0,
	0, 0
};
const byte MOUSE2[] = {
	// hotspot x and y, uint16 LE
	8, 0, 8, 0,
	// byte 1: number of skipped pixels
	// byte 2: number of plotted pixels
	// then, pixels
	0, 0,
	0, 0,
	7, 2, 4, 5,
	7, 2, 4, 5,
	7, 2, 4, 5,
	7, 2, 4, 5,
	7, 2, 4, 5,
	2, 12, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 5,
	7, 2, 4, 5,
	7, 2, 4, 5,
	7, 2, 4, 5,
	7, 2, 4, 5,
	7, 2, 4, 5,
	0, 0,
	0, 0,
	0, 0
};
const byte MOUSE3[] = {
	// hotspot x and y, uint16 LE
	0, 0, 0, 0,
	// byte 1: number of skipped pixels
	// byte 2: number of plotted pixels
	// then, pixels
	0, 11, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	0, 12, 6, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 5,
	0, 12, 6, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 5,
	0, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 5, 0, 0, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 6, 5, 6, 6, 6, 6, 6, 5,
	0, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5,
	1, 11, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	0, 0,
	0, 0,
	0, 0
};

const byte *const CURSORS[MARTIAN_NUM_CURSORS] = {
	MOUSE0, MOUSE1, MOUSE2, MOUSE3
};
const uint CURSOR_SIZES[MARTIAN_NUM_CURSORS] = {
	sizeof(MOUSE0), sizeof(MOUSE1), sizeof(MOUSE2), sizeof(MOUSE3)
};

const char *const ROOM_DESCR[48] = {
	nullptr, "TBD ROOM_TABLE1", "TBD ROOM_TABLE2", "TBD ROOM_TABLE3", "TBD ROOM_TABLE4",
	"TBD ROOM_TABLE5", "TBD ROOM_TABLE6", "TBD ROOM_TABLE7", "TBD ROOM_TABLE8", "TBD ROOM_TABLE9",
	nullptr, "TBD ROOM_TABLE11", nullptr, "TBD ROOM_TABLE13", "TBD ROOM_TABLE14",
	"TBD ROOM_TABLE15", "TBD ROOM_TABLE16", "TBD ROOM_TABLE17", "TBD ROOM_TABLE18", nullptr,
	nullptr, "TBD ROOM_TABLE21", "TBD ROOM_TABLE22", "TBD ROOM_TABLE23", "TBD ROOM_TABLE24",
	"TBD ROOM_TABLE25", "TBD ROOM_TABLE26", "TBD ROOM_TABLE27", "TBD ROOM_TABLE28", "TBD ROOM_TABLE29",
	"TBD ROOM_TABLE30", "TBD ROOM_TABLE31", "TBD ROOM_TABLE32", "TBD ROOM_TABLE33", nullptr,
	"TBD ROOM_TABLE35", nullptr, "TBD ROOM_TABLE37", "TBD ROOM_TABLE38", "TBD ROOM_TABLE39",
	nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, "TBD ROOM_TABLE47"
};

const byte DEATH_SCREENS_ENG[20] = {
	5, 5, 3, 3, 7, 4, 6, 2, 2, 2, 1, 5, 3, 5, 2, 8, 5, 3, 8, 5
};

const char *const DEATH_TEXT_ENG[20] = {
	"A VICIOUS THUG PULLS OUT HIS GUN AND AIR CONDITIONS YOUR BRAIN.",
	"BIG DICK COMES BACK AND ANNOUNCES YOUR TIME IS UP. ONE OF HIS BOYS PROCEEDS TO PART YOUR EYEBROWS.",
	"ALTHOUGH HIS FIRST SHOT MISSED, THE PUNK FINDS YOU AND TURNS YOU INTO A DOUGHNUT.",
	"THE CREEP SPOTS YOU. HE TURNS AND FIRES HIS WEAPON. IT BURNS A HOLE A BUZZARD CAN FLY THROUGH.",
	"OBVIOUSLY RICK LOGAN HAS A FEW TRICK UP HIS SLEEVE. A TREMENDOUS WEIGHT HITS YOUR HEAD. YOU MUMBLE; WATCH OUT FOR THAT TREE...",
	"SLOWLY SINKING IN THE SLIMY OOZE, YOU THINK OF SEVERAL JELLO WRESTLING MATCHES YOU'VE ATTENDED. BUT NO MORE...",
	"THE PATH SUDDENLY GIVES WAY AND YOU FEEL MANY STAKES TEAR THROUGH YOUR FLESH. HOW DO YOU LIKE YOUR STAKE",
	"THE SNAKE SINKS ITS FANGS INTO YOU LEG. THE POISON WORKS QUICKLY. THE SNAKE THEN SWALLOWS YOU WHOLE.",
	"YOU FADE AWAY, GLOWING LIKE A LIGHTBULB.",
	"YOU TOUCH THE BUBBLING RADIOACTIVE SELTZER. IT IMMEDIATELY CAUSES VITAL ORGANS TO ELONGATE AND EXPLODE. YOU DIE WITH AN ABSURD AND FOOLISH LOOK ON YOUR FACE.",
	"THE DOGS PRETTY HUNGRY. IT WON'T TAKE HIM LONG TO FINISH SO SIT BACK AND ENJOY IT.",
	"ROCKY DOESN'T LIKE BEING FOLLOWED. HE DECIDES TO BEAT YOU. WITHIN AND INCH OF YOUR LIFE. UNFORTUNATELY, HE MISJUDGED THE DISTANCE",
	"YOU STUMBLE INTO DEADLY LASER FIRE.",
	"THE OUTPOST AND YOUR BODY PARTS ARE BLOWN TO KINGDOM COME.",
	"YOU REACH THE TOP, BUT YOUR AIR SOON RUNS OUT LEAVING YOU BREATHLESS.",
	"YOU DIE IN THE FIERY EXPLOSION.",
	"YOU FALL HUNDREDS OF FEET TO YOUR DEATH.",
	"YOU WALK ONTO A PRESSURE SENSITIVE SECURITY PAD. A LASER ZEROS IN AND BLOWS A HOLE THE SIZE OF A SUBARU TIRE THROUGH YOU.",
	"DANGERFIELD'S EXPERIMENT BACKFIRES. IT RELEASES A DEMON FROM HIS SUBCONSCIOUS WHICH DESTROYS THE ENTIRE PLANET.",
	"ONCE DANGERFIELD GETS OUT OF HIS CHAMBER, HE PULLS OUT A WEAPON AND LETS YOU HAVE IT."
};

const char *const INVENTORY_NAMES_ENG[55] = {
	"CAMERA", "LENS", "PHOTOS", "MAIL", "GUN",
	"CASH", "COMLINK", "AMMO", "LOCKPICK KIT", "EARRING",
	"RECIEPTS", "PAPER", "LADDER", "BOOTS", "DOCUMENTS",
	"KNIFE", "DAGGER", "KEYS", "ROCK", "LOG",
	"SHOVEL", "STONE", "REMOTE CONTROL", "FOOD AND WATER", "DOOR CARD KEY",
	"FLASHLIGHT", "INTERLOCK KEY", "TOOLS", "REBREATHER", "JET PACK",
	"ROD", "HCL2", "SAFE CARD KEY", "TUNING FORK", "STONE",
	"ROSE", "KEY", "NOTE", "ALLEN WRENCH", "HOVER BOARD",
	"BLUE PRINTS", "LETTER", "MEMORANDUM", "MARKERS", "FILM",
	"ANDRETTI FILM", "GLASSES", "AMULET", "FACIAL KIT", "CAT FOOD",
	"MONKEY WRENCH", "BIG DICK CARD", "BRA", "BOLT", nullptr
};

} // End of namespace Amazon
