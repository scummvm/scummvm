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

#include "cge/fileio.h"

namespace CGE {

#define GAMEOPTION_COLOR_BLIND_DEFAULT_OFF  GUIO_GAMEOPTIONS1

static const PlainGameDescriptor CGEGames[] = {
	{ "soltys", "Soltys" },
	{ 0, 0 }
};

static const ADGameDescription gameDescriptions[] = {
	{
		"soltys", "Freeware",
		AD_ENTRY2s("vol.cat", "0c33e2c304821a2444d297fc5e2d67c6", 50176,
				   "vol.dat", "f9ae2e7f8f7cac91378cdafca43faf1e", 8437676),
		Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
	},
	{
		"soltys", _s("Missing game code"),
		AD_ENTRY2s("vol.cat", "1e077c8ff58109a187f07ac54b0c873a", 18788,
				   "vol.dat", "75d385a6074c58b69f7730481f256051", 1796710),
		Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
	},
	{
		"soltys", _s("Missing game code"),
		AD_ENTRY2s("vol.cat", "f17987487fab1ebddd781d8d02fedecc", 7168,
				   "vol.dat", "c5d9b15863cab61dc125551576dece04", 1075272),
		Common::PL_POL, Common::kPlatformDOS, ADGF_DEMO | ADGF_UNSUPPORTED , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
	},
	{
		"soltys", "Freeware v1.0",
		AD_ENTRY2s("vol.cat", "f1675684c68ab90272f5776f8f2c3974", 50176,
				   "vol.dat", "4ffeff4abc99ac5999b55ccfc56ab1df", 8430868),
		Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
	},
	{
		"soltys", "Freeware v1.0",
		AD_ENTRY2s("vol.cat", "20fdce799adb618100ef9ee2362be875", 50176,
				   "vol.dat", "0e43331c846094d77f5dd201827e0a3b", 8439339),
		Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
	},
	{
		"soltys", "Freeware v1.0",
		AD_ENTRY2s("vol.cat", "fcae86b20eaa5cedec17b24fa5e85eb4", 50176,
				   "vol.dat", "ff10d54acc2c95696c57e05819b6906f", 8450151),
		Common::ES_ESP, Common::kPlatformDOS, ADGF_NO_FLAGS , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
	},

	{
		"soltys", "Russian fan-translation v1.0",
		AD_ENTRY2s("vol.cat", "e9f6069c9c4d4156a8dbe004a9c1960d", 50176,
				   "vol.dat", "f9ae2e7f8f7cac91378cdafca43faf1e", 8749623),
		Common::RU_RUS, Common::kPlatformDOS, ADGF_NO_FLAGS , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
	},

	{
		"soltys", "Russian fan-translation v1.1",
		AD_ENTRY2s("vol.cat", "72a93fa5b81f29aaf362f1dc8a5956e0", 50176,
				   "vol.dat", "b93e053cabf8dfdcece3de59a8e2f9e1", 8739481),
		Common::RU_RUS, Common::kPlatformDOS, ADGF_NO_FLAGS , GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
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
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class CGEMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	CGEMetaEngineDetection() : AdvancedMetaEngineDetection(CGE::gameDescriptions, sizeof(ADGameDescription), CGEGames, optionsList) {
	}

	const char *getEngineId() const override {
		return "cge";
	}

	const char *getName() const override {
		return "CGE";
	}

	const char *getOriginalCopyright() const override {
		return "Soltys (C) 1994-1996 L.K. Avalon";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override;
};

static ADGameDescription s_fallbackDesc = {
	"soltys",
	"Unknown version",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformDOS,
	ADGF_NO_FLAGS,
	GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &s_fallbackDesc, { "vol.cat", "vol.dat", 0 } },
	{ 0, { 0 } }
};

ADDetectedGame CGEMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	ADDetectedGame game = detectGameFilebased(allFiles, CGE::fileBasedFallback);

	if (!game.desc)
		return ADDetectedGame();

	SearchMan.addDirectory("CGEMetaEngineDetection::fallbackDetect", fslist.begin()->getParent());
	ResourceManager *resman;
	resman = new ResourceManager();
	bool sayFileFound = resman->exist("CGE.SAY");
	delete resman;

	SearchMan.remove("CGEMetaEngineDetection::fallbackDetect");

	if (!sayFileFound)
		return ADDetectedGame();

	return game;
}

} // End of namespace CGE

REGISTER_PLUGIN_STATIC(CGE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CGE::CGEMetaEngineDetection);
