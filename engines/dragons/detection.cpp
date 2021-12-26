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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "dragons/detection.h"

static const PlainGameDescriptor dragonsGames[] = {
		{ "dragons", "Blazing Dragons" },
		{ nullptr, nullptr }
};

namespace Dragons {

static const DragonsGameDescription gameDescriptions[] = {
	{
			{
					"dragons",
					nullptr,
					AD_ENTRY1s("bigfile.dat", "02c26712bee57266f28235fdc0207725", 44990464),
					Common::EN_USA,
					Common::kPlatformPSX,
					ADGF_DROPPLATFORM,
					GUIO0()
			},
			kGameIdDragons
	},
	{
			{
					"dragons",
					nullptr,
					AD_ENTRY1s("bigfile.dat", "02c26712bee57266f28235fdc0207725", 44992512),
					Common::EN_GRB,
					Common::kPlatformPSX,
					ADGF_DROPPLATFORM,
					GUIO0()
			},
			kGameIdDragons
	},
	{
			{
					"dragons",
					nullptr,
					AD_ENTRY1s("bigfile.dat", "9854fed0d2b48522a62973e99b52a0be", 45099008),
					Common::DE_DEU,
					Common::kPlatformPSX,
					ADGF_DROPPLATFORM,
					GUIO0()
			},
			kGameIdDragons
	},
	{
			{
					"dragons",
					nullptr,
					AD_ENTRY1s("bigfile.dat", "9854fed0d2b48522a62973e99b52a0be", 45107200),
					Common::FR_FRA,
					Common::kPlatformPSX,
					ADGF_DROPPLATFORM,
					GUIO0()
			},
			kGameIdDragons
	},
	// Russian localization by Russian Versions
	{
			{
					"dragons",
					nullptr,
					AD_ENTRY2s("bigfile.dat", "02c26712bee57266f28235fdc0207725", 44990464,
							   "dtspeech.xa", "7f7ace860e5dd3696b51eace20215274", 182138880),
					Common::RU_RUS,
					Common::kPlatformPSX,
					ADGF_DROPPLATFORM,
					GUIO0()
			},
			kGameIdDragons
	},


	// BAD EXTRACTIONS
	{
			{
					"dragons",
					nullptr,
					AD_ENTRY1s("bigfile.dat", "92b938703611789e1a007d6dfac7ef7e", 51668736),
					Common::EN_USA,
					Common::kPlatformPSX,
					ADGF_DROPPLATFORM,
					GUIO0()
			},
			kGameIdDragonsBadExtraction
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Dragons

static const char * const directoryGlobs[] = {
	"resource",
	nullptr
};

class DragonsMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	DragonsMetaEngineDetection() : AdvancedMetaEngineDetection(Dragons::gameDescriptions, sizeof(Dragons::DragonsGameDescription), dragonsGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "dragons";
	}

	const char *getName() const override {
		return "Blazing Dragons";
	}

	const char *getOriginalCopyright() const override {
		return "(C) 1996 The Illusions Gaming Company";
	}
};

REGISTER_PLUGIN_STATIC(DRAGONS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DragonsMetaEngineDetection);
