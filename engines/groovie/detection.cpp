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

#include "common/system.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "groovie/detection.h"

namespace Groovie {

#define GAMEOPTION_T7G_FAST_MOVIE_SPEED  GUIO_GAMEOPTIONS1

static const PlainGameDescriptor groovieGames[] = {
	// Games
	{"t7g", "The 7th Guest"},
	{"11h", "The 11th Hour: The Sequel to The 7th Guest"},
	{"clandestiny", "Clandestiny"},
	{"unclehenry", "Uncle Henry's Playhouse"},
	{"tlc", "Tender Loving Care"},

	{0, 0}
};

static const GroovieGameDescription gameDescriptions[] = {

	// The 7th Guest DOS English
	{
		{
			"t7g", "",
			AD_ENTRY1s("script.grv", "d1b8033b40aa67c076039881eccce90d", 16659),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_T7G_FAST_MOVIE_SPEED)
		},
		kGroovieT7G, 0
	},

	// The 7th Guest Mac English
	{
		{
			"t7g", "",
			AD_ENTRY1s("T7GMac", "acdc4a58dd3f007f65e99b99d78e0bce", 1814029),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_MACRESFORK,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_T7G_FAST_MOVIE_SPEED)
		},
		kGroovieT7G, 0
	},

#if 0
	// These entries should now be identical to the first T7G Mac entry after
	// changing the app to only use the data part of the resource fork. They
	// are left disabled here as a reference.

	// The 7th Guest Mac English (Aztec single disc)
	{
		{
			"t7g", "",
			AD_ENTRY1s("T7GMac", "6bdee8d0f9eef6d58d02fcd7deec3fb2", 1830783),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_MACRESFORK,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_T7G_FAST_MOVIE_SPEED)
		},
		kGroovieT7G, 0
	},

	// The 7th Guest Mac English (Aztec bundle, provided by Thefinaleofseem)
	{
		{
			"t7g", "",
			AD_ENTRY1s("T7GMac", "0d595d4b44ae1814082938d051e5174e", 1830783),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_MACRESFORK,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_T7G_FAST_MOVIE_SPEED)
		},
		kGroovieT7G, 0
	},
