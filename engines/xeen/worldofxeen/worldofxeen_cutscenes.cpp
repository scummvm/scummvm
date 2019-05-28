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

#include "xeen/worldofxeen/worldofxeen_cutscenes.h"
#include "xeen/sound.h"
#include "xeen/xeen.h"

#define WAIT(TIME) if (_subtitles.wait(TIME)) return false

namespace Xeen {
namespace WorldOfXeen {

void WorldOfXeenCutscenes::showWorldOfXeenEnding(GooberState state, uint score) {
	FileManager &files = *_vm->_files;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	files.setGameCc(2);

	_goober = state;
	_finalScore = score;
	screen.loadPalette("skymain.pal");

	if (worldEnding1())
		if (worldEnding2())
				worldEnding3();

	sound.stopAllAudio();
	screen.fadeOut();
	screen.freePages();
	files.setGameCc(1);
}

bool WorldOfXeenCutscenes::worldEnding1() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w0 = windows[0];
	Graphics::ManagedSurface savedBg(SCREEN_WIDTH, SCREEN_HEIGHT);

	SpriteResource::setClippedBottom(185);
	sound.playSong("outday3.m");
	if (!showPharaohEndText(Res.WORLD_END_TEXT[0]))
		return false;

	screen.loadBackground("skymain.raw");
	savedBg.blitFrom(screen);
	screen.loadBackground("twrsky1.raw");
	screen.loadPage(0);
	screen.loadPage(1);

	SpriteResource sc02("sc02.eg2"), tower1("tower1.eg2"), tower2("tower2.eg2"),
		sc3a("sc3a.eg2"), sc06("sc06.eg2"), sc14("sc14.eg2"), sc13("sc13.eg2"),
		sc17("sc17.eg2"), cube("cube.eg2"), hands("hands.eg2"),
		sc10("sc10.eg2"), staff("staff.eg2");
	SpriteResource sc3b[2] = {
		SpriteResource("sc3b1.eg2"), SpriteResource("sc3b2.eg2")
	};
	SpriteResource sc20[4] = {
		SpriteResource("sc20a.eg2"), SpriteResource("sc20b.eg2"),
		SpriteResource("sc20c.eg2"), SpriteResource("sc20d.eg2")
	};
	SpriteResource sc22[2] = {
		SpriteResource("sc22a.eg2"), SpriteResource("sc22b.eg2")
	};

	// Fade out the screen and the music
	sound.songCommand(223);
	windows[41].writeString("\x1\xD");

	screen.fadeOut();
	while (!_vm->shouldExit() && sound.isSoundPlaying())
		events.pollEventsAndWait();

	// And so the call went out to the people through the lands of Xeen that the
	// prophecy was nearing completion.
	sound.playSong("nwblksmt.m");
	screen.blitFrom(savedBg);
	setSubtitle(Res.WORLD_END_TEXT[1]);
	w0.update();
	screen.fadeIn();

	WAIT(60);

	// Dragon approaching tower in distance
	for (int idx = 0; idx < 50; ++idx) {
		if (idx == 9 || idx == 22 || idx == 33 || idx == 44)
			sound.playSound("whoosh.voc");

		screen.blitFrom(savedBg);
		sc02.draw(0, idx);
		setSubtitle(Res.WORLD_END_TEXT[1]);
		w0.update();

		WAIT(2);
	}

	// Dragon landing on tower
	for (int idx = 0; idx < 40; ++idx) {
		screen.horizMerge();
		tower1.draw(0, 0, Common::Point(0, 0), SPRFLAG_800);
		sc3a.draw(0, idx, Common::Point(91, 86), SPRFLAG_800);

		w0.update();
		WAIT(2);
	}

