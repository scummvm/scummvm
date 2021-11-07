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

#include "common/events.h"
#include "common/config-manager.h"

#include "hypno/hypno.h"

namespace Hypno {

static const chapterEntry rawChapterTable[] = {
	{11, {44, 172}, {218, 172}, {0, 0}}, 		// c11
	{31, {70, 160}, {180, 160}, {220, 185}}, 	// c31
	{52, {60, 167}, {190, 167}, {135, 187}}, 	// c52
	{0,  {0,  0},   {0,   0},   {0,   0}}    	// NULL
};

WetEngine::WetEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {
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

	Hotspot h(MakeMenu, "");
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

	Transition *intro = new Transition();
	intro->level = "c31.mi_";
	intro->intros.push_back("movie/nw_logo.smk");
	intro->intros.push_back("movie/hypnotix.smk");
	intro->intros.push_back("movie/wetlogo.smk");
	intro->frameImage = "wetlands/c_misc/c.s";
	intro->frameNumber = 0;
	_levels["<intro>"] = intro;

	Transition *movies = new Transition();
	movies->level = "<quit>";
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

	loadArcadeLevel("c31", "c52", "wetlands");
	loadArcadeLevel("c52", "<gameover>", "wetlands");

	Transition *over = new Transition();
	over->level = "<quit>";
	over->intros.push_back("movie/gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("", "wetlands/c_misc/fonts.lib", true);
	loadLib("wetlands/sound/", "wetlands/c_misc/sound.lib", true);
}

void WetEngine::loadAssetsPCW() {

	LibFile *missions = loadLib("", "c_misc/missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Transition *intro = new Transition();
	intro->level = "c11.mis";
	intro->intros.push_back("c_misc/nw_logo.smk");
	intro->intros.push_back("c_misc/h.s");
	intro->intros.push_back("c_misc/wet.smk");
	intro->frameImage.clear();
	_levels["<start>"] = intro;

	loadArcadeLevel("c11", "<gameover>", "");

	Transition *over = new Transition();
	over->level = "<quit>";
	over->intros.push_back("movie/gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("", "c_misc/sound.lib", false);
}

void WetEngine::loadAssetsPCG() {

	LibFile *missions = loadLib("", "missions.lib", false);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Transition *intro = new Transition();
	intro->level = "c31.mi_";	
	intro->intros.push_back("nw_logo.smk");
	intro->intros.push_back("h.s");
	intro->intros.push_back("wet.smk");
	intro->frameImage.clear();
	_levels["<start>"] = intro;

	loadArcadeLevel("c31", "<gameover>", "");

	Transition *over = new Transition();
	over->level = "<quit>";
	over->intros.push_back("gameover.smk");
	_levels["<gameover>"] = over;

	loadLib("", "sound.lib", false);
}

void WetEngine::loadAssetsFullGame() {
	LibFile *missions = loadLib("", "c_misc/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions == nullptr || missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Transition *logos = new Transition();
	logos->level = "wetlands/main_menu.mis"; //"c11" + _difficulty + ".mi_";
	logos->intros.push_back("c_misc/logo.smk");
	logos->intros.push_back("c_misc/nw_logo.smk");
	logos->intros.push_back("c_misc/hypnotix.smk");
	logos->intros.push_back("c_misc/wetlogo.smk");
	_levels["<start>"] = logos;

	Code *menu = new Code();
	menu->name = "wetlands/main_menu.mis";
	_levels["wetlands/main_menu.mis"] = menu;
	_levels["wetlands/main_menu.mis"]->levelIfWin = "<intro>";

	Transition *intro = new Transition();
	intro->level = "c11" + _difficulty + ".mi_";
	intro->intros.push_back("c_misc/intros.smk");
	_levels["<intro>"] = intro;

	//loadLevel("c10", "c11", "");
	loadArcadeLevel("c11", "c20", "");
	loadArcadeLevel("c20", "", "");

	loadLib("", "c_misc/fonts.lib", true);
	loadLib("sound/", "c_misc/sound.lib", true);
}

void WetEngine::showCredits() {
	MVideo video("c_misc/credits.smk", Common::Point(0, 0), false, false, false);
	runIntro(video);
}

void WetEngine::runCode(Code *code) {
	changeScreenMode("320x200");
	if (code->name == "wetlands/main_menu.mis")
		runMainMenu(code);
	else
		error("invalid hardcoded level: %s", code->name.c_str());
}

void WetEngine::runMainMenu(Code *code) {
	Common::Event event;
	_font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	uint32 c = _pixelFormat.RGBToColor(0, 252, 0);
	Graphics::Surface *frame = decodeFrame("c_misc/menus.smk", 16, true);
	Common::String _name = "";
	drawImage(*frame, 0, 0, false);
	_font->drawString(_compositeSurface, "ENTER NAME :", 48, 50, 100, c);
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
				_font->drawString(_compositeSurface, "ENTER NAME :", 48, 50, 100, c);
				_font->drawString(_compositeSurface, _name, 140, 50, 170, c);
				break;


			default:
				break;
			}
		}

		drawScreen();
		g_system->delayMillis(10);
	}
}

} // End of namespace Hypno
