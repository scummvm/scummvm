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

#include "hypno/grammar.h"
#include "hypno/hypno.h"

#include "common/events.h"

namespace Hypno {

static const chapterEntry rawChapterTable[] = {
	{19, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{11, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{12, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{13, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{14, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{15, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{16, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{17, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{18, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{21, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{22, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{31, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{32, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{33, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{34, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{3591, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{3592, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{36, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{41, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor},
	{0,  {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, kHypnoNoColor}
};

BoyzEngine::BoyzEngine(OSystem *syst, const ADGameDescription *gd) : HypnoEngine(syst, gd) {
	_screenW = 320;
	_screenH = 200;
	_lives = uint32(-1); // This counts the number of lives used
	_currentWeapon = 0;
	_currentActor = 0;
	_currentMode = NonInteractive;
	_crosshairsPalette = nullptr;
	_lastLevel = 0;

    const chapterEntry *entry = rawChapterTable;
    while (entry->id) {
		_ids.push_back(entry->id);
		entry++;
    }

	for (int i = 0; i < 6; i++) {
		_weaponMaxAmmo[i] = 0;
	}
}

static const char *selectBoyz = "\
MENU preload\\slct_boy.smk\n\
HOTS /BBOX= 19  3   66  199\n\
SOND tollop1.raw 22K\n\
HOTS /BBOX= 69  3   116 199\n\
SOND tollam.raw  22K\n\
HOTS /BBOX= 119 3   166 199\n\
SOND tolvas1.raw 22K\n\
HOTS /BBOX= 169 3   216 199\n\
SOND tolbut1.raw 22K\n\
HOTS /BBOX= 219 3   266 199\n\
SOND tolbro1.raw 22K\n\
HOTS /BBOX= 269 3   316 170\n\
SOND tolmon1.raw 22K\n\
HOTS /BBOX= 262 172 312 194\n\
SOND pstlfire.raw\n\
END\n";

static const char *selectC3 = "\
MENU preload\\slct_c3.smk\n\
HOTS /BBOX= 143 78 286 138\n\
SOND tollopz2.raw 22K\n\
HOTS /BBOX=  159 141 319 197\n\
SOND tolbuts3.raw 22K\n\
HOTS /BBOX= 26 14 126 89\n\
SOND tolvasq1.raw 22K\n\
HOTS /BBOX= 24 114 126 190\n\
SOND tolmons1.raw 22K\n\
HOTS /BBOX= 190 4 292 76\n\
SOND tollamb1.raw  22K\n\
END\n";

static const char *selectHo = "\
MENU preload\\slct_ho.smk\n\
HOTS /BBOX= 19  18  85  199\n\
SOND tien1bb.raw 22K\n\
HOTS /BBOX= 87  36  143 199\n\
SOND lan1.raw  22K\n\
HOTS /BBOX= 144 36  193 199\n\
SOND mai1.raw 22K\n\
HOTS /BBOX= 195 55  249 199\n\
SOND hoa1b.raw 22K\n\
HOTS /BBOX= 250 32  301 199\n\
SOND van2.raw 22K\n\
END\n";

void BoyzEngine::loadAssets() {
	LibFile *missions = loadLib("", "preload/missions.lib", true);
	Common::ArchiveMemberList files;
	if (missions->listMembers(files) == 0)
		error("Failed to load any files from missions.lib");

	Transition *logos = new Transition("<main_menu>");
	logos->intros.push_back("intro/dclogos.smk");
	logos->intros.push_back("intro/mplogos.smk");
	logos->intros.push_back("intro/hyplogos.smk");
	logos->intros.push_back("intro/sblogos.smk");
	_levels["<start>"] = logos;

	Code *main_menu = new Code("<main_menu>");
	_levels["<main_menu>"] = main_menu;
	_levels["<main_menu>"]->levelIfWin = "<difficulty_menu>";

	Code *difficulty_menu = new Code("<difficulty_menu>");
	_levels["<difficulty_menu>"] = difficulty_menu;
	_levels["<difficulty_menu>"]->levelIfWin = "<select_boyz>";

	Code *retry = new Code("<retry_menu>");
	_levels["<retry_menu>"] = retry;

	loadArcadeLevel("c19.mi_", "c11.mi_", "<retry_menu>", "");
	loadArcadeLevel("c11.mi_", "c12.mi_", "<retry_menu>", "");
	loadArcadeLevel("c12.mi_", "c14.mi_", "<retry_menu>", "");
	loadArcadeLevel("c14.mi_", "c13.mi_", "<retry_menu>", "");
	loadArcadeLevel("c13.mi_", "c15.mi_", "<retry_menu>", "");
	loadArcadeLevel("c15.mi_", "c16.mi_", "<retry_menu>", "");
	loadArcadeLevel("c16.mi_", "c17.mi_", "<retry_menu>", "");
	loadArcadeLevel("c17.mi_", "c18.mi_", "<retry_menu>", "");
	loadArcadeLevel("c18.mi_", "c21.mi_", "<retry_menu>", "");

	loadArcadeLevel("c21.mi_", "c22.mi_", "<retry_menu>", "");
	loadArcadeLevel("c22.mi_", "<select_c3>", "<retry_menu>", "");
	loadArcadeLevel("c31.mi_", "<check_c3>", "<retry_menu>", "");
	loadArcadeLevel("c32.mi_", "<check_c3>", "<retry_menu>", "");
	loadArcadeLevel("c33.mi_", "<check_c3>", "<retry_menu>", "");
	loadArcadeLevel("c34.mi_", "<check_c3>", "<retry_menu>", "");
	loadArcadeLevel("c35.mi_", "<check_ho>", "<select_c3>", "");
	ArcadeShooting *ar = (ArcadeShooting *) _levels["c35.mi_"];
	ar->backgroundVideo = ""; // This will be manually populated

	loadArcadeLevel("c351.mi_", "<check_ho>", "<retry_menu>", "");
	loadArcadeLevel("c352.mi_", "<check_ho>", "<retry_menu>", "");
	loadArcadeLevel("c353.mi_", "<check_ho>", "<retry_menu>", "");
	ar = (ArcadeShooting *) _levels["c353.mi_"];
	ar->id = 353; // This corrects a mistake in the game scripts
	loadArcadeLevel("c354.mi_", "<check_ho>", "<retry_menu>", "");
	loadArcadeLevel("c355.mi_", "<check_ho>", "<retry_menu>", "");

	loadArcadeLevel("c36.mi_", "c41.mi_", "<retry_menu>", "");
	loadArcadeLevel("c41.mi_", "c42.mi_", "<retry_menu>", "");
	loadArcadeLevel("c42.mi_", "c51.mi_", "<retry_menu>", "");

	loadArcadeLevel("c51.mi_", "c52.mi_", "<retry_menu>", "");
	loadArcadeLevel("c52.mi_", "c53.mi_", "<retry_menu>", "");
	loadArcadeLevel("c53.mi_", "c54.mi_", "<retry_menu>", "");
	loadArcadeLevel("c54.mi_", "c55.mi_", "<retry_menu>", "");
	loadArcadeLevel("c55.mi_", "c56.mi_", "<retry_menu>", "");
	loadArcadeLevel("c56.mi_", "c57.mi_", "<retry_menu>", "");
	loadArcadeLevel("c57.mi_", "c58.mi_", "<retry_menu>", "");
	loadArcadeLevel("c58.mi_", "c59.mi_", "<retry_menu>", "");
	loadArcadeLevel("c59.mi_", "<credits>", "<retry_menu>", "");

	Global *gl;
	ChangeLevel *cl;
	Cutscene *cs;
	Highlight *hl;

	loadSceneLevel(selectBoyz, "<select_boyz>", "", "");
	Scene *sc = (Scene *) _levels["<select_boyz>"];
	sc->resolution = "320x200";

	hl = new Highlight("GS_SWITCH1");
	sc->hots[1].actions.push_back(hl);
	gl = new Global("GS_SWITCH1", "TURNON");
	sc->hots[1].actions.push_back(gl);
	cs = new Cutscene("intro/c0i01s.smk");
	sc->hots[1].actions.push_back(cs);

	hl = new Highlight("GS_SWITCH2");
	sc->hots[2].actions.push_back(hl);
	gl = new Global("GS_SWITCH2", "TURNON");
	sc->hots[2].actions.push_back(gl);
	cs = new Cutscene("intro/c0i02s.smk");
	sc->hots[2].actions.push_back(cs);

	hl = new Highlight("GS_SWITCH3");
	sc->hots[3].actions.push_back(hl);
	gl = new Global("GS_SWITCH3", "TURNON");
	sc->hots[3].actions.push_back(gl);
	cs = new Cutscene("intro/c0i03s.smk");
	sc->hots[3].actions.push_back(cs);

	hl = new Highlight("GS_SWITCH4");
	sc->hots[4].actions.push_back(hl);
	gl = new Global("GS_SWITCH4", "TURNON");
	sc->hots[4].actions.push_back(gl);
	cs = new Cutscene("intro/c0i04s.smk");
	sc->hots[4].actions.push_back(cs);

	hl = new Highlight("GS_SWITCH5");
	sc->hots[5].actions.push_back(hl);
	gl = new Global("GS_SWITCH5", "TURNON");
	sc->hots[5].actions.push_back(gl);
	cs = new Cutscene("intro/c0i05s.smk");
	sc->hots[5].actions.push_back(cs);

	hl = new Highlight("GS_SWITCH6");
	sc->hots[6].actions.push_back(hl);
	gl = new Global("GS_SWITCH6", "TURNON");
	sc->hots[6].actions.push_back(gl);
	cs = new Cutscene("intro/c0i06s.smk");
	sc->hots[6].actions.push_back(cs);

	cl = new ChangeLevel("c19.mi_");
	sc->hots[7].actions.push_back(cl);

	loadSceneLevel(selectC3, "<select_c3>", "", "");
	sc = (Scene *) _levels["<select_c3>"];
	sc->resolution = "320x200";

	hl = new Highlight("GS_SEQ_31");
	sc->hots[1].actions.push_back(hl);
	gl = new Global("GS_SEQ_31", "NCHECK");
	sc->hots[1].actions.push_back(gl);
	cl = new ChangeLevel("c31.mi_");
	sc->hots[1].actions.push_back(cl);

	hl = new Highlight("GS_SEQ_32");
	sc->hots[2].actions.push_back(hl);
	gl = new Global("GS_SEQ_32", "NCHECK");
	sc->hots[2].actions.push_back(gl);
	cl = new ChangeLevel("c32.mi_");
	sc->hots[2].actions.push_back(cl);

	hl = new Highlight("GS_SEQ_33");
	sc->hots[3].actions.push_back(hl);
	gl = new Global("GS_SEQ_33", "NCHECK");
	sc->hots[3].actions.push_back(gl);
	cl = new ChangeLevel("c33.mi_");
	sc->hots[3].actions.push_back(cl);

	hl = new Highlight("GS_SEQ_34");
	sc->hots[4].actions.push_back(hl);
	gl = new Global("GS_SEQ_34", "NCHECK");
	sc->hots[4].actions.push_back(gl);
	cl = new ChangeLevel("c34.mi_");
	sc->hots[4].actions.push_back(cl);

	hl = new Highlight("GS_WONSHELLGAME");
	sc->hots[5].actions.push_back(hl);
	gl = new Global("GS_WONSHELLGAME", "NCHECK");
	gl = new Global("GS_HOTELDONE", "NCHECK");
	sc->hots[5].actions.push_back(gl);
	cl = new ChangeLevel("c35.mi_");
	sc->hots[5].actions.push_back(cl);

	loadSceneLevel(selectHo, "<select_ho>", "", "");
	sc = (Scene *) _levels["<select_ho>"];
	sc->resolution = "320x200";

	hl = new Highlight("GS_SEQ_351");
	sc->hots[1].actions.push_back(hl);
	gl = new Global("GS_SEQ_351", "NCHECK");
	sc->hots[1].actions.push_back(gl);
	cl = new ChangeLevel("c351.mi_");
	sc->hots[1].actions.push_back(cl);

	hl = new Highlight("GS_SEQ_352");
	sc->hots[2].actions.push_back(hl);
	gl = new Global("GS_SEQ_352", "NCHECK");
	sc->hots[2].actions.push_back(gl);
	cl = new ChangeLevel("c352.mi_");
	sc->hots[2].actions.push_back(cl);

	hl = new Highlight("GS_SEQ_353");
	sc->hots[3].actions.push_back(hl);
	gl = new Global("GS_SEQ_353", "NCHECK");
	sc->hots[3].actions.push_back(gl);
	cl = new ChangeLevel("c353.mi_");
	sc->hots[3].actions.push_back(cl);

	hl = new Highlight("GS_SEQ_354");
	sc->hots[4].actions.push_back(hl);
	gl = new Global("GS_SEQ_354", "NCHECK");
	sc->hots[4].actions.push_back(gl);
	cl = new ChangeLevel("c354.mi_");
	sc->hots[4].actions.push_back(cl);

	hl = new Highlight("GS_SEQ_355");
	sc->hots[5].actions.push_back(hl);
	gl = new Global("GS_SEQ_355", "NCHECK");
	sc->hots[5].actions.push_back(gl);
	cl = new ChangeLevel("c355.mi_");
	sc->hots[5].actions.push_back(cl);

	Code *check_c3 = new Code("<check_c3>");
	_levels["<check_c3>"] = check_c3;

	Code *check_ho = new Code("<check_ho>");
	_levels["<check_ho>"] = check_ho;

	loadLib("sound/", "misc/sound.lib", true);

	_weaponShootSound[0] = "";
	_weaponShootSound[1] = "pstlfire.raw";
	_weaponShootSound[2] = "ak47fire.raw";
	_weaponShootSound[3] = "dblfire.raw";
	_weaponShootSound[4] = "m16fire.raw";
	_weaponShootSound[5] = "shotfire.raw";
	_weaponShootSound[6] = "glm60fr.raw";
	_weaponShootSound[7] = "dblfire.raw"; // Not sure

	_weaponReloadSound[0] = "";
	_weaponReloadSound[1] = "pstlload.raw";
	_weaponReloadSound[2] = "ak47load.raw";
	_weaponReloadSound[3] = "dblload.raw";
	_weaponReloadSound[4] = "m16load.raw";
	_weaponReloadSound[5] = "shotload.raw";
	_weaponReloadSound[6] = "glm60rl.raw";
	_weaponReloadSound[7] = "dblload.raw"; // Not sure

	_deathDay[0] = "";
	_deathDay[1] = "preload/deathd1s.smk";
	_deathDay[2] = "preload/deathd2s.smk";
	_deathDay[3] = "preload/deathd3s.smk";
	_deathDay[4] = "preload/deathd4s.smk";
	_deathDay[5] = "preload/deathd5s.smk";
	_deathDay[6] = "preload/deathd6s.smk";

	_deathNight[0] = "";
	_deathNight[1] = "preload/deathn2s.smk";
	_deathNight[2] = "preload/deathn3s.smk";
	_deathNight[3] = "preload/deathn4s.smk";
	_deathNight[4] = "";
	_deathNight[5] = "";
	_deathNight[6] = "";

	_deathVideo.push_back("preload/cdeath1s.smk");
	_deathVideo.push_back("preload/cdeath2s.smk");
	_deathVideo.push_back("preload/cdeath3s.smk");

	_heySound[0] = "";
	_heySound[1] = "lopzheys.raw";
	_heySound[2] = "lambheys.raw";
	_heySound[3] = "vasqheyl.raw";
	_heySound[4] = "butsheyl.raw";
	_heySound[5] = "bropheys.raw";
	_heySound[6] = "monsheys.raw";

	Graphics::Surface *targets = decodeFrame("preload/targets.smk", 0, &_crosshairsPalette);

	Common::Rect cursorBox;

	// No weapon, same as pistol
	cursorBox = Common::Rect(62, 6, 83, 26);
	_crosshairsInactive[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 38, 83, 58);
	_crosshairsActive[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 70, 83, 90);
	_crosshairsTarget[0].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[0].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Pistol
	cursorBox = Common::Rect(62, 6, 83, 26);
	_crosshairsInactive[1].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[1].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 38, 83, 58);
	_crosshairsActive[1].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[1].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(62, 70, 83, 90);
	_crosshairsTarget[1].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[1].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Double big crossair, shotgun?
	cursorBox = Common::Rect(104, 7, 136, 25);
	_crosshairsInactive[2].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[2].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 39, 136, 57);
	_crosshairsActive[2].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[2].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 71, 136, 83);
	_crosshairsTarget[2].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[2].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 8, 32, 24);
	_crosshairsInactive[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 40, 32, 56);
	_crosshairsActive[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(16, 72, 32, 88);
	_crosshairsTarget[3].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[3].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 11, 173, 21);
	_crosshairsInactive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 43, 173, 53);
	_crosshairsActive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(163, 75, 173, 85);
	_crosshairsTarget[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Shotgun
	cursorBox = Common::Rect(104, 7, 136, 25);
	_crosshairsInactive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 39, 136, 57);
	_crosshairsActive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(104, 71, 136, 83);
	_crosshairsTarget[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[5].copyRectToSurface(*targets, 0, 0, cursorBox);

    // Double small
	/*
	cursorBox = Common::Rect(203, 11, 229, 21);
	_crosshairsInactive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(203, 43, 229, 53);
	_crosshairsActive[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(203, 75, 229, 85);
	_crosshairsTarget[4].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[4].copyRectToSurface(*targets, 0, 0, cursorBox);

	// Granade

	cursorBox = Common::Rect(256, 2, 273, 29);
	_crosshairsInactive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(256, 34, 273, 61);
	_crosshairsActive[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[5].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(256, 66, 273, 93);
	_crosshairsTarget[5].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[5].copyRectToSurface(*targets, 0, 0, cursorBox);
    */
	// Large crossair

	cursorBox = Common::Rect(56, 98, 88, 125);
	_crosshairsInactive[7].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsInactive[7].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(57, 130, 88, 158);
	_crosshairsActive[7].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsActive[7].copyRectToSurface(*targets, 0, 0, cursorBox);

	cursorBox = Common::Rect(57, 162, 88, 189);
	_crosshairsTarget[7].create(cursorBox.width(), cursorBox.height(), _pixelFormat);
	_crosshairsTarget[7].copyRectToSurface(*targets, 0, 0, cursorBox);

	_weaponMaxAmmo[0] = 0;
	_weaponMaxAmmo[1] = 10;
	_weaponMaxAmmo[2] = 2; // large shotgun
	_weaponMaxAmmo[3] = 8;
	_weaponMaxAmmo[4] = 6; // pistol?
	_weaponMaxAmmo[5] = 6; // small shotgun
	_weaponMaxAmmo[6] = 0;
	_weaponMaxAmmo[7] = 25; // Boat machine-gun

	_maxHealth = 100;
	_civiliansShoot = 0;
	_warningVideosDay.push_back("");
	_warningVideosDay.push_back("warnings/w01s.smk");
	_warningVideosDay.push_back("warnings/w02s.smk");
	_warningVideosDay.push_back("warnings/w03s.smk");
	_warningVideosDay.push_back("warnings/w04s.smk");
	_warningVideosDay.push_back("warnings/w05s.smk");
	_warningVideosDay.push_back("warnings/w06s.smk");
	_warningVideosDay.push_back("warnings/w07s.smk");

	_warningAlarmVideos.push_back("warnings/w10s.smk");
	_warningAlarmVideos.push_back("warnings/w11s.smk");

	_warningHostage = "warnings/w08s.smk";

	// Set initial health for the team
	_health = _maxHealth;
	_previousHealth = _maxHealth;

	targets->free();
	delete targets;

	loadLib("", "misc/fonts.lib", true);
	loadFonts();
	_defaultCursor = "crosshair";
	_defaultCursorIdx = uint32(-1);
	resetSceneState();
	_nextLevel = "<start>";
}

void BoyzEngine::loadFonts() {
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

void BoyzEngine::drawString(const Common::String &font, const Common::String &str, int x, int y, int w, uint32 color) {
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

void BoyzEngine::saveProfile(const Common::String &name, int levelId) {
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
	debugC(1, kHypnoDebugMedia, "Saving profile %s with last level %d", name.c_str(), _lastLevel);
	saveGameState(slot, name, false);
}

Common::Array<Common::String> BoyzEngine::listProfiles() {
	Common::Array<Common::String> profiles;
	SaveStateList saves = getMetaEngine()->listSaves(_targetName.c_str());
	for (SaveStateList::iterator save = saves.begin(); save != saves.end(); ++save) {
		Common::String profile = save->getDescription();
		profile.toUppercase();
		profiles.push_back(profile);
	}
	return profiles;
}

bool BoyzEngine::loadProfile(const Common::String &name) {
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

Common::Error BoyzEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (isAutosave)
		return Common::kNoError;

	if (_lastLevel < 0 || _lastLevel >= 20)
		error("Invalid last level!");

	stream->writeString(_name);
	stream->writeByte(0);

	stream->writeString(_difficulty);
	stream->writeByte(0);

	stream->writeUint32LE(_lives);
	stream->writeUint32LE(_previousHealth);
	stream->writeUint32LE(_score);

	stream->writeUint32LE(_lastLevel);

	stream->writeUint32LE(_sceneState["GS_C5MAP"]);
	stream->writeUint32LE(_sceneState["GS_WONSHELLGAME"]);
	stream->writeUint32LE(_sceneState["GS_C36_READY"]);
	stream->writeUint32LE(_sceneState["GS_MINEMAP"]);
	stream->writeUint32LE(_sceneState["GS_MINEMAP_VIEWED"]);
	stream->writeUint32LE(_sceneState["GS_HOTELDONE"]);

	stream->writeUint32LE(_sceneState["GS_SEQ_31"]);
	stream->writeUint32LE(_sceneState["GS_SEQ_32"]);
	stream->writeUint32LE(_sceneState["GS_SEQ_33"]);
	stream->writeUint32LE(_sceneState["GS_SEQ_34"]);
	stream->writeUint32LE(_sceneState["GS_SEQ_35"]);
	stream->writeUint32LE(_sceneState["GS_SEQ_36"]);

	return Common::kNoError;
}

Common::Error BoyzEngine::loadGameStream(Common::SeekableReadStream *stream) {
	_name = stream->readString();
	_difficulty = stream->readString();
	_lives = stream->readUint32LE();
	_previousHealth = stream->readUint32LE();
	_score = stream->readUint32LE();
	_lastLevel = stream->readUint32LE();

	_sceneState["GS_C5MAP"] = stream->readUint32LE();
	_sceneState["GS_WONSHELLGAME"] = stream->readUint32LE();
	_sceneState["GS_C36_READY"] = stream->readUint32LE();
	_sceneState["GS_MINEMAP"] = stream->readUint32LE();
	_sceneState["GS_MINEMAP_VIEWED"] = stream->readUint32LE();
	_sceneState["GS_HOTELDONE"] = stream->readUint32LE();

	_sceneState["GS_SEQ_31"] = stream->readUint32LE();
	_sceneState["GS_SEQ_32"] = stream->readUint32LE();
	_sceneState["GS_SEQ_33"] = stream->readUint32LE();
	_sceneState["GS_SEQ_34"] = stream->readUint32LE();
	_sceneState["GS_SEQ_35"] = stream->readUint32LE();


	if (_ids[_lastLevel] == 3591)
		_nextLevel = "<select_c3>";
	else if (_ids[_lastLevel] == 3592)
		_nextLevel = "<select_ho>";
	else
		_nextLevel = Common::String::format("c%d.mi_", _ids[_lastLevel]);
	return Common::kNoError;
}


Common::String BoyzEngine::findNextLevel(const Transition *trans) {
	if (trans->nextLevel.empty())
		error("Invalid transition!");
	return trans->nextLevel;
}

Common::String BoyzEngine::findNextLevel(const Common::String &level) { return level; }

} // namespace Hypno