#endif

	// The 7th Guest DOS Russian (Akella)
	{
		{
			"t7g", "",
			{
				{ "script.grv", 0, "d1b8033b40aa67c076039881eccce90d", 16659},
				{ "intro.gjd", 0, NULL, 31711554},
				AD_LISTEND
			},
			Common::RU_RUS, Common::kPlatformDOS, ADGF_NO_FLAGS,
			GUIO5(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_T7G_FAST_MOVIE_SPEED)
		},
		kGroovieT7G, 0
	},

	{
		{
			"t7g", "",
			{
				{ "script.grv", 0, "d1b8033b40aa67c076039881eccce90d", 16659},
				{ "SeventhGuest", 0, NULL, -1},
				AD_LISTEND
			},
			Common::EN_ANY, Common::kPlatformIOS, ADGF_NO_FLAGS,
			GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_T7G_FAST_MOVIE_SPEED)
		},
		kGroovieT7G, 0
	},

	// The 11th Hour DOS English
	{
		{
			"11h", "",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// The 11th Hour Windows English
	{
		{
			"11h", "",
			AD_ENTRY1s("disk.1", "4c1d0549f544f052fba2b7a9aebd1077", 220),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// The 11th Hour Macintosh English
	{
		{
			"11h", "",
			AD_ENTRY2s("disk.1",					"5c0428cd3659fc7bbcd0aa16485ed5da", 227,
					   "The 11th Hour Installer",	"bcdb4040b27f15b18f39fb9e496d384a", 1002987),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// The 11th Hour Macintosh English (Installed)
	{
		{
			"11h", "Installed",
			AD_ENTRY2s("disk.1",	"5c0428cd3659fc7bbcd0aa16485ed5da", 227,
					   "el01.mov",	"70f42dfc25b1488a08011dc45bb5145d", 6039),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// The 11th Hour DOS Demo English
	{
		{
			"11h", "Demo",
			AD_ENTRY1s("disk.1", "aacb32ce07e0df2894bd83a3dee40c12", 70),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO5(GUIO_NOLAUNCHLOAD, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// The Making of The 11th Hour DOS English
	{
		{
			"11h", "Making Of",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT)
		},
		kGroovieV2, 2
	},

	// The Making of The 11th Hour Macintosh English
	{
		{
			"11h", "Making Of",
			AD_ENTRY2s("disk.1",				  "5c0428cd3659fc7bbcd0aa16485ed5da", 227,
					   "The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 2
	},

	// The Making of The 11th Hour Macintosh English (Installed)
	{
		{
			"11h", "Making Of (Installed)",
			AD_ENTRY2s("disk.1",	"5c0428cd3659fc7bbcd0aa16485ed5da", 227,
					   "el01.mov",	"70f42dfc25b1488a08011dc45bb5145d", 6039),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 2
	},

	// Clandestiny Trailer DOS English
	{
		{
			"clandestiny", "Trailer",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT)
		},
		kGroovieV2, 3
	},

	// Clandestiny Trailer Macintosh English
	{
		{
			"clandestiny", "Trailer",
			AD_ENTRY2s("disk.1",				  "5c0428cd3659fc7bbcd0aa16485ed5da", 227,
					   "The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 3
	},

	// Clandestiny Trailer Macintosh English (Installed)
	{
		{
			"clandestiny", "Trailer (Installed)",
			AD_ENTRY2s("disk.1",	"5c0428cd3659fc7bbcd0aa16485ed5da", 227,
					   "el01.mov",	"70f42dfc25b1488a08011dc45bb5145d", 6039),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 3
	},

	// Clandestiny DOS English
	{
		{
			"clandestiny", "",
			AD_ENTRY1s("disk.1", "f79fc1515174540fef6a34132efc4c53", 76),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// Uncle Henry's Playhouse PC English
	{
		{
			"unclehenry", "",
			AD_ENTRY1s("disk.1", "0e1b1d3cecc4fc7efa62a968844d1f7a", 72),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// Tender Loving Care PC English
	{
		{
			"tlc", "",
			AD_ENTRY1s("disk.1", "32a1afa68478f1f9d2b25eeea427f2e3", 84),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	{AD_TABLE_END_MARKER, kGroovieT7G, 0}
};

static const char *directoryGlobs[] = {
	"MIDI",
	0
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_T7G_FAST_MOVIE_SPEED,
		{
			_s("Fast movie speed"),
			_s("Play movies at an increased speed"),
			"fast_movie_speed",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class GroovieMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	GroovieMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(GroovieGameDescription), groovieGames, optionsList) {
		// Use kADFlagUseExtraAsHint in order to distinguish the 11th hour from
		// its "Making of" as well as the Clandestiny Trailer; they all share
		// the same MD5.
		// TODO: Is this the only reason, or are there others (like the three
		// potentially sharing a single directory) ? In the former case, then
		// perhaps a better solution would be to add additional files
		// to the detection entries. In the latter case, this TODO should be
		// replaced with an according explanation.
		_flags = kADFlagUseExtraAsHint;
		_guiOptions = GUIO3(GUIO_NOSUBTITLES, GUIO_NOSFX, GUIO_NOASPECT);

		// Need MIDI directory to detect 11H Mac Installed
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "groovie";
	}

	const char *getName() const override {
		return "Groovie";
	}

	const char *getOriginalCopyright() const override {
		return "Groovie Engine (C) 1990-1996 Trilobyte";
	}
};

} // End of namespace Groovie

REGISTER_PLUGIN_STATIC(GROOVIE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Groovie::GroovieMetaEngineDetection);
