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

#include "common/events.h"
#include "common/config-manager.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"
#include "hypno/libfile.h"

namespace Hypno {

SpiderEngine::SpiderEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {}

void SpiderEngine::loadAssets() {
	if (!isDemo())
		loadAssetsFullGame();
	else
		loadAssetsDemo();
}

void SpiderEngine::loadAssetsFullGame() {

	Common::Language language = Common::parseLanguage(ConfMan.get("language"));
	Common::String prefix = "spider";
	if (language == Common::EN_USA) {
		if (!_installerArchive.open("DATA.Z"))
			error("Failed to open DATA.Z");

		SearchMan.add("DATA.Z", (Common::Archive *) &_installerArchive, 0, false);
	}

	Common::ArchiveMemberList files;
	LibFile *missions = loadLib("", "spider/c_misc/missions.lib", true);
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any file from missions.lib");

	//loadLib("", "c_misc/fonts.lib", true);
	loadLib("spider/sound.lib/", "spider/c_misc/sound.lib", true);

	Code *credits = new Code();
	credits->name = "<credits>";
	credits->prefix = prefix;
	_levels["<credits>"] = credits;
	credits->levelIfWin = "options.mi_";

	Scene *sc;
	ChangeLevel *cl;

	loadSceneLevel("mainmenu.mi_", "", prefix);
	loadSceneLevel("tryagain.mi_", "", prefix);

	cl = new ChangeLevel("mainmenu.mi_");
	sc = (Scene *) _levels["tryagain.mi_"];
	sc->hots[1].actions.push_back(cl);

	loadSceneLevel("options.mi_", "", prefix);
	loadSceneLevel("levels.mi_", "mv0t.mi_", prefix);
	loadSceneLevel("combmenu.mi_", "", prefix);

	loadSceneLevel("mv0t.mi_", "roof.mi_", prefix);
	_levels["mv0t.mi_"]->intros.push_back("cine/ints001s.smk");
	_levels["mv0t.mi_"]->music = "sound.lib/sm001.raw";

	loadSceneLevel("roof.mi_", "decide1.mi_", prefix);
	_levels["roof.mi_"]->music = "sound.lib/ros_mus.raw";

	loadSceneLevel("decide1.mi_", "", prefix);
	_levels["decide1.mi_"]->intros.push_back("cine/siren1s.smk");
	sc = (Scene *) _levels["decide1.mi_"];
	cl = new ChangeLevel("bank.mi_");
	sc->hots[2].actions.push_back(cl);
	cl = new ChangeLevel("c1"); // depens on the difficulty
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("bank.mi_", "", prefix);
	_levels["bank.mi_"]->intros.push_back("cine/swcs001s.smk");
	_levels["bank.mi_"]->levelIfWin = "<alley_selector>";

	Transition *alley_selector = new Transition("alley.mi_", "<puz_matr>");
	_levels["<alley_selector>"] = alley_selector;

	loadSceneLevel("alley.mi_", "<trans_apt_2>", prefix);
	_levels["alley.mi_"]->music = "alleymus.raw";
	_levels["alley.mi_"]->intros.push_back("cine/aleyc01s.smk");

	loadArcadeLevel("c1.mi_", "<trans_apt_1>", prefix);
	_levels["c1.mi_"]->intros.push_back("cine/ross002s.smk");
	_levels["c1.mi_"]->levelIfLose = "<over_apt_1>";

	loadArcadeLevel("c1h.mi_", "<trans_apt_1>", prefix);
	_levels["c1h.mi_"]->intros.push_back("cine/ross002s.smk");
	_levels["c1h.mi_"]->levelIfLose = "<over_apt_1>";

	// Hardcoded levels
	Code *matrix = new Code();
	matrix->name = "<puz_matr>";
	matrix->intros.push_back("spider/cine/aleyc01s.smk");
	matrix->levelIfWin = "<trans_apt_2>";
	_levels["<puz_matr>"] = matrix;

	// Transitions
	Transition *trans_apt_1 = new Transition("busint.mi_");
	trans_apt_1->frameImage = "spider/cine/blcs00.smk";
	trans_apt_1->frameNumber = 0;
	trans_apt_1->intros.push_back("spider/cine/ross004s.smk");
	trans_apt_1->intros.push_back("spider/cine/apts002s.smk");
	_levels["<trans_apt_1>"] = trans_apt_1;

	Transition *trans_apt_2 = new Transition("busint.mi_");
	trans_apt_2->frameImage = "spider/cine/blcs00.smk";
	trans_apt_2->frameNumber = 0;
	trans_apt_2->intros.push_back("spider/cine/dia002s.smk");
	trans_apt_2->intros.push_back("spider/cine/apts004s.smk");
	_levels["<trans_apt_2>"] = trans_apt_2;

	loadSceneLevel("busint.mi_", "busintro.mi_", prefix);
	loadSceneLevel("busintro.mi_", "<bus_selector>", prefix);
	_levels["busintro.mi_"]->intros.push_back("cine/blcs001s.smk");

	Transition *bus_selector = new Transition("buspuz.mi_", "bushard1.mi_");
	_levels["<bus_selector>"] = bus_selector;

	loadSceneLevel("bushard1.mi_", "bushard1.mi_", prefix);
	loadSceneLevel("bushard2.mi_", "<after_bus_selector>", prefix);

	loadSceneLevel("buspuz.mi_", "<after_bus_selector>", prefix);

	Transition *after_bus_selector = new Transition("<after_bus_easy>", "<after_bus_hard>");
	_levels["<after_bus_selector>"] = after_bus_selector;

	Transition *after_bus_easy = new Transition("decide3.mi_");
	after_bus_easy->intros.push_back("spider/cine/blcs003s.smk");
	_levels["<after_bus_easy>"] = after_bus_easy;

	Transition *after_bus_hard = new Transition("decide3.mi_");
	after_bus_hard->intros.push_back("spider/cine/blcs004s.smk");
	_levels["<after_bus_hard>"] = after_bus_hard;

	loadSceneLevel("decide3.mi_", "", prefix);

	sc = (Scene *) _levels["decide3.mi_"];
	cl = new ChangeLevel("alofintr.mi_");
	sc->hots[2].actions.push_back(cl);
	cl = new ChangeLevel("c4");
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("int_roof.mi_", "", prefix);
	sc = (Scene *) _levels["int_roof.mi_"];
	cl = new ChangeLevel("<recept>");
	sc->hots[1].actions.push_back(cl);
	cl = new ChangeLevel("<boil_selector_1>");
	sc->hots[2].actions.push_back(cl);
	Overlay *over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\ROOFB1.SMK"; // seems to be a bug?

	loadSceneLevel("alofintr.mi_", "<boil_selector_1>", prefix);
	sc = (Scene *) _levels["alofintr.mi_"];
	sc->intros.push_back("cine/swc002as.smk");
	// This is necessary, for some reason
	Global *gl = new Global("GS_SWITCH1", "TURNON");	// alarm system
	sc->hots[2].actions.push_back(gl);
	gl = new Global("GS_SWITCH2", "TURNON");			// camera
	sc->hots[2].actions.push_back(gl);
	gl = new Global("GS_SWITCH3", "TURNON");			// lights recept
	sc->hots[2].actions.push_back(gl);
	gl = new Global("GS_SWITCH4", "TURNON");			// Side A door
	sc->hots[2].actions.push_back(gl);
	gl = new Global("GS_SWITCH5", "TURNON");			// Side B door
	sc->hots[2].actions.push_back(gl);
	gl = new Global("GS_SWITCH6", "TURNON");			// Office light
	sc->hots[2].actions.push_back(gl);

	Transition *boil_selector_1 = new Transition("boiler.mi_", "boilhard.mi_");
	_levels["<boil_selector_1>"] = boil_selector_1;
	_levels["<boil_selector_1>"]->intros.push_back("spider/cine/leapdown.smk");

	Transition *boil_selector_2 = new Transition("boiler.mi_", "boilhard.mi_");
	_levels["<boil_selector_2>"] = boil_selector_2;

	loadSceneLevel("boiler.mi_", "", prefix);
	sc = (Scene *) _levels["boiler.mi_"];
	over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\BOILB1.SMK"; // seems to be a bug?

	Code *fuse_panel = new Code();
	fuse_panel->name = "<fuse_panel>";
	fuse_panel->levelIfWin = "<boil_selector_2>";
	_levels["<fuse_panel>"] = fuse_panel;

	Code *office = new Code();
	office->name = "<office>";
	_levels["<office>"] = office;
	
	cl = new ChangeLevel("<back_roof_1>");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<fuse_panel>");
	sc->hots[3].actions.push_back(cl);

	Transition *back_roof_1 = new Transition("int_roof.mi_");
	_levels["<back_roof_1>"] = back_roof_1;
	_levels["<back_roof_1>"]->intros.push_back("spider/cine/leapup.smk");

	loadSceneLevel("boilhard.mi_", "", prefix);
	sc = (Scene *) _levels["boilhard.mi_"];
	over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\BOILB1.SMK"; // seems to be a bug?

	cl = new ChangeLevel("<back_roof_1>");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<fuse_panel>");
	sc->hots[3].actions.push_back(cl);

	Code *file_cabinet = new Code();
	file_cabinet->name = "<file_cabinet>";
	file_cabinet->levelIfWin = "<alveroff_selector>";
	_levels["<file_cabinet>"] = file_cabinet;

	loadSceneLevel("alverofh.mi_", "", prefix);
	loadSceneLevel("alveroff.mi_", "", prefix);
	sc = (Scene *) _levels["alveroff.mi_"];

	cl = new ChangeLevel("<tape>");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<file_cabinet>");
	sc->hots[3].actions.push_back(cl);

	Transition *tape = new Transition("decide4.mi_");
	_levels["<tape>"] = tape;
	_levels["<tape>"]->intros.push_back("spider/cine/iaos001s.smk");

	Transition *alveroff_selector = new Transition("alveroff.mi_", "alverofh.mi_");
	_levels["<alveroff_selector>"] = alveroff_selector;

	Code *recept = new Code();
	recept->name = "<recept>";
	_levels["<recept>"] = recept;

	loadSceneLevel("recept.mi_", "", prefix);
	sc = (Scene *) _levels["recept.mi_"];
	sc->intros.push_back("cine/recpinto.smk");
	over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\rec0B1.SMK"; // seems to be a bug?

	cl = new ChangeLevel("<back_roof_2>");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<door_a>");
	sc->hots[3].actions.push_back(cl);

	cl = new ChangeLevel("<office>");
	sc->hots[4].actions.push_back(cl);

	Transition *door_a = new Transition("<over_apt_5>");
	door_a->intros.push_back("spider/cine/iobs002s.smk");
	_levels["<door_a>"] = door_a;

	Transition *back_roof_2 = new Transition("int_roof.mi_");
	_levels["<back_roof_2>"] = back_roof_2;
	_levels["<back_roof_2>"]->intros.push_back("spider/cine/recpout.smk");

	loadArcadeLevel("c4.mi_", "c2", prefix);
	_levels["c4.mi_"]->intros.push_back("cine/dals001s.smk");

	loadArcadeLevel("c2.mi_", "<after_c2>", prefix);
	_levels["c2.mi_"]->intros.push_back("cine/dals002s.smk");

	loadArcadeLevel("c4h.mi_", "c2", prefix);
	_levels["c4h.mi_"]->intros.push_back("cine/dals001s.smk");

	loadArcadeLevel("c2h.mi_", "<after_c2>", prefix);
	_levels["c2h.mi_"]->intros.push_back("cine/dals002s.smk");

	Transition *after_c2 = new Transition("decide4.mi_");
	after_c2->intros.push_back("spider/cine/dals003s.smk");
	_levels["<after_c2>"] = after_c2;

	loadSceneLevel("decide4.mi_", "", prefix);
	sc = (Scene *) _levels["decide4.mi_"];
	sc->intros.push_back("cine/apts006s.smk");
	cl = new ChangeLevel("ball1.mi_");
	sc->hots[2].actions.push_back(cl);
	cl = new ChangeLevel("c5"); // depens on the difficulty
	sc->hots[4].actions.push_back(cl);

	loadArcadeLevel("c5.mi_", "<trans_apt_6>", prefix);
	_levels["c5.mi_"]->intros.push_back("cine/ctss001s.smk");
	loadArcadeLevel("c5h.mi_", "<trans_apt_6>", prefix);
	_levels["c5h.mi_"]->intros.push_back("cine/ctss001s.smk");

	Transition *trans_apt_6 = new Transition("factory1.mi_");
	trans_apt_6->intros.push_back("spider/cine/apts06as.smk");
	_levels["<trans_apt_6>"] = trans_apt_6;

	loadSceneLevel("ball1.mi_", "<note>", prefix);
	loadSceneLevel("ball2.mi_", "balcony.mi_", prefix);
	loadSceneLevel("balcony.mi_", "factory1.mi_", prefix);

	Code *note = new Code();
	note->name = "<note>";
	note->levelIfWin = "ball2.mi_";
	_levels["<note>"] = note;

	loadSceneLevel("factory1.mi_", "intercom.mi_", prefix);
	_levels["factory1.mi_"]->intros.push_back("cine/swc003s.smk");
	loadSceneLevel("intercom.mi_", "c3", prefix);

	loadArcadeLevel("c3.mi_", "c6", prefix);
	_levels["c3.mi_"]->intros.push_back("cine/vrfs001s.smk");

	loadArcadeLevel("c3h.mi_", "c6", prefix);
	_levels["c3h.mi_"]->intros.push_back("cine/vrfs001s.smk");

	loadArcadeLevel("c6.mi_", "<lock>", "spider");
	_levels["c6.mi_"]->intros.push_back("cine/vrfs002s.smk");
	_levels["c6.mi_"]->intros.push_back("cine/dia007s.smk");

	Code *lock = new Code();
	lock->name = "<lock>";
	lock->levelIfWin = "movie2.mi_";
	_levels["<lock>"] = lock;

	loadSceneLevel("movie2.mi_", "decide5.mi_", prefix);
	_levels["movie2.mi_"]->intros.push_back("cine/vrfs003s.smk");
	loadSceneLevel("decide5.mi_", "", prefix);
	sc = (Scene *) _levels["decide5.mi_"];

	cl = new ChangeLevel("movie3.mi_");
	sc->hots[2].actions.push_back(cl);

	loadSceneLevel("movie3.mi_", "movie4.mi_", prefix);
	_levels["movie3.mi_"]->intros.push_back("cine/imss001s.smk");
	loadSceneLevel("movie4.mi_", "<fuse_box>", prefix);
	_levels["movie4.mi_"]->intros.push_back("cine/imss002s.smk");
	Code *fuse_box = new Code();
	fuse_box->name = "<fuse_box>";
	fuse_box->levelIfWin = "<trans_fuse_box>";
	_levels["<fuse_box>"] = fuse_box;

	Transition *trans_fuse_box = new Transition("decide6.mi_");
	trans_fuse_box->intros.push_back("spider/cine/dia009s.smk");
	trans_fuse_box->intros.push_back("spider/cine/imss003s.smk");
	_levels["<trans_fuse_box>"] = trans_fuse_box;

	loadSceneLevel("decide6.mi_", "", prefix);
	sc = (Scene *) _levels["decide6.mi_"];

	cl = new ChangeLevel("shoctalk.mi_");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("decide10.mi_");
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("shoctalk.mi_", "decide7.mi_", prefix);
	_levels["shoctalk.mi_"]->intros.push_back("cine/vrfs004s.smk");

	loadSceneLevel("decide7.mi_", "", prefix);
	sc = (Scene *) _levels["decide7.mi_"];

	cl = new ChangeLevel("decide8.mi_");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("c13.mi_");
	sc->hots[4].actions.push_back(cl);

	loadArcadeLevel("c13.mi_", "<after_c13>", prefix);
	_levels["c13.mi_"]->intros.push_back("cine/spf007bs.smk");

	loadArcadeLevel("c13h.mi_", "<after_c13>", prefix);
	_levels["c13h.mi_"]->intros.push_back("cine/spf007bs.smk");

	Transition *after_c13 = new Transition("c12");
	after_c13->intros.push_back("spider/cine/vrfs06bs.smk");
	_levels["<after_c13>"] = after_c13;

	loadSceneLevel("decide8.mi_", "", prefix);
	sc = (Scene *) _levels["decide8.mi_"];

	cl = new ChangeLevel("c8");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("c9"); // TODO
	sc->hots[4].actions.push_back(cl);

	loadArcadeLevel("c8.mi_", "<after_c8>", prefix);
	_levels["c8.mi_"]->intros.push_back("cine/utns001s.smk");
	loadArcadeLevel("c8h.mi_", "<after_c8>", prefix);
	_levels["c8.mi_"]->intros.push_back("cine/utns001s.smk");

	Transition *after_c8 = new Transition("c10");
	after_c8->intros.push_back("spider/cine/utns002s");
	_levels["<after_c8>"] = after_c8;

	loadArcadeLevel("c9.mi_", "<after_c9>", prefix);
	_levels["c9.mi_"]->intros.push_back("cine/vrfs005s.smk");
	loadArcadeLevel("c9h.mi_", "<after_c9>", prefix);
	_levels["c9h.mi_"]->intros.push_back("cine/vrfs005s.smk");

	Transition *after_c9 = new Transition("c10");
	after_c9->intros.push_back("spider/cine/utns006s.smk");
	_levels["<after_c9>"] = after_c9;

	loadArcadeLevel("c10.mi_", "<after_c10>", prefix);
	_levels["c10.mi_"]->intros.push_back("cine/utns003s.smk");
	loadArcadeLevel("c10h.mi_", "<after_c10>", prefix);
	_levels["c10h.mi_"]->intros.push_back("cine/utns003s.smk");

	Transition *after_c10 = new Transition("docoffic.mi_");
	after_c10->intros.push_back("spider/cine/utns004s.smk");
	_levels["<after_c10>"] = after_c10;

	loadSceneLevel("docoffic.mi_", "decide9.mi_", prefix);
	//_levels["docoffic.mi_"]->intros.push_back("cine/????.smk");
	_levels["decide9_0.mi_"] = _levels["decide9"];

	loadSceneLevel("decide9.mi_", "", prefix);

	loadSceneLevel("decide10.mi_", "", prefix);
	sc = (Scene *) _levels["decide10.mi_"];
	sc->intros.push_back("cine/dia012s.smk");

	cl = new ChangeLevel("docoffi1.mi_");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<dont_believe_mason>");
	sc->hots[4].actions.push_back(cl);

	loadSceneLevel("docoffi1.mi_", "decide9.mi_", prefix);
	_levels["docoffi1.mi_"]->intros.push_back("cine/doocin2s.smk");
	_levels["decide9_1.mi_"] = _levels["decide9"];

	loadArcadeLevel("c12.mi_", "", prefix);
	loadArcadeLevel("c12h.mi_", "", prefix);

	// No c7/c7h level?
	loadArcadeLevel("c11.mi_", "", prefix);
	loadArcadeLevel("c11h.mi_", "", prefix);

	loadArcadeLevel("c6h.mi_", "<lock>", prefix);
	_levels["c6h.mi_"]->intros.push_back("cine/vrfs002s.smk");
	_levels["c6h.mi_"]->intros.push_back("cine/dia007s.smk");

	loadSceneLevel("decide11.mi_", "", prefix);

	// start level
	Transition *start = new Transition("mainmenu.mi_");
	start->intros.push_back("spider/cine/dcine1.smk");
	start->intros.push_back("spider/cine/dcine2.smk");
	_levels["<start>"] = start;

	sc = (Scene *) _levels["mainmenu.mi_"];
	cl = new ChangeLevel("levels.mi_");
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("options.mi_");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("<quit>");
	sc->hots[5].actions.push_back(cl);

	sc = (Scene *) _levels["levels.mi_"];
	cl = new ChangeLevel("levels.mi_");
	sc->hots[1].actions.push_back(cl);
	sc->hots[2].actions.push_back(cl);
	sc->hots[3].actions.push_back(cl);
	sc->hots[4].actions.push_back(cl);

	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[5].actions.push_back(gl);
	cl = new ChangeLevel("mainmenu.mi_");
	sc->hots[6].actions.push_back(cl);

	sc = (Scene *) _levels["options.mi_"]; 

	cl = new ChangeLevel("combmenu.mi_");
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("mainmenu.mi_");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("<credits>");
	sc->hots[5].actions.push_back(cl);

	sc = (Scene *) _levels["combmenu.mi_"]; 

	cl = new ChangeLevel("options.mi_");
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("c1h.mi_");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("c2h.mi_");
	sc->hots[3].actions.push_back(cl);

	cl = new ChangeLevel("c3h.mi_");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("c4h.mi_");
	sc->hots[5].actions.push_back(cl);

	cl = new ChangeLevel("c5h.mi_");
	sc->hots[6].actions.push_back(cl);

	cl = new ChangeLevel("c8h.mi_");
	sc->hots[7].actions.push_back(cl);
	
	cl = new ChangeLevel("c9h.mi_");
	sc->hots[8].actions.push_back(cl);

	cl = new ChangeLevel("c10h.mi_");
	sc->hots[9].actions.push_back(cl);

	cl = new ChangeLevel("c11h.mi_");
	sc->hots[10].actions.push_back(cl);

	cl = new ChangeLevel("c12h.mi_");
	sc->hots[11].actions.push_back(cl);

	// Game overs
	Transition *over_apt_1 = new Transition("tryagain.mi_");
	over_apt_1->intros.push_back("spider/cine/apts01as.smk");
	_levels["<over_apt_1>"] = over_apt_1;

	Transition *over_apt_5 = new Transition("tryagain.mi_");
	over_apt_5->intros.push_back("spider/cine/apts05as.smk");
	_levels["<over_apt_5>"] = over_apt_5;

	Transition *over_bus = new Transition("tryagain.mi_");
	over_bus->intros.push_back("spider/cine/blcs002s.smk");
	over_bus->intros.push_back("spider/cine/apt04as.smk");
	_levels["<over_bus>"] = over_bus;

	Transition *dont_believe_mason = new Transition("<credits>");
	dont_believe_mason->intros.push_back("spider/cine/doos004s.smk");
	_levels["<dont_believe_mason>"] = dont_believe_mason;

	Transition *over_apt_7 = new Transition("<credits>");
	over_apt_7->intros.push_back("spider/cine/apts007s.smk");
	_levels["<over_apt_7>"] = over_apt_7;

	_nextLevel = "<start>";
}

void SpiderEngine::loadAssetsDemo() {

	if (!_installerArchive.open("DATA.Z"))
		error("Failed to open DATA.Z");

	SearchMan.add("DATA.Z", (Common::Archive *) &_installerArchive, 0, false);

	Common::ArchiveMemberList files;
	LibFile *missions = loadLib("", "sixdemo/c_misc/missions.lib", true);
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any file from missions.lib");

	// start level
	Transition *start = new Transition("sixdemo/mis/demo.mis");
	start->intros.push_back("sixdemo/demo/dcine1.smk");
	start->intros.push_back("sixdemo/demo/dcine2.smk");
	_levels["<start>"] = start;

	loadArcadeLevel("c1.mi_", "sixdemo/mis/demo.mis", "sixdemo");

	loadLib("", "sixdemo/c_misc/fonts.lib", true);
	loadLib("sixdemo/c_misc/sound.lib/", "sixdemo/c_misc/sound.lib", true);
	loadLib("sixdemo/demo/sound.lib/", "sixdemo/demo/sound.lib", true);

	// Read assets from mis files
	loadSceneLevel("sixdemo/mis/demo.mis", "", "sixdemo");
	ChangeLevel *cl = new ChangeLevel("c1.mi_");

	Scene *sc = (Scene *) _levels["sixdemo/mis/demo.mis"];
	sc->hots[1].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/alley.mis");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<puz_matr>");
	sc->hots[3].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/shoctalk.mis");
	sc->hots[4].actions.push_back(cl);

