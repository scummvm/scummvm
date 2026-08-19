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

#ifndef DUNE_INTRO_H
#define DUNE_INTRO_H

#include "dune/archive.h"
#include "dune/graphics.h"

#include "common/random.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace Dune {

class Intro {
	DuneEngine *_engine;
public:
	Intro(DuneEngine *engine)
		: _engine(engine)
	{}

	void run();

	bool user_requested_skip();

	void load_virgin_hnm();
	void play_virgin_hnm();
	void clear_screen();
	void empty();
	void load_cryo_hnm();
	void play_cryo_hnm();
	void load_cryo2_hnm();
	void _sub_10678_load_PRESENT_HNM();
	void _sub_10684_play_PRESENT_HNM();
	void _sub_1CEFC_load_IRULn_HSQ();
	void _sub_1CF1B_play_IRULx_HSQ();
	void _sub_1069E_load_INTRO_HNM();
	void _sub_106AA_play_hnm_86_frames();
	void _sub_106BD_play_hnm_skippable();
	void sub_107FD();
	void loc_1085D();
	void sub_106CE();
	void sub_10704();
	void sub_10972();
	void sub_1098A();
	void sub_10995();
	void loc_10798();
	void sub_10771();
	void sub_107EE();
	void sub_109A5();
	void sub_106D3();
	void sub_1077C();
	void sub_10788();
	void sub_1078D();
	void sub_107A3();
	void sub_107C6();
	void sub_10868();
	void sub_1087B();
	void sub_10886();
	void sub_109AD();
	void sub_108B6();
	void sub_10ACD();
	void sub_106D8();
	void sub_106FC();
	void sub_10740();
	void sub_1075A();
	void sub_10752();
	void sub_1073C();
	void sub_10756();
	void sub_1075E();
	void sub_10737();
	void sub_10747();
	void sub_107E0();
	void sub_106EA();
	void sub_1074B();
	void sub_10711();
	void sub_1071D();
	void sub_1076A();
	void sub_10762();
	void sub_10766();
};

} // End of namespace Dune

#endif
