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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace Alan2 {

const PlainGameDescriptor ALAN2_GAME_LIST[] = {
	{ "alan2", "Alan2 Game" },

	// Unsupported 2.6 games
	{ "hollywoodmurders", "The Hollywood Murders" },

	// Supported 2.8.1 games
	{ "artifiction", "Artifiction" },
	{ "bantams", "Even Bantams get the Blues" },
	{ "bugged", "Bugged" },
	{ "cc", "CC" },
	{ "chasing", "The Chasing" },
	{ "closet", "Closet" },
	{ "crystalball", "Crystal Ball" },
	{ "dinnertime", "Dinnertime" },
	{ "hebgb", "The HeBGB Horror!" },
	{ "arthursnightout", "King Arthur's Night Out" },
	{ "knotundone", "Knot To Be Undone" },
	{ "lostinnewyork", "Lost In New York" },
	{ "mazemapper", "Mazemapper" },
	{ "meanstory", "The Mean Story" },
	{ "outofthestudy", "Out Of The Study" },
	{ "plsghints", "Painless Little Stupid Games Hints" },
	{ "puddlespath", "Puddles on the Path" },
	{ "sardoria", "Sardoria" },
	{ "skipbreak", "Skipping Breakfast" },
	{ "speedif11p6", "SpeedIF 11 - part six" },
	{ "presidentadventures", "The Adventures of the President of the United States" },
	{ "thechasing", "The Chasing" },
	{ "nobread", "There Is No Bread" },
	{ "afteryou", "They're After You" },
	{ "tgttos", "To Get To The Other Side" },
	{ "zeroone", "Zero One" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry ALAN2_GAMES[] = {
	// Unsupported 2.6 games
	DT_ENTRY0("hollywoodmurders", "e6da6dfa85dd471a251ce485af710730", 95232),

	// Supported 2.8.1 games
	DT_ENTRY0("artifiction", "2615ba29c6030ec50454fdded1049b44", 40960),
	DT_ENTRY0("bantams", "8c4be4f2e21f9770523ea0c4a180cb8a", 40960),
	DT_ENTRY0("bugged", "f4eed3db3771a456143744a0e36653db", 112640),
	DT_ENTRY0("cc", "47f4a5436ce07d65bb828e2938f81952", 53248),
	DT_ENTRY1("chasing", "1.0", "10ad37b6dd6d9f399578b28e644914ca", 144384),
	DT_ENTRY1("chasing", "1.1", "718ffcc9dfe85cfd8c6f50f541a3926e", 147456),
	DT_ENTRY0("closet", "5d724469e6240cde0c16c959f50ebc93", 37888),
	DT_ENTRY0("crystalball", "625e15e14081847cd26842a52839a07c", 11264),
	DT_ENTRY0("dinnertime", "1c0bad19156e8bdefe9e19d99f96f7d8", 9216),
	DT_ENTRY0("hebgb", "87f9516bc4217afb5c329cb1ae01d861", 173056),
	DT_ENTRY0("hebgb", "96f016f4657a801686d1d1da5dd6f382", 157696),
	DT_ENTRY0("knotundone", "a4249a82d7211398adc9917c773c58fa", 25600),
	DT_ENTRY0("arthursnightout", "c8c87b88a9250cec2f03af3a347d3c05", 50176),
	DT_ENTRY0("lostinnewyork", "483a8c7c84f3bb5af61150fd770806e5", 30720),
	DT_ENTRY0("mazemapper", "8e7409758c3535201aeb901923b20064", 30720),
	DT_ENTRY0("meanstory", "e4ae6873d6f2ab74fb2ec35b27752397", 13312),
	DT_ENTRY1("outofthestudy", "1.0", "089082d50b232b07fc31cf60aeefb5ae", 102400),
	DT_ENTRY1("outofthestudy", "1.3", "cc4ab3f1b406a8ce04adcfb641c3b250", 110592),
	DT_ENTRY0("plsghints", "fb9df41138691a77ea3489986fe8856c", 10240),
	DT_ENTRY0("puddlespath", "5b6bf831263f43b6e9f26517fae6294b", 104448),
	DT_ENTRY0("sardoria", "b48ba08ae33b5cb224bcb4ce0eea36bc", 150528),
	DT_ENTRY0("sardoria", "1b0e03cbdcb74d59a92920d093fc7446", 143360),
	DT_ENTRY0("skipbreak", "a0f7e71f00fd4d26eacc82637da84cb0", 44032),
	DT_ENTRY0("speedif11p6", "0c8a517bd49c3f6ed28d44e5a294de06", 32768),
	DT_ENTRY0("presidentadventures", "74caac5475f8801ac27d76a3d2e034ad", 53248),
	DT_ENTRY0("thechasing", "10ad37b6dd6d9f399578b28e644914ca", 144384),
	DT_ENTRY0("nobread", "7a85bef5bc441bbaa8a6321e063ec711", 30720),
	DT_ENTRY0("afteryou", "998ca167b0e9ffb671203b2057d06bef", 17408),
	DT_ENTRY0("tgttos", "0fed94b37b8add48938d8288ca5e7e4f", 29696),
	DT_ENTRY0("zeroone", "1e2f65611c41040f60caaf5f26e61367", 59392),

	DT_END_MARKER
};

} // End of namespace Alan2
} // End of namespace Glk
