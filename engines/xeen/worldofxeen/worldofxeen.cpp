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

#include "xeen/worldofxeen/worldofxeen.h"
#include "xeen/worldofxeen/darkside_cutscenes.h"
#include "xeen/worldofxeen/clouds_cutscenes.h"
#include "xeen/worldofxeen/worldofxeen_menu.h"
#include "xeen/sound.h"

namespace Xeen {
namespace WorldOfXeen {

WorldOfXeenEngine::WorldOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: XeenEngine(syst, gameDesc), CloudsCutscenes(this),
		DarkSideCutscenes(this) {
	_seenDarkSideIntro = false;
	_pendingAction = WOX_PLAY_GAME;
}

void WorldOfXeenEngine::outerGameLoop() {
	//_pendingAction = getGameID() == GType_DarkSide ? WOX_DARKSIDE_INTRO : WOX_CLOUDS_INTRO;
	_pendingAction = WOX_MENU;

	if (_loadSaveSlot != -1)
		// Loading savegame from launcher, so Skip menu and go straight to game
		_pendingAction = WOX_PLAY_GAME;

	while (!shouldQuit() && _pendingAction != WOX_QUIT) {
		WOXGameAction action = _pendingAction;
		_pendingAction = WOX_MENU;

		switch (action) {
		case WOX_CLOUDS_INTRO:
			if (showCloudsTitle())
				showCloudsIntro();
			break;

		case WOX_CLOUDS_ENDING:
			showCloudsEnding();
			break;

		case WOX_DARKSIDE_INTRO:
			if (showDarkSideTitle())
				showDarkSideIntro();
			break;

		case WOX_DARKSIDE_ENDING:
			showDarkSideEnding();
			break;

		case WOX_WORLD_ENDING:
			// TODO
			return;

		case WOX_MENU:
			WorldOfXeenMenu::show(this);
			break;

		case WOX_PLAY_GAME:
			playGame();
			break;

		default:
			break;
		}
	}
}

void WorldOfXeenEngine::death() {
	Window &w = (*_windows)[0];
	_sound->stopAllAudio();
	SpriteResource fireSprites[4] = {
		SpriteResource("fire1.vga"),
		SpriteResource("fire2.vga"),
		SpriteResource("fire3.vga"),
		SpriteResource("fire4.vga")
	};
	SpriteResource deathSprites("death.vga"), death1Sprites("death1.vga");
	const int Y_LIST[] = {
		196, 187, 179, 169, 159, 147, 138, 127, 113, 101, 86,
		73, 60, 48, 36, 23, 10, 0, 0
	};

	Graphics::ManagedSurface savedBg;
	savedBg.copyFrom(*_screen);

	fireSprites[0].draw(0, 0, Common::Point(0, 0));
	fireSprites[0].draw(0, 1, Common::Point(160, 0));
	w.update();
	_sound->playSound("fire.voc");

	// Fire will vertically consume the screen
	for (int idx = 2; idx < 36; idx += 2) {
		_events->updateGameCounter();
		_screen->blitFrom(savedBg);

		fireSprites[idx / 10].draw(0, idx % 10, Common::Point(0, 0));
		fireSprites[idx / 10].draw(0, (idx % 10) + 1, Common::Point(160, 0));

		for (int yCtr = 0, frame = 0; yCtr < (idx / 2); ++yCtr, frame += 2) {
			deathSprites.draw(0, frame, Common::Point(0, Y_LIST[yCtr]));
			deathSprites.draw(0, frame + 1, Common::Point(160, Y_LIST[yCtr]));
		}

		w.update();
		_events->wait(1);
	}

	deathSprites.draw(0, 34, Common::Point(0, 0));
	deathSprites.draw(0, 35, Common::Point(160, 0));
	w.update();
	savedBg.blitFrom(*_screen);

	_sound->playSong(_files->_isDarkCc ? "laff1.voc" : "xeenlaff.voc");

	// Animation of Xeen or Alamar laughing
	for (int idx = 0, idx2 = 0; idx < (_files->_isDarkCc ? 10 : 23); ++idx) {
		_events->updateGameCounter();
		_screen->blitFrom(savedBg);

		if (idx != 0)
			death1Sprites.draw(0, idx - 1);
		w.update();

		if (_files->_isDarkCc) {
			_events->wait(2);
		} else {
			if (idx == 1 || idx == 11)
				_sound->playFX(33);
			_events->wait(2);
			if (idx == 15)
				_sound->playFX(34);
		}

		if (idx == (_files->_isDarkCc ? 9 : 10)) {
			if (idx2 < (_files->_isDarkCc ? 2 : 1)) {
				idx = -1;
				++idx2;
			}
		}

		if (!_sound->isPlaying())
			idx = 23;
	}

	_screen->blitFrom(savedBg);
	w.update();
}

void WorldOfXeenEngine::showCutscene(const Common::String &name) {
	_sound->stopAllAudio();

	if (name == "ENDGAME")
		showCloudsEnding();
	else if (name == "ENDGAME2")
		showDarkSideEnding();
	else if (name == "WORLDEND")
		showWorldOfXeenEnding();
}

void WorldOfXeenEngine::showWorldOfXeenEnding() {
	Windows &windows = *_windows;
	Window &w0 = windows[0];

	_files->setGameCc(1);
	_sound->playSong("outday3.m");
	showPharaohEndText(Res.WORLD_END_TEXT[0], nullptr, nullptr);
	_sound->playSound("elect.voc");

	Graphics::ManagedSurface savedBg;
	_screen->loadBackground("skymain.raw");
	savedBg.copyFrom(*_screen);
	_screen->loadBackground("twrsky1.raw");
	_screen->loadPage(0);
	_screen->loadPage(1);
	_screen->loadPalette("skymain.pal");

	SpriteResource sc02("sc02.eg2"), tower1("tower1.eg2"), tower2("tower2.eg2"),
		sc3a("sc3a.eg2"), sc06("sc06.eg2"), sc14("sc14.eg2"), sc13("sc13.eg2"),
		sc17("sc17.eg2"), cube("cube.eg2"), hands("hands.eg2"),
		sc10("sc10.eg2"), staff("staff.eg2");
	SpriteResource sc3b[2] = {
		SpriteResource("sc3b1.eg2"), SpriteResource("sc3b2.eg2")
	};
	SpriteResource sc20[4] = {
		SpriteResource("sc20[0].eg2"), SpriteResource("sc20b.eg2"),
		SpriteResource("sc20c.eg2"), SpriteResource("sc20d.eg2")
	};
	SpriteResource sc22[2] = {
		SpriteResource("sc22a.eg2"), SpriteResource("sc22b.eg2")
	};

	windows[41].writeString("\x1\xD");
	_screen->fadeOut();
	while (!shouldQuit() && _sound->isPlaying())
		_events->pollEventsAndWait();

	_sound->playSong("nwblksmt.m");
	_screen->blitFrom(savedBg);
	setWorldEndingSubtitle(Res.WORLD_END_TEXT[1]);
	w0.update();
	_screen->fadeIn();

	_events->updateGameCounter();
	_events->wait(60);

	for (int idx = 0; idx < 50 && !shouldQuit(); ++idx) {
		if (idx == 9 || idx == 22 || idx == 33 || idx == 44)
			_sound->playSound("whoosh.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc02.draw(0, idx);
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[1]);
		w0.update();

		_events->wait(2);
	}

	for (int idx = 0; idx < 40 && !shouldQuit(); ++idx) {
		_screen->horizMerge();
		_events->updateGameCounter();
		tower1.draw(0, 0, Common::Point(0, 0), SPRFLAG_800);
		sc3a.draw(0, idx, Common::Point(91, 86), SPRFLAG_800);

		w0.update();
		_events->wait(2);
	}

	int frame = 40, frame2 = 0;
	for (int idx = 0, xp = 0; idx < SCREEN_WIDTH && !shouldQuit(); ++idx) {
		_events->updateGameCounter();
		_screen->horizMerge(xp);
		tower1.draw(0, 0, Common::Point(idx, 0), SPRFLAG_800);
		sc3a.draw(0, frame, Common::Point(idx + 91, 86), SPRFLAG_800);
		tower2.draw(0, 0, Common::Point(idx - 320, 0), SPRFLAG_800);
		tower2.draw(0, 1, Common::Point(idx - (SCREEN_WIDTH / 2), 0), SPRFLAG_800);
		sc3b[frame2 / 30].draw(0, frame2 % 30, Common::Point(idx - 277, 65), SPRFLAG_800);

		frame = (frame + 1) % 57;
		if (idx > (SCREEN_WIDTH / 2))
			frame2 = (frame2 + 1) % 60;
		xp -= 2;
		if (xp < 1)
			xp = SCREEN_WIDTH;

		if (idx > 120)
			setWorldEndingSubtitle(Res.WORLD_END_TEXT[2]);
		w0.update();
		_events->wait(2);
	}

	for (; frame2 < 60 && !shouldQuit(); ++frame2) {
		_events->updateGameCounter();
		_screen->horizMerge(frame);
		tower2.draw(0, 0, Common::Point(0, 0), SPRFLAG_800);
		tower2.draw(0, 1, Common::Point(SCREEN_WIDTH / 2, 0), SPRFLAG_800);
		sc3b[frame2 / 30].draw(frame2 % 30, 0, Common::Point(43, 65), SPRFLAG_800);

		setWorldEndingSubtitle(Res.WORLD_END_TEXT[2]);
		w0.update();
		_events->wait(2);
	}

	if (shouldQuit())
		return;
	_screen->freePages();
	_screen->fadeOut();
	_screen->loadBackground("foura.raw");
	savedBg.blitFrom(*_screen);

	sc06.draw(0, 0, Common::Point(26, 25));
	w0.update();
	_screen->fadeIn();

	for (int idx = 0; idx < 26 && !shouldQuit(); ++idx) {
		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc06.draw(0, idx, Common::Point(26, 75));
		_events->wait(2);
	}

	if (shouldQuit())
		return;
	_screen->fadeOut();
	_screen->loadBackground("eg140001.raw");
	savedBg.blitFrom(*_screen);
	w0.update();
	setWorldEndingSubtitle(Res.WORLD_END_TEXT[3]);
	_screen->fadeIn();

	for (int idx1 = 0; idx1 < 2 && !shouldQuit(); ++idx1) {
		for (int idx2 = 0; idx2 < 15 && !shouldQuit(); ++idx2) {
			_events->updateGameCounter();
			_screen->blitFrom(savedBg);
			sc14.draw(0, idx2, Common::Point(141, 63));
			setWorldEndingSubtitle(Res.WORLD_END_TEXT[3]);

			w0.update();
			_events->wait(2);
		}
	}

	_screen->loadBackground("eg100001.raw");
	_screen->loadPage(0);
	savedBg.blitFrom(*_screen);

	for (int idx1 = 0; idx1 < 2 && !shouldQuit(); ++idx1) {
		for (int idx2 = 0; idx2 < 6 && !shouldQuit(); ++idx2) {
			_events->updateGameCounter();
			_screen->blitFrom(savedBg);
			sc14.draw(0, idx2, Common::Point(26, 21));
			setWorldEndingSubtitle(Res.WORLD_END_TEXT[4]);

			w0.update();
			_events->wait(2);
		}
	}

	_screen->blitFrom(savedBg);
	sc13.draw(0, 5, Common::Point(26, 21));
	savedBg.blitFrom(*_screen);
	setWorldEndingSubtitle(Res.WORLD_END_TEXT[4]);
	w0.update();

	frame = 0;
	for (int idx = 185; idx > 68 && !shouldQuit(); idx -= 3) {
		if (!_sound->isPlaying())
			_sound->playSound("cast.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc17.draw(0, 0, Common::Point(33, idx), SPRFLAG_4000);
		sc17.draw(0, frame, Common::Point(33, idx), SPRFLAG_4000);
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[4]);

		w0.update();
		_events->wait(2);
		frame = (frame + 1) % 17;
	}

	for (int idx = 0; idx < 17 && !shouldQuit(); ++idx) {
		if (!_sound->isPlaying())
			_sound->playSound("cast.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc17.draw(0, 0, Common::Point(33, 68), SPRFLAG_4000);
		sc17.draw(0, idx, Common::Point(33, 68), SPRFLAG_4000);
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[4]);

		w0.update();
		_events->wait(2);
	}

	if (shouldQuit())
		return;
	_screen->blitFrom(savedBg);
	sc17.draw(0, 0, Common::Point(33, 68), SPRFLAG_4000);
	setWorldEndingSubtitle(Res.WORLD_END_TEXT[4]);
	w0.update();
	_screen->fadeOut();

	_screen->loadBackground("eg140001.raw");
	savedBg.blitFrom(*_screen);
	w0.update();
	_screen->fadeIn();

	for (int idx1 = 0; idx1 < 2 && !shouldQuit(); ++idx1) {
		for (int idx2 = 0; idx2 < 15 && !shouldQuit(); ++idx2) {
			_events->updateGameCounter();
			_screen->blitFrom(savedBg);
			sc14.draw(0, idx2, Common::Point(141, 63));

			w0.update();
			_events->wait(3);
		}
	}

	_screen->horizMerge(0);
	savedBg.blitFrom(*_screen);

	for (int idx1 = 0; idx1 < 3 && !shouldQuit(); ++idx1) {
		for (int idx2 = 0; idx2 < 15 && !shouldQuit(); ++idx2) {
			_events->updateGameCounter();
			_screen->blitFrom(savedBg);
			sc10.draw(0, idx2, Common::Point(26, 21));
			setWorldEndingSubtitle(Res.WORLD_END_TEXT[5]);

			w0.update();
			_events->wait(3);
		}
	}

	_screen->blitFrom(savedBg);
	setWorldEndingSubtitle(Res.WORLD_END_TEXT[5]);
	w0.update();

	for (int idx = 185; idx > 13 && !shouldQuit(); idx -= 6) {
		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		staff.draw(0, 0, Common::Point(196, idx), SPRFLAG_4000);
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[5]);

		w0.update();
		_events->wait(2);
	}

