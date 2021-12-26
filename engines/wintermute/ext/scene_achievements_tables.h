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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

namespace Wintermute {

struct Achievement {
	const char *sceneFilename;
	const char *id;
};

struct AchievementsList {
	const char *gameId;
	const Achievement mapping[64];
};

static const AchievementsList achievementsList[] = {
	{
		"carolreed10",
		{
			{"scenes\\barn\\barn_intro\\barn_intro.scene", "BARN"},
			{"scenes\\brother\\bro_intro\\bro_intro.scene", "CAR_GRAVEYARD"},
			{"scenes\\church\\church_intro\\church_intro.scene", "SAINT_MARIA_CHURCH"},
			{"scenes\\emhart\\emhart_intro\\emhart_intro.scene", "EMHART_ZURICH"},
			{"scenes\\falls_day\\falls_intro\\falls_intro.scene", "BLOOD_FALLS"},
			{"scenes\\forge\\forge_intro\\forge_intro.scene", "SONIC_FORGE"},
			{"scenes\\mansion\\mans_intro\\mans_intro.scene", "STIERN_HOUSE"},
			{"scenes\\mus_malte\\mus_malte_intro\\mus_malte_intro.scene", "MALTE_MUSEUM"},
			{"scenes\\mus_school\\mus_sch_intro\\mus_sch_intro.scene", "SCHOOL_MUSEUM"},
			{"scenes\\peak\\kvarn_intro\\kvarn_intro.scene", "JOHANSSON_PEAK"},
			{"scenes\\pyramid\\pyr_intro\\pyr_intro.scene", "PYRAMID"},
			{"scenes\\ski\\dala_intro\\dala_intro.scene", "SKIING_HOUSE"},
			{"scenes\\train\\train_intro\\train_intro.scene", "MALTES_TRAIN"},
			{0,0}
		}
	},

	{
		"carolreed11",
		{
			{"scenes\\barn\\barn15d_cl\\barn15d_cl.scene", "PIPE"},
			{"scenes\\black\\black02b_lift\\black02b_lift.scene", "NOTE"},
			{"scenes\\black\\black02d_op_lift\\black02d_op_lift.scene", "BOTTLE"},
			{"scenes\\cannon\\can11a_op\\can11a_op.scene", "WINDOW"},
			{"scenes\\hobby\\hobby01a_cl1_moved\\hobby01a_cl1_moved.scene", "STONE"},
			{"scenes\\m_house\\m03c_op2_lift3\\m03c_op2_lift3.scene", "BLINDFOLD"},
			{"scenes\\m_house\\m04b_cl1_op1_lift3\\m04b_cl1_op1_lift3.scene", "POEM"},
			{"scenes\\m_house\\m07d_tap1\\m07d_tap1.scene", "TESTING"},
			{"scenes\\mine\\mine15d_lift1\\mine15d_lift1.scene", "MINE_LADY"},
			{"scenes\\office\\office02d_cl2_lift\\office02d_cl2_lift.scene", "PENCIL"},
			{0,0}
		}
	},

	{0, {{0,0}}}
};

} // End of namespace Wintermute