	// They came in great numbers to witness the momentous occasion
	int frame = 40, frame2 = 0;
	for (int idx = 0, xp = 0; idx < SCREEN_WIDTH; idx += 5) {
		screen.horizMerge(xp);
		tower1.draw(0, 0, Common::Point(idx, 0), SPRFLAG_800);
		sc3a.draw(0, frame, Common::Point(idx + 91, 86), SPRFLAG_800);
		tower2.draw(0, 0, Common::Point(idx - SCREEN_WIDTH, 0), SPRFLAG_800);
		tower2.draw(0, 1, Common::Point(idx - (SCREEN_WIDTH / 2), 0), SPRFLAG_800);
		sc3b[frame2 / 30].draw(0, frame2 % 30, Common::Point(idx - 277, 65), SPRFLAG_800);

		frame = (frame + 1) % 57;
		if (idx > (SCREEN_WIDTH / 2))
			frame2 = (frame2 + 1) % 60;
		xp -= 2;
		if (xp < 1)
			xp = SCREEN_WIDTH;

		if (idx > 120)
			setSubtitle(Res.WORLD_END_TEXT[2]);
		w0.update();
		WAIT(2);
	}

	for (; frame2 < 60; ++frame2) {
		screen.horizMerge(frame);
		tower2.draw(0, 0, Common::Point(0, 0), SPRFLAG_800);
		tower2.draw(0, 1, Common::Point(SCREEN_WIDTH / 2, 0), SPRFLAG_800);
		sc3b[frame2 / 30].draw(0, frame2 % 30, Common::Point(43, 65), SPRFLAG_800);

		setSubtitle(Res.WORLD_END_TEXT[2]);
		w0.update();
		WAIT(2);
	}

	screen.freePages();
	screen.fadeOut();
	screen.loadBackground("foura.raw");
	savedBg.blitFrom(screen);

	// Crowd sitting down in chamber
	sc06.draw(0, 0, Common::Point(26, 75));
	w0.update();
	screen.fadeIn();

	for (int idx = 0; idx < 26; ++idx) {
		screen.blitFrom(savedBg);
		sc06.draw(0, idx, Common::Point(26, 75));
		WAIT(2);
	}

	// The Dragon Pharoah presided over the ceremony
	screen.fadeOut();
	screen.loadBackground("eg140001.raw");
	savedBg.blitFrom(screen);
	w0.update();
	setSubtitle(Res.WORLD_END_TEXT[3]);
	screen.fadeIn();

	for (int idx1 = 0; idx1 < 2; ++idx1) {
		for (int idx2 = 0; idx2 < 15; ++idx2) {
			screen.blitFrom(savedBg);
			sc14.draw(0, idx2, Common::Point(141, 63));
			setSubtitle(Res.WORLD_END_TEXT[3]);

			w0.update();
			WAIT(2);
		}
	}

	// Queen Kalindra presented the Cube of Power
	screen.loadBackground("eg100001.raw");
	screen.loadPage(0);
	savedBg.blitFrom(screen);

	for (int idx1 = 0; idx1 < 2; ++idx1) {
		for (int idx2 = 0; idx2 < 6; ++idx2) {
			screen.blitFrom(savedBg);
			sc13.draw(0, idx2, Common::Point(26, 21));
			setSubtitle(Res.WORLD_END_TEXT[4]);

			w0.update();
			WAIT(2);
		}
	}

	// Queen Kalindra lifts up the Cube of Power
	screen.blitFrom(savedBg);
	sc13.draw(0, 5, Common::Point(26, 21));
	savedBg.blitFrom(screen);
	setSubtitle(Res.WORLD_END_TEXT[4]);
	w0.update();

	frame = 0;
	for (int idx = 185; idx > 68; idx -= 3) {
		if (!sound.isSoundPlaying())
			sound.playSound("cast.voc");

		screen.blitFrom(savedBg);
		sc17.draw(41, 0, Common::Point(33, idx), SPRFLAG_BOTTOM_CLIPPED);
		sc17.draw(41, frame, Common::Point(33, idx), SPRFLAG_BOTTOM_CLIPPED);
		setSubtitle(Res.WORLD_END_TEXT[4]);

		w0.update();
		WAIT(2);

		// WORKAROUND: sc17.eg2 has 17 frames, but the last is malformed. So skip it
		frame = (frame + 1) % 16;
	}