	cl = new ChangeLevel("sixdemo/mis/order.mis");
	sc->hots[5].actions.push_back(cl);
	sc->music = "demo/sound.lib/menu_mus.raw";

	loadSceneLevel("sixdemo/mis/order.mis", "", "sixdemo");
	sc = (Scene *) _levels["sixdemo/mis/order.mis"];
	cl = new ChangeLevel("<quit>");
	sc->hots[1].actions.push_back(cl);

	loadSceneLevel("sixdemo/mis/alley.mis", "", "sixdemo");
	sc = (Scene *) _levels["sixdemo/mis/alley.mis"];

	sc->intros.push_back("demo/aleyc01s.smk");
	sc->music = "demo/sound.lib/alleymus.raw";
	sc->levelIfWin = "sixdemo/mis/demo.mis";
	sc->levelIfLose = "sixdemo/mis/demo.mis";

	loadSceneLevel("sixdemo/mis/shoctalk.mis", "", "sixdemo");

	Code *matrix = new Code();
	matrix->name = "<puz_matr>";
	matrix->intros.push_back("sixdemo/demo/aleyc01s.smk");
	matrix->levelIfWin = "sixdemo/mis/demo.mis";
	matrix->levelIfLose = "sixdemo/mis/demo.mis";
	_levels["<puz_matr>"] = matrix;
	_soundPath = "c_misc/sound.lib/";
	_nextLevel = "<start>";
}

Common::String SpiderEngine::findNextLevel(const Transition *trans) { 
	if (trans->nextLevel.empty())
		return _sceneState["GS_PUZZLELEVEL"] == 0 ? trans->levelEasy : trans->levelHard;

	return trans->nextLevel;
}

Common::String SpiderEngine::findNextLevel(const Common::String &level) {
	if (Common::matchString(level.c_str(), "c#") || Common::matchString(level.c_str(), "c##"))
		return level + (_sceneState["GS_COMBATLEVEL"] == 0 ? "" : "h") + ".mi_";
	else {
		return level;
	}
}

} // End of namespace Hypno
