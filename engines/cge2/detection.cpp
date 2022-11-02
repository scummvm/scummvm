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
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#include "engines/advancedDetector.h"
#include "common/translation.h"
#include "cge2/fileio.h"
#include "cge2/cge2.h"

static const DebugChannelDef debugFlagList[] = {
	{CGE2::kCGE2DebugOpcode, "opcode", "CGE2 opcode debug channel"},
	DEBUG_CHANNEL_END
};

namespace CGE2 {

#define GAMEOPTION_COLOR_BLIND_DEFAULT_OFF  GUIO_GAMEOPTIONS1
#define GAMEOPTION_TTS_OBJECTS				GUIO_GAMEOPTIONS2
#define GAMEOPTION_TTS_SPEECH				GUIO_GAMEOPTIONS3

static const PlainGameDescriptor CGE2Games[] = {
		{ "sfinx", "Sfinx" },
		{ nullptr, nullptr }
};

static const ADGameDescription gameDescriptions[] = {
		{
			"sfinx", "Freeware",
			AD_ENTRY2s("vol.cat", "21197b287d397c53261b6616bf0dd880", 129024,
					   "vol.dat", "de14291869a8eb7c2732ab783c7542ef", 34180844),
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO2(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF, GAMEOPTION_TTS_OBJECTS)
		},

		{
			"sfinx", "Freeware v1.0",
			AD_ENTRY2s("vol.cat", "aa402aed24a72c53a4d1211c456b79dd", 129024,
					   "vol.dat", "5966ac26d91d664714349669f9dd09b5", 34180164),
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO2(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF, GAMEOPTION_TTS_OBJECTS)
		},

		{
			"sfinx", "Freeware v1.1",
			AD_ENTRY2s("vol.cat", "aa402aed24a72c53a4d1211c456b79dd", 129024,
					   "vol.dat", "5966ac26d91d664714349669f9dd09b5", 34180367),
			Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO2(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF, GAMEOPTION_TTS_OBJECTS)
		},

		{
			"sfinx", "Demo",
			AD_ENTRY2s("vol.cat", "68271dc56aa37c4e83b792df69625a00", 24576,
					   "vol.dat", "989c26861bc56fafa3a85784e374ca15", 2710971),
			Common::PL_POL, Common::kPlatformDOS, ADGF_DEMO, GUIO3(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF, GAMEOPTION_TTS_OBJECTS, GAMEOPTION_TTS_SPEECH)
		},

		{
			"sfinx", "Freeware v0.3",
			AD_ENTRY2s("vol.cat", "f158e469dccbebc5a632eb848df89779", 129024,
					   "vol.dat", "d40a6b4ae173d6930be54ba56bee15d5", 34183430),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO3(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF, GAMEOPTION_TTS_OBJECTS, GAMEOPTION_TTS_SPEECH)
		},

		{
			"sfinx", "Freeware v1.0",
			AD_ENTRY2s("vol.cat", "f158e469dccbebc5a632eb848df89779", 129024,
					   "vol.dat", "d40a6b4ae173d6930be54ba56bee15d5", 34183443),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO3(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF, GAMEOPTION_TTS_OBJECTS, GAMEOPTION_TTS_SPEECH)
		},

		{
			"sfinx", "Freeware v1.1",
			AD_ENTRY2s("vol.cat", "f158e469dccbebc5a632eb848df89779", 129024,
					   "vol.dat", "d40a6b4ae173d6930be54ba56bee15d5", 34182773),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO3(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF, GAMEOPTION_TTS_OBJECTS, GAMEOPTION_TTS_SPEECH)
		},

		AD_TABLE_END_MARKER
};

static const ADExtraGuiOptionsMap optionsList[] = {
		{
			GAMEOPTION_COLOR_BLIND_DEFAULT_OFF,
			{
				_s("Color Blind Mode"),
				_s("Enable Color Blind Mode by default"),
				"enable_color_blind",
				false,
				0,
				0
			}
		},

#ifdef USE_TTS
	{
		GAMEOPTION_TTS_OBJECTS,
		{
			_s("Enable Text to Speech for Objects and Options"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_enabled_objects",
			false,
			0,
			0
		}
	},

	{
		GAMEOPTION_TTS_SPEECH,
		{
			_s("Enable Text to Speech for Subtitles"),
			_s("Use TTS to read the subtitles (if TTS is available)"),
			"tts_enabled_speech",
			false,
			0,
			0
		}
	},
#endif

		AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CGE2MetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	CGE2MetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(ADGameDescription), CGE2Games, optionsList) {
	}

	const char *getName() const override {
		return "cge2";
	}

	const char *getEngineName() const override {
		return "CGE2";
	}

	const char *getOriginalCopyright() const override {
		return "Sfinx (C) 1994-1997 Janusz B. Wisniewski and L.K. Avalon";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;
};

static ADGameDescription s_fallbackDesc = {
	"sfinx",
	"Unknown version",
	AD_ENTRY1(nullptr, nullptr), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_NO_FLAGS,
	GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &s_fallbackDesc, { "vol.cat", "vol.dat", nullptr } },
	{ nullptr, { nullptr } }
};

// This fallback detection looks identical to the one used for CGE. In fact, the difference resides
// in the ResourceManager which handles a different archive format. The rest of the detection is identical.
ADDetectedGame CGE2MetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	ADDetectedGame game = detectGameFilebased(allFiles, CGE2::fileBasedFallback);

	if (!game.desc)
		return ADDetectedGame();

	SearchMan.addDirectory("CGE2MetaEngineDetection::fallbackDetect", fslist.begin()->getParent());
	ResourceManager *resman;
	resman = new ResourceManager();
	bool sayFileFound = resman->exist("CGE.SAY");
	delete resman;

	SearchMan.remove("CGE2MetaEngineDetection::fallbackDetect");

	if (!sayFileFound)
		return ADDetectedGame();

	return game;
}

} // End of namespace CGE2

REGISTER_PLUGIN_STATIC(CGE2_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CGE2::CGE2MetaEngineDetection);
