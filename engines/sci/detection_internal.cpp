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

#include "common/fs.h"
#include "common/gui_options.h"

#include "sci/detection_internal.h"

namespace Sci {

const GameIdStrToEnum gameIdStrToEnum[] = {
	{ "astrochicken",    "",                GID_ASTROCHICKEN,     false, SCI_VERSION_NONE },	// Sierra ID is "sq3", distinguished by resource count
	{ "camelot",         "arthur",          GID_CAMELOT,          false, SCI_VERSION_NONE },
	{ "castlebrain",     "brain",           GID_CASTLEBRAIN,      false, SCI_VERSION_1_LATE },	// Amiga is SCI1 middle, PC SCI1 late
	{ "chest",           "archive",         GID_CHEST,            true,  SCI_VERSION_NONE },
	{ "christmas1988",   "demo",            GID_CHRISTMAS1988,    false, SCI_VERSION_0_EARLY },
	{ "christmas1990",   "card",            GID_CHRISTMAS1990,    false, SCI_VERSION_1_EARLY },
	{ "christmas1992",   "card",            GID_CHRISTMAS1992,    false, SCI_VERSION_1_1 },
	{ "cnick-kq",        "",                GID_CNICK_KQ,         false, SCI_VERSION_NONE },	// Sierra ID is "hoyle3", distinguished by resource count
	{ "cnick-laurabow",  "",                GID_CNICK_LAURABOW,   false, SCI_VERSION_NONE },
	{ "cnick-longbow",   "RH Budget",       GID_CNICK_LONGBOW,    false, SCI_VERSION_NONE },
	{ "cnick-lsl",       "",                GID_CNICK_LSL,        false, SCI_VERSION_NONE },	// Sierra ID is "lsl1", distinguished by resource count
	{ "cnick-sq",        "",                GID_CNICK_SQ,         false, SCI_VERSION_NONE },	// Sierra ID is "sq4", distinguished by resource count
	{ "ecoquest",        "eco",             GID_ECOQUEST,         false, SCI_VERSION_NONE },
	{ "ecoquest2",       "rain",            GID_ECOQUEST2,        false, SCI_VERSION_NONE },
	{ "fairytales",      "tales",           GID_FAIRYTALES,       false, SCI_VERSION_NONE },
	{ "freddypharkas",   "fp",              GID_FREDDYPHARKAS,    false, SCI_VERSION_NONE },
	{ "funseeker",       "emc",             GID_FUNSEEKER,        false, SCI_VERSION_NONE },
	{ "gk1demo",         "",                GID_GK1DEMO,          false, SCI_VERSION_NONE },
	{ "gk1",             "gk",              GID_GK1,              true,  SCI_VERSION_NONE },
	{ "gk2",             "gk2",             GID_GK2,              true,  SCI_VERSION_NONE },
	{ "hoyle1",          "cardgames",       GID_HOYLE1,           false, SCI_VERSION_NONE },
	{ "hoyle2",          "solitaire",       GID_HOYLE2,           false, SCI_VERSION_NONE },
	{ "hoyle3",          "hoyle3",          GID_HOYLE3,           false, SCI_VERSION_NONE },
	{ "hoyle4",          "hoyle4",          GID_HOYLE4,           false, SCI_VERSION_1_1 },
	{ "hoyle5",          "hoyle4",          GID_HOYLE5,           true,  SCI_VERSION_2_1_MIDDLE },
	{ "hoyle5bridge",    "",                GID_HOYLE5,           true,  SCI_VERSION_2_1_MIDDLE },
	{ "hoyle5children",  "",                GID_HOYLE5,           true,  SCI_VERSION_2_1_MIDDLE },
	{ "hoyle5school",    "",                GID_HOYLE5,           true,  SCI_VERSION_2_1_MIDDLE },
	{ "hoyle5solitaire", "",                GID_HOYLE5,           true,  SCI_VERSION_2_1_MIDDLE },
	{ "iceman",          "iceman",          GID_ICEMAN,           false, SCI_VERSION_NONE },
	{ "inndemo",         "",                GID_INNDEMO,          false, SCI_VERSION_NONE },
	{ "islandbrain",     "brain",           GID_ISLANDBRAIN,      false, SCI_VERSION_1_1 },
	{ "jones",           "jones",           GID_JONES,            false, SCI_VERSION_1_1 },
	{ "kq1sci",          "kq1",             GID_KQ1,              false, SCI_VERSION_NONE },
	{ "kq4sci",          "kq4",             GID_KQ4,              false, SCI_VERSION_NONE },
	{ "kq5",             "kq5",             GID_KQ5,              false, SCI_VERSION_NONE },
	{ "kq6",             "kq6",             GID_KQ6,              false, SCI_VERSION_NONE },
	{ "kq7",             "kq7cd",           GID_KQ7,              true,  SCI_VERSION_NONE },
	{ "kquestions",      "quizgame-demo",   GID_KQUESTIONS,       true,  SCI_VERSION_NONE },
	{ "laurabow",        "cb1",             GID_LAURABOW,         false, SCI_VERSION_NONE },
	{ "laurabow2",       "lb2",             GID_LAURABOW2,        false, SCI_VERSION_NONE },
	{ "lighthouse",      "lite",            GID_LIGHTHOUSE,       true,  SCI_VERSION_NONE },
	{ "longbow",         "longbow",         GID_LONGBOW,          false, SCI_VERSION_NONE },
	{ "lsl1sci",         "lsl1",            GID_LSL1,             false, SCI_VERSION_NONE },
	{ "lsl2",            "lsl2",            GID_LSL2,             false, SCI_VERSION_NONE },
	{ "lsl3",            "lsl3",            GID_LSL3,             false, SCI_VERSION_NONE },
	{ "lsl5",            "lsl5",            GID_LSL5,             false, SCI_VERSION_NONE },
	{ "lsl6",            "lsl6",            GID_LSL6,             false, SCI_VERSION_NONE },
	{ "lsl6hires",       "",                GID_LSL6HIRES,        true,  SCI_VERSION_NONE },
	{ "lsl7",            "l7",              GID_LSL7,             true,  SCI_VERSION_NONE },
	{ "mothergoose",     "mg",              GID_MOTHERGOOSE,      false, SCI_VERSION_NONE },
	{ "mothergoose256",  "",                GID_MOTHERGOOSE256,   false, SCI_VERSION_NONE },
	{ "mothergoosehires","",                GID_MOTHERGOOSEHIRES, true,  SCI_VERSION_NONE },
	{ "msastrochicken",  "",                GID_MSASTROCHICKEN,   false, SCI_VERSION_NONE },	// Sierra ID is "sq4", distinguished by resource count
	{ "pepper",          "twisty",          GID_PEPPER,           false, SCI_VERSION_NONE },
	{ "phantasmagoria",  "scary",           GID_PHANTASMAGORIA,   true,  SCI_VERSION_NONE },
	{ "phantasmagoria2", "p2",              GID_PHANTASMAGORIA2,  true,  SCI_VERSION_NONE },
	{ "pq1sci",          "pq1",             GID_PQ1,              false, SCI_VERSION_NONE },
	{ "pq2",             "pq",              GID_PQ2,              false, SCI_VERSION_NONE },
	{ "pq3",             "pq3",             GID_PQ3,              false, SCI_VERSION_NONE },
	{ "pq4",             "pq4",             GID_PQ4,              true,  SCI_VERSION_NONE },
	{ "pq4demo",         "",                GID_PQ4DEMO,          false, SCI_VERSION_NONE },
	{ "pqswat",          "swat",            GID_PQSWAT,           true,  SCI_VERSION_NONE },
	{ "qfg1",            "gfg1",            GID_QFG1,             false, SCI_VERSION_NONE },
	{ "qfg1vga",         "",                GID_QFG1VGA,          false, SCI_VERSION_NONE },	// Sierra ID is "glory", distinguished by resources
	{ "qfg2",            "trial",           GID_QFG2,             false, SCI_VERSION_NONE },
	{ "qfg3",            "",                GID_QFG3,             false, SCI_VERSION_NONE },	// Sierra ID is "glory", distinguished by resources
	{ "qfg4",            "",                GID_QFG4,             true,  SCI_VERSION_NONE },	// Sierra ID is "glory", distinguished by resources
	{ "qfg4demo",        "",                GID_QFG4DEMO,         false, SCI_VERSION_NONE },	// Sierra ID is "glory", distinguished by resources
	{ "rama",            "rama",            GID_RAMA,             true,  SCI_VERSION_NONE },
	{ "sci-fanmade",     "",                GID_FANMADE,          false, SCI_VERSION_NONE },
	{ "shivers",         "",                GID_SHIVERS,          true,  SCI_VERSION_NONE },
	//{ "shivers2",        "shivers2",        GID_SHIVERS2,       true,  SCI_VERSION_NONE },	// Not SCI
	{ "slater",          "thegame",         GID_SLATER,           false, SCI_VERSION_NONE },
	{ "sq1sci",          "sq1",             GID_SQ1,              false, SCI_VERSION_NONE },
	{ "sq3",             "sq3",             GID_SQ3,              false, SCI_VERSION_NONE },
	{ "sq4",             "sq4",             GID_SQ4,              false, SCI_VERSION_NONE },
	{ "sq5",             "sq5",             GID_SQ5,              false, SCI_VERSION_NONE },
	{ "sq6",             "sq6",             GID_SQ6,              true,  SCI_VERSION_NONE },
	{ "torin",           "torin",           GID_TORIN,            true,  SCI_VERSION_NONE },
	{ nullptr,           nullptr,           GID_ALL,              false, SCI_VERSION_NONE }
};

Common::String customizeGuiOptions(Common::Path gamePath, Common::String guiOptions, Common::Platform platform, Common::String idStr, SciVersion version) {
	struct RMode {
		SciVersion min;
		SciVersion max;
		const char gfxDriverName[13];
		const char *guio;
	} rmodes[] = {
		{ SCI_VERSION_0_EARLY,	SCI_VERSION_1_EGA_ONLY,		"EGA320.DRV",		GUIO_RENDEREGA },
		{ SCI_VERSION_0_EARLY,	SCI_VERSION_1_EGA_ONLY,		"CGA320C.DRV",		GUIO_RENDERCGA },
		{ SCI_VERSION_0_EARLY,	SCI_VERSION_1_EGA_ONLY,		"CGA320BW.DRV",		GUIO_RENDERCGABW },
		{ SCI_VERSION_0_EARLY,	SCI_VERSION_1_EGA_ONLY,		"CGA320M.DRV",		GUIO_RENDERCGABW },
		{ SCI_VERSION_0_EARLY,	SCI_VERSION_1_EGA_ONLY,		"HERCMONO.DRV",		GUIO_RENDERHERCAMBER },
		{ SCI_VERSION_0_EARLY,	SCI_VERSION_1_EGA_ONLY,		"HERCMONO.DRV",		GUIO_RENDERHERCGREEN },
		{ SCI_VERSION_1_EARLY,	SCI_VERSION_1_1,			"VGA320.DRV",		GUIO_RENDERVGA },
		{ SCI_VERSION_1_EARLY,	SCI_VERSION_1_1,			"VGA320BW.DRV",		GUIO_RENDERVGAGREY },
		{ SCI_VERSION_1_EARLY,	SCI_VERSION_1_1,			"EGA640.DRV",		GUIO_RENDEREGA },
		{ SCI_VERSION_01,		SCI_VERSION_1_LATE,			"9801V16.DRV",		GUIO_RENDERPC98_16C },
		{ SCI_VERSION_01,		SCI_VERSION_01,				"9801VID.DRV",		GUIO_RENDERPC98_16C },
		{ SCI_VERSION_1_LATE,	SCI_VERSION_1_LATE,			"9801V8.DRV",		GUIO_RENDERPC98_8C },
		{ SCI_VERSION_01,		SCI_VERSION_01,				"9801V8M.DRV",		GUIO_RENDERPC98_8C },
		{ SCI_VERSION_01,		SCI_VERSION_01,				"9801VID.DRV",		GUIO_RENDERPC98_8C }
	};

	if (idStr.equals("kq6") && platform == Common::kPlatformWindows)
		return guiOptions + GUIO_RENDERWIN_256C + GUIO_RENDERWIN_16C;

	Common::FSNode node(gamePath);
	Common::FSList files;
	if (!node.getChildren(files, Common::FSNode::kListFilesOnly)) {
		warning("Game path '%s' could not be accessed", gamePath.toString().c_str());
		return guiOptions;
	}

	for (Common::FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
		for (int ii = 0; ii < ARRAYSIZE(rmodes); ii++) {
			if (version == SCI_VERSION_NONE || (rmodes[ii].min <= version && version <= rmodes[ii].max)) {
				if (i->getFileName().equalsIgnoreCase(rmodes[ii].gfxDriverName)) {
					guiOptions += rmodes[ii].guio;
				}
			}
		}
	}

	return guiOptions;
}

} // End of namespace Sci
