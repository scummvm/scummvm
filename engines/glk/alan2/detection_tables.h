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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace Alan2 {

const PlainGameDescriptor ALAN2_GAME_LIST[] = {
	{ "alan2", "Alan2 Game" },

	{ "bugged", "Bugged" },
	{ "chasing", "The Chasing" },
	{ "closet", "Closet" },
	{ "dinnertime", "Dinnertime" },
	{ "hebgb", "The HeBGB Horror!" },
	{ "lostinnewyork", "Lost In New York" },
	{ "mazemapper", "Mazemapper" },
	{ "meanstory", "The Mean Story" },
	{ "outofthestudy", "Out Of The Study" },
	{ "plsghints", "Painless Little Stupid Games Hints" },
	{ "sardoria", "Sardoria" },
	{ "tgttos", "To Get To The Other Side" },
	{ "afteryou", "They're After You" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry ALAN2_GAMES[] = {
	DT_ENTRY0("bugged", "f4eed3db3771a456143744a0e36653db", 112640),
	DT_ENTRY0("chasing", "718ffcc9dfe85cfd8c6f50f541a3926e", 147456),
	DT_ENTRY0("closet", "5d724469e6240cde0c16c959f50ebc93", 37888),
	DT_ENTRY0("dinnertime", "1c0bad19156e8bdefe9e19d99f96f7d8", 9216),
	DT_ENTRY0("hebgb", "87f9516bc4217afb5c329cb1ae01d861", 173056),
	DT_ENTRY0("lostinnewyork", "483a8c7c84f3bb5af61150fd770806e5", 30720),
	DT_ENTRY0("mazemapper", "8e7409758c3535201aeb901923b20064", 30720),
	DT_ENTRY0("meanstory", "e4ae6873d6f2ab74fb2ec35b27752397", 13312),
	DT_ENTRY0("outofthestudy", "cc4ab3f1b406a8ce04adcfb641c3b250", 110592),
	DT_ENTRY0("plsghints", "fb9df41138691a77ea3489986fe8856c", 10240),
	DT_ENTRY0("sardoria", "b48ba08ae33b5cb224bcb4ce0eea36bc", 150528),
	DT_ENTRY0("tgttos", "0fed94b37b8add48938d8288ca5e7e4f", 29696),
	DT_ENTRY0("afteryou", "998ca167b0e9ffb671203b2057d06bef", 17408),

	DT_END_MARKER
};

} // End of namespace Alan2
} // End of namespace Glk
