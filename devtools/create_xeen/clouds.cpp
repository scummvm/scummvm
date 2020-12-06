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

#include "file.h"
#include "clouds.h"

static const char *const MAP_NAMES[86] = {
	"", "Area A1", "Area A2", "Area A3", "Area A4", "Area B1", "Area B2",
	"Area B3", "Area B4", "Area C1", "Area C2", "Area C3", "Area C4",
	"Area D1", "Area D2", "Area D3", "Area D4", "Area E1", "Area E2",
	"Area E3", "Area E4", "Area F1", "Area F2", "Area F3", "Area F4",
	"Witch Clouds", "High Magic Clouds", "Clouds of Xeen", "Vertigo",
	"Nightshadow", "Rivercity", "Asp", "Winterkill", "Dwarf Mine 1",
	"Dwarf Mine 2", "Dwarf Mine 3", "Dwarf Mine 4", "Dwarf Mine 5",
	"Deep Mine Alpha", "Deep Mine Theta", "Deep Mine Kappa",
	"Deep Mine Omega", "Cave of Illusion Level 1", "Cave of Illusion Level 2",
	"Cave of Illusion Level 3", "Cave of Illusion Level 4",
	"Volcano Cave Level 1", "Volcano Cave Level 2", "Volcano Cave Level 3",
	"Shangri-La", "Dragon Cave", "Witch Tower Level 1", "Witch Tower Level 2",
	"Witch Tower Level 3", "Witch Tower Level 4", "Tower of High Magic Level 1",
	"Tower of High Magic Level 2", "Tower of High Magic Level 3",
	"Tower of High Magic Level 4", "Darzog's Tower Level 1",
	"Darzog's Tower Level 2", "Darzog's Tower Level 3", "Darzog's Tower Level 4",
	"Burlock Dungeon", "Castle Burlock Level 1", "Castle Burlock Level 2",
	"Castle Burlock Level 3", "Basenji Dungeon", "Castle Basenji Level 1",
	"Castle Basenji Level 2", "Castle Basenji Level 3", "Newcastle Dungeon",
	"Newcastle Foundation", "Newcastle Level 1", "Newcastle Level 2",
	"Xeen's Castle Level 1", "Xeen's Castle Level 2", "Xeen's Castle Level 3",
	"Xeen's Castle Level 4", "Ancient Temple of Yak", "Tomb of a 1000 Terrors",
	"Golem Dungeon", "Sphinx Body", "Sphinx Head", "Sphinx Dungeon",
	"The Warzone"
};

void writeCloudsData(CCArchive &cc, const char *darkName) {
	Common::File darkFile;
	if (darkFile.open(darkName, Common::kFileReadMode)) {
		CCArchive darkCc(darkFile, kRead);

		Common::MemFile mae = darkCc.getMember("mae.xen");
		Common::MemFile spells = darkCc.getMember("spells.xen");
		Common::MemFile animInfo = darkCc.getMember("clouds.dat");
		Common::MemFile monsters = darkCc.getMember("xeen.mon");
		Common::MemFile wallPics = darkCc.getMember("xeenpic.dat");
		Common::MemFile mirror = darkCc.getMember("xeenmirr.txt");

		cc.add("mae.cld", mae);
		cc.add("spells.cld", spells);
		cc.add("animinfo.cld", animInfo);
		cc.add("monsters.cld", monsters);
		cc.add("wallpics.cld", wallPics);
		cc.add("xeenmirr.txt", mirror);

		Common::MemFile mapNames;
		for (int idx = 0; idx < 86; ++idx)
			mapNames.syncString(MAP_NAMES[idx]);
		cc.add("mapnames.cld", mapNames);

		darkFile.close();
	} else {
		error("Could not find %s to get Clouds data", darkName);
	}
}
