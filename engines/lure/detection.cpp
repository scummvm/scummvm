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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "lure/detection.h"

static const PlainGameDescriptor lureGames[] = {
	{"lure", "Lure of the Temptress"},
	{0, 0}
};


#ifdef USE_TTS
#define GAMEOPTION_TTS_NARRATOR 	GUIO_GAMEOPTIONS1

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_TTS_NARRATOR,
		{
			_s("TTS Narrator"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_narrator",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

#endif

namespace Lure {

static const LureGameDescription gameDescriptions[] = {
	{
		{
			"lure",
			"VGA",
			AD_ENTRY1("disk1.vga", "b2a8aa6d7865813a17a3c636e063572e"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
#ifdef USE_TTS
			GUIO1(GAMEOPTION_TTS_NARRATOR)
#else
			GUIO0()
#endif
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"EGA",
			AD_ENTRY1("disk1.ega", "e9c9fdd8a19f7910d68e53cb84651273"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
#ifdef USE_TTS
			GUIO1(GAMEOPTION_TTS_NARRATOR)
#else
			GUIO0()
#endif
		},
		GF_FLOPPY | GF_EGA,
	},

	{
		{
			"lure",
			"VGA",
			AD_ENTRY1("disk1.vga", "cf69d5ada228dd74f89046691c16aafb"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"EGA",
			AD_ENTRY1("disk1.ega", "b80aced0321f64c58df2c7d3d74dfe79"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY | GF_EGA,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "7aa19e444dab1ac7194d9f7a40ffe54a"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "894a2c2caeccbad2fc2f4a79a8ee47b0"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "1c94475c1bb7e0e88c1757d3b5377e94"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "1751145b653959f7a64fe1618d6b97ac"),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	// Russian OG Edition v1.0
	{
		{
			"lure",
			"1.0",
			AD_ENTRY1("disk1.vga", "04cdcaa9f0cadca492f7aff0c8adfe06"),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	// Russian OG Edition v1.1
	{
		{
			"lure",
			"1.1",
			AD_ENTRY1("disk1.vga", "3f27adff8e8b279f12aaf3d808e84f02"),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},


	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Lure

class LureMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	LureMetaEngineDetection() : AdvancedMetaEngineDetection(Lure::gameDescriptions, sizeof(Lure::LureGameDescription), lureGames
#ifdef USE_TTS
			, optionsList
#endif
			) {
		_md5Bytes = 1024;

		// Use kADFlagUseExtraAsHint to distinguish between EGA and VGA versions
		// of italian Lure when their datafiles sit in the same directory.
		_flags = kADFlagUseExtraAsHint;
		_guiOptions = GUIO1(GUIO_NOSPEECH);
	}

	const char *getEngineId() const override {
		return "lure";
	}

	const char *getName() const override {
		return "Lure of the Temptress";
	}

	const char *getOriginalCopyright() const override {
		return "Lure of the Temptress (C) Revolution";
	}
};

REGISTER_PLUGIN_STATIC(LURE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, LureMetaEngineDetection);
