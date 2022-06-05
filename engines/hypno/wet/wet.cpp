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

#include "common/bitarray.h"
#include "common/events.h"
#include "common/savefile.h"

#include "hypno/hypno.h"

namespace Hypno {

static const char *failedDetectionError = \
  "Failed to load any files from missions.lib.\
   Please review https://wiki.scummvm.org/index.php?title=Wetlands\
   and re-add the game.";

static const chapterEntry rawChapterTable[] = {
	{11, {44, 172}, {218, 172}, {0,   0},   {127, 172}, 0,   kHypnoColorRed}, 	 // c11
	{10, {19, 3},   {246, 3}, 	{246, 11},  {2, 2},     0,   kHypnoNoColor}, 	 // c10
	{21, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215, kHypnoColorYellow}, // c21
	{22, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215,   kHypnoColorGreen},  // c22
	{23, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215, kHypnoColorCyan}, 	// c23
	{20, {128, 150}, {238, 150},{0,   0},   {209, 146}, 0, kHypnoColorCyan}, 	// c20
	{31, {70, 160}, {180, 160}, {220, 185}, {44, 164},  215, kHypnoColorGreen}, 	// c31
	{32, {70, 160}, {180, 160}, {220, 185}, {44, 164},  215, kHypnoColorRed}, 	// c32
	{33, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215, kHypnoColorRed}, 	// c33
	{30, {19, 3},   {246, 3}, 	{246, 11},  {2, 2},     0, kHypnoColorRed}, 	// c30
	{41, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215, kHypnoColorRed}, 	// c41
	{42, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215, kHypnoColorRed}, 	// c42
	{43, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215, kHypnoColorRed}, 	// c43
	{44, {70, 160}, {180, 160}, {220, 185}, {44, 162},  215, kHypnoColorRed}, 	// c44
	{40, {19, 3},   {246, 3}, 	{246, 11},  {2, 2},     0, kHypnoColorRed}, 	// c40
	{51, {60, 167}, {190, 167}, {135, 187}, {136, 163}, 36, kHypnoColorRed}, 	// c51
	{52, {60, 167}, {190, 167}, {135, 187}, {136, 165}, 36, kHypnoColorCyan}, 	// c52
	{50, {19, 3},   {246, 3}, 	{246, 11},  {2, 2}, 	0, kHypnoColorRed}, 	// c50 (fixed)
	{61, {63, 167}, {187, 167}, {192, 188}, {152, 185}, 0, kHypnoColorCyan}, 	// c61
	{60, {63, 167}, {187, 167}, {192, 188}, {152, 185}, 0, kHypnoColorCyan}, 	// c60
	{0,  {0,  0},   {0,   0},   {0,   0},   {0, 0},     0, kHypnoColorRed}    	// NULL
};

WetEngine::WetEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {
	_screenW = 320;
	_screenH = 200;
	_lives = 2;
	_lastLevel = 0;

	_c33UseMouse = true;

	_c40SegmentIdx = -1;
	_c40lastTurn = -1;

	_c50LeftTurns = 0;
	_c50RigthTurns = 0;

	const chapterEntry *entry = rawChapterTable;
	while (entry->id) {
		_ids.push_back(entry->id);
		_chapterTable[entry->id] = entry;
		entry++;
	}

	_healthString = getLocalizedString("health");
	_scoreString = getLocalizedString("score");
	_objString = getLocalizedString("objectives");
	_targetString = getLocalizedString("target");
	_directionString = getLocalizedString("direction");
	_enterNameString = getLocalizedString("name");
}

void WetEngine::loadAssets() {
	if (!isDemo()) {
		_difficulty = "1"; // Medium difficulty by default
		loadAssetsFullGame();
		return;
	}
	_difficulty = ""; // No difficulty selection in demo

	if (_variant == "Demo" || _variant == "DemoHebrew")
		loadAssetsDemoDisc();
	else if (_variant == "Gen4")
		loadAssetsGen4();
	else if (_variant == "PCWDemo")
		loadAssetsPCW();
	else if (_variant == "PCGDemo")
		loadAssetsPCG();
	else if (_variant == "NonInteractive" || _variant == "NonInteractiveJoystick")
		loadAssetsNI();
	else
		error("Invalid demo version: \"%s\"", _variant.c_str());
}


void WetEngine::loadAssetsDemoDisc() {

	bool encrypted = _variant == "Demo" ? true : false;
	LibFile *missions = loadLib("", "wetlands/c_misc/missions.lib", encrypted);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("%s", failedDetectionError);

	Hotspot h(MakeMenu);
	Hotspots hs;
	Ambient *a = new Ambient("movie/selector.smk", Common::Point(0, 0), "/LOOP");
	a->fullscreen = true;
	h.actions.push_back(a);

	hs.push_back(h);

	h.type = MakeHotspot;
	h.rect = Common::Rect(0, 177, 116, 192);
	h.actions.clear();
	h.smenu = nullptr;
	ChangeLevel *cl = new ChangeLevel("<intro>");
	h.actions.push_back(cl);

	hs.push_back(h);

	h.rect = Common::Rect(121, 177, 250, 200);
	cl = new ChangeLevel("<movies>");
	h.actions.clear();
	h.actions.push_back(cl);

	hs.push_back(h);

	h.rect = Common::Rect(252, 177, 318, 200);
	Quit *q = new Quit();
	h.actions.clear();
	h.actions.push_back(q);

	hs.push_back(h);

	Scene *start = new Scene();
	start->resolution = "320x200";
	start->hots = hs;
	_levels["<start>"] = start;

	Transition *intro;
	if (_variant == "Demo")
		intro = new Transition("c31");
	else if (_variant == "DemoHebrew")
		intro = new Transition("c31.mis");
	else
		error("Unsupported language");

	intro->intros.push_back("movie/nw_logo.smk");
	intro->intros.push_back("movie/hypnotix.smk");
	intro->intros.push_back("movie/wetlogo.smk");
	intro->frameImage = "wetlands/c_misc/c.s";
	intro->frameNumber = 0;
	_levels["<intro>"] = intro;

	Transition *movies = new Transition("<quit>");
	movies->intros.push_back("movie/nw_logo.smk");
	movies->intros.push_back("movie/hypnotix.smk");
	movies->intros.push_back("movie/wetlogo.smk");
	movies->intros.push_back("movie/c42e1s.smk");
	movies->intros.push_back("movie/c23e1s.smk");
	movies->intros.push_back("movie/c20o1s.smk");
	movies->intros.push_back("movie/c23d1s.smk");
	movies->intros.push_back("movie/c40o1s.smk");
	movies->intros.push_back("movie/c31d1s.smk");
	movies->intros.push_back("movie/c42d1s.smk");
	movies->intros.push_back("movie/c44d1s.smk");
	movies->intros.push_back("movie/c44e1s.smk");
	movies->intros.push_back("movie/c32d1s.smk");
	movies->intros.push_back("movie/c22e1s.smk");
	movies->intros.push_back("movie/c31e1s.smk");
	movies->intros.push_back("movie/gameover.smk");
	movies->frameImage = "";
	movies->frameNumber = 0;
	_levels["<movies>"] = movies;
	ArcadeShooting *arc;

	if (_variant == "Demo") {
		loadArcadeLevel("c31.mi_", "c52", "c52", "wetlands");
		if (_restoredContentEnabled) {
			arc = (ArcadeShooting*) _levels["c31.mi_"];
			arc->segments[0].size = 1354;
			arc->objKillsRequired[0] = 2;
		}
		loadArcadeLevel("c52.mi_", "<game_over>", "<quit>", "wetlands");
		if (_restoredContentEnabled) {
			arc = (ArcadeShooting*) _levels["c52.mi_"];
			arc->segments[0].size = 2383;
			arc->objKillsRequired[0] = 2;
			arc->objKillsRequired[1] = 13;
		}
	} else if (_variant == "DemoHebrew") {
		loadArcadeLevel("c31.mis", "c52.mis", "c52.mis", "wetlands");
		loadArcadeLevel("c52.mis", "<game_over>", "<quit>", "wetlands");
	} else {
		error("Unsupported variant");
	}

	Transition *over = new Transition("<quit>");
	over->intros.push_back("movie/gameover.smk");
	_levels["<game_over>"] = over;

	loadLib("", "wetlands/c_misc/fonts.lib", true);
	loadFonts();
	loadLib("wetlands/sound/", "wetlands/c_misc/sound.lib", true);
	_nextLevel = "<start>";
}

void WetEngine::loadAssetsGen4() {

	bool encrypted = false;
	LibFile *missions = loadLib("", "c_misc/missions.lib", encrypted);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("%s", failedDetectionError);

	Transition *intro;
	intro = new Transition("c31.mis");

	intro->intros.push_back("c_misc/nw_logo.smk");
	intro->intros.push_back("c_misc/h.s");
	intro->intros.push_back("c_misc/w.s");
	intro->frameImage = "c_misc/c.s";
	intro->frameNumber = 0;
	_levels["<start>"] = intro;

	loadArcadeLevel("c31.mis", "c52.mis", "c52.mis", "");
	ArcadeShooting *arc;
	if (_restoredContentEnabled) {
		arc = (ArcadeShooting*) _levels["c31.mis"];
		arc->segments[0].size = 1354;
		arc->objKillsRequired[0] = 2;
	}
	loadArcadeLevel("c52.mis", "<game_over>", "<quit>", "");
	if (_restoredContentEnabled) {
		arc = (ArcadeShooting*) _levels["c52.mis"];
		arc->segments[0].size = 2383;
		arc->objKillsRequired[0] = 2;
		arc->objKillsRequired[1] = 13;
	}

	Transition *over = new Transition("<quit>");
	over->intros.push_back("c_misc/g.s");
	_levels["<game_over>"] = over;

	loadLib("", "c_misc/fonts.lib", true);
	loadFonts();
	loadLib("sound/", "c_misc/sound.lib", true);
	_nextLevel = "<start>";
}

void WetEngine::loadAssetsNI() {
	Common::String musicFile = _variant == "NonInteractive" ? "wetmusic.81m" : "c44_22k.raw";
	int musicRate = _variant == "NonInteractive" ? 11025 : 22050;

	Transition *movies = new Transition("<quit>");
	movies->music = musicFile;
	movies->musicRate = musicRate;
	movies->playMusicDuringIntro = true;
	movies->intros.push_back("demo/nw_logo.smk");
	movies->intros.push_back("demo/hypnotix.smk");
	movies->intros.push_back("demo/wetlogo.smk");
	movies->intros.push_back("demo/c31c1.smk");
	movies->intros.push_back("demo/demo31.smk");
	movies->intros.push_back("demo/c31c2.smk");
	movies->intros.push_back("demo/c31e1.smk");
	movies->intros.push_back("demo/logo_w.smk");
	movies->intros.push_back("demo/bar01b.smk");
	movies->intros.push_back("demo/gun_320.smk");
	movies->intros.push_back("demo/logo_e.smk");
	movies->intros.push_back("demo/c30peek.smk");
	movies->intros.push_back("demo/demo30.smk");
	movies->intros.push_back("demo/c30knife.smk");
	movies->intros.push_back("demo/logo_t.smk");
	movies->intros.push_back("demo/c51teez.smk");
	movies->intros.push_back("demo/demo21.smk");
	movies->intros.push_back("demo/c51kill.smk");
	movies->intros.push_back("demo/logo_l.smk");
	movies->intros.push_back("demo/run_320.smk");
	movies->intros.push_back("demo/logo_a.smk");
	movies->intros.push_back("demo/demo50.smk");
	movies->intros.push_back("demo/c50gate.smk");
	movies->intros.push_back("demo/logo_n.smk");
	movies->intros.push_back("demo/c22end.smk");
	movies->intros.push_back("demo/logo_d.smk");
	movies->intros.push_back("demo/demo44.smk");
	movies->intros.push_back("demo/c44boom.smk");
	movies->intros.push_back("demo/logo_s.smk");
	movies->intros.push_back("demo/xi.smk");
	movies->intros.push_back("demo/wetlogo.smk");
	movies->intros.push_back("demo/c30shoot.smk");
	movies->frameImage = "";
	movies->frameNumber = 0;
	_levels["<start>"] = movies;
	_nextLevel = "<start>";
}


void WetEngine::loadAssetsPCW() {

	LibFile *missions = loadLib("", "c_misc/missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("%s", failedDetectionError);

	Transition *intro = new Transition("c11.mis");
	intro->intros.push_back("c_misc/nw_logo.smk");
	intro->intros.push_back("c_misc/h.s");
	intro->intros.push_back("c_misc/wet.smk");
	_levels["<start>"] = intro;

	loadArcadeLevel("c11.mis", "<quit>", "<quit>", "");
	ArcadeShooting *arc;
	if (_restoredContentEnabled) {
		arc = (ArcadeShooting*) _levels["c11.mis"];
		arc->segments[0].size = 2002;
		arc->objKillsRequired[0] = 1;
		arc->transitions.push_back(ArcadeTransition("", "c11/c11p2.col", "", 0, 1501));
		// These videos were not included in the demo, so we replace them
		arc->defeatMissBossVideo = "c11\\c11d1.smk";
		arc->defeatNoEnergySecondVideo = "c11\\c11d1.smk";
	}

	Transition *over = new Transition("<quit>");
	_levels["<game_over>"] = over;

	loadLib("sound/", "c_misc/sound.lib", false);
	loadLib("", "c_misc/fonts.lib", true);
	loadFonts();
	_nextLevel = "<start>";
}

void WetEngine::loadAssetsPCG() {

	LibFile *missions = loadLib("", "missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("%s", failedDetectionError);

	Transition *intro = new Transition("c31.mis");
	intro->intros.push_back("nw_logo.smk");
	intro->intros.push_back("h.s");
	intro->intros.push_back("wet.smk");
	intro->frameImage = "c.s";
	intro->frameNumber = 0;
	_levels["<start>"] = intro;

	loadArcadeLevel("c31.mis", "<quit>", "<quit>", "");
	ArcadeShooting *arc;
	if (_restoredContentEnabled) {
		arc = (ArcadeShooting*) _levels["c31.mis"];
		arc->segments[0].size = 1354;
		arc->objKillsRequired[0] = 2;
		// These videos were not included in the demo, so we replace or remove them
		arc->hitBoss1Video = "";
		arc->hitBoss2Video = "";
		arc->missBoss1Video = "";
		arc->missBoss2Video = "";
		arc->defeatMissBossVideo = "c31\\c31d1s.smk";
	}

	Transition *over = new Transition("<quit>");
	over->intros.push_back("g.s");
	_levels["<game_over>"] = over;

	loadLib("sound/", "sound.lib", false);
	loadLib("", "fonts.lib", true);
	loadFonts();
	_nextLevel = "<start>";
}

void WetEngine::loadAssetsFullGame() {
	LibFile *missions = loadLib("", "c_misc/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("%s", failedDetectionError);

	Transition *logos = new Transition("<main_menu>");
	logos->intros.push_back("c_misc/logo.smk");
	logos->intros.push_back("c_misc/nw_logo.smk");
	logos->intros.push_back("c_misc/hypnotix.smk");
	logos->intros.push_back("c_misc/wetlogo.smk");
	_levels["<start>"] = logos;

	Code *menu = new Code("<main_menu>");
	_levels["<main_menu>"] = menu;
	_levels["<main_menu>"]->levelIfWin = "<intros>";

	Code *level_menu = new Code("<level_menu>");
	_levels["<level_menu>"] = level_menu;
	_levels["<level_menu>"]->levelIfWin = "?";

	Transition *over = new Transition("<main_menu>");
	over->intros.push_back("c_misc/gameover.smk");
	_levels["<game_over>"] = over;

	Transition *intros = new Transition("<level_menu>");
	intros->intros.push_back("c_misc/stardate.smk");
	intros->intros.push_back("c_misc/intros.smk");
	intros->intros.push_back("c_misc/confs.smk");
	_levels["<intros>"] = intros;

	Code *check_lives = new Code("<check_lives>");
	_levels["<check_lives>"] = check_lives;

	Code *end_credits = new Code("<credits>");
	_levels["<credits>"] = end_credits;

	ArcadeShooting *arc;
	loadArcadeLevel("c110.mi_", "c10", "<check_lives>", "");
	loadArcadeLevel("c111.mi_", "c10", "<check_lives>", "");
	loadArcadeLevel("c112.mi_", "c10", "<check_lives>", "");

	loadArcadeLevel("c100.mi_", "c21", "<check_lives>", "");
	loadArcadeLevel("c101.mi_", "c21", "<check_lives>", "");
	loadArcadeLevel("c102.mi_", "c21", "<check_lives>", "");

	loadArcadeLevel("c210.mi_", "c22", "<check_lives>", "");
	loadArcadeLevel("c211.mi_", "c22", "<check_lives>", "");
	loadArcadeLevel("c212.mi_", "c22", "<check_lives>", "");

	loadArcadeLevel("c220.mi_", "c23", "<check_lives>", "");
	loadArcadeLevel("c221.mi_", "c23", "<check_lives>", "");
	loadArcadeLevel("c222.mi_", "c23", "<check_lives>", "");

	loadArcadeLevel("c230.mi_", "c20", "<check_lives>", "");
	loadArcadeLevel("c231.mi_", "c20", "<check_lives>", "");
	loadArcadeLevel("c232.mi_", "c20", "<check_lives>", "");

	loadArcadeLevel("c200.mi_", "c31", "<check_lives>", "");
	loadArcadeLevel("c201.mi_", "c31", "<check_lives>", "");
	loadArcadeLevel("c202.mi_", "c31", "<check_lives>", "");

	arc = (ArcadeShooting*) _levels["c200.mi_"];
	arc->mouseBox.right = 320;
	arc->mouseBox.bottom = 135;

	arc = (ArcadeShooting*) _levels["c201.mi_"];
	arc->mouseBox.right = 320;
	arc->mouseBox.bottom = 135;

	arc = (ArcadeShooting*) _levels["c202.mi_"];
	arc->mouseBox.right = 320;
	arc->mouseBox.bottom = 135;

	loadArcadeLevel("c310.mi_", "c32", "<check_lives>", "");
	loadArcadeLevel("c311.mi_", "c32", "<check_lives>", "");
	loadArcadeLevel("c312.mi_", "c32", "<check_lives>", "");

	loadArcadeLevel("c320.mi_", "c33", "<check_lives>", "");
	loadArcadeLevel("c321.mi_", "c33", "<check_lives>", "");
	loadArcadeLevel("c322.mi_", "c33", "<check_lives>", "");

	loadArcadeLevel("c330.mi_", "c30", "<check_lives>", "");
	loadArcadeLevel("c331.mi_", "c30", "<check_lives>", "");
	loadArcadeLevel("c332.mi_", "c30", "<check_lives>", "");

	loadArcadeLevel("c300.mi_", "c41", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c300.mi_"];
	arc->id = 30; // Fixed from the original (3)

	loadArcadeLevel("c301.mi_", "c41", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c301.mi_"];
	arc->id = 30; // Fixed from the original (3)

	loadArcadeLevel("c302.mi_", "c41", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c302.mi_"];
	arc->id = 30; // Fixed from the original (3)

	loadArcadeLevel("c410.mi_", "c42", "<check_lives>", "");
	loadArcadeLevel("c411.mi_", "c42", "<check_lives>", "");
	loadArcadeLevel("c412.mi_", "c42", "<check_lives>", "");

	loadArcadeLevel("c420.mi_", "c43", "<check_lives>", "");
	loadArcadeLevel("c421.mi_", "c43", "<check_lives>", "");
	loadArcadeLevel("c422.mi_", "c43", "<check_lives>", "");

	loadArcadeLevel("c430.mi_", "c44", "<check_lives>", "");
	loadArcadeLevel("c431.mi_", "c44", "<check_lives>", "");
	loadArcadeLevel("c432.mi_", "c44", "<check_lives>", "");

	loadArcadeLevel("c440.mi_", "c40", "<check_lives>", "");
	loadArcadeLevel("c441.mi_", "c40", "<check_lives>", "");
	loadArcadeLevel("c442.mi_", "c40", "<check_lives>", "");

	loadArcadeLevel("c400.mi_", "c51", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c400.mi_"];
	arc->id = 40; // Fixed from the original (4)

	loadArcadeLevel("c401.mi_", "c51", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c401.mi_"];
	arc->id = 40; // Fixed from the original (4)

	loadArcadeLevel("c402.mi_", "c51", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c402.mi_"];
	arc->id = 40; // Fixed from the original (4)

	loadArcadeLevel("c510.mi_", "c52", "<check_lives>", "");
	loadArcadeLevel("c511.mi_", "c52", "<check_lives>", "");
	loadArcadeLevel("c512.mi_", "c52", "<check_lives>", "");

	loadArcadeLevel("c520.mi_", "c50", "<check_lives>", "");
	loadArcadeLevel("c521.mi_", "c50", "<check_lives>", "");
	loadArcadeLevel("c522.mi_", "c50", "<check_lives>", "");

	loadArcadeLevel("c500.mi_", "c61", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c500.mi_"];
	arc->id = 50; // Fixed from the original (5)

	loadArcadeLevel("c501.mi_", "c61", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c501.mi_"];
	arc->id = 50; // Fixed from the original (5)

	loadArcadeLevel("c502.mi_", "c61", "<check_lives>", "");
	arc = (ArcadeShooting*) _levels["c502.mi_"];
	arc->id = 50; // Fixed from the original (5)

	loadArcadeLevel("c610.mi_", "c60", "<check_lives>", "");
	loadArcadeLevel("c611.mi_", "c60", "<check_lives>", "");
	loadArcadeLevel("c612.mi_", "c60", "<check_lives>", "");

	loadArcadeLevel("c600.mi_", "<credits>", "<check_lives>", "");
	loadArcadeLevel("c601.mi_", "<credits>", "<check_lives>", "");
	loadArcadeLevel("c602.mi_", "<credits>", "<check_lives>", "");

	loadLib("", "c_misc/fonts.lib", true);
	loadFonts();
	loadLib("sound/", "c_misc/sound.lib", true);
	restoreScoreMilestones(0);
	_nextLevel = "<start>";
}

void WetEngine::showCredits() {
	if (!isDemo() || (_variant == "Demo" && _language == Common::EN_USA)) {
		MVideo video("c_misc/credits.smk", Common::Point(0, 0), false, true, false);
		runIntro(video);
	}
}

void WetEngine::loadFonts() {
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
}

void WetEngine::drawString(const Common::String &font, const Common::String &str, int x, int y, int w, uint32 color) {
	int offset = 0;
	if (font == "block05.fgx") {
		for (uint32 c = 0; c < str.size(); c++) {

			offset = 0;
			if (str[c] == ':')
				offset = 1;
			else if (str[c] == '.')
				offset = 4;

			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					if (!_font05.get(275 + 40*str[c] + j*8 + i))
						_compositeSurface->setPixel(x + 5 - i + 6*c, offset + y + j, color);
				}
			}
		}
	} else if (font == "scifi08.fgx") {
		for (uint32 c = 0; c < str.size(); c++) {
			if (str[c] == 0)
				continue;
			assert(str[c] >= 32);
			offset = 0;
			if (str[c] == 't')
				offset = 0;
			else if (str[c] == 'i' || str[c] == '%')
				offset = 1;
			else if (Common::isLower(str[c]) || str[c] == ':')
				offset = 2;

			for (int i = 0; i < 6; i++) {
				for (int j = 0; j < 8; j++) {
					if (!_font08.get(1554 + 72*(str[c]-32) + j*8 + i))
						_compositeSurface->setPixel(x + 6 - i + 7*c, offset + y + j, color);
				}
			}
		}
	} else
		error("Invalid font: '%s'", font.c_str());
}

void WetEngine::saveProfile(const Common::String &name, int levelId) {
	SaveStateList saves = getMetaEngine()->listSaves(_targetName.c_str());

	// Find the correct level index to before saving
	for (uint32 i = 0; i < _ids.size(); i++) {
		if (levelId == _ids[i]) {
			if (_lastLevel < int(i))
				_lastLevel = int(i);
			break;
		}
	}

	uint32 slot = 0;
	for (SaveStateList::iterator save = saves.begin(); save != saves.end(); ++save) {
		if (save->getDescription() == name)
			break;
		slot++;
	}

	saveGameState(slot, name, false);
}

bool WetEngine::loadProfile(const Common::String &name) {
	SaveStateList saves = getMetaEngine()->listSaves(_targetName.c_str());
	uint32 slot = 0;
	for (SaveStateList::iterator save = saves.begin(); save != saves.end(); ++save) {
		if (save->getDescription() == name)
			break;
		slot++;
	}

	if (slot == saves.size()) {
		debugC(1, kHypnoDebugMedia, "Failed to load %s", name.c_str());
		return false;
	}

	loadGameState(slot);
	return true;
}

Common::Error WetEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (isAutosave)
		return Common::kNoError;

	if (_lastLevel < 0 || _lastLevel >= 20)
		error("Invalid last level!");

	stream->writeString(_name);
	stream->writeByte(0);

	stream->writeString(_difficulty);
	stream->writeByte(0);

	stream->writeUint32LE(_lives);
	stream->writeUint32LE(_score);

	stream->writeUint32LE(_lastLevel);
	return Common::kNoError;
}

Common::Error WetEngine::loadGameStream(Common::SeekableReadStream *stream) {
	_name = stream->readString();
	_difficulty = stream->readString();
	_lives = stream->readUint32LE();
	_score = stream->readUint32LE();
	_lastLevel = stream->readUint32LE();

	if (_lastLevel == 0)
		_nextLevel = Common::String::format("c%d", _ids[0]);
	else
		_nextLevel = "<level_menu>";

	restoreScoreMilestones(_score);
	return Common::kNoError;
}

Common::String WetEngine::findNextLevel(const Transition *trans) {
	if (trans->nextLevel.empty())
		error("Invalid transition!");
	return trans->nextLevel;
}

Common::String WetEngine::findNextLevel(const Common::String &level) {
	Common::String nextLevel;
	if (Common::matchString(level.c_str(), "c#") || Common::matchString(level.c_str(), "c##"))
		nextLevel = level + _difficulty + ".mi_";
	else {
		nextLevel = level;
	}

	return nextLevel;
}

} // End of namespace Hypno
