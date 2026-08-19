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

#include "dune/intro.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/memstream.h"
#include "common/util.h"

#include "dune/dune.h"

namespace Dune {

typedef void (Intro::*stepfn)();

struct Scene {
	int32_t a;
	stepfn  b;
	int32_t c;
	int32_t d;
	stepfn  e;
	int32_t f;
};

static Scene intro_script[] = {
	{      0,      &Intro::load_virgin_hnm,                        0, 0x003a, &Intro::play_virgin_hnm,                        1 },
	{      0,      &Intro::clear_screen,                           0, 0x003a, &Intro::empty,                                  1 },
	{      0,      &Intro::load_cryo_hnm,                          0, 0x0030, &Intro::play_cryo_hnm,                          1 },
	{      0,      &Intro::load_cryo2_hnm,                    0x006f, 0x0030, &Intro::play_cryo_hnm,                          1 },
	{      0,      &Intro::empty,                             0x00a8,     -1, &Intro::empty,                                  1 },
	// {      0,      &Intro::_sub_10678_load_PRESENT_HNM,            0, 0x003a, &Intro::_sub_10684_play_PRESENT_HNM,            1 },
	// {      0,      &Intro::_sub_1CEFC_load_IRULn_HSQ,              0, 0x003a, &Intro::_sub_1CF1B_play_IRULx_HSQ,              1 },
	// {      0,      &Intro::clear_screen,                           0, 0x003a, &Intro::empty,                                  1 },
	// {      0,      &Intro::_sub_1069E_load_INTRO_HNM,              0, 0x0036, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::empty,                             0x0090, 0x0030, &Intro::_sub_106AA_play_hnm_86_frames,     0x0190 },
	// {      0,      &Intro::empty,                             0x010c,     -1, &Intro::_sub_106BD_play_hnm_skippable,          1 },
	// {      0,      &Intro::sub_107FD,                              0, 0x003a, &Intro::loc_1085D,                         0x04b0 },
	// { 0x0148,      &Intro::sub_106CE,                         0x014e, 0x0010, &Intro::sub_10704,                         0x1900 },
	// { 0x024b,      &Intro::sub_10972,                         0x024e, 0x0010, &Intro::empty,                             0x0085 },
	// {      0,      &Intro::sub_1098A,                         0x0258, 0x0010, &Intro::empty,                             0x0085 },
	// {      0,      &Intro::sub_10995,                         0x0262, 0x0010, &Intro::loc_10798,                         0x0258 },
	// { 0x02ca,      &Intro::sub_10771,                         0x02cf, 0x0010, &Intro::loc_10798,                         0x0258 },
	// { 0x032e,      &Intro::sub_107EE,                         0x0330, 0x0010, &Intro::empty,                             0x00c8 },
	// { 0x033c,      &Intro::sub_109A5,                         0x0344, 0x0010, &Intro::empty,                             0x0085 },
	// {      0,      &Intro::sub_106D3,                         0x034e, 0x0010, &Intro::sub_10704,                         0x2648 },
	// { 0x04be,      &Intro::sub_1077C,                         0x04c0, 0x0010, &Intro::empty,                             0x0085 },
	// {      0,      &Intro::sub_10788,                         0x04d0,     -1, &Intro::sub_1078D,                         0x0258 },
	// { 0x050c,      &Intro::sub_107A3,                         0x0510, 0x0010, &Intro::loc_10798,                         0x0258 },
	// { 0x054c,      &Intro::sub_107C6,                         0x0550, 0x0010, &Intro::loc_10798,                         0x0258 },
	// { 0x0588,      &Intro::sub_10868,                         0x058f, 0x0010, &Intro::sub_1087B,                         0x0320 },
	// { 0x05ac,      &Intro::sub_10886,                         0x05ae, 0x0030, &Intro::loc_10798,                         0x0258 },
	// { 0x05ee,      &Intro::sub_109AD,                         0x05f0, 0x0030, &Intro::loc_10798,                         0x0258 },
	// { 0x0614,      &Intro::empty,                                  0,     -1, &Intro::sub_108B6,                         0x0032 },
	// { 0x061e,      &Intro::sub_10ACD,                         0x061e, 0x0010, &Intro::empty,                             0x07d0 },
	// { 0x064c,      &Intro::sub_106D8,                              0, 0x0010, &Intro::sub_106FC,                         0x0c80 },
	// { 0x06c6,      &Intro::sub_10740,                         0x06c8, 0x003a, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_1075A,                         0x06ee, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_10752,                         0x06fe, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_1073C,                         0x070e, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_10756,                         0x072e, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_1075E,                         0x073e, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_10737,                         0x074e, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_10747,                         0x076e, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_107E0,                         0x077e, 0x0010, &Intro::empty,                             0x04b0 },
	// { 0x07ac,      &Intro::sub_106EA,                         0x07af, 0x0010, &Intro::sub_10704,                         0x03e8 },
	// { 0x07ce,      &Intro::sub_1074B,                         0x07ce, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_10711,                         0x07dc, 0x003a, &Intro::sub_1071D,                              1 },
	// {      0,      &Intro::sub_1076A,                         0x081c, 0x003a, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_10762,                         0x083e, 0x0010, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::sub_10766,                         0x084e, 0x0010, &Intro::empty,                             0x00c8 },
	// {      0,      &Intro::clear_screen,                      0x085e, 0x003a, &Intro::empty,                                  1 },
	// {      0,      &Intro::sub_1076A,                         0x085e, 0x0036, &Intro::empty,                             0x0190 },
	// {      0,      &Intro::clear_screen,                      0x08e0, 0x0038, &Intro::empty,                                  1 },
	{     -1 }
};

void Intro::run() {
	for (;;) {
		_engine->_graphics.setGlobalYOffset(24);
		Scene *scene = intro_script;

		while (scene->a) {
			(this->*(scene->b))();

			if (scene->c) {
				// transition
			}

			(this->*(scene->e))();

			if (scene->f) {
				// transition
			}

			++scene;
		}
	}

	_engine->_graphics.setGlobalYOffset(0);
}

bool Intro::user_requested_skip() {
	Common::Event event;
	Common::EventManager *eventMan = _engine->getEventManager();
	while (eventMan->pollEvent(event)) {
	}
	return false;
}

void Intro::load_virgin_hnm() {
}

void Intro::play_virgin_hnm() {
	do {
		if (user_requested_skip()) {
			return;
		}

	} while (1 /* hnm not complete */);
}

void Intro::clear_screen() {
}

void Intro::empty() {
}

void Intro::load_cryo_hnm() {
}

void Intro::play_cryo_hnm() {
}

void Intro::load_cryo2_hnm() {
}

void Intro::_sub_10678_load_PRESENT_HNM() {
}

void Intro::_sub_10684_play_PRESENT_HNM() {
}

void Intro::_sub_1CEFC_load_IRULn_HSQ() {
}

void Intro::_sub_1CF1B_play_IRULx_HSQ() {
}

void Intro::_sub_1069E_load_INTRO_HNM() {
}

void Intro::_sub_106AA_play_hnm_86_frames() {
}

void Intro::_sub_106BD_play_hnm_skippable() {
}

void Intro::sub_107FD() {
}

void Intro::loc_1085D() {
}

void Intro::sub_106CE() {
}

void Intro::sub_10704() {
}

void Intro::sub_10972() {
}

void Intro::sub_1098A() {
}

void Intro::sub_10995() {
}

void Intro::loc_10798() {
}

void Intro::sub_10771() {
}

void Intro::sub_107EE() {
}

void Intro::sub_109A5() {
}

void Intro::sub_106D3() {
}

void Intro::sub_1077C() {
}

void Intro::sub_10788() {
}

void Intro::sub_1078D() {
}

void Intro::sub_107A3() {
}

void Intro::sub_107C6() {
}

void Intro::sub_10868() {
}

void Intro::sub_1087B() {
}

void Intro::sub_10886() {
}

void Intro::sub_109AD() {
}

void Intro::sub_108B6() {
}

void Intro::sub_10ACD() {
}

void Intro::sub_106D8() {
}

void Intro::sub_106FC() {
}

void Intro::sub_10740() {
}

void Intro::sub_1075A() {
}

void Intro::sub_10752() {
}

void Intro::sub_1073C() {
}

void Intro::sub_10756() {
}

void Intro::sub_1075E() {
}

void Intro::sub_10737() {
}

void Intro::sub_10747() {
}

void Intro::sub_107E0() {
}

void Intro::sub_106EA() {
}

void Intro::sub_1074B() {
}

void sub_10711() {
}

void sub_1071D() {
}

void sub_1076A() {
}

void sub_10762() {
}

void sub_10766() {
}

} // End of namespace Dune
