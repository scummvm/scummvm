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

extern const char *sceneVariables[];

SpiderEngine::SpiderEngine(OSystem *syst, const ADGameDescription *gd)
  : HypnoEngine(syst, gd),
  _currentPlayerPosition(kPlayerLeft), _lastPlayerPosition(kPlayerLeft) {
	_screenW = 640;
	_screenH = 480;
	_font = nullptr;
}

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

	loadLib("", "spider/c_misc/fonts.lib", true);
	loadFonts();

	loadLib("spider/sound.lib/", "spider/c_misc/sound.lib", true);

	_levels["<quit>"]->prefix = prefix;

	Code *credits = new Code("<credits>");
	credits->prefix = prefix;
	_levels["<credits>"] = credits;
	credits->levelIfWin = "options.mi_";

	Scene *sc;
	ChangeLevel *cl;
	Global *gl;

	Sound *snd = new Sound("mouse.raw");
	loadSceneLevel("mainmenu.mi_", "", prefix);
	sc = (Scene *) _levels["mainmenu.mi_"];
	sc->hots[1].actions.push_back(snd);
	sc->hots[2].actions.push_back(snd);
	sc->hots[3].actions.push_back(snd);
	sc->hots[4].actions.push_back(snd);
	sc->hots[5].actions.push_back(snd);
	sc->hots[6].actions.push_back(snd);
	loadSceneLevel("tryagain.mi_", "", prefix);

	cl = new ChangeLevel("<give_up>");
	sc = (Scene *) _levels["tryagain.mi_"];
	sc->hots[1].actions.push_back(cl);

	LoadCheckpoint *lc = new LoadCheckpoint();
	sc->hots[2].actions.push_back(lc);

	loadSceneLevel("options.mi_", "", prefix);
	loadSceneLevel("levels.mi_", "mv0t.mi_", prefix);
	sc = (Scene *) _levels["levels.mi_"];
	sc->hots[1].actions.push_back(snd);
	sc->hots[2].actions.push_back(snd);
	sc->hots[3].actions.push_back(snd);
	sc->hots[4].actions.push_back(snd);
	sc->hots[5].actions.push_back(snd);
	sc->hots[6].actions.push_back(snd);

	loadSceneLevel("combmenu.mi_", "", prefix);

	// start level
	Transition *start = new Transition("mainmenu.mi_");
	start->intros.push_back("spider/cine/dcine1.smk");
	start->intros.push_back("spider/cine/dcine2.smk");
	_levels["<start>"] = start;

	sc = (Scene *) _levels["mainmenu.mi_"];
	cl = new ChangeLevel("levels.mi_");
	sc->hots[1].actions.push_back(cl);
	sc->music = "sound.lib/menu_mus.raw";

	Load *ld = new Load();
	sc->hots[2].actions.push_back(ld);

	//Save *sv = new Save();
	//sc->hots[3].actions.push_back(sv);

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

	Transition *practice_win = new Transition("combmenu.mi_");
	practice_win->intros.push_back("spider/cine/practwin.smk");
	_levels["<practice_win>"] = practice_win;

	Transition *practice_lose = new Transition("combmenu.mi_");
	practice_lose->intros.push_back("spider/cine/praclose.smk");
	_levels["<practice_lose>"] = practice_lose;

	loadArcadeLevel("c1h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c1h_solo.mi_"] = _levels["c1h.mi_"];

	cl = new ChangeLevel("c1h_solo.mi_");
	sc->hots[2].actions.push_back(cl);

	loadArcadeLevel("c2h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c2h_solo.mi_"] = _levels["c2h.mi_"];

	cl = new ChangeLevel("c2h_solo.mi_");
	sc->hots[4].actions.push_back(cl);

	loadArcadeLevel("c3h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c3h_solo.mi_"] = _levels["c3h.mi_"];

	cl = new ChangeLevel("c3h_solo.mi_");
	sc->hots[6].actions.push_back(cl);

	loadArcadeLevel("c4h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c4h_solo.mi_"] = _levels["c4h.mi_"];

	cl = new ChangeLevel("c4h_solo.mi_");
	sc->hots[3].actions.push_back(cl);

	loadArcadeLevel("c5h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c5h_solo.mi_"] = _levels["c5h.mi_"];

	cl = new ChangeLevel("c5h_solo.mi_");
	sc->hots[5].actions.push_back(cl);

	loadArcadeLevel("c8h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c8h_solo.mi_"] = _levels["c8h.mi_"];

	cl = new ChangeLevel("c8h_solo.mi_");
	sc->hots[7].actions.push_back(cl);

	loadArcadeLevel("c9h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c9h_solo.mi_"] = _levels["c9h.mi_"];

	cl = new ChangeLevel("c9h_solo.mi_");
	sc->hots[8].actions.push_back(cl);

	loadArcadeLevel("c10h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c10h_solo.mi_"] = _levels["c10h.mi_"];

	cl = new ChangeLevel("c10h_solo.mi_");
	sc->hots[9].actions.push_back(cl);

	loadArcadeLevel("c11h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c11h_solo.mi_"] = _levels["c11h.mi_"];

	cl = new ChangeLevel("c11h_solo.mi_");
	sc->hots[10].actions.push_back(cl);

	loadArcadeLevel("c12h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c12h_solo.mi_"] = _levels["c12h.mi_"];

	cl = new ChangeLevel("c12h_solo.mi_");
	sc->hots[12].actions.push_back(cl);

	loadArcadeLevel("c13h.mi_", "<practice_win>", "<practice_lose>", prefix);
	_levels["c13h_solo.mi_"] = _levels["c13h.mi_"];

	cl = new ChangeLevel("c13h_solo.mi_");
	sc->hots[11].actions.push_back(cl);

	loadSceneLevel("mv0t.mi_", "roof.mi_", prefix);
	_levels["mv0t.mi_"]->intros.push_back("cine/ints001s.smk");
	_levels["mv0t.mi_"]->music = "sound.lib/sm001.raw";

	loadSceneLevel("roof.mi_", "decide1.mi_", prefix);
	_levels["roof.mi_"]->music = "sound.lib/ros_mus.raw";

	loadSceneLevel("decide1.mi_", "", prefix);
	_levels["decide1.mi_"]->music = "sound.lib/sh_mus.raw";
	_levels["decide1.mi_"]->musicRate = 11025;
	_levels["decide1.mi_"]->intros.push_back("cine/siren1s.smk");
	sc = (Scene *) _levels["decide1.mi_"];
	cl = new ChangeLevel("bank.mi_");
	sc->hots[2].actions.push_back(cl);

	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("c1");
	sc->hots[4].actions.push_back(cl);

	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	loadSceneLevel("bank.mi_", "", prefix);
	_levels["bank.mi_"]->intros.push_back("cine/swcs001s.smk");
	_levels["bank.mi_"]->levelIfWin = "<alley_selector>";
	_levels["bank.mi_"]->music = "sound.lib/cbh_mus.raw";

	Transition *alley_selector = new Transition("alley.mi_", "<puz_matr>");
	_levels["<alley_selector>"] = alley_selector;

	loadSceneLevel("alley.mi_", "<after_bank>", prefix);
	_levels["alley.mi_"]->music = "sound.lib/alleymus.raw";
	_levels["alley.mi_"]->intros.push_back("cine/aleyc01s.smk");

	loadArcadeLevel("c1.mi_", "<trans_apt_1>", "<over_apt_1>", prefix);
	_levels["c1.mi_"]->intros.push_back("cine/ross002s.smk");

	loadArcadeLevel("c1h.mi_", "<trans_apt_1>", "<over_apt_1>", prefix);
	_levels["c1h.mi_"]->intros.push_back("cine/ross002s.smk");

	// Hardcoded levels
	Code *matrix = new Code("<puz_matr>");
	matrix->intros.push_back("cine/aleyc01s.smk");
	matrix->levelIfWin = "<after_bank>";
	matrix->prefix = prefix;
	_levels["<puz_matr>"] = matrix;

	// Transitions
	Transition *after_bank = new Transition("<trans_apt_2>");
	after_bank->intros.push_back("spider/cine/bals003s.smk");
	_levels["<after_bank>"] = after_bank;

	Transition *trans_apt_1 = new Transition("busint.mi_");
	trans_apt_1->intros.push_back("spider/cine/ross004s.smk");
	trans_apt_1->intros.push_back("spider/cine/ross005s.smk");
	trans_apt_1->intros.push_back("spider/cine/apts002s.smk");
	trans_apt_1->intros.push_back("spider/cine/blcs00.smk");
	_levels["<trans_apt_1>"] = trans_apt_1;

	Transition *trans_apt_2 = new Transition("busint.mi_");
	trans_apt_2->intros.push_back("spider/cine/dia002s.smk");
	trans_apt_2->intros.push_back("spider/cine/apts004s.smk");
	trans_apt_1->intros.push_back("spider/cine/blcs00.smk");
	_levels["<trans_apt_2>"] = trans_apt_2;

	loadSceneLevel("busint.mi_", "<bus_selector>", prefix);
	_levels["busint.mi_"]->music = "sound.lib/blc_mus1.raw";
	loadSceneLevel("busintro.mi_", "buspuz.mi_", prefix);
	_levels["busintro.mi_"]->intros.push_back("cine/blcs00a.smk");
	_levels["busintro.mi_"]->intros.push_back("cine/blcs001s.smk");

	Transition *bus_selector = new Transition("busintro.mi_", "<bus_hard_intro>");
	_levels["<bus_selector>"] = bus_selector;

	Transition *bus_hard_intro = new Transition("bushard2.mi_");
	bus_hard_intro->intros.push_back("spider/cine/blcs00a.smk");
	bus_hard_intro->intros.push_back("spider/cine/blcs001s.smk");
	bus_hard_intro->intros.push_back("spider/cine/ppv001s.smk");
	_levels["<bus_hard_intro>"] = bus_hard_intro;

	Code *add_ingredient = new Code("<add_ingredient>");
	add_ingredient->prefix = prefix;
	_levels["<add_ingredient>"] = add_ingredient;

	Code *check_mixture = new Code("<check_mixture>");
	check_mixture->prefix = prefix;
	_levels["<check_mixture>"] = check_mixture;

	loadSceneLevel("bushard2.mi_", "", prefix);
	sc = (Scene *) _levels["bushard2.mi_"];
	Escape *escape = new Escape();

	Hotspots *hs = sc->hots[1].smenu;
	(*hs)[1].actions.push_back(escape);

	cl = new ChangeLevel("<check_mixture>");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<add_ingredient>");
	sc->hots[3].actions.push_back(snd);
	sc->hots[3].actions.push_back(cl);

	gl = new Global("", "CLEAR");
	sc->hots[4].actions.push_back(snd);
	sc->hots[4].actions.push_back(gl);

	gl = new Global("GS_SWITCH1", "TURNON"); // hairspray
	sc->hots[4].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[5].actions.push_back(snd);
	sc->hots[5].actions.push_back(gl);

	gl = new Global("GS_SWITCH2", "TURNON"); // spot remover
	sc->hots[5].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[6].actions.push_back(snd);
	sc->hots[6].actions.push_back(gl);

	gl = new Global("GS_SWITCH3", "TURNON"); // rubbing alcohol
	sc->hots[6].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[7].actions.push_back(snd);
	sc->hots[7].actions.push_back(gl);

	gl = new Global("GS_SWITCH4", "TURNON"); // turpentine
	sc->hots[7].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[8].actions.push_back(snd);
	sc->hots[8].actions.push_back(gl);

	gl = new Global("GS_SWITCH5", "TURNON"); // spray paint
	sc->hots[8].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[9].actions.push_back(snd);
	sc->hots[9].actions.push_back(gl);

	gl = new Global("GS_SWITCH0", "TURNON"); // other
	sc->hots[9].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[10].actions.push_back(snd);
	sc->hots[10].actions.push_back(gl);

	gl = new Global("GS_SWITCH0", "TURNON"); // other
	sc->hots[10].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[11].actions.push_back(snd);
	sc->hots[11].actions.push_back(gl);

	gl = new Global("GS_SWITCH0", "TURNON"); // other
	sc->hots[11].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[12].actions.push_back(snd);
	sc->hots[12].actions.push_back(gl);

	gl = new Global("GS_SWITCH6", "TURNON"); // fingernail polish
	sc->hots[12].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[13].actions.push_back(snd);
	sc->hots[13].actions.push_back(gl);

	gl = new Global("GS_SWITCH0", "TURNON"); // other
	sc->hots[13].actions.push_back(gl);

	gl = new Global("", "CLEAR");
	sc->hots[14].actions.push_back(snd);
	sc->hots[14].actions.push_back(gl);

	gl = new Global("GS_SWITCH0", "TURNON"); // other
	sc->hots[14].actions.push_back(gl);

	loadSceneLevel("buspuz.mi_", "<after_bus_easy>", prefix);
	_levels["buspuz.mi_"]->music = "sound.lib/blc_mus2.raw";
	_levels["buspuz.mi_"]->musicRate = 11025;

	Transition *after_bus_easy = new Transition("decide3.mi_");
	after_bus_easy->intros.push_back("spider/cine/blcs003s.smk");
	_levels["<after_bus_easy>"] = after_bus_easy;

	Transition *after_bus_hard = new Transition("decide3.mi_");
	after_bus_hard->intros.push_back("spider/cine/blcs004s.smk");
	_levels["<after_bus_hard>"] = after_bus_hard;

	loadSceneLevel("decide3.mi_", "", prefix);
	_levels["decide3.mi_"]->music = "sound.lib/sh_mus.raw";
	_levels["decide3.mi_"]->musicRate = 11025;

	sc = (Scene *) _levels["decide3.mi_"];
	cl = new ChangeLevel("alofintr.mi_");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("c4");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

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
	gl = new Global("GS_SWITCH1", "TURNON");			// alarm system
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

	Code *fuse_panel = new Code("<fuse_panel>");
	fuse_panel->levelIfWin = "<boil_selector_2>";
	fuse_panel->prefix = prefix;
	_levels["<fuse_panel>"] = fuse_panel;

	Code *office = new Code("<office>");
	office->prefix = prefix;
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
	Hotspots *hots = sc->hots[1].smenu;

	SwapPointer *swpt = (SwapPointer *) (*hots)[1].actions[4];
	swpt->index--;

	swpt = (SwapPointer *) (*hots)[2].actions[4];
	swpt->index--;

	swpt = (SwapPointer *) (*hots)[3].actions[4];
	swpt->index--;

	swpt = (SwapPointer *) (*hots)[4].actions[4];
	swpt->index--;

	swpt = (SwapPointer *) (*hots)[5].actions[4];
	swpt->index--;

	swpt = (SwapPointer *) (*hots)[6].actions[4];
	swpt->index--;

	swpt = (SwapPointer *) (*hots)[7].actions[4];
	swpt->index--;

	over = (Overlay*) sc->hots[0].actions[2];
	over->path = "int_alof\\BOILB1.SMK"; // seems to be a bug?

	cl = new ChangeLevel("<back_roof_1>");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<fuse_panel>");
	sc->hots[3].actions.push_back(cl);

	Code *file_cabinet = new Code("<file_cabinet>");
	file_cabinet->levelIfWin = "<alveroff_selector>";
	file_cabinet->prefix = prefix;
	_levels["<file_cabinet>"] = file_cabinet;

	loadSceneLevel("alverofh.mi_", "", prefix);
	sc = (Scene *) _levels["alverofh.mi_"];

	cl = new ChangeLevel("<tape>");
	sc->hots[2].actions.push_back(cl);

	cl = new ChangeLevel("<file_cabinet>");
	sc->hots[3].actions.push_back(cl);

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

	Code *recept = new Code("<recept>");
	recept->prefix = prefix;
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

	loadArcadeLevel("c4.mi_", "c2", "<over_hob_vul>", prefix);
	_levels["c4.mi_"]->intros.push_back("cine/dals001s.smk");

	loadArcadeLevel("c2.mi_", "<after_c2>", "<over_hob_vul>", prefix);
	_levels["c2.mi_"]->intros.push_back("cine/dals002s.smk");

	loadArcadeLevel("c4h.mi_", "c2", "<over_hob_vul>", prefix);
	_levels["c4h.mi_"]->intros.push_back("cine/dals001s.smk");

	loadArcadeLevel("c2h.mi_", "<after_c2>", "<over_hob_vul>", prefix);
	_levels["c2h.mi_"]->intros.push_back("cine/dals002s.smk");

	Transition *after_c2 = new Transition("decide4.mi_");
	after_c2->intros.push_back("spider/cine/dals003s.smk");
	_levels["<after_c2>"] = after_c2;

	loadSceneLevel("decide4.mi_", "", prefix);
	_levels["decide4.mi_"]->music = "sound.lib/sh_mus.raw";
	_levels["decide4.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide4.mi_"];
	sc->intros.push_back("cine/apts006s.smk");
	cl = new ChangeLevel("ball1.mi_");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("c5");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	loadArcadeLevel("c5.mi_", "<trans_apt_6>", "<over_hob2>", prefix);
	if (_restoredContentEnabled)
		_levels["c5.mi_"]->intros.push_back("cine/ppv006bs.smk");
	_levels["c5.mi_"]->intros.push_back("cine/ctss001s.smk");

	loadArcadeLevel("c5h.mi_", "<trans_apt_6>", "<over_hob2>", prefix);
	if (_restoredContentEnabled)
		_levels["c5h.mi_"]->intros.push_back("cine/ppv006bs.smk");
	_levels["c5h.mi_"]->intros.push_back("cine/ctss001s.smk");

	Transition *trans_apt_6 = new Transition("factory1.mi_");
	trans_apt_6->intros.push_back("spider/cine/ctss002s.smk");
	trans_apt_6->intros.push_back("spider/cine/apts06as.smk");
	_levels["<trans_apt_6>"] = trans_apt_6;

	loadSceneLevel("ball1.mi_", "<note>", prefix);
	_levels["ball1.mi_"]->music = "sound.lib/ballroom.raw";
	_levels["ball1.mi_"]->musicRate = 11025;
	if (_restoredContentEnabled)
		_levels["ball1.mi_"]->intros.push_back("cine/ppv006as.smk");
	loadSceneLevel("coat.mi_", "ball2.mi_", prefix);
	_levels["coat.mi_"]->music = "sound.lib/ballroom.raw";
	_levels["coat.mi_"]->musicRate = 11025;

	Code *note = new Code("<note>");
	note->levelIfWin = "coat.mi_";
	note->intros.push_back("cine/dia005s.smk");
	note->prefix = prefix;
	_levels["<note>"] = note;

	loadSceneLevel("ball2.mi_", "balcony.mi_", prefix);
	_levels["ball2.mi_"]->intros.push_back("cine/register.smk");
	_levels["ball2.mi_"]->music = "sound.lib/ballroom.raw";
	_levels["ball2.mi_"]->musicRate = 11025;

	loadSceneLevel("balcony.mi_", "factory1.mi_", prefix);
	_levels["balcony.mi_"]->music = "sound.lib/balcony.raw";
	_levels["balcony.mi_"]->musicRate = 11025;

	loadSceneLevel("factory1.mi_", "intercom.mi_", prefix);
	_levels["factory1.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["factory1.mi_"]->musicRate = 11025;
	_levels["factory1.mi_"]->intros.push_back("cine/swc003s.smk");
	loadSceneLevel("intercom.mi_", "c3", prefix);
	_levels["intercom.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["intercom.mi_"]->musicRate = 11025;

	loadArcadeLevel("c3.mi_", "c6", "<over_octo1>", prefix);
	_levels["c3.mi_"]->intros.push_back("cine/vrfs001s.smk");

	loadArcadeLevel("c3h.mi_", "c6", "<over_octo1>", prefix);
	_levels["c3h.mi_"]->intros.push_back("cine/vrfs001s.smk");

	loadArcadeLevel("c6.mi_", "<lock>", "<over_gas>", prefix);
	_levels["c6.mi_"]->intros.push_back("cine/vrfs002s.smk");
	_levels["c6.mi_"]->intros.push_back("cine/dia007s.smk");
	_levels["c6.mi_"]->intros.push_back("cine/rdss001s.smk");

	loadArcadeLevel("c6h.mi_", "<lock>", "<over_gas>", prefix);
	_levels["c6h.mi_"]->intros.push_back("cine/vrfs002s.smk");
	_levels["c6h.mi_"]->intros.push_back("cine/dia007s.smk");
	_levels["c6h.mi_"]->intros.push_back("cine/rdss001s.smk");

	Code *lock = new Code("<lock>");
	lock->intros.push_back("cine/rdss003s.smk");
	lock->levelIfWin = "movie2.mi_";
	lock->prefix = prefix;
	_levels["<lock>"] = lock;

	loadSceneLevel("movie2.mi_", "decide5.mi_", prefix);
	_levels["movie2.mi_"]->intros.push_back("cine/vrfs003s.smk");
	_levels["movie2.mi_"]->music = "sound.lib/ims_mus2.raw";
	_levels["movie2.mi_"]->musicRate = 11025;

	loadSceneLevel("decide5.mi_", "", prefix);
	_levels["decide5.mi_"]->music = "sound.lib/sh_mus.raw";
	_levels["decide5.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide5.mi_"];

	cl = new ChangeLevel("movie3.mi_");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("decide6.mi_");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	loadSceneLevel("movie3.mi_", "movie4.mi_", prefix);
	_levels["movie3.mi_"]->music = "sound.lib/ims_mus2.raw";
	_levels["movie3.mi_"]->musicRate = 11025;
	_levels["movie3.mi_"]->intros.push_back("cine/imss001s.smk");
	loadSceneLevel("movie4.mi_", "<fuse_box>", prefix);
	_levels["movie4.mi_"]->intros.push_back("cine/imss002s.smk");
	_levels["movie4.mi_"]->music = "sound.lib/ims_mus2.raw";
	_levels["movie4.mi_"]->musicRate = 11025;

	Code *fuse_box = new Code("<fuse_box>");
	fuse_box->levelIfWin = "<trans_fuse_box>";
	fuse_box->prefix = prefix;
	_levels["<fuse_box>"] = fuse_box;

	Transition *trans_fuse_box = new Transition("decide6.mi_");
	trans_fuse_box->intros.push_back("spider/cine/dia009s.smk");
	trans_fuse_box->intros.push_back("spider/cine/imss003s.smk");
	_levels["<trans_fuse_box>"] = trans_fuse_box;

	loadSceneLevel("decide6.mi_", "", prefix);
	_levels["decide6.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["decide6.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide6.mi_"];

	cl = new ChangeLevel("shoctalk.mi_");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("decide10.mi_");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	loadSceneLevel("shoctalk.mi_", "decide7.mi_", prefix);
	_levels["shoctalk.mi_"]->music = "sound.lib/sh_mus.raw";
	_levels["shoctalk.mi_"]->musicRate = 11025;
	_levels["shoctalk.mi_"]->intros.push_back("cine/vrfs004s.smk");

	loadSceneLevel("decide7.mi_", "", prefix);
	_levels["decide7.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["decide7.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide7.mi_"];

	cl = new ChangeLevel("decide8.mi_");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("c13.mi_");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	loadArcadeLevel("c13.mi_", "<after_c13>", "<over_shock>", prefix);
	_levels["c13.mi_"]->intros.push_back("cine/spf007bs.smk");

	loadArcadeLevel("c13h.mi_", "<after_c13>", "<over_shock>", prefix);
	_levels["c13h.mi_"]->intros.push_back("cine/spf007bs.smk");

	Transition *after_c13 = new Transition("docoffi2.mi_");
	after_c13->intros.push_back("spider/cine/vrfs06bs.smk");
	after_c13->intros.push_back("spider/cine/spv064as.smk");
	//after_c13->intros.push_back("spider/cine/spv064s.smk"); low-quality version?
	_levels["<after_c13>"] = after_c13;

	loadSceneLevel("docoffi2.mi_", "c12a", prefix);
	_levels["docoffi2.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["docoffi2.mi_"]->musicRate = 11025;

	loadArcadeLevel("c12.mi_", "<chip_lives_with_spiderman>", "<over_vr>", prefix);
	_levels["c12.mi_"]->intros.push_back("cine/vrws010s.smk");
	_levels["c12.mi_"]->intros.push_back("cine/cybs001s.smk");
	_levels["c12a.mi_"] = _levels["c12.mi_"];

	loadArcadeLevel("c12h.mi_", "<chip_lives_with_spiderman>", "<over_vr>", prefix);
	_levels["c12h.mi_"]->intros.push_back("cine/vrws010s.smk");
	_levels["c12h.mi_"]->intros.push_back("cine/cybs001s.smk");
	_levels["c12ah.mi_"] = _levels["c12h.mi_"];

	loadSceneLevel("decide8.mi_", "", prefix);
	_levels["decide8.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["decide8.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide8.mi_"];

	cl = new ChangeLevel("c8");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("c9");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	loadArcadeLevel("c8.mi_", "<after_c8>", "<over_myst2>", prefix);
	_levels["c8.mi_"]->intros.push_back("cine/utns001s.smk");

	loadArcadeLevel("c8h.mi_", "<after_c8>", "<over_myst2>", prefix);
	_levels["c8h.mi_"]->intros.push_back("cine/utns001s.smk");

	Transition *after_c8 = new Transition("c10");
	after_c8->intros.push_back("spider/cine/utns002s.smk");
	_levels["<after_c8>"] = after_c8;

	loadArcadeLevel("c9.mi_", "<after_c9>", "<over_vul2>", prefix);
	_levels["c9.mi_"]->intros.push_back("cine/vrfs005s.smk");

	loadArcadeLevel("c9h.mi_", "<after_c9>", "<over_vul2>", prefix);
	_levels["c9h.mi_"]->intros.push_back("cine/vrfs005s.smk");

	Transition *after_c9 = new Transition("c10");
	after_c9->intros.push_back("spider/cine/vrfs006s.smk");
	_levels["<after_c9>"] = after_c9;

	loadArcadeLevel("c10.mi_", "<after_c10>", "<over_cam>", prefix);
	_levels["c10.mi_"]->intros.push_back("cine/utns003s.smk");

	loadArcadeLevel("c10h.mi_", "<after_c10>", "<over_cam>", prefix);
	_levels["c10h.mi_"]->intros.push_back("cine/utns003s.smk");

	Transition *after_c10 = new Transition("docoffic.mi_");
	after_c10->intros.push_back("spider/cine/utns004s.smk");
	after_c10->intros.push_back("spider/cine/bustitxs.smk");
	_levels["<after_c10>"] = after_c10;

	loadSceneLevel("docoffic.mi_", "decide9.mi_", prefix);
	_levels["docoffic.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["docoffic.mi_"]->musicRate = 11025;

	loadSceneLevel("decide9.mi_", "", prefix);
	_levels["decide9.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["decide9.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide9.mi_"];
	sc->intros.push_back("cine/doocin1s.smk");

	cl = new ChangeLevel("c11s");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("c12s");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	// Octopus fight without Mason
	loadArcadeLevel("c11.mi_", "<chip_dies_with_shocker>", "<over_octo2>", prefix);
	_levels["c11s.mi_"] = _levels["c11.mi_"];

	loadArcadeLevel("c11h.mi_", "<chip_dies_with_shocker>", "<over_octo2>", prefix);
	_levels["c11sh.mi_"] = _levels["c11h.mi_"];

	loadArcadeLevel("c12.mi_", "<chip_lives_with_shocker>", "<over_vr>", prefix);
	_levels["c12.mi_"]->intros.push_back("cine/vrwd001s.smk");
	_levels["c12.mi_"]->intros.push_back("cine/cybs001s.smk");
	_levels["c12s.mi_"] = _levels["c12.mi_"];

	loadArcadeLevel("c12h.mi_", "<chip_lives_with_shocker>", "<over_vr>", prefix);
	_levels["c12h.mi_"]->intros.push_back("cine/vrwd001s.smk");
	_levels["c12h.mi_"]->intros.push_back("cine/cybs001s.smk");

	_levels["c12sh.mi_"] = _levels["c12h.mi_"];

	loadSceneLevel("decide10.mi_", "", prefix);
	_levels["decide10.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["decide10.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide10.mi_"];
	sc->intros.push_back("cine/dia012s.smk");

	cl = new ChangeLevel("docoffi1.mi_");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("<dont_believe_mason>");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	loadSceneLevel("docoffi1.mi_", "decide11.mi_", prefix);
	_levels["docoffi1.mi_"]->intros.push_back("cine/doos002s.smk");
	_levels["docoffi1.mi_"]->intros.push_back("cine/doocin2s.smk");
	_levels["docoffi1.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["docoffi1.mi_"]->musicRate = 11025;

	loadSceneLevel("decide11.mi_", "", prefix);
	_levels["decide11.mi_"]->music = "sound.lib/vm_mus.raw";
	_levels["decide11.mi_"]->musicRate = 11025;
	sc = (Scene *) _levels["decide11.mi_"];
	//sc->intros.push_back("cine/dia012s.smk");

	cl = new ChangeLevel("c11m");
	sc->hots[2].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[2].actions.push_back(gl);

	cl = new ChangeLevel("c12m");
	sc->hots[4].actions.push_back(cl);
	gl = new Global("GS_LEVELWON", "TURNON");
	sc->hots[4].actions.push_back(gl);

	// No c7/c7h level?
	// Octopus fight with Mason
	loadArcadeLevel("c11.mi_", "<chip_dies_with_mason>", "<over_octo>", prefix);
	_levels["c11m.mi_"] = _levels["c11.mi_"];
	loadArcadeLevel("c11h.mi_", "<chip_dies_with_mason>", "<over_octo>", prefix);
	_levels["c11mh.mi_"] = _levels["c11h.mi_"];

	loadArcadeLevel("c12.mi_", "<chip_lives_with_mason>", "<over_vr>", prefix);
	_levels["c12.mi_"]->intros.push_back("cine/cybs001s.smk");
	_levels["c12m.mi_"] = _levels["c12.mi_"];

	loadArcadeLevel("c12h.mi_", "<chip_lives_with_mason>", "<over_vr>", prefix);
	_levels["c12h.mi_"]->intros.push_back("cine/cybs001s.smk");
	_levels["c12mh.mi_"] = _levels["c12h.mi_"];

	// Game overs
	Transition *over_apt_1 = new Transition("tryagain.mi_");
	over_apt_1->intros.push_back("spider/cine/ross003s.smk");
	over_apt_1->intros.push_back("spider/cine/apts01as.smk");
	_levels["<over_apt_1>"] = over_apt_1;

	Transition *over_apt_5 = new Transition("tryagain.mi_");
	over_apt_5->intros.push_back("spider/cine/apts05as.smk");
	_levels["<over_apt_5>"] = over_apt_5;

	Transition *over_alley = new Transition("tryagain.mi_");
	over_alley->intros.push_back("spider/cine/bals002s.smk");
	over_alley->intros.push_back("spider/cine/apts01as.smk");
	_levels["<over_alley>"] = over_alley;

	Transition *over_bus = new Transition("tryagain.mi_");
	over_bus->intros.push_back("spider/cine/blcs002s.smk");
	over_bus->intros.push_back("spider/cine/apt04as.smk");
	_levels["<over_bus>"] = over_bus;

	Transition *over_octo1 = new Transition("tryagain.mi_");
	over_octo1->intros.push_back("spider/cine/doos001a.smk");
	_levels["<over_octo1>"] = over_octo1;

	Transition *over_hob_vul = new Transition("tryagain.mi_");
	over_hob_vul->intros.push_back("spider/cine/dals001a.smk");
	over_hob_vul->intros.push_back("spider/cine/apts001s.smk");
	_levels["<over_hob_vul>"] = over_hob_vul;

	Transition *over_hob2 = new Transition("tryagain.mi_");
	over_hob2->intros.push_back("spider/cine/ctss01as.smk");
	over_hob2->intros.push_back("spider/cine/apts001s.smk");
	_levels["<over_hob2>"] = over_hob2;

	Transition *over_gas = new Transition("tryagain.mi_");
	over_gas->intros.push_back("spider/cine/rdss002s.smk");
	_levels["<over_gas>"] = over_gas;

	Transition *over_myst2 = new Transition("tryagain.mi_");
	over_myst2->intros.push_back("spider/cine/utns01as.smk");
	_levels["<over_myst2>"] = over_myst2;

	Transition *over_cam = new Transition("tryagain.mi_");
	over_cam->intros.push_back("spider/cine/utns04as.smk");
	_levels["<over_cam>"] = over_cam;

	Transition *over_octo2 = new Transition("tryagain.mi_");
	over_octo2->intros.push_back("spider/cine/vrfs01as.smk");
	_levels["<over_octo2>"] = over_octo2;

	Transition *over_vul2 = new Transition("tryagain.mi_");
	over_vul2->intros.push_back("spider/cine/vrfs05as.smk");
	_levels["<over_vul2>"] = over_vul2;

	Transition *over_shock = new Transition("tryagain.mi_");
	over_shock->intros.push_back("spider/cine/vrfs06as.smk");
	_levels["<over_shock>"] = over_shock;

	Transition *over_vr = new Transition("tryagain.mi_");
	over_vr->intros.push_back("spider/cine/cybs002s.smk");
	_levels["<over_vr>"] = over_vr;

	// Endings

	// Worst ending
	Transition *dont_believe_mason = new Transition("<credits>");
	dont_believe_mason->intros.push_back("spider/cine/doos004s.smk");
	dont_believe_mason->intros.push_back("spider/cine/apts008s.smk");
	_levels["<dont_believe_mason>"] = dont_believe_mason;

	Transition *chip_dies_with_mason = new Transition("<credits>");
	chip_dies_with_mason->intros.push_back("spider/cine/doos003s.smk");
	chip_dies_with_mason->intros.push_back("spider/cine/vrws002s.smk");
	_levels["<chip_dies_with_mason>"] = chip_dies_with_mason;

	Transition *chip_dies_with_shocker = new Transition("<credits>");
	chip_dies_with_shocker->intros.push_back("spider/cine/doos001s.smk");
	chip_dies_with_shocker->intros.push_back("spider/cine/vrws001s.smk");
	_levels["<chip_dies_with_shocker>"] = chip_dies_with_shocker;

	// Unreachable?
	Transition *chip_dies_with_spiderman = new Transition("<credits>");
	chip_dies_with_spiderman->intros.push_back("spider/cine/vrws010s.smk");
	_levels["<chip_dies_with_spiderman>"] = chip_dies_with_spiderman;

	Transition *chip_lives_with_mason = new Transition("<credits>");
	chip_lives_with_mason->intros.push_back("spider/cine/vrja001s.smk");
	_levels["<chip_lives_with_mason>"] = chip_lives_with_mason;

	Transition *chip_lives_with_shocker = new Transition("<credits>");
	chip_lives_with_shocker->intros.push_back("spider/cine/vrja002s.smk");
	chip_lives_with_shocker->intros.push_back("spider/cine/apts06cs.smk");
	_levels["<chip_lives_with_shocker>"] = chip_lives_with_shocker;

	// Best ending
	Transition *chip_lives_with_spiderman = new Transition("<credits>");
	chip_lives_with_spiderman->intros.push_back("spider/cine/vrja003s.smk");
	chip_lives_with_spiderman->intros.push_back("spider/cine/wins001s.smk");
	_levels["<chip_lives_with_spiderman>"] = chip_lives_with_spiderman;

	Code *give_up = new Code("<give_up>");
	give_up->prefix = prefix;
	_levels["<give_up>"] = give_up;

	_defaultCursor = "mouse/cursor1.smk";

	// hints areas
	_h1Area = Common::Rect(9, 45, 28, 60);
	_h2Area = Common::Rect(37, 45, 56, 60);
	_h3Area = Common::Rect(64, 45, 83, 60);
	_nextLevel = "<start>";
}

void SpiderEngine::loadAssetsDemo() {

	if (!_installerArchive.open("DATA.Z"))
		error("Failed to open DATA.Z");

	SearchMan.add("DATA.Z", (Common::Archive *) &_installerArchive, 0, false);
	Common::String prefix = "sixdemo";

	Common::ArchiveMemberList files;
	LibFile *missions = loadLib("", "sixdemo/c_misc/missions.lib", true);
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any file from missions.lib");

	_levels["<quit>"]->prefix = prefix;
	// start level
	Transition *start = new Transition("sixdemo/mis/demo.mis");
	start->prefix = prefix;
	start->intros.push_back("demo/dcine1.smk");
	start->intros.push_back("demo/dcine2.smk");
	_levels["<start>"] = start;

	loadArcadeLevel("c1.mi_", "sixdemo/mis/demo.mis", "sixdemo/mis/demo.mis", prefix);

	loadLib("", "sixdemo/c_misc/fonts.lib", true);
	loadFonts();
	loadLib("c_misc/sound.lib/", "sixdemo/c_misc/sound.lib", true);
	loadLib("demo/sound.lib/", "sixdemo/demo/sound.lib", true);

	// Read assets from mis files
	loadSceneLevel("sixdemo/mis/demo.mis", "", prefix);
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

	loadSceneLevel("sixdemo/mis/order.mis", "", prefix);
	sc = (Scene *) _levels["sixdemo/mis/order.mis"];
	cl = new ChangeLevel("<quit>");
	sc->hots[1].actions.push_back(cl);

	loadSceneLevel("sixdemo/mis/alley.mis", "", prefix);
	sc = (Scene *) _levels["sixdemo/mis/alley.mis"];

	sc->intros.push_back("demo/aleyc01s.smk");
	sc->music = "demo/sound.lib/alleymus.raw";
	sc->levelIfWin = "sixdemo/mis/demo.mis";
	sc->levelIfLose = "sixdemo/mis/demo.mis";

	loadSceneLevel("sixdemo/mis/shoctalk.mis", "", prefix);
	sc = (Scene *) _levels["sixdemo/mis/shoctalk.mis"];
	sc->music = "demo/sound.lib/sh_mus.raw";
	sc->musicRate = 11025;

	Code *matrix = new Code("<puz_matr>");
	matrix->prefix = prefix;
	matrix->intros.push_back("demo/aleyc01s.smk");
	matrix->levelIfWin = "sixdemo/mis/demo.mis";
	matrix->levelIfLose = "sixdemo/mis/demo.mis";
	_levels["<puz_matr>"] = matrix;
	_soundPath = "c_misc/sound.lib/";
	_defaultCursor = "mouse/cursor1.smk";
	_nextLevel = "<start>";
}

Common::String SpiderEngine::findNextLevel(const Transition *trans) {
	if (trans->nextLevel.empty())
		return _sceneState["GS_PUZZLELEVEL"] == 0 ? trans->levelEasy : trans->levelHard;

	return trans->nextLevel;
}

void SpiderEngine::loadFonts() {
	Common::File file;

	if (!file.open("block05.fgx"))
		error("Cannot open font");

	byte *font = (byte *)malloc(file.size());
	file.read(font, file.size());

	_font05.set_size(file.size()*8);
	_font05.set_bits((byte *)font);

	file.close();
	free(font);
	if (!file.open("scifi08.fgx"))
		error("Cannot open font");

	font = (byte *)malloc(file.size());
	file.read(font, file.size());

	_font08.set_size(file.size()*8);
	_font08.set_bits((byte *)font);

	free(font);

	// Additional fonts
	_font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
}

void SpiderEngine::drawString(const Common::String &font, const Common::String &str, int x, int y, int w, uint32 color) {
	if (font == "block05.fgx") {
		for (uint32 c = 0; c < str.size(); c++) {
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					if (!_font05.get(275 + 40*str[c] + j*8 + i))
						_compositeSurface->setPixel(x + 5 - i + 6*c, y + j, color);
				}
			}
		}
	} else if (font == "scifi08.fgx") {
		for (uint32 c = 0; c < str.size(); c++) {
			if (str[c] == 0)
				continue;
			assert(str[c] >= 32);
			for (int i = 0; i < 6; i++) {
				for (int j = 0; j < 8; j++) {
					if (!_font08.get(1554 + 72*(str[c]-32) + j*8 + i))
						_compositeSurface->setPixel(x + 6 - i + 7*c, y + j, color);
				}
			}
		}
	} else if (font == "console") {
		_font->drawString(_compositeSurface, str, x, y, w, color);
	} else
		error("Invalid font: '%s'", font.c_str());
}

Common::Error SpiderEngine::loadGameStream(Common::SeekableReadStream *stream) {
	int puzzleDifficulty = stream->readUint32LE();
	int combatDifficulty = stream->readUint32LE();
	int score = stream->readUint32LE();
	const Common::String nextLevel = stream->readString();
	loadGame(nextLevel, score, puzzleDifficulty, combatDifficulty);
	return Common::kNoError;
}

void SpiderEngine::loadGame(const Common::String &nextLevel, int score, int puzzleDifficulty, int combatDifficulty) {

	// We don't want to continue with any sound from a previous game
	stopSound();
	_sceneState["GS_PUZZLELEVEL"] = puzzleDifficulty;
	_sceneState["GS_COMBATLEVEL"] = combatDifficulty;
	_score = score;
	_nextLevel = nextLevel;
	_checkpoint = _nextLevel;

	// Reset played intros
	_intros.clear();

	// Reset state variables from puzzles
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 10; j++)
			_fuseState[i][j] = 0;

	_isFuseRust = true;
	_isFuseUnreadable = false;
	for (int i = 0; i < 7; i++)
		ingredients[i] = 0;
}

Common::Error SpiderEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (isAutosave)
		return Common::kNoError;

	if (_checkpoint.empty())
		error("Invalid checkpoint!");

	stream->writeUint32LE(_sceneState["GS_PUZZLELEVEL"]);
	stream->writeUint32LE(_sceneState["GS_COMBATLEVEL"]);
	stream->writeUint32LE(_score);
	stream->writeString(_checkpoint);
	stream->writeByte(0);
	return Common::kNoError;
}

void SpiderEngine::drawBackToMenu(Hotspot *h) {
	Graphics::Surface *menu = nullptr;
	bool transparent = false;
	if (_conversation.empty()) {
		if (h->flags[0] == "HINTS" || h->flags[1] == "HINTS" || h->flags[2] == "HINTS") {
			menu = decodeFrame("int_main/hint1.smk", 0);
		} else if (h->flags[0] == "AUTO_BUTTONS" || h->flags[0] == "SINGLE_RUN") {
			if (isDemo()) {
				if (_currentLevel != "sixdemo/mis/demo.mis" && _currentLevel != "sixdemo/mis/order.mis") {
					menu = decodeFrame("int_main/resume.smk", 0);
					transparent = true;
				}
			} else {
				if (_currentLevel != "mainmenu.mi_" && _currentLevel != "options.mi_" && _currentLevel != "combmenu.mi_")
					menu = decodeFrame("int_main/menu.smk", 0);
			}
		}

		if (menu) {
			h->rect = Common::Rect(0, 0, menu->w, menu->h);
			drawImage(*menu, 0, 0, transparent);
		}
	}
}

Common::String SpiderEngine::findNextLevel(const Common::String &level) {
	if (Common::matchString(level.c_str(), "c#") || Common::matchString(level.c_str(), "c##") || Common::matchString(level.c_str(), "c##?"))
		return level + (_sceneState["GS_COMBATLEVEL"] == 0 ? "" : "h") + ".mi_";
	else {
		return level;
	}
}

} // End of namespace Hypno
