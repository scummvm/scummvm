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
	if (!isDemo()) {
		loadAssetsFullGame();
		return;
	}
	Common::String demoVersion = "Wetlands Demo Disc";
	if (demoVersion == "Wetlands Demo Disc")
		loadAssetsDemoDisc();
	else if (demoVersion == "Personal Computer World Disc")
		loadAssetsPCW();
	else if (demoVersion == "Personal Computer Gamer Disc")
		loadAssetsPCG();
	else
		error("Invalid demo version: \"%s\"", demoVersion.c_str());
}


void WetEngine::loadAssetsDemoDisc() {
	LibFile *missions = loadLib("", "wetlands/c_misc/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

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

	h.rect = Common::Rect(242, 424, 500, 480);
	cl = new ChangeLevel();
	cl->level = "<movies>";
	h.actions.clear();
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
	intro.trans.level = "c31.mi_";
	intro.trans.intros.push_back("movie/nw_logo.smk");
	intro.trans.intros.push_back("movie/hypnotix.smk");
	intro.trans.intros.push_back("movie/wetlogo.smk");
	intro.trans.frameImage = "wetlands/c_misc/c.s";
	intro.trans.frameNumber = 0;
	_levels["<intro>"] = intro;

	Level movies;
	movies.trans.level = "<quit>";
	movies.trans.intros.push_back("movie/nw_logo.smk");
	movies.trans.intros.push_back("movie/hypnotix.smk");
	movies.trans.intros.push_back("movie/wetlogo.smk");
	movies.trans.intros.push_back("movie/c42e1s.smk");
	movies.trans.intros.push_back("movie/c23e1s.smk");
	movies.trans.intros.push_back("movie/c20o1s.smk");
	movies.trans.intros.push_back("movie/c23d1s.smk");
	movies.trans.intros.push_back("movie/c40o1s.smk");
	movies.trans.intros.push_back("movie/c31d1s.smk");
	movies.trans.intros.push_back("movie/c42d1s.smk");
	movies.trans.intros.push_back("movie/c44d1s.smk");
	movies.trans.intros.push_back("movie/c44e1s.smk");
	movies.trans.intros.push_back("movie/c32d1s.smk");
	movies.trans.intros.push_back("movie/c22e1s.smk");
	movies.trans.intros.push_back("movie/c31e1s.smk");
	movies.trans.intros.push_back("movie/gameover.smk");
	movies.trans.frameImage = "";
	movies.trans.frameNumber = 0;
	_levels["<movies>"] = movies;

	Common::String arclevel = "c31.mi_";
	debugC(1, kHypnoDebugParser, "Parsing %s", arclevel.c_str());
	Common::String arc;
	Common::String list;
	splitArcadeFile(arclevel, arc, list);
	parseArcadeShooting("wetlands", arclevel, arc);
	_levels[arclevel].arcade.id = 0;
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
	_levels[arclevel].arcade.prefix = "wetlands";
	_levels[arclevel].arcade.levelIfWin = "c52.mi_";
	_levels[arclevel].arcade.levelIfLose = "c52.mi_";

	arclevel = "c52.mi_";
	arc.clear();
	list.clear();
	splitArcadeFile(arclevel, arc, list);
	parseArcadeShooting("wetlands", arclevel, arc);
	_levels[arclevel].arcade.id = 1;
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
	_levels[arclevel].arcade.prefix = "wetlands";
	_levels[arclevel].arcade.levelIfWin = "<gameover>";
	_levels[arclevel].arcade.levelIfLose = "<gameover>";

	Level over;
	over.trans.level = "<quit>";
	over.trans.intros.push_back("movie/gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("", "wetlands/c_misc/fonts.lib", true);
	loadLib("wetlands/sound/", "wetlands/c_misc/sound.lib", true);
}

void WetEngine::loadAssetsPCW() {

	LibFile *missions = loadLib("", "c_misc/missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Level intro;
	intro.trans.level = "c11.mis";
	intro.trans.intros.push_back("c_misc/nw_logo.smk");
	intro.trans.intros.push_back("c_misc/h.s");
	intro.trans.intros.push_back("c_misc/wet.smk");
	intro.trans.frameImage.clear();
	_levels["<start>"] = intro;

	Common::String arclevel = "c11.mis";
	Common::String arc;
	Common::String list;
	splitArcadeFile(arclevel, arc, list);
	parseArcadeShooting("wetlands", arclevel, arc);
	_levels[arclevel].arcade.id = 0;
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
	_levels[arclevel].arcade.prefix = "";
	_levels[arclevel].arcade.levelIfWin = "<gameover>";
	_levels[arclevel].arcade.levelIfLose = "<gameover>";

	Level over;
	over.trans.level = "<quit>";
	over.trans.intros.push_back("movie/gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("", "c_misc/sound.lib", false);
}

void WetEngine::loadAssetsPCG() {

	LibFile *missions = loadLib("", "missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Level intro;
	intro.trans.level = "c31.mi_";	
	intro.trans.intros.push_back("nw_logo.smk");
	intro.trans.intros.push_back("h.s");
	intro.trans.intros.push_back("wet.smk");
	intro.trans.frameImage.clear();
	_levels["<start>"] = intro;

	Common::String arclevel = "c31.mi_";
	debugC(1, kHypnoDebugParser, "Parsing %s", arclevel.c_str());
	Common::String arc;
	Common::String list;
	splitArcadeFile(arclevel, arc, list);
	parseArcadeShooting("wetlands", arclevel, arc);
	_levels[arclevel].arcade.id = 0;
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
	_levels[arclevel].arcade.prefix = "";
	_levels[arclevel].arcade.levelIfWin = "<gameover>";
	_levels[arclevel].arcade.levelIfLose = "<gameover>";

	Level over;
	over.trans.level = "<quit>";
	over.trans.intros.push_back("gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("", "sound.lib", false);
}


void WetEngine::loadAssetsFullGame() {
	LibFile *missions = loadLib("", "c_misc/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Level intro;
	intro.trans.level = "c111.mi_";
	intro.trans.intros.push_back("c_misc/logo.smk");
	intro.trans.intros.push_back("c_misc/nw_logo.smk");
	intro.trans.intros.push_back("c_misc/hypnotix.smk");
	intro.trans.intros.push_back("c_misc/wetlogo.smk");
	_levels["<start>"] = intro;

	Common::String arclevel = "c111.mi_";
	debugC(1, kHypnoDebugParser, "Parsing %s", arclevel.c_str());
	Common::String arc;
	Common::String list;
	splitArcadeFile(arclevel, arc, list);
	debug("%s", arc.c_str());
	parseArcadeShooting("", arclevel, arc);
	_levels[arclevel].arcade.id = 0;
	_levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
	_levels[arclevel].arcade.prefix = "";
	//_levels[arclevel].arcade.levelIfWin = "c52.mi_";
	//_levels[arclevel].arcade.levelIfLose = "c52.mi_";

	//arclevel = "c52.mi_";
	// arc.clear();
	// list.clear();
	// splitArcadeFile(arclevel, arc, list);
	// parseArcadeShooting("wetlands", arclevel, arc);
	// _levels[arclevel].arcade.id = 1;
	// _levels[arclevel].arcade.shootSequence = parseShootList(arclevel, list);
	// _levels[arclevel].arcade.prefix = "wetlands";
	// _levels[arclevel].arcade.levelIfWin = "<gameover>";
	// _levels[arclevel].arcade.levelIfLose = "<gameover>";

	// Level over;
	// over.trans.level = "<quit>";
	// over.trans.intros.push_back("movie/gameover.smk");
	// _levels["<gameover>"] = over;

	loadLib("", "c_misc/fonts.lib", true);
	loadLib("sound/", "c_misc/sound.lib", true);
}

void WetEngine::showCredits() {
	MVideo video("c_misc/credits.smk", Common::Point(0, 0), false, false, false);
	runIntro(video);
}


} // End of namespace Hypno
