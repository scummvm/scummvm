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

#include "common/translation.h"

namespace Sherlock {

static const SherlockGameDescription gameDescriptions[] = {
	{
		// Case of the Serrated Scalpel - English 3.5" Floppy
		// The HitSquad CD version has the same MD5
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "ad0c4d6865edf15da4e9204c08815875", 238928),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
#ifdef USE_TTS
			GUIO7(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE,
			      GAMEOPTION_HELP_STYLE,
			      GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE, GAMEOPTION_TTS_NARRATOR)
#else
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE,
			      GAMEOPTION_HELP_STYLE,
			      GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
#endif
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - French
		// Provided by InsertDisk2 - Patch #11040
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "295ef96a50641d44f44c02c90cf7948e", 259541),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - German CD (from multilingual CD)
		// Provided by m_kiewitz
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "40a5f9f37c0e0d2ad48d8f44d8e393c9", 284278),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - German
		// Provided by mgerhardy
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "44652e54172e13b1b075b1ef7d89de24", 284043),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - German Floppy
		// Provided by max565
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "3d813fd8505b391a1f8b3a16b1aa7f2e", 284195),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Spanish CD (from multilingual CD)
		// Provided by m_kiewitz
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "27697804b637a7f3b77234bf16f15dce", 171419),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Spanish 3.5" Floppy
		// Provided by lorz on bug #11283
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "27697804b637a7f3b77234bf16f15dce", 171419),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Italian fan translation (v1.0a)
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "1a15e358c0ac7bd81c0b86276f2cd1a2", 247606),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Italian fan translation (v2.0)
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "1a547f96476a3a50cf6f0d599a4f5bcc", 247406),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,
				GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - English 3DO
		{
			"scalpel",
			0,
			AD_ENTRY1s("talk.lib", "20f74a29f2db6475e85b029ac9fc03bc", 240610),
			Common::EN_ANY,
			Common::kPlatform3DO,
			ADGF_UNSTABLE,
			GUIO4(GAMEOPTION_FADE_STYLE, GAMEOPTION_HELP_STYLE,GAMEOPTION_PORTRAITS_ON, GAMEOPTION_WINDOW_STYLE)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Interactive English Demo
		// Provided by Strangerke
		{
			"scalpel",
			"Interactive Demo",
			AD_ENTRY1s("talk.lib", "dbdc8a20c96900aa7e4d02f3fe8a274c", 121102),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Non-Interactive English Demo
		// Provided by Strangerke
		{
			"scalpel",
			"Non-Interactive Demo",
			AD_ENTRY1s("music.lib", "ec19a09b7fef6fd90b1ab812ce6e9739", 38563),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Serrated Scalpel - Traditional Chinese
		// Provided by AquariumTroop
		{
			"scalpel",
			nullptr,
			AD_ENTRY1s("talk.lib", "334c7d468860f20eafbcd002891f0c6b", 173935),
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_SerratedScalpel,
	},

	{
		// Case of the Rose Tattoo - French CD
		// Provided by Strangerke
		{
			"rosetattoo",
			"CD",
			AD_ENTRY1s("talk.lib", "22e8e6406dd2fbbb238c9898928df42e", 770756),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_HELP_STYLE, GAMEOPTION_TRANSPARENT_WINDOWS)
		},
		GType_RoseTattoo
	},

	{
		// Case of the Rose Tattoo - English CD
		// Provided by dreammaster
		{
			"rosetattoo",
			"CD",
			AD_ENTRY1s("talk.lib", "9639a756b0993ebd71cb5f4d8b78b2dc", 765134),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_HELP_STYLE, GAMEOPTION_TRANSPARENT_WINDOWS)
		},
		GType_RoseTattoo,
	},

	{
		// Case of the Rose Tattoo - Russian CD
		{
			"rosetattoo",
			"CD",
			/* talk.lib is actually different from English but first 5000 bytes and sizes are identical.  */
			AD_ENTRY2s("talk.lib", "9639a756b0993ebd71cb5f4d8b78b2dc", 765134,
				   "font.vgs", "t:11a7df2e63cfc6e48415f62c3ec291d5", 8959),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_HELP_STYLE, GAMEOPTION_TRANSPARENT_WINDOWS)
		},
		GType_RoseTattoo,
	},

	{
		// Case of the Rose Tattoo - German CD
		// Provided by m_kiewitz
		{
			"rosetattoo",
			"CD",
			AD_ENTRY1s("talk.lib", "5027aa72f0d263ed3b1c764a6c397911", 873864),
  			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_HELP_STYLE, GAMEOPTION_TRANSPARENT_WINDOWS)
		},
		GType_RoseTattoo,
	},

	{
		// Case of the Rose Tattoo - Spanish CD
		// Provided by dianiu
		{
			"rosetattoo",
			"CD",
			AD_ENTRY1s("talk.lib", "4f3ccf50e1012445624569cd605d7449", 783713),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_HELP_STYLE, GAMEOPTION_TRANSPARENT_WINDOWS)
		},
		GType_RoseTattoo,
	},

	{
		// Case of the Rose Tattoo - Chinese CD
		{
			"rosetattoo",
			"CD",
			AD_ENTRY1s("talk.lib", "dfd699efdce02a045f46d15da2d44b76", 582381),
			Common::ZH_CHN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_ORIGINAL_SAVES, GAMEOPTION_HELP_STYLE, GAMEOPTION_TRANSPARENT_WINDOWS)
		},
		GType_RoseTattoo,
	},

	{ AD_TABLE_END_MARKER, (GameType)0 }
};

} // End of namespace Sherlock
