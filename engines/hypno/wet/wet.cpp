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

#include "hypno/hypno.h"

namespace Hypno {

WetEngine::WetEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void WetEngine::loadAssets() {
	LibFile *missions = loadLib("", "wetlands/c_misc/missions.lib");
	Common::ArchiveMemberList files;
	assert(missions->listMembers(files) > 0);

	// We need the list of files in an array, instead of a list
	Common::Array<Common::ArchiveMemberPtr> afiles;
	afiles.reserve(files.size());
	for (Common::ArchiveMemberList::const_iterator it = files.begin(); it != files.end(); ++it) {
		afiles.push_back(*it);
	}

	uint32 k = 0;

	Common::String arc;
	Common::String list;
	Common::String arclevel;

	Level start;
	Hotspot h;
	Hotspots hs;

	h.type = MakeMenu;
	Ambient *a = new Ambient();
	a->path = "movie/selector.smk";
	a->fullscreen = true;
	a->flag = "/LOOP";
	a->origin = Common::Point(0, 0);
	h.actions.push_back(a);

	hs.push_back(h);

	h.type = MakeHotspot;
	h.rect = Common::Rect(0, 424, 233, 462);
	h.actions.clear();
	h.smenu = nullptr;
	ChangeLevel *cl = new ChangeLevel();
	cl->level = "<intro>";
	h.actions.push_back(cl);

	hs.push_back(h);

	h.rect = Common::Rect(504, 424, 637, 480);
	Quit *q = new Quit();
	h.actions.clear();
	h.actions.push_back(q);

	hs.push_back(h);

	start.scene.hots = hs;
	_levels["<start>"] = start;

	Level intro;
	intro.trans.level = afiles[0]->getName();
	intro.trans.intros.push_back("movie/nw_logo.smk");
	intro.trans.intros.push_back("movie/hypnotix.smk");
	intro.trans.intros.push_back("movie/wetlogo.smk");
	intro.trans.frameImage = "wetlands/c_misc/c.s";
	intro.trans.frameNumber = 0;
	_levels["<intro>"] = intro;

	byte x;
	for (k = 0; k < afiles.size(); k++) {
		arc.clear();
		list.clear();
		arclevel = afiles[k]->getName();
		debugC(1, kHypnoDebugParser, "Parsing %s", arclevel.c_str());
		Common::SeekableReadStream *file = afiles[k]->createReadStream();
		while (!file->eos()) {
			x = file->readByte();
			arc += x;
			if (x == 'X') {
				while (!file->eos()) {
					x = file->readByte();
					if (x == 'Y')
						break;
					list += x;
				}
				break; // No need to keep parsing
			}
		}
		delete file;

		parseArcadeShooting("wetlands", arclevel, arc);
		_levels[arclevel].arcade.id = k;
		_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
		_levels[arclevel].arcade.prefix = "wetlands";
		if (k < files.size() - 1) {
			_levels[arclevel].arcade.levelIfWin = afiles[k + 1]->getName();
			// failing a level in the demo takes you to the next one anyway
			_levels[arclevel].arcade.levelIfLose = afiles[k + 1]->getName();
		}
	}

	// After finish the second level, it's always game over
	_levels[afiles[k - 1]->getName()].arcade.levelIfWin = "<gameover>";
	_levels[afiles[k - 1]->getName()].arcade.levelIfLose = "<gameover>";

	Level over;
	over.trans.level = "<quit>";
	over.trans.intros.push_back("movie/gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("", "wetlands/c_misc/fonts.lib");
	loadLib("wetlands/sound/", "wetlands/c_misc/sound.lib");
}

void WetEngine::showCredits() {
	MVideo video("c_misc/credits.smk", Common::Point(0, 0), false, false, false);
	runIntro(video);
}


} // End of namespace Hypno
