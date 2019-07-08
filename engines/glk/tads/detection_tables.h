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
namespace TADS {

const PlainGameDescriptor TADS2_GAME_LIST[] = {
	{ "tads2", "TADS 2 Game" },

	// English games
	{ "oncefuture", "Once and Future" },

	// Adventions games
	{ "ccr", "Colossal Cave Revisited" },
	{ "legendlives", "The Legend Lives!" },
	{ "rylvania", "The Horror of Rylvania" },
	{ "u0", "Unnkulia Zero: The Search for Amanda" },
	{ "uhalf", "Unnkulia One-Half: The Salesman Triumphant" },
	{ "uu1", "Unnkulian Underworld: The Unknown Unventure" },
	{ "uu2", "Unnkulian Unventure II: The Secret of Acme" },

	{ nullptr, nullptr }
};

const PlainGameDescriptor TADS3_GAME_LIST[] = {
	{ "tads3", "TADS 3 Game" },

	{ "allhope", "All Hope Abandon" },
	{ "blightedisle", "Blighted Isle" },
	{ "combatdemo", "TADS 3 Combat Demos" },
	{ "comp02tr", "Comp02ter Game" },
	{ "cookoff", "Cook Off" },
	{ "darkangel", "Dark Angel" },
	{ "deadmansparty", "Dead Mans Party" },
	{ "dearbrian", "Dear Brian" },
	{ "devildetails", "The Devil in the Details" },
	{ "elysiumenigma", "The Elysium Enigma" },
	{ "foreveralways", "Elizabeth Hawk's Forever Always" },
	{ "golden-banana", "The Quest of the Golden Banana" },
	{ "gunmute", "Gun Mute" },
	{ "homeopen", "Home Open" },
	{ "indigo", "Indigo" },
	{ "it", "It" },
	{ "korenvliet", "Korenvliet" },
	{ "lookaroundthecorner", "Look Around the Corner" },
	{ "lydiasheart", "Lydia's Heart" },
	{ "macrocosm", "Macrocosm" },
	{ "marthasbigdate", "Martha's Big Date" },
	{ "mrspepper", "Mrs. Pepper's Nasty Secret" },
	{ "multiverserogue", "Rogue of the Multiverse" },
	{ "mylastrodeo", "My Last Rodeo" },
	{ "pekostory", "Peko's Story" },
	{ "ratincontrol", "Rat In Control" },
	{ "recluse", "Recluse " },
	{ "returntoditchday", "Return to Ditch Day" },
	{ "ribosoperation", "Dr Who and the Ribos Operation" },
	{ "saturnschild", "Saturn's Child" },
	{ "sawesome", "Hide and Seek - Cindy's Something Awesome Project" },
	{ "spaceshooter", "Space Shooter: A TADS 3 abuse" },
	{ "squarecircle", "Square Circle" },
	{ "stormshelter", "Shelter from the Storm" },
	{ "thehouse", "The House" },
	{ "thereveal", "The Reveal" },
	{ "threedaysofnight", "Three Days of Night" },
	{ "thesleepingprincess", "The Sleeping Princess" },
	{ "undertaleepilogue", "Undertale: Epilogue" },
	{ "venice", "A Lady in Waiting" },
	{ "vividity", "Vividity" },
	{ "wardz", "Ward Z" },
	{ "walkersilhouette", "Walker and Silhouette" },
	{ "wilderness", "In the Wilderness" },
	{ "wolfenschtein", "Wolfenschtein" },
	{ "writingonthewall", "The Writing on the Wall" },
	{ "wutheringheights", "Wuthering Heights" },
	{ "youmatched", "It's a Match!" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry TADS_GAMES[] = {
	// TADS 2 - English

	// TADS 2 - Commercial games
	DT_ENTRY0("oncefuture", "4ed995d0784520ca6f0ec5391d92f4d8", 909993),

	// TADS 2 - Adventions games
	DT_ENTRY0("ccr", "2da7dba524075aed0167ae02e6484a32", 291305),
	DT_ENTRY0("legendlives", "03a651ef3d904dbef6a80cd2c041423c", 622989),
	DT_ENTRY0("rylvania", "98027d23f0da0e38c02f1326a2357713", 302903),
	DT_ENTRY1("u0", "Demo", "d9615e08336cc0d50984cdc0879f2006", 500949),
	DT_ENTRY0("uhalf", "80c18f27f656d7e83d1f3f8075115720", 242269),
	DT_ENTRY0("uu1", "705b91f9e54c591e361d6c2e15bff8ef", 367506),
	DT_ENTRY0("uu2", "69a4f7cd914bd32bd7a3bef5c68f9f3a", 400113),

	// TADS 3 - English
	DT_ENTRY0("allhope", "1e842218ad6eeacb2d284ad34adbeaaa", 1816249),
	DT_ENTRY1("allhope", "WebUI", "c65cff5c63cfecad3d8c922f0086816f", 2434146),
	DT_ENTRY0("blightedisle", "a400f71d73c0efdf301635e73ff00c79", 3326591),
	DT_ENTRY1("blightedisle", "WebUI", "4ee80cdf2e39f41e721440db7ddbc05e", 3999970),
	DT_ENTRY0("combatdemo", "23a59262f9d8534562379b123e1272b7", 515392),
	DT_ENTRY0("combatdemo", "23a59262f9d8534562379b123e1272b7", 620151),
	DT_ENTRY0("comp02tr", "23327e39fa44259fcabcb99917302c22", 960653),
	DT_ENTRY0("cookoff", "581d1996db3a442064dae237e2e962f7", 1147531),
	DT_ENTRY0("darkangel", "04f418d9e6ad8fb5a8d8291e11a788e6", 2873281),
	DT_ENTRY1("darkangel", "WebUI", "f7b2b242d4029623171f35214ef213c8", 2873281),
	DT_ENTRY0("deadmansparty", "bc78f6e390df143a13aeabcdb9de1fed", 1638558),
	DT_ENTRY1("deadmansparty", "WebUI", "831ba48e61a40e442a7459aa798da3ea", 2056080),
	DT_ENTRY0("dearbrian", "8488c4f5c18a76ab65dd9e8eb72393ac", 999423),
	DT_ENTRY0("devildetails", "0cca588557a79231daa78ef948f62397", 2892947),
	DT_ENTRY0("elysiumenigma", "f98f0e4de8c7cf5d6f471b56c6a96d81", 2095654),
	DT_ENTRY1("elysiumenigma", "WebUI", "7ca73a03b433b28a831808066a843d79", 2149533),
	DT_ENTRY0("foreveralways", "1e901bfebf416a910ba55e301101034c", 656194),
	DT_ENTRY0("golden-banana", "c5df512d642503556cd9c0d487a3c95f", 726650),
	DT_ENTRY0("gunmute", "bf196b6b53fca37c8fec25d27bdb4f23", 713636),
	DT_ENTRY0("homeopen", "ae46177333238267362f9465f65fac80", 1869401),
	DT_ENTRY1("homeopen", "WebUI", "7db13d7785329886ad5f24533115a749", 2303325),
	DT_ENTRY0("indigo", "ca322b887ba56ef503b96ee462a2df30", 535570),
	DT_ENTRY0("it", "aae7a1a7eb812ef216406875df1035bc", 1161138),
	DT_ENTRY1("it", "WebUI", "e69e00d227df2ea9179bb3bc20d779c9", 1589779),
	DT_ENTRY0("korenvliet", "4ab26eb23354a000f66b77bcbb820be4", 719957),
	DT_ENTRY0("lookaroundthecorner", "c9ac24c941c39ef57b23a55ed36f844d", 664271),
	DT_ENTRY1("lookaroundthecorner", "WebUI", "5e7d1e0c7f93662fb455972797795f15", 1079205),
	DT_ENTRY0("lydiasheart", "aa92fa47c4e04e90d358e9cfd50de671", 2620069),
	DT_ENTRY0("macrocosm", "44492ef14565d51cbfe8ae6ddf07af1f", 30678531),
	DT_ENTRY0("marthasbigdate", "8b65ae35ad2165d8bdc48f33e7b2bbb2", 646813),
	DT_ENTRY0("mrspepper", "d9b3c2f7b954fb066561febf195b27d4", 1118901),
	DT_ENTRY1("mrspepper", "WebUI", "9bcfcdc5a37e9c5beb9b961a98929b67", 1721042),
	DT_ENTRY0("multiverserogue", "551b85aaba2d958f2bfd1080c7787844", 811469),
	DT_ENTRY0("mylastrodeo", "82f4a03bfdd5b8da41c6e3dbba0d9666", 1614970),
	DT_ENTRY1("mylastrodeo", "WebUI", "d7f480254686e114037de11f9c82782b", 2042237),
	DT_ENTRY0("pekostory", "0d703060d485aa4aeefe0d267a819ab3", 1100463),
	DT_ENTRY0("ratincontrol", "607502409e7143cb20896d09201744ac", 401192),
	DT_ENTRY0("recluse", "c41b5ab77e458f38c487dc1a1e0778f2", 1073030),
	DT_ENTRY0("returntoditchday", "47577d2001035ac30332bdb5ad9bcf3c", 2141661),
	DT_ENTRY0("ribosoperation", "7ce5c5c74b5adecc8520a99e720a6a0d", 718057),
	DT_ENTRY0("saturnschild", "2644265c684d153777dc8477e9e51ce9", 2321035),
	DT_ENTRY1("saturnschild", "WebUI", "67b0926a6b96a37a88e4564e3481f386", 2795440),
	DT_ENTRY0("sawesome", "558a930adf7995b566d901719cb90041", 1227313),
	DT_ENTRY0("spaceshooter", "cb7275fb06421696153aef808bc28d53", 79989),
	DT_ENTRY0("squarecircle", "935775a75e141df9a6f9d94b41ae71ee", 1024972),
	DT_ENTRY0("stormshelter", "72d6180f16066cdb20725176d03d7ded", 1702075),
	DT_ENTRY1("stormshelter", "WebUI", "db84c1f25835763494fc8124fcade37f", 2293053),
	DT_ENTRY0("thehouse", "8b92f7f00ef0ca5713f7ffffa13b8a01", 1688640),
	DT_ENTRY0("thereveal", "ff5485af8cbc5c0b4211bf87db23ceeb", 1590214),
	DT_ENTRY1("thereveal", "WebUI", "d4f196d2e6bd3dd49f70e52bb504e793", 2018665),
	DT_ENTRY0("thesleepingprincess", "63255d1bde649f907725a6c328047809", 736075),
	DT_ENTRY1("thesleepingprincess", "WebUI", "711cf6098b0670821af907dc94793e90", 1337974),
	DT_ENTRY0("threedaysofnight", "8271c716c6c11b4aec8b0b95aacb276d", 1711576),
	DT_ENTRY1("threedaysofnight", "WebUI", "1de414e58b667eab103361dbcd95353a", 2139562),
	DT_ENTRY0("undertaleepilogue", "e76c15c3446865916bb1647ed4f83b3e", 1189729),
	DT_ENTRY0("venice", "2db783be567e7761b4501ec12bc2eb35", 880111),
	DT_ENTRY1("venice", "WebUI", "e0e9efc4c6a680ac60ddc380c13cc099", 1306352),
	DT_ENTRY0("vividity", "7da90c0a140c73249b83b5c3e459ca83", 692049),
	DT_ENTRY1("vividity", "WebUI", "f535eb35d8f4776b2262dd8aca3281b3", 1106715),
	DT_ENTRY0("walkersilhouette", "05bd96f0782fefa3d09d0adad67249c9", 687377),
	DT_ENTRY0("wardz", "f17c720b4a80693534fea474a54f15b4", 1170346),
	DT_ENTRY0("wilderness", "5ea2cc9ab1193d416c907b9c1f0934e3", 663353),
	DT_ENTRY1("wilderness", "WebUI", "b2a76f20abf3f255dfe23d53cbaedcf0", 1089562),
	DT_ENTRY1("wolfenschtein", "WebUI", "31ccf0d040271950ddf3ca91f8ff22e2", 1200645),
	DT_ENTRY0("writingonthewall", "85555df67c45ed04cfd18effb7bbd670", 1138559),
	DT_ENTRY0("wutheringheights", "0d2042689bdf63e719b77485d591f1fe", 1083215),
	DT_ENTRY0("youmatched", "63007bd45363f5ddbcc2e4fe4a1a52de", 1621574),
	DT_ENTRY1("youmatched", "WebUI", "055efcc37f945071ea2486a207703951", 2050047),

	DT_END_MARKER
};

} // End of namespace Frotz
} // End of namespace Glk
