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

#include "cruise/detection.h"

static const PlainGameDescriptor cruiseGames[] = {
	{"cruise", "Cruise for a Corpse"},
	{0, 0}
};

namespace Cruise {

static const CRUISEGameDescription gameDescriptions[] = {
	{
		{
			"cruise",
			"16 colors",
			AD_ENTRY1("D1", "cd29a4cd9162076e9a18495fe56a48f3"),
			Common::EN_GRB,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			"16 colors",
			AD_ENTRY1("D1", "41a7a4d426dbd048eb369cfee4bb2717"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "a90d2b9ead6b4d812cd14268672cf178"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "e258865807ea31b2d523340e6f0a606b"),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			"16 colors",
			AD_ENTRY1("D1", "287d2ec1799e2f881dee23c70be96e81"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "f2a26522d49983c4ae32bcccbb801b02"),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "70f42a21cc257b01d58667853335f4f1"),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{ // Amiga English US GOLD edition.
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "de084e9d2c6e4b2cc14803bf849eda3e"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{ // Amiga Italian US GOLD edition.
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "a0011075413b7335e003e8e3c9cf51b9"),
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{ // AtariST English KixxXL edition.
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "be78614d5fa34bdb68bb03a2a6130280"),
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "e19a4ab2e24a69087e4ea994a5506231"),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "9a302ada55600d96061fda1d63a6ccda"),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},
	{AD_TABLE_END_MARKER}
};

}

class CruiseMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	CruiseMetaEngineDetection() : AdvancedMetaEngineDetection(Cruise::gameDescriptions, sizeof(Cruise::CRUISEGameDescription), cruiseGames) {
		_guiOptions = GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI);
	}

	const char *getEngineId() const override {
		return "cruise";
	}

	const char *getName() const override {
		return "Cinematique evo 2";
	}

	const char *getOriginalCopyright() const override {
		return "Cinematique evo 2 (C) Delphine Software";
	}
};

REGISTER_PLUGIN_STATIC(CRUISE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CruiseMetaEngineDetection);
