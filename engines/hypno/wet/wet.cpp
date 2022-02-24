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
#include "common/config-manager.h"

#include "hypno/hypno.h"

namespace Hypno {

static const chapterEntry rawChapterTable[] = {
	{11, {44, 172}, {218, 172}, {0,   0}}, 		// c11
	{10, {19, 3},   {246, 3}, 	{246, 11}}, 	// c10
	{20, {44, 172}, {218, 172}, {0,   0}}, 		// c20
	{21, {70, 160}, {180, 160}, {220, 185}}, 	// c21
	{22, {70, 160}, {180, 160}, {220, 185}}, 	// c22
	{23, {70, 160}, {180, 160}, {220, 185}}, 	// c23
	{31, {70, 160}, {180, 160}, {220, 185}}, 	// c31
	{32, {70, 160}, {180, 160}, {220, 185}}, 	// c32
	{33, {70, 160}, {180, 160}, {220, 185}}, 	// c33
	{41, {70, 160}, {180, 160}, {220, 185}}, 	// c41
	{42, {70, 160}, {180, 160}, {220, 185}}, 	// c42
	{43, {70, 160}, {180, 160}, {220, 185}}, 	// c43
	{44, {70, 160}, {180, 160}, {220, 185}}, 	// c44
	{40, {70, 160}, {180, 160}, {220, 185}}, 	// c40
	{51, {60, 167}, {190, 167}, {135, 187}}, 	// c51
	{52, {60, 167}, {190, 167}, {135, 187}}, 	// c52
	{50, {19, 3},   {246, 3}, 	{246, 11}}, 	// c50 (fixed)
	{61, {44, 172}, {218, 172}, {0, 0}}, 		// c61
	{60, {44, 172}, {218, 172}, {0, 0}}, 		// c60
	{0,  {0,  0},   {0,   0},   {0,   0}}    	// NULL
};

WetEngine::WetEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {
	_screenW = 320;
	_screenH = 200;
	_lives = 2;
 
    const chapterEntry *entry = rawChapterTable;
    while (entry->id) {
		_chapterTable[entry->id] = entry;
		entry++;
    }

}

void WetEngine::loadAssets() {
	if (!isDemo()) {
		_difficulty = "1"; // Medium difficulty by default
		loadAssetsFullGame();
		return;
	}
	_difficulty = ""; // No difficulty selection in demo

	if (_variant == "Demo")
		loadAssetsDemoDisc();
	else if (_variant == "PCWDemo")
		loadAssetsPCW();
	else if (_variant == "PCGDemo")
		loadAssetsPCG();
	else
		error("Invalid demo version: \"%s\"", _variant.c_str());
}


void WetEngine::loadAssetsDemoDisc() {

	bool encrypted = _language == Common::HE_ISR ? false : true;
	LibFile *missions = loadLib("", "wetlands/c_misc/missions.lib", encrypted);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Hotspot h(MakeMenu);
	Hotspots hs;
	Ambient *a = new Ambient("movie/selector.smk", Common::Point(0, 0), "/LOOP");
	a->fullscreen = true;
	h.actions.push_back(a);

	hs.push_back(h);

	h.type = MakeHotspot;
	h.rect = Common::Rect(0, 424, 233, 462);
	h.actions.clear();
	h.smenu = nullptr;
	ChangeLevel *cl = new ChangeLevel("<intro>");
	h.actions.push_back(cl);

	hs.push_back(h);

	h.rect = Common::Rect(242, 424, 500, 480);
	cl = new ChangeLevel("<movies>");
	h.actions.clear();
	h.actions.push_back(cl);

	hs.push_back(h);

	h.rect = Common::Rect(504, 424, 637, 480);
	Quit *q = new Quit();
	h.actions.clear();
	h.actions.push_back(q);

	hs.push_back(h);

	Scene *start = new Scene();
	start->hots = hs;
	_levels["<start>"] = start;

	Transition *intro = new Transition("c31");
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

	if (_language == Common::EN_USA) {
		loadArcadeLevel("c31.mi_", "c52", "c52", "wetlands");
		loadArcadeLevel("c52.mi_", "<game_over>", "<quit>", "wetlands");
	} else if (_language == Common::HE_ISR) {
		loadArcadeLevel("c31.mis", "c52.mis", "c52.mis", "wetlands");
		loadArcadeLevel("c52.mis", "<game_over>", "<quit>", "wetlands");
	} else {
		error("Unsupported language");
	}

	Transition *over = new Transition("<quit>");
	over->intros.push_back("movie/gameover.smk");
	_levels["<game_over>"] = over;

	loadLib("", "wetlands/c_misc/fonts.lib", true);
	loadFonts();
	loadLib("wetlands/sound/", "wetlands/c_misc/sound.lib", true);
	_nextLevel = "<start>";
}

void WetEngine::loadAssetsPCW() {

	LibFile *missions = loadLib("", "c_misc/missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Transition *intro = new Transition("c11.mis");
	intro->intros.push_back("c_misc/nw_logo.smk");
	intro->intros.push_back("c_misc/h.s");
	intro->intros.push_back("c_misc/wet.smk");
	_levels["<start>"] = intro;

	loadArcadeLevel("c11.mis", "<quit>", "<check_lives>", "");

	Transition *over = new Transition("<quit>");
	_levels["<game_over>"] = over;

	loadLib("", "c_misc/sound.lib", false);
	loadLib("", "c_misc/fonts.lib", true);
	loadFonts();
	_nextLevel = "<start>";
}

void WetEngine::loadAssetsPCG() {

	LibFile *missions = loadLib("", "missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Transition *intro = new Transition("c31.mis");
	intro->intros.push_back("nw_logo.smk");
	intro->intros.push_back("h.s");
	intro->intros.push_back("wet.smk");
	intro->frameImage = "c.s";
	intro->frameNumber = 0;
	_levels["<start>"] = intro;

	loadArcadeLevel("c31.mis", "<quit>", "<check_lives>", "");

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
		error("Failed to load any files from missions.lib");

	Transition *logos = new Transition("<main_menu>");
	logos->intros.push_back("c_misc/logo.smk");
	logos->intros.push_back("c_misc/nw_logo.smk");
	logos->intros.push_back("c_misc/hypnotix.smk");
	logos->intros.push_back("c_misc/wetlogo.smk");
	_levels["<start>"] = logos;

	Code *menu = new Code("<main_menu>");
	_levels["<main_menu>"] = menu;
	_levels["<main_menu>"]->levelIfWin = "<intros>";

	Transition *over = new Transition("<quit>");
	over->intros.push_back("c_misc/gameover.smk");
	_levels["<game_over>"] = over;

	Transition *intros = new Transition("c11");
	intros->intros.push_back("c_misc/stardate.smk");
	intros->intros.push_back("c_misc/intros.smk");
	intros->intros.push_back("c_misc/confs.smk");
	_levels["<intros>"] = intros;

	Code *check_lives = new Code("<check_lives>");
	_levels["<check_lives>"] = check_lives;

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

	loadArcadeLevel("c310.mi_", "c32", "<check_lives>", "");
	loadArcadeLevel("c311.mi_", "c32", "<check_lives>", "");
	loadArcadeLevel("c312.mi_", "c32", "<check_lives>", "");

	loadArcadeLevel("c320.mi_", "c41", "<check_lives>", "");
	loadArcadeLevel("c321.mi_", "c41", "<check_lives>", "");
	loadArcadeLevel("c322.mi_", "c41", "<check_lives>", "");

	loadArcadeLevel("c330.mi_", "c30", "<check_lives>", "");
	loadArcadeLevel("c331.mi_", "c30", "<check_lives>", "");
	loadArcadeLevel("c332.mi_", "c30", "<check_lives>", "");

	loadArcadeLevel("c300.mi_", "c41", "<check_lives>", "");
	loadArcadeLevel("c301.mi_", "c41", "<check_lives>", "");
	loadArcadeLevel("c302.mi_", "c41", "<check_lives>", "");

	loadArcadeLevel("c410.mi_", "c42", "<check_lives>", "");
	loadArcadeLevel("c411.mi_", "c42", "<check_lives>", "");
	loadArcadeLevel("c412.mi_", "c42", "<check_lives>", "");

	loadArcadeLevel("c420.mi_", "c43", "<check_lives>", "");
	loadArcadeLevel("c421.mi_", "c43", "<check_lives>", "");
	loadArcadeLevel("c422.mi_", "c43", "<check_lives>", "");

	loadArcadeLevel("c430.mi_", "c44", "<check_lives>", "");
	loadArcadeLevel("c431.mi_", "c44", "<check_lives>", "");
	loadArcadeLevel("c432.mi_", "c44", "<check_lives>", "");

	loadArcadeLevel("c440.mi_", "c51", "<check_lives>", "");
	loadArcadeLevel("c441.mi_", "c51", "<check_lives>", "");
	loadArcadeLevel("c442.mi_", "c51", "<check_lives>", "");

	loadArcadeLevel("c400.mi_", "c51", "<check_lives>", "");
	ArcadeShooting *arc = (ArcadeShooting*) _levels["c400.mi_"];
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

	loadArcadeLevel("c520.mi_", "c61", "<check_lives>", "");
	loadArcadeLevel("c521.mi_", "c61", "<check_lives>", "");
	loadArcadeLevel("c522.mi_", "c61", "<check_lives>", "");

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

	loadArcadeLevel("c600.mi_", "<quit>", "<check_lives>", "");
	loadArcadeLevel("c601.mi_", "<quit>", "<check_lives>", "");
	loadArcadeLevel("c602.mi_", "<quit>", "<check_lives>", "");

	loadLib("", "c_misc/fonts.lib", true);
	loadFonts();
	loadLib("sound/", "c_misc/sound.lib", true);
	_nextLevel = "<start>";
}

void WetEngine::showCredits() {
	if (_cheatsEnabled && !_arcadeMode.empty()) {
		_skipLevel = true;
		return;
	}

	if (!isDemo() || _variant == "Demo") {
		MVideo video("c_misc/credits.smk", Common::Point(0, 0), false, false, false);
		runIntro(video);
	}
}

void WetEngine::runCode(Code *code) {
	changeScreenMode("320x200");
	if (code->name == "<main_menu>")
		runMainMenu(code);
	else if (code->name == "<check_lives>")
		runCheckLives(code);
	else
		error("invalid hardcoded level: %s", code->name.c_str());
}

void WetEngine::runCheckLives(Code *code) {
	if (_lives < 0)
		_nextLevel = "<game_over>";
	else
		_nextLevel = _checkpoint;
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
	} else
		error("Invalid font: '%s'", font.c_str());
}

void WetEngine::runMainMenu(Code *code) {
	Common::Event event;
	uint32 c = 252; // green
	byte *palette;
	Graphics::Surface *frame = decodeFrame("c_misc/menus.smk", 16, &palette);
	loadPalette(palette, 0, 256);
	Common::String _name = "";
	drawImage(*frame, 0, 0, false);
	drawString("scifi08.fgx", "ENTER NAME :", 48, 50, 100, c);
	while (!shouldQuit()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_BACKSPACE)
					_name.deleteLastChar();
				else if (event.kbd.keycode == Common::KEYCODE_RETURN && !_name.empty()) {
					_nextLevel = code->levelIfWin;
					return;
				}
				else if (Common::isAlnum(event.kbd.keycode)) {
					_name = _name + char(event.kbd.keycode - 32);
				}

				drawImage(*frame, 0, 0, false);
				drawString("scifi08.fgx", "ENTER NAME :", 48, 50, 100, c);
				drawString("scifi08.fgx", _name, 140, 50, 170, c);
				break;


			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}
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