	for (int idx = 0; idx < 16; ++idx) {
		if (!sound.isSoundPlaying())
			sound.playSound("cast.voc");

		screen.blitFrom(savedBg);
		sc17.draw(41, 0, Common::Point(33, 68), SPRFLAG_BOTTOM_CLIPPED);
		sc17.draw(41, idx, Common::Point(33, 68), SPRFLAG_BOTTOM_CLIPPED);
		setSubtitle(Res.WORLD_END_TEXT[4]);

		w0.update();
		WAIT(2);
	}

	screen.blitFrom(savedBg);
	sc17.draw(0, 0, Common::Point(33, 68), SPRFLAG_BOTTOM_CLIPPED);
	setSubtitle(Res.WORLD_END_TEXT[4]);
	w0.update();
	screen.fadeOut();

	screen.loadBackground("eg140001.raw");
	savedBg.blitFrom(screen);
	w0.update();
	screen.fadeIn();

	for (int idx1 = 0; idx1 < 2; ++idx1) {
		for (int idx2 = 0; idx2 < 15; ++idx2) {
			screen.blitFrom(savedBg);
			sc14.draw(0, idx2, Common::Point(141, 63));

			w0.update();
			WAIT(3);
		}
	}

	// Prince Roland presented the Xeen Sceptre
	screen.horizMerge(0);
	savedBg.blitFrom(screen);

	for (int idx1 = 0; idx1 < 3; ++idx1) {
		for (int idx2 = 0; idx2 < 5; ++idx2) {
			screen.blitFrom(savedBg);
			sc10.draw(0, idx2, Common::Point(26, 21));
			setSubtitle(Res.WORLD_END_TEXT[5]);

			w0.update();
			WAIT(3);
		}
	}

	screen.blitFrom(savedBg);
	setSubtitle(Res.WORLD_END_TEXT[5]);
	w0.update();
	SpriteResource::setClippedBottom(185);

	for (int yp = 185; yp > 13; yp -= 6) {
		screen.blitFrom(savedBg);
		staff.draw(41, 0, Common::Point(196, yp), SPRFLAG_BOTTOM_CLIPPED);
		setSubtitle(Res.WORLD_END_TEXT[5]);

		w0.update();
		WAIT(2);
	}

	WAIT(30);
	screen.fadeOut();

	screen.loadBackground("eg140001.raw");
	savedBg.blitFrom(screen);
	w0.update();
	screen.fadeIn();

	for (int idx1 = 0; idx1 < 2; ++idx1) {
		for (int idx2 = 0; idx2 < 15; ++idx2) {
			screen.blitFrom(savedBg);
			sc14.draw(0, idx2, Common::Point(141, 63));

			w0.update();
			WAIT(3);
		}
	}

	// Together, they placed the Cube of Power...
	SpriteResource::setClippedBottom(118);
	screen.loadBackground("tablmain.raw");
	savedBg.blitFrom(screen);
	screen.loadPage(1);
	sc20[0].draw(0, 0, Common::Point(26, 55));
	hands.draw(0, 0, Common::Point(58, 17));
	cube.draw(0, 0, Common::Point(101, 11), SPRFLAG_BOTTOM_CLIPPED);
	w0.update();
	sound.playSound("cast.voc");

	for (int idx = 0; idx < 5; ++idx) {
		if (!sound.isSoundPlaying())
			sound.playSound("cast.voc");

		screen.blitFrom(savedBg);
		sc20[0].draw(0, 0, Common::Point(26, 55));
		hands.draw(0, 0, Common::Point(58, 17));
		cube.draw(0, idx, Common::Point(101, 11), SPRFLAG_BOTTOM_CLIPPED);
		setSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		WAIT(2);
	}