	_events->updateGameCounter();
	_events->wait(30);
	if (shouldQuit())
		return;

	_screen->fadeOut();
	_screen->loadBackground("eg140001.raw");
	savedBg.blitFrom(*_screen);
	w0.update();
	_screen->fadeIn();

	for (int idx1 = 0; idx1 < 2 && !shouldQuit(); ++idx1) {
		for (int idx2 = 0; idx2 < 15 && !shouldQuit(); ++idx2) {
			_events->updateGameCounter();
			_screen->blitFrom(savedBg);
			sc14.draw(0, idx2, Common::Point(141, 63));

			w0.update();
			_events->wait(3);
		}
	}

	if (shouldQuit())
		return;
	_screen->loadBackground("tablmain.raw");
	savedBg.blitFrom(*_screen);
	_screen->loadPage(1);
	sc20[0].draw(0, 0, Common::Point(26, 55));
	hands.draw(0, 0, Common::Point(58, 17));
	cube.draw(0, 0, Common::Point(101, 11), SPRFLAG_4000);
	w0.update();
	_sound->playSound("cast.voc");

	for (int idx = 0; idx < 5 && !shouldQuit(); ++idx) {
		if (!_sound->isPlaying())
			_sound->playSound("cast.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc20[0].draw(0, 0, Common::Point(26, 55));
		hands.draw(0, 0, Common::Point(58, 17));
		cube.draw(0, 0, Common::Point(101, 11), SPRFLAG_4000);
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		_events->wait(2);
	}

	frame = 0;
	for (int idx = 0; idx < 16 && !shouldQuit(); ++idx) {
		if (!_sound->isPlaying())
			_sound->playSound("cast.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc20[0].draw(0, 0, Common::Point(26, 55));
		hands.draw(0, 0, Common::Point(58, 17));
		cube.draw(0, frame, Common::Point(101, 11), SPRFLAG_4000);
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		_events->wait(2);
		frame = (frame + 1) % 5;
	}

	frame = 0;
	for (int idx = 11; idx < 82 && !shouldQuit(); ++idx) {
		if (!_sound->isPlaying())
			_sound->playSound("cast.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc20[0].draw(0, 0, Common::Point(26, 55));
		cube.draw(0, frame, Common::Point(101, idx), SPRFLAG_4000);
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		_events->wait(2);
		frame = (frame + 1) % 5;
	}

	_sound->stopSound();
	if (shouldQuit())
		return;

	_sound->playSound("click.voc");
	_sound->playSound("padspell.voc");

	for (int idx = 0; idx < 14 && !shouldQuit(); ++idx) {
		if (idx == 10)
			_sound->playSound("padspell.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc20[idx / 7].draw(0, idx % 7, Common::Point(26, 55));
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		_events->wait(2);
	}

	for (int idx = 0; idx < 17 && !shouldQuit(); ++idx) {
		if (idx == 10)
			_sound->playSound("padspell.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc20[2].draw(0, idx, Common::Point(26, 55));
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		_events->wait(3);
	}

	for (int idx = 0; idx < 6 && !shouldQuit(); ++idx) {
		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc20[3].draw(0, idx, Common::Point(26, 55));
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		_events->wait(4);
	}

	if (shouldQuit())
		return;
	_screen->blitFrom(savedBg);
	setWorldEndingSubtitle(Res.WORLD_END_TEXT[6]);
	w0.update();
	_events->updateGameCounter();
	_events->wait(30);

	_screen->horizMerge(SCREEN_WIDTH);
	savedBg.blitFrom(*_screen);
	_screen->freePages();

	for (int idx = 0; idx < 35 && !shouldQuit(); ++idx) {
		if (idx == 3)
			_sound->playSound("click.voc");
		else if (idx == 5 || idx == 15)
			_sound->playSound("elect.voc");
		else if (idx == 22)
			_sound->playSound("explosio.voc");

		_events->updateGameCounter();
		_screen->blitFrom(savedBg);
		sc22[idx / 20].draw(0, idx % 20, Common::Point(112, 17));
		setWorldEndingSubtitle(Res.WORLD_END_TEXT[7]);

		w0.update();
		_events->wait(3);
	}

	if (shouldQuit())
		return;
	SpriteResource sc23[8] = {
		SpriteResource("sc23a.eg2"), SpriteResource("sc23b.eg2"),
		SpriteResource("sc23c.eg2"), SpriteResource("sc23d.eg2"),
		SpriteResource("sc23e.eg2"), SpriteResource("sc23f.eg2"),
		SpriteResource("sc23g.eg2"), SpriteResource("sc23h.eg2")
	};
	SpriteResource sc24("sc24.eg2");

	_screen->fadeOut();
	_screen->loadBackground("eg23prt2.raw");
	savedBg.blitFrom(*_screen);
	sc23[0].draw(0, 0);
	w0.update();
	_screen->fadeIn();

	for (int idx = 0; idx < 61 && !shouldQuit(); ++idx) {
		if (idx == 2 || idx == 15 || idx == 25 || idx == 33 || idx == 41)
			_sound->playSound("gascompr.voc");

		_events->updateGameCounter();
		sc23[idx / 8].draw(0, frame % 8);
		w0.update();
		_events->wait(4);
	}

	// TODO
}

void WorldOfXeenEngine::setWorldEndingSubtitle(const Common::String &msg) {
	Windows &windows = *_windows;
	const char *const FORMAT1 = "\xB""000\t000\xC""38\x3""c%s";
	const char *const FORMAT2 = "\xB""000\t000\xC""39\x3""c%s";
	windows[28].writeString(Common::String::format(FORMAT1, msg.c_str()));
	windows[28].writeString(Common::String::format(FORMAT2, msg.c_str()));
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
