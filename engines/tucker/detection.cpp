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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "tucker/detection.h"

static const PlainGameDescriptor tuckerGames[] = {
	{ "tucker", "Bud Tucker in Double Trouble" },
	{ nullptr,  nullptr }
};

static const ADGameDescription tuckerGameDescriptions[] = {
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "f1e42a95972643462b9c3c2ea79d6683", 543),
		Common::FR_FRA,
		Common::kPlatformDOS,
		Tucker::kGameFlagNoSubtitles,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "9c1ddeafc5283b90d1a284bd0924831c", 462),
		Common::EN_ANY,
		Common::kPlatformDOS,
		Tucker::kGameFlagEncodedData,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "1b3ea79d8528ea3c7df83dd0ed345e37", 525),
		Common::ES_ESP,
		Common::kPlatformDOS,
		Tucker::kGameFlagEncodedData,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobrgr.txt", "4df9eb65722418d1a1723508115b146c", 552),
		Common::DE_DEU,
		Common::kPlatformDOS,
		Tucker::kGameFlagEncodedData,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "5f85285bbc23ce57cbc164021ee1f23c", 525),
		Common::PL_POL,
		Common::kPlatformDOS,
		0,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "e548994877ff31ca304f6352ce022a8e", 497),
		Common::CZ_CZE,
		Common::kPlatformDOS,
		Tucker::kGameFlagEncodedData,
		GUIO1(GUIO_NOMIDI)
	},
	{ // Russian fan translation
		"tucker",
		"",
		AD_ENTRY1s("infobrgr.txt", "4b5a315e449a7f9eaf2025ec87466cd8", 552),
		Common::RU_RUS,
		Common::kPlatformDOS,
		Tucker::kGameFlagEncodedData,
		GUIO1(GUIO_NOMIDI)
	},
	{
		"tucker",
		"Demo",
		AD_ENTRY1s("infobar.txt", "010b055de42097b140d5bcb6e95a5c7c", 203),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO | Tucker::kGameFlagDemo,
		GUIO1(GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};

static const ADGameDescription tuckerDemoGameDescription = {
	"tucker",
	"Non-Interactive Demo",
	AD_ENTRY1(0, 0),
	Common::EN_ANY,
	Common::kPlatformDOS,
	ADGF_DEMO | Tucker::kGameFlagDemo | Tucker::kGameFlagIntroOnly,
	GUIO1(GUIO_NOMIDI)
};

class TuckerMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TuckerMetaEngineDetection() : AdvancedMetaEngineDetection(tuckerGameDescriptions, sizeof(ADGameDescription), tuckerGames) {
		_md5Bytes = 512;
	}

	const char *getEngineId() const override {
		return "tucker";
	}

	const char *getName() const override {
		return "Bud Tucker in Double Trouble";
	}

	const char *getOriginalCopyright() const override {
		return "Bud Tucker in Double Trouble (C) Merit Studios";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const override {
		for (Common::FSList::const_iterator d = fslist.begin(); d != fslist.end(); ++d) {
			Common::FSList audiofslist;
			if (d->isDirectory() && d->getName().equalsIgnoreCase("audio") && d->getChildren(audiofslist, Common::FSNode::kListFilesOnly)) {
				for (Common::FSList::const_iterator f = audiofslist.begin(); f != audiofslist.end(); ++f) {
					if (!f->isDirectory() && f->getName().equalsIgnoreCase("demorolc.raw")) {
						return ADDetectedGame(&tuckerDemoGameDescription);
					}
				}
			}
		}

		return ADDetectedGame();
	}

};

REGISTER_PLUGIN_STATIC(TUCKER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TuckerMetaEngineDetection);