	frame = 0;
	for (int idx = 0; idx < 16; ++idx) {
		if (!sound.isSoundPlaying())
			sound.playSound("cast.voc");

		screen.blitFrom(savedBg);
		sc20[0].draw(0, 0, Common::Point(26, 55));
		hands.draw(0, idx, Common::Point(58, 17));
		cube.draw(0, frame, Common::Point(101, 11), SPRFLAG_BOTTOM_CLIPPED);
		setSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		WAIT(2);
		frame = (frame + 1) % 5;
	}

	// Cube lowers into table
	frame = 0;
	for (int yp = 11; yp < 82; ++yp) {
		if (!sound.isSoundPlaying())
			sound.playSound("cast.voc");

		screen.blitFrom(savedBg);
		sc20[0].draw(0, 0, Common::Point(26, 55));
		cube.draw(0, frame, Common::Point(101, yp), SPRFLAG_BOTTOM_CLIPPED);
		setSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		WAIT(2);
		frame = (frame + 1) % 5;
	}

	sound.stopSound();

	// Table is activating
	sound.playSound("click.voc");
	sound.playSound("padspell.voc");

	for (int idx = 0; idx < 14; ++idx) {
		if (idx == 10)
			sound.playSound("padspell.voc");

		screen.blitFrom(savedBg);
		sc20[idx / 7].draw(0, idx % 7, Common::Point(26, 55));
		setSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		WAIT(2);
	}

	for (int idx = 0; idx < 17; ++idx) {
		if (idx == 10)
			sound.playSound("padspell.voc");

		screen.blitFrom(savedBg);
		sc20[2].draw(0, idx, Common::Point(26, 55));
		setSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		WAIT(3);
	}

	for (int idx = 0; idx < 6; ++idx) {
		screen.blitFrom(savedBg);
		sc20[3].draw(0, idx, Common::Point(26, 55));
		setSubtitle(Res.WORLD_END_TEXT[6]);

		w0.update();
		WAIT(4);
	}

	screen.blitFrom(savedBg);
	setSubtitle(Res.WORLD_END_TEXT[6]);
	w0.update();
	WAIT(30);

	// And the Scepotre, onto the Altar of Joining
	screen.horizMerge(SCREEN_WIDTH);
	savedBg.blitFrom(screen);
	screen.freePages();

	for (int idx = 0; idx < 35; ++idx) {
		if (idx == 3)
			sound.playSound("click.voc");
		else if (idx == 5 || idx == 15)
			sound.playSound("elect.voc", 1, 0);
		else if (idx == 22)
			sound.playSound("explosio.voc");

		screen.blitFrom(savedBg);
		sc22[idx / 20].draw(0, idx % 20, Common::Point(112, 17));
		setSubtitle(Res.WORLD_END_TEXT[7]);

		w0.update();
		WAIT(3);
	}

