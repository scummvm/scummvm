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

#include "common/debug.h"

#include "common/config-manager.h"

#include "dune/dune.h"
#include "dune/intro.h"
#include "dune/music.h"

namespace Dune {

/*
 * The scenes of the original intro, in the order it plays them.  Each scene
 * names the routine that loads it, the routine that plays it, the position in
 * the intro music it waits for, and the fade it comes in on.  The opening, scenes and
 * narration below follow that script; individual renderers implement each
 * loader and player.
 *
 *   {      0, load_virgin_hnm,          0, 0x3a, play_virgin_hnm,      1 },
 *   {      0, clear_screen,             0, 0x3a, empty,                1 },
 *   {      0, load_cryo_hnm,            0, 0x30, play_cryo_hnm,        1 },
 *   {      0, load_cryo2_hnm,       0x006f, 0x30, play_cryo_hnm,       1 },
 *   {      0, empty,                0x00a8,   -1, empty,               1 },
 *   {      0, load_PRESENT_HNM,         0, 0x3a, play_PRESENT_HNM,     1 },
 *   {      0, load_IRULn_HSQ,           0, 0x3a, play_IRULx_HSQ,      1 },
 *   {      0, clear_screen,             0, 0x3a, empty,               1 },
 *   {      0, load_INTRO_HNM,           0, 0x36, empty,          0x0190 },
 *   ... thirty more scenes of the desert and the Atreides leaving Caladan
 */

bool Intro::run() {
	// The script at 0x0337, in its order and at its
	// times.  Each step names what to load and what to play; a step names a
	// movie by a number the player at 0xCA1B counts from two hundred and ten,
	// so the twenty-one of the first step is the resource at two hundred and
	// thirty-one, VIRGIN.HNM, and the step after Irulan is TITLE.HNM.
	//
	// The third word of a step is when it happens, counted in the intro's own
	// ticks, so how long a step stands is how far it is to the next one.  The
	// steps that are not movies either show a picture, which 0xC2F2 does by
	// loading the resource and drawing it whole at the corner, or go to a
	// place with 0x08F0, saying first who is standing there and afterwards who
	// is speaking.
	enum Kind { kNothing, kMovie, kScene, kPicture, kBuilt, kSlide, kStaged, kNarration, kFeyd, kClear };

	static const struct {
		Kind kind;
		uint16 at;          // when the step happens, in the script's ticks
		uint16 what;        // kMovie: the movie; kPicture/kBuilt: the resource
		uint16 place;       // kScene: where
		byte   arrival;     // kScene: the kind of arrival
		uint16 present;     // kScene: who is standing there; kBuilt: the list
		int    speaker;     // kScene: who speaks, or none
		uint16 line;        // kScene: the line said, or none
		uint16 wipe;        // the fourth word: how the step is brought in
		const char *song;   // the song this step starts, if it starts one
		int overX, overY;   // kBuilt: position of the optional extra sprite
		int portraitX;     // 09C7: requested left of a named intro portrait
	} steps[] = {
		{ kMovie,      0, HNM_VIRGIN,  0,  0, 0, -1, 0xffff, 0x003a, "MORNING", 19, 74, 0 },
		{ kClear,      0, 0,           0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		{ kMovie,      0, HNM_CRYO,    0,  0, 0, -1, 0xffff, 0x0030, "CRYOMUS", 19, 74, 0 },
		{ kMovie,    111, HNM_CRYO2,   0,  0, 0, -1, 0xffff, 0x0030, nullptr, 19, 74, 0 },
		{ kNothing,  168, 0,           0,  0, 0, -1, 0xffff, 0xffff, nullptr, 19, 74, 0 },
		{ kMovie,      0, HNM_PRESENT, 0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		{ kMovie,      0, HNM_IRULAN,  0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		{ kClear,      0, 0,           0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		// The title is run in three steps, as 0x0397, 0x03A3 and 0x03AF have
		// it: its first frame loaded, then to its eighty-sixth frame, then out
		// to the end.  The place field carries how far to play - nought for
		// its initial frame, 0xFFFF for all that is left - and between the three the
		// title stands on the screen.
		{ kStaged,     0, HNM_TITLE,      0,  0, 0, -1, 0xffff, 0x0036, nullptr, 19, 74, 0 },
		{ kStaged,   144, HNM_TITLE,     86,  0, 0, -1, 0xffff, 0x0030, nullptr, 19, 74, 0 },
		{ kStaged,   268, HNM_TITLE, 0xffff,  0, 0, -1, 0xffff, 0xffff, nullptr, 19, 74, 0 },
		// 0x07FD: the sunrise, four bands of SUNRS.HSQ down the screen
		{ kBuilt,    300, 46,          0,  0, 0x1500, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		{ kMovie,    334, HNM_MTG1,    0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		// 0x0972, 0x098A, 0x0995: back to the palace, filling and emptying
		{ kScene,    590, 0,           0,  2, 0x0000, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kScene,    600, 0,           0,  4, 0x0002, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kScene,    610, 0,           0, 10, 0x0000,  0, 0x0190, 0x0010, nullptr, 19, 74, 0 },
		// 0x0771: the same room, and the second of them speaks
		{ kScene,    719, 0,           0,  4, 0x0000,  1, 0x0190, 0x0010, nullptr, 19, 74, 0 },
		// 0x07EE: built out of BACK.HSQ
		{ kBuilt,    816, 48,          0,  0, 0x1526, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kPicture,  836, 98,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // PALAIS.HSQ
		{ kMovie,    846, HNM_MTG2,    0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		// 0x077C and 0x07C6: the sixteenth place, under two kinds
		{ kScene,   1216, 0,          15,  3, 0x0000, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kScene,   1232, 0,          15,  3, 0x0000,  7, 0x0190, 0xffff, nullptr, 19, 74, 0 },
		// 07A3: retain the built page and extra sprite, then introduce Kynes.
		{ kBuilt,   1296, 46,          1,  0, 0x1500,  6, 0x0190, 0x0010, nullptr, 84, 11, 0 },
		// 07C6 retains the roster before clearing it and introducing Stilgar.
		{ kScene,   1360, 0,          15,  2, 0x0100,  5, 0x0190, 0x0010, nullptr, 19, 74, 0 },
		// 0x0868: the sunrise again, with the seventh sprite of it on top
		{ kBuilt,   1423, 46,          0,  7, 0x1500, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		// 0886 uses a filled scene and C32F's clipped BACK sprites for Feyd.
		{ kFeyd,    1454, 48,          0,  0, 0x154e, 10, 0x0190, 0x0010, nullptr, 19, 74, 58 },
		// 09AD retains BACK's list and shifts the Baron's bank window by 82.
		{ kBuilt,   1520, 48,          0,  0, 0x153a,  9, 0x0190, 0x0030, nullptr, 19, 74, 82 },
		// 0x08B6: the fifth and sixth of BACK.HSQ sliding in from the left
		{ kSlide,   1556, 48,          0, 13, 0x0000, -1, 0xffff, 0xffff, nullptr, 19, 74, 0 },
		// 0x0ACD: ATTACK.HSQ, the list at DS:0x11DD
		{ kBuilt,   1566, 43,          0,  0, 0x11dd, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kMovie,   1612, HNM_MTG3,    0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kPicture, 1736, 88,          0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },   // INT05.HSQ
		{ kPicture, 1774, 93,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT10.HSQ
		{ kPicture, 1790, 91,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT08.HSQ
		// The remaining DOS script continues through time 2272: more
		// illustrations, the sixteenth place, PLANT and VER, then fades.
		{ kPicture, 1806, 87,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT04.HSQ
		{ kPicture, 1838, 92,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT09.HSQ
		{ kPicture, 1854, 94,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT11.HSQ
		{ kPicture, 1870, 86,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT02.HSQ
		{ kPicture, 1902, 89,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT06.HSQ
		// 0x07E0: the sixteenth place again, under the fourth kind
		{ kScene,   1918, 0,          15,  4, 0x0000, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kMovie,   1967, HNM_PLANT,   0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },
		{ kPicture, 1998, 90,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 },   // INT07.HSQ
		// 0711 opens VER; 071D calls AB15 with AL=8, a digital sound
		// effect. It does not start another song or reset the music clock.
		{ kMovie,   2012, HNM_VER,   0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		// Original script entries 42..47: the last three illustrations,
		// then a clear and the final picture faded in and out separately.
		// DFL2 is a travelling landscape, not an intro credits movie.
		{ kPicture, 2076, 97,          0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 }, // INT15.HSQ
		{ kPicture, 2110, 95,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 }, // INT13.HSQ
		{ kPicture, 2126, 96,          0,  0, 0, -1, 0xffff, 0x0010, nullptr, 19, 74, 0 }, // INT14.HSQ
		{ kClear,   2142, 0,           0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		{ kPicture, 2142, 97,          0,  0, 0, -1, 0xffff, 0x0036, nullptr, 19, 74, 0 }, // INT15.HSQ
		{ kClear,   2272, 0,           0,  0, 0, -1, 0xffff, 0x0038, nullptr, 19, 74, 0 },
		// Startup calls 0309 after the timed script (000D/0010/0013).
		// Its 09EF loader opens movie 20, CREDITS.HNM, before narration.
		{ kMovie,   2272, HNM_CREDITS, 0,  0, 0, -1, 0xffff, 0x003a, nullptr, 19, 74, 0 },
		// And the story is told: eight still screens, each with a line of the
		// command file read over it - the stars, the planet, the desert,
		// Paul, the Baron, and the empty palace the story begins in.  The
		// recording gives each of them about twelve seconds.
		{ kNarration, 2272, kNarrStars,  0, 0, 0, -1,   279, 0xffff, nullptr, 19, 74, 0 },
		{ kNarration, 2368, kNarrPlanet, 0, 0, 0, -1,   280, 0xffff, nullptr, 19, 74, 0 },
		{ kNarration, 2464, kNarrDesert, 0, 0, 0, -1,   281, 0xffff, nullptr, 19, 74, 0 },
		{ kNarration, 2560, kNarrPaul,   0, 0, 0, -1,   282, 0xffff, nullptr, 19, 74, 0 },
		{ kNarration, 2656, kNarrBaron,  0, 0, 0, -1,   283, 0xffff, nullptr, 19, 74, 0 },
		{ kNarration, 2752, kNarrPlanet, 0, 0, 0, -1,   284, 0xffff, nullptr, 19, 74, 0 },
		{ kNarration, 2848, kNarrPaul,   0, 0, 0, -1,   285, 0xffff, nullptr, 19, 74, 0 },
		{ kNarration, 2944, kNarrPalace, 0, 0, 0, -1,   286, 0xffff, nullptr, 19, 74, 0 },
		{ kNothing, 3040, 0,           0,  0, 0, -1, 0xffff, 0x0038, nullptr, 19, 74, 0 }
	};

	// The script does not hold a step for a length of time; it waits until the
	// clock reaches the step's own time, which is what 0xDE0C does - it
	// compares the wanted time against the tick counter at DS:0xDBCE and the
	// sixteenth of a tick under it, and returns at once when that moment has
	// already gone by.
	//
	// So the intro is kept on one timeline rather than a run of holds.  A step
	// that overruns its place, or one the player cuts short, leaves the steps
	// after it to catch up on their own, and nothing drifts.
	// The clock the script is written against is the sound driver's, which is
	// not something this engine has, so how long one of its units lasts is
	// measured off a recording of the game instead.  Three gaps give it: the
	// two rooms of the palace before Leto speaks are twenty units and three
	// seconds, Leto to Jessica is a hundred and nine units and fourteen and a
	// half seconds, and Jessica to the ground after her is ninety-seven units
	// and thirteen seconds, which is near enough a hundred and thirty-five
	// milliseconds each.  Set against the whole of the recording, with the
	// movies running at the rate they run at there, a hundred and twenty-five
	// lands the intro within half a second of the seven minutes and forty-two
	// it takes, so that is what is used.
	const uint32 kTickMs = 125;

	// The script waits until the clock reaches a step's own time, so its times
	// are one line measured from the beginning and the gaps between them are
	// what they say.  A step that overruns its place - and the movies do, one
	// after another - would leave every step after it late for good and the
	// palace going by in a blink, so what a step runs over by is carried
	// forward and the line is kept, gap for gap.
	uint32 startedAt = _vm->getMillis();
	uint32 carried = 0;

	// Which song is playing is not one thing for the whole intro.  A step can
	// change it: 0xAD95 is handed a number, keeps it at DS:0xDBCB and loads
	// the song, and 0xAE62 turns the number into a resource by adding a
	// hundred and sixty-four to it.  The step that plays the Virgin logo asks
	// for the sixth, which is MORNING, and the one that plays the Cryo logo
	// asks for the tenth, which is CRYOMUS - the Cryo jingle itself, and not
	// something to have playing before it.

	for (int i = 0; i != ARRAYSIZE(steps); i++) {
		// Wait for this step's own moment, if it has one and it has not gone
		// by already.  A great many of the steps carry nought, and nought is
		// not a moment: 0xDE0C returns the instant the time it is given is not
		// past the clock, so a step with nought waits for nothing and follows
		// straight on from the one before it.  The steps that do carry a time
		// are what the rest are held together by.
		if (steps[i].at != 0) {
			const uint32 due = startedAt + steps[i].at * kTickMs + carried;
			const uint32 now = _vm->getMillis();

			if (now < due) {
				if (_vm->hold(due - now) == kPlaybackAborted) {
					_vm->stopSpeech();
					_vm->_music->stop();
					return false;
				}
			} else {
				carried += now - due;
			}
		}

		// How long there is until the next step that carries a time of its own.
		// A scene stands for that long; the step it stands until finds its own
		// moment already reached and goes straight on.
		uint32 until = 2000;

		for (int ahead = i + 1; ahead != ARRAYSIZE(steps); ahead++) {
			if (steps[ahead].at == 0)
				continue;

			const uint32 nextDue = startedAt + steps[ahead].at * kTickMs + carried;
			const uint32 here = _vm->getMillis();

			until = nextDue > here ? nextDue - here : 0;
			break;
		}

		// The clock the script's times are measured against is the sound
		// driver's own, which 0xEFBA reads every timer tick, so it begins
		// again with each song.  The times on the steps around the Cryo logo -
		// a hundred and eleven and a hundred and sixty-eight - are counted
		// from where CRYOMUS starts and not from the top of the intro, which
		// is why they never bit while the clock ran from the first frame.
		if (steps[i].song) {
			_vm->_music->play(steps[i].song);
			startedAt = _vm->getMillis();
			carried = 0;
		}

		// dune_timeline reports what the intro plays and when, so that it can
		// be set beside a recording of the game itself.  It is taken after a
		// song has had its say on the clock, so that it counts from the same
		// place the step's own time does.
		const uint32 began = _vm->getMillis();
		const bool transition = steps[i].wipe == 0x10 || steps[i].wipe == 0x3a ||
				steps[i].wipe == 0x36 || steps[i].wipe == 0x38;

		// 0592 supplies 24 before each step. CF0C overrides it for Irulan.
		// A pure wait keeps the presented page's origin. In particular the
		// final wait must not shift the following narration-tail wipe.
		if (steps[i].kind != kNothing)
			_vm->_introYOffset = (steps[i].kind == kMovie && steps[i].what == HNM_IRULAN) ||
					steps[i].kind == kNarration ? 0 : 24;

		PlaybackResult asked = kPlaybackFinished;

		switch (steps[i].kind) {
		case kMovie:
			// 0309/AD50 starts WORMSUIT for the separate credits phase.
			// The following narration is not scheduled against that song's
			// new music clock; retain the completed script's timeline here.
			if (steps[i].what == HNM_CREDITS)
				_vm->_music->play("WORMSUIT");
			// 071D starts SN8 before C9F4 decodes the first VER frame.
			if (steps[i].at == 2012)
				_vm->playSoundEffect(8);
			asked = _vm->playVideo(steps[i].what, 0);
			break;

		case kScene:
			// What is on the screen is kept so that the step can be brought in
			// the way its fourth word asks, rather than cut to.
			_vm->keepScreen();
			_vm->introScene(steps[i].place, steps[i].arrival, steps[i].present, steps[i].speaker,
							0xffff, !transition);
			asked = _vm->applyWipe(steps[i].wipe);
			if (asked != kPlaybackFinished)
				break;
			// 05DC/05ED reaches 078D only after the scene transition.
			if (steps[i].speaker >= 0 && steps[i].line != 0xffff)
				_vm->playSpeech(steps[i].line, steps[i].speaker, false, false);
			{
				const uint32 elapsed = transition ? _vm->getMillis() - began : 0;
				asked = _vm->holdSpeaking(elapsed < until ? until - elapsed : 0, steps[i].speaker);
			}

			break;

		case kClear:
		case kPicture:
			_vm->keepScreen();
			if (steps[i].kind == kClear)
				memset(_vm->_framebuffer1, 0, kScreenWidth * kScreenHeight);
			else {
				// 076A clears before drawing INT15; C2F2 itself does not.
				if (steps[i].what == 97)
					memset(_vm->_framebuffer1, 0, kScreenWidth * kScreenHeight);
				_vm->showPicture(steps[i].what, !transition);
			}
			asked = _vm->applyWipe(steps[i].wipe);
			if (asked != kPlaybackFinished)
				break;

			// Entries 1 and 7 have an empty player and a one-tick hold.
			// They clear immediately; they do not wait for the next timed scene.
			if (steps[i].kind == kClear && steps[i].at == 0) {
				asked = _vm->hold(5);
				break;
			}

			// The handoff is part of this step's scheduled interval.
			if (transition) {
				const uint32 elapsed = _vm->getMillis() - began;
				asked = _vm->hold(elapsed < until ? until - elapsed : 0);
			} else {
				asked = _vm->hold(until);
			}

			break;

		case kBuilt:
		case kFeyd:
			_vm->keepScreen();
			if (steps[i].kind == kFeyd)
				_vm->drawIntroFeydBackground();
			else
				_vm->drawList(steps[i].what, steps[i].present,
							  steps[i].place ? (int)steps[i].place : -1,
							  steps[i].overX, steps[i].overY,
							  !transition && steps[i].speaker < 0);
			if (steps[i].speaker >= 0) {
				_vm->prepareIntroSpeaker(steps[i].speaker, steps[i].portraitX);
				_vm->drawCurrentSpeakerFigure();
				if (!transition) {
					_vm->copyFrameRectToScreen(_vm->_framebuffer1, kScreenWidth, 0, 0, kScreenWidth, kScreenHeight);
					_vm->_system->updateScreen();
				}
			}
			asked = _vm->applyWipe(steps[i].wipe);
			if (asked != kPlaybackFinished)
				break;
			if (steps[i].speaker >= 0 && steps[i].line != 0xffff)
				_vm->playSpeech(steps[i].line, steps[i].speaker, false, false);
			{
				const uint32 elapsed = transition ? _vm->getMillis() - began : 0;
				asked = _vm->holdSpeaking(elapsed < until ? until - elapsed : 0, steps[i].speaker);
			}
			break;

		case kStaged:
			if (steps[i].place == 0)
				_vm->keepScreen();
			asked = _vm->playVideoUntil(steps[i].what,
										steps[i].place == 0xffff ? -1 : (int)steps[i].place);
			// CA1B decodes the initial frame before the script applies 36.
			if (asked == kPlaybackFinished && steps[i].place == 0)
				asked = _vm->applyWipe(steps[i].wipe);
			if (asked == kPlaybackFinished && until) {
				const uint32 elapsed = _vm->getMillis() - began;
				asked = _vm->hold(elapsed < until ? until - elapsed : 0);
			}
			break;

		case kSlide:
			_vm->slideIn(steps[i].what, 5, 6, steps[i].arrival);
			break;

		case kNarration:
			_vm->_state[0x28e7] = 1; // 021C forces the story captions on.
			// 024A/DDB0 waits 4000 ticks once, after starting the voice.
			asked = _vm->narrate(steps[i].what, steps[i].line, 4000 * 5);
			break;

		default:
			break;
		}

		if (ConfMan.hasKey("dune_timeline")) {
			static const char *const kKinds[] = { "wait", "movie", "scene", "picture", "built", "slide", "staged", "narration", "built", "clear" };
			debug("%7.2f  %-7s %-12s for %6.2f s", (began - startedAt) / 1000.0,
				  kKinds[steps[i].kind], _vm->nameOfStep(steps[i].kind, steps[i].what),
				  (_vm->getMillis() - began) / 1000.0);
		}

		if (asked == kPlaybackAborted) {
			_vm->stopSpeech();
			_vm->_music->stop();
			return false;
		}
	}

	_vm->stopSpeech();
	return _vm->finishIntroNarration();
}

} // End of namespace Dune
