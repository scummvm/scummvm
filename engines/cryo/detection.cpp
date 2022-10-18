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

#include "base/plugins.h"

#include "engines/advancedDetector.h"


static const PlainGameDescriptor cryoGames[] = {
	{"losteden", "Lost Eden"},
	{nullptr, nullptr}
};

namespace Cryo {

static const ADGameDescription gameDescriptions[] = {

	// Lost Eden PC non-interactive demo version
	// Probably not worth it
	{
		"losteden",
		nullptr,
		AD_ENTRY1s("EDEN6.HSQ", "00b43c44cf2ac50b1a45dfad5fa5360d", 17093),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// Lost Eden PC interactive demo version
	{
		"losteden",
		nullptr,
		AD_ENTRY1s("EDEN.DAT", nullptr, 205473728),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// Lost Eden PC version
	{
		"losteden",
		nullptr,
		AD_ENTRY1s("EDEN.DAT", nullptr, 449853776),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// Lost Eden EN PC version
	// Added by Strangerke
	{
		"losteden",
		nullptr,
		AD_ENTRY1s("EDEN.DAT", "2126f14fe38b47c7a132f7937c79a2f0", 451205552),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// Lost Eden FR PC version
	// Added by Strangerke
	{
		"losteden",
		nullptr,
		AD_ENTRY1s("EDEN.DAT", "378b1260ac400ecf35f8843357adcca6", 448040496),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// Lost Eden DE PC version
	{
		"losteden",
		nullptr,
		AD_ENTRY1s("EDEN.DAT", nullptr, 457719104),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// Lost Eden Mac version
	{
		"losteden",
		nullptr,
		AD_ENTRY1s("EDEN.DAT", nullptr, 489739536),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Cryo

class CryoMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	CryoMetaEngineDetection() : AdvancedMetaEngineDetection(Cryo::gameDescriptions, sizeof(ADGameDescription), cryoGames) {
	}

	const char *getName() const override {
		return "cryo";
	}

	const char *getEngineName() const override {
		return "Cryo";
	}

	const char *getOriginalCopyright() const override {
		return "Cryo Engine (C) Cryo Interactive";
	}
};

REGISTER_PLUGIN_STATIC(CRYO_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CryoMetaEngineDetection);