	screen.fadeOut();
	return true;
}

bool WorldOfXeenCutscenes::worldEnding2() {
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w0 = windows[0];
	Graphics::ManagedSurface savedBg(SCREEN_WIDTH, SCREEN_HEIGHT);

	SpriteResource sc23[8] = {
		SpriteResource("sc23a.eg2"), SpriteResource("sc23b.eg2"),
		SpriteResource("sc23c.eg2"), SpriteResource("sc23d.eg2"),
		SpriteResource("sc23e.eg2"), SpriteResource("sc23f.eg2"),
		SpriteResource("sc23g.eg2"), SpriteResource("sc23h.eg2")
	};
	SpriteResource sc24("sc24.eg2");

	screen.loadBackground("eg23prt2.raw");
	savedBg.blitFrom(screen);
	sc23[0].draw(0, 0);
	w0.update();
	screen.fadeIn();

	for (int idx = 0; idx < 61; ++idx) {
		if (idx == 2 || idx == 15 || idx == 25 || idx == 33 || idx == 41)
			sound.playSound("gascompr.voc");

		sc23[idx / 8].draw(0, idx % 8);
		w0.update();
		WAIT(4);
	}

	// Far out view of light emerging from tower
	screen.loadBackground("skymain.raw");
	savedBg.blitFrom(screen);
	sc24.draw(0, 0, Common::Point(103, 6));

	for (int idx = 0; idx < 35; ++idx) {
		if (idx == 2 || idx == 15 || idx == 21)
			sound.playSound("photon.voc", 1, 0);

		screen.blitFrom(savedBg);
		sc24.draw(0, idx, Common::Point(103, 6));

		WAIT(3);
	}

	for (int idx = 20; idx < 35; ++idx) {
		screen.blitFrom(savedBg);
		sc24.draw(0, idx, Common::Point(103, 6));

		WAIT(3);
	}

	screen.fadeOut();

	// Green power lines spreading to four corners of the top half of Xeen
	SpriteResource sc25("sc25.eg2"), sc262("sc262.eg2"), sc263("sc263.eg2"),
		sc264("sc264.eg2");
	SpriteResource sc261[2] = {
		SpriteResource("sc261a.eg2"), SpriteResource("sc261b.eg2")
	};

	screen.loadBackground("eg250001.raw");
	screen.loadPalette("eg250001.pal");
	w0.update();
	screen.fadeIn();
	sound.playSound("comet.voc");

	for (int idx = 0; idx < 52; ++idx) {
		if (idx == 28)
			sound.playSound("click.voc");
		if (!sound.isSoundPlaying())
			sound.playSound("comet.voc");

		sc25.draw(0, idx);
		w0.update();
		WAIT(2);
	}
	sound.stopSound();

	// Closeup view of the four collectors receiving the power beams
	screen.loadBackground("blank.raw");
	screen.loadPalette("skymain.pal");
	sc261[0].draw(0, 0, Common::Point(7, 4));
	sc262.draw(0, 0, Common::Point(86, 4));
	sc263.draw(0, 0, Common::Point(164, 4));
	sc264.draw(0, 0, Common::Point(242, 4));
	w0.update();
	screen.fadeIn(0x81);

	int frame1 = 0, frame2 = 0, frame3 = 0, frame4 = 0;
	for (int idx = 0; idx < 78; ++idx) {
		sc261[frame4 / 14].draw(0, frame4 % 14, Common::Point(7, 4));
		sc262.draw(0, frame1, Common::Point(86, 4));
		sc263.draw(0, frame2, Common::Point(164, 4));
		sc264.draw(0, frame3, Common::Point(242, 4));

		if (idx == 10 || idx == 28 || idx == 43 || idx == 56)
			sound.playSound("photon.voc", 1, 0);

		frame4 = (frame4 + 1) % 28;
		frame1 = (frame1 + 1) % 9;
		frame2 = (frame2 + 1) % 19;
		frame3 = (frame3 + 1) % 10;

		if (idx > 12 && frame4 < 13)
			frame4 = 13;
		if (idx < 23)
			frame1 = 0;
		else if (idx > 26 && frame1 < 5)
			frame1 = 5;

		if (idx < 34)
			frame2 = 0;
		else if (idx > 43 && frame2 < 12)
			frame2 = 12;

		if (idx < 53)
			frame3 = 0;
		else if (idx > 57 && frame3 < 5)
			frame3 = 5;

		w0.update();
		WAIT(2);
	}

	screen.fadeOut();

	// Four beams arc across bottom of Xeen
	SpriteResource sc27("sc27.eg2");
	screen.loadBackground("eg270001.raw");
	screen.loadPalette("eg250001.pal");
	screen.fadeIn();

	for (int idx = 0; idx < 89; ++idx) {
		if (!sound.isSoundPlaying())
			sound.playSound("comet.voc");
		if (idx == 19 || idx == 60)
			sound.playSound("click.voc");

		sc27.draw(0, idx);
		w0.update();
		WAIT(2);
	}

	sound.stopSound();
	screen.fadeOut();

	return true;
}

bool WorldOfXeenCutscenes::worldEnding3() {
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w0 = windows[0];
	Graphics::ManagedSurface savedBg(SCREEN_WIDTH, SCREEN_HEIGHT);

	SpriteResource sc30("sc30.eg2");
	SpriteResource sc28[14] = {
		SpriteResource("sc28a.eg2", 2), SpriteResource("sc28b1.eg2", 2),
		SpriteResource("sc28c.eg2", 2), SpriteResource("sc28d.eg2", 2),
		SpriteResource("sc28e.eg2", 2), SpriteResource("sc28f.eg2", 2),
		SpriteResource("sc28g.eg2", 2), SpriteResource("sc28h.eg2", 2),
		SpriteResource("sc28i.eg2", 2), SpriteResource("sc28j.eg2", 2),
		SpriteResource("sc28k.eg2", 2), SpriteResource("sc28l.eg2", 2),
		SpriteResource("sc28m.eg2", 2), SpriteResource("sc28n.eg2", 2),
	};

	// Transformation of Xeen into a globe
	screen.loadBackground("eg280001.raw");
	screen.loadPalette("eg250001.pal");
	savedBg.blitFrom(screen);
	w0.update();
	screen.fadeIn();

	for (int idx = 0; idx < 138; ++idx) {
		if (!sound.isSoundPlaying() && idx > 98)
			sound.playSound("rumble.voc");

		sc28[idx / 10].draw(0, idx % 10, Common::Point(52, 15));
		w0.update();
		WAIT(2);
	}

	sound.stopSound();

	// White screen with explosion sound
	screen.loadPalette("white.pal");
	screen.fadeIn();
	sound.playSound("explosio.voc");
	WAIT(10);

	screen.loadPalette("eg250001.pal");
	screen.fadeOut();

	// With the prophecy complete, the two sides of Xeen were united as one
	for (int idx1 = 0; idx1 < 20; ++idx1) {
		for (int idx2 = 0; idx2 < 4; ++idx2) {
			sc30.draw(0, idx2);
			setSubtitle2(Res.WORLD_END_TEXT[8]);
			w0.update();

			if (!idx1 && !idx2)
				screen.fadeIn();
			if (idx1 == 17)
				sound.songCommand(207);

			WAIT(2);
		}
	}

	screen.fadeOut();
	while (sound.isSoundPlaying()) {
		WAIT(2);
	}

	sound.playSong("outday3.m");

	Common::String gooberStr = Res.GOOBER[_goober];
	Common::String congratsStr1 = Common::String::format(Res.WORLD_CONGRATULATIONS, _finalScore);
	showPharaohEndText(congratsStr1.c_str(),
		_goober == NON_GOOBER ? nullptr :
		Common::String::format(Res.WORLD_CONGRATULATIONS2, gooberStr.c_str()).c_str()
	);

	return true;
}

void WorldOfXeenCutscenes::setSubtitle(const Common::String &msg) {
	Windows &windows = *_vm->_windows;
	Window &w = windows[28];

	const char *const FORMAT1 = "\v000\t000\f38\x3""c%s";
	w.setBounds(Common::Rect(2, 157, SCREEN_WIDTH, 157 + 61));
	w.writeString(Common::String::format(FORMAT1, msg.c_str()));

	const char *const FORMAT2 = "\v000\t000\f39\x3""c%s";
	w.setBounds(Common::Rect(1, 156, SCREEN_WIDTH - 1, 156 + 60));
	w.writeString(Common::String::format(FORMAT2, msg.c_str()));
}

void WorldOfXeenCutscenes::setSubtitle2(const Common::String &msg) {
	Windows &windows = *_vm->_windows;
	Window &w = windows[28];

	const char *const FORMAT1 = "\v000\t000\f05\x3""c%s";
	w.setBounds(Common::Rect(2, 157, SCREEN_WIDTH, SCREEN_HEIGHT - 2));
	w.writeString(Common::String::format(FORMAT1, msg.c_str()));

	const char *const FORMAT2 = "\v000\t000\f11\x3""c%s";
	w.setBounds(Common::Rect(1, 156, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 4));
	w.writeString(Common::String::format(FORMAT2, msg.c_str()));
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
