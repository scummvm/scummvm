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

#include "engines/advancedDetector.h"
#include "engines/myst3/detection.h"
#include "engines/myst3/myst3.h"


namespace Myst3 {

static const PlainGameDescriptor myst3Games[] = {
	{ "myst3", "Myst III: Exile" },
	{ nullptr, nullptr }
};

static const DebugChannelDef debugFlagList[] = {
	{Myst3::kDebugVariable, "Variable", "Track Variable Accesses"},
	{Myst3::kDebugSaveLoad, "SaveLoad", "Track Save/Load Function"},
	{Myst3::kDebugScript, "Script", "Track Script Execution"},
	{Myst3::kDebugNode, "Node", "Track Node Changes"},
	DEBUG_CHANNEL_END
};

static const char *const directoryGlobs[] = {
	"bin",
	"M3Data",
	"MYST3BIN",
	"TEXT",
	"Myst III Spanish", // For the DVD original layout
	"Dutch",            // For the CD multilingual original layout
	"Spanish",
	nullptr
};

#define MYST3ENTRY(lang, langFile, md5lang, extra, flags)                       \
{                                                                               \
	{                                                                           \
		"myst3",                                                                \
		extra,                                                                  \
		{                                                                       \
			{ "RSRC.m3r", 0, "a2c8ed69800f60bf5667e5c76a88e481", 1223862 },     \
			{ langFile, 0, md5lang, AD_NO_SIZE },                                       \
		},                                                                      \
		lang,                                                                   \
		Common::kPlatformWindows,                                               \
		ADGF_NO_FLAGS,                                                          \
		GUIO_NONE                                                               \
	},                                                                          \
	flags                                                                       \
},

#define MYST3ENTRY_DVD(lang, langFile, md5lang, extra, flags)                   \
{                                                                               \
	{                                                                           \
		"myst3",                                                                \
		extra,                                                                  \
		{                                                                       \
			{ "RSRC.m3r",    0, "a2c8ed69800f60bf5667e5c76a88e481", 1223862 },  \
			{ "ENGLISH.m3t", 0, "74726de866c0594d3f2a05ff754c973d", 3407120 },  \
			{ langFile, 0, md5lang, AD_NO_SIZE },                                       \
		},                                                                      \
		lang,                                                                   \
		Common::kPlatformWindows,                                               \
		ADGF_NO_FLAGS,                                                          \
		GUIO_NONE                                                               \
	},                                                                          \
	flags                                                                       \
},

#define MYST3ENTRY_XBOX(lang, langFile, md5lang)                                \
{                                                                               \
	{                                                                           \
		"myst3",                                                                \
		0,                                                                      \
		{                                                                       \
			{ "RSRC.m3r", 0, "3de23eb5a036a62819186105478f9dde", 1226192 },     \
			{ langFile, 0, md5lang, AD_NO_SIZE },                                       \
		},                                                                      \
		lang,                                                                   \
		Common::kPlatformXbox,                                                  \
		ADGF_UNSTABLE,                                                          \
		GUIO_NONE                                                               \
	},                                                                          \
	kLocMulti6                                                                  \
},


static const Myst3GameDescription gameDescriptions[] = {
	// Initial US release (English only) v1.0
	MYST3ENTRY(Common::EN_ANY, "ENGLISH.m3t",  "19dcba1074f235ec2119313242d891de", nullptr, kLocMonolingual)

	// Initial US release (English only) v1.22
	MYST3ENTRY(Common::EN_ANY, "ENGLISH.m3t",  "3ca92b097c4319a2ace7fd6e911d6b0f", nullptr, kLocMonolingual)

	// European releases (Country language + English) (1.2)
	MYST3ENTRY(Common::NL_NLD, "DUTCH.m3u",    "0e8019cfaeb58c2de00ac114cf122220", nullptr, kLocMulti2)
	MYST3ENTRY(Common::FR_FRA, "FRENCH.m3u",   "3a7e270c686806dfc31c2091e09c03ec", nullptr, kLocMulti2)
	MYST3ENTRY(Common::DE_DEU, "GERMAN.m3u",   "1b2fa162a951fa4ed65617dd3f0c8a53", nullptr, kLocMulti2) // #1323, andrews05
	MYST3ENTRY(Common::IT_ITA, "ITALIAN.m3u",  "906645a87ac1cbbd2b88c277c2b4fda2", nullptr, kLocMulti2) // #1323, andrews05
	MYST3ENTRY(Common::ES_ESP, "SPANISH.m3u",  "28003569d9536cbdf6020aee8e9bcd15", nullptr, kLocMulti2) // #1323, goodoldgeorge
	MYST3ENTRY(Common::PL_POL, "POLISH.m3u",   "8075e4e822e100ec79a5842a530dbe24", nullptr, kLocMulti2)

	// Russian release (Russian only) (1.2)
	MYST3ENTRY(Common::RU_RUS, "ENGLISH.m3t",  "57d36d8610043fda554a0708d71d2681", nullptr, kLocMonolingual)

	// Hebrew release (Hebrew only) (1.2 - Patched using the patch CD)
	MYST3ENTRY(Common::HE_ISR, "HEBREW.m3u",   "16fbbe420fed366249a8d44a759f966c", nullptr, kLocMonolingual) // #1348, BLooperZ

	// Japanese release (1.2)
	MYST3ENTRY(Common::JA_JPN, "JAPANESE.m3u", "21bbd040bcfadd13b9dc84360c3de01d", nullptr, kLocMulti2)
	MYST3ENTRY(Common::JA_JPN, "JAPANESE.m3u", "1e7c3156417978a1187fa6bc0e2cfafc", "Subtitles only", kLocMulti2)

	// Multilingual CD release (1.21)
	MYST3ENTRY(Common::EN_ANY, "ENGLISH.m3u",  "b62ca55aa17724cddbbcc78cba988337", nullptr, kLocMulti6)
	MYST3ENTRY(Common::FR_FRA, "FRENCH.m3u",   "73519070cba1c7bea599adbddeae304f", nullptr, kLocMulti6)
	MYST3ENTRY(Common::NL_NLD, "DUTCH.m3u",    "c4a8d8fb0eb3fecb9c435a8517bc1f9a", nullptr, kLocMulti6)
	MYST3ENTRY(Common::DE_DEU, "GERMAN.m3u",   "5b3be343dd20f03ebdf16381b873f035", nullptr, kLocMulti6)
	MYST3ENTRY(Common::IT_ITA, "ITALIAN.m3u",  "73db43aac3fe8671e2c4e227977fbb61", nullptr, kLocMulti6)
	MYST3ENTRY(Common::ES_ESP, "SPANISH.m3u",  "55ceb165dad02211ef2d25946c3aac8e", nullptr, kLocMulti6)

	// Multilingual CD release (1.21, original layout)
	{
		{
			"myst3",
			nullptr,
			{
				{ "RSRC.m3r",     0, "a2c8ed69800f60bf5667e5c76a88e481", 1223862 },
				{ "SPANISH.m3u",  0, "55ceb165dad02211ef2d25946c3aac8e", 2604702 }, // Use the spanish language file
				{ "DUTCH.m3u",    0, "c4a8d8fb0eb3fecb9c435a8517bc1f9a", 2607925 }, // and the dutch one to exclude Multi2 versions
			},
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kLocMulti6 | kLayoutCD
	},

	{
		// Chinese (Simplified) CD release (1.22LC)
		{
			"myst3",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED, // Lacks OVER101.m3o file
			{
				{ "RSRC.m3r",      0, "a2c8ed69800f60bf5667e5c76a88e481", 1223862 },
				{ "localized.m3t", 0, "3a9f299f8d061ce3d2862d985edb84e3", 2341588 },
				{ "ENGLISHjp.m3t", 0, "19dcba1074f235ec2119313242d891de", 5658925 },
			},
			Common::ZH_CHN,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO_NONE
		},
		kLocMulti2 // CHS, English
	},

	// Japanese DVD release (1.24) (TRAC report #14298)
	MYST3ENTRY(Common::JA_JPN, "JAPANESE.m3u", "5c18c9c124ff92d2b95ae5d128228f7b", "DVD", kLocMulti2)

	// DVD releases (1.27)
	MYST3ENTRY_DVD(Common::EN_ANY, "ENGLISH.m3u",  "e200b416f43e70fee76148a80d195d5c", "DVD", kLocMulti6)
	MYST3ENTRY_DVD(Common::FR_FRA, "FRENCH.m3u",   "5679ce65c5e9af8899835ef9af398f1a", "DVD", kLocMulti6)
	MYST3ENTRY_DVD(Common::NL_NLD, "DUTCH.m3u",    "2997afdb4306c573153fdbb391ed2fff", "DVD", kLocMulti6)
	MYST3ENTRY_DVD(Common::DE_DEU, "GERMAN.m3u",   "09f32e6ceb414463e8fc22ca1a9564d3", "DVD", kLocMulti6)
	MYST3ENTRY_DVD(Common::IT_ITA, "ITALIAN.m3u",  "51fb02f6bf37dde811d7cde648365260", "DVD", kLocMulti6)
	MYST3ENTRY_DVD(Common::ES_ESP, "SPANISH.m3u",  "e27e610fe8ce35223a3239ff170a85ec", "DVD", kLocMulti6)

	// DVD release (1.27, original layout)
	{
		{
			"myst3",
			"DVD",
			{
				{ "RSRC.m3r",     0, "a2c8ed69800f60bf5667e5c76a88e481", 1223862 },
				{ "ENGLISH.m3t",  0, "74726de866c0594d3f2a05ff754c973d", 3407120 },
				{ "language.m3u", 0, "e27e610fe8ce35223a3239ff170a85ec", 2604318 }, // Use the spanish language file
			},
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		kLocMulti6 | kLayoutDVD
	},

	// Myst 3 Xbox (PAL)
	MYST3ENTRY_XBOX(Common::EN_ANY, "ENGLISHX.m3t", "c4d012ab02b8ca7d0c7e79f4dbd4e676")
	MYST3ENTRY_XBOX(Common::FR_FRA, "FRENCHX.m3t",  "94c9dcdec8794751e4d773776552751a")
	MYST3ENTRY_XBOX(Common::DE_DEU, "GERMANX.m3t",  "b9b66fcd5d4fbb95ac2d7157577991a5")
	MYST3ENTRY_XBOX(Common::IT_ITA, "ITALIANX.m3t", "3ca266019eba68123f6b7cae57cfc200")
	MYST3ENTRY_XBOX(Common::ES_ESP, "SPANISHX.m3t", "a9aca36ccf6709164249f3fb6b1ef148")

	// Myst 3 Xbox (RUS)
	MYST3ENTRY_XBOX(Common::RU_RUS, "ENGLISHX.m3t", "18cb50f5c5317586a128ca9eb3e03279")

	{
		// Myst 3 PS2 (NTSC-U/C)
		{
			"myst3",
			_s("PS2 version is not yet supported"),
			AD_ENTRY1s("RSRC.m3r", "c60d37bfd3bb8b0bee143018447bb460", 346618151),
			Common::UNK_LANG,
			Common::kPlatformPS2,
			ADGF_UNSUPPORTED,
			GUIO_NONE
		},
		0
	},

	{
		// Myst 3 PS2 (PAL)
		{
			"myst3",
			_s("PS2 version is not yet supported"),
			AD_ENTRY1s("RSRC.m3r", "f0e0c502f77157e6b5272686c661ea75", 91371793),
			Common::UNK_LANG,
			Common::kPlatformPS2,
			ADGF_UNSUPPORTED,
			GUIO_NONE
		},
		0
	},

	{ AD_TABLE_END_MARKER, 0 }
};

class Myst3MetaEngineDetection : public AdvancedMetaEngineDetection<Myst3GameDescription> {
public:
	Myst3MetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, myst3Games) {
		_guiOptions = GUIO5(GUIO_NOMIDI, GUIO_NOSFX, GUIO_NOSPEECH, GUIO_NOSUBTITLES, GAMEOPTION_WIDESCREEN_MOD);
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineName() const override {
		return "Myst III";
	}

	const char *getName() const override {
		return "myst3";
	}

	const char *getOriginalCopyright() const override {
		return "Myst III Exile (C) Presto Studios";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override {
		DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame);

		// The AdvancedDetector model only allows specifying a single supported
		// game language. The 10th anniversary edition Myst III is multilanguage.
		// Here we amend the detected games to set the list of supported languages.
		if ((reinterpret_cast<const Myst3::Myst3GameDescription *>(
				adGame.desc)->flags & Myst3::kGameLayoutTypeMask) != Myst3::kLayoutFlattened) {
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::EN_ANY));
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::DE_DEU));
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::ES_ESP));
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::FR_FRA));
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::IT_ITA));
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::IT_ITA));
			game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::NL_NLD));
		}

		return game;
	}

};

} // End of namespace Myst3

REGISTER_PLUGIN_STATIC(MYST3_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Myst3::Myst3MetaEngineDetection);
