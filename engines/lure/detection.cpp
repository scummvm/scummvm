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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "base/plugins.h"

#include "common/advancedDetector.h"

#include "lure/lure.h"

namespace Lure {

struct LureGameDescription {
	Common::ADGameDescription desc;

	uint32 features;
};

uint32 LureEngine::getFeatures() const { return _gameDescription->features; }
Common::Language LureEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform LureEngine::getPlatform() const { return _gameDescription->desc.platform; }

} // End of namespace Lure

static const PlainGameDescriptor lureGames[] = {
	{"lure", "Lure of the Temptress"},
	{0, 0}
};


namespace Lure {

static const LureGameDescription gameDescriptions[] = {
	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "b2a8aa6d7865813a17a3c636e063572e"),
			Common::EN_ANY,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "cf69d5ada228dd74f89046691c16aafb"),
			Common::IT_ITA,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "7aa19e444dab1ac7194d9f7a40ffe54a"),
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "894a2c2caeccbad2fc2f4a79a8ee47b0"),
			Common::DE_DEU,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "1c94475c1bb7e0e88c1757d3b5377e94"),
			Common::FR_FRA,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "1751145b653959f7a64fe1618d6b97ac"),
			Common::ES_ESP,
			Common::kPlatformPC,
			Common::ADGF_NO_FLAGS
		},
		GF_FLOPPY,
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Lure

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Lure::gameDescriptions,
	// Size of that superset structure
	sizeof(Lure::LureGameDescription),
	// Number of bytes to compute MD5 sum for
	1024,
	// List of all engine targets
	lureGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"lure",
	// List of files for file-based fallback detection (optional)
	0,
	// Fallback callback
	0,
	// Flags
	Common::kADFlagAugmentPreferredTarget
};

static bool Engine_LURE_createInstance(OSystem *syst, Engine **engine, Common::EncapsulatedADGameDesc encapsulatedDesc) {
	const Lure::LureGameDescription *gd = (const Lure::LureGameDescription *)(encapsulatedDesc.realDesc);
	if (gd) {
		*engine = new Lure::LureEngine(syst, gd);
	}
	return gd != 0;
}

ADVANCED_DETECTOR_DEFINE_PLUGIN(LURE, Engine_LURE_createInstance, detectionParams);

REGISTER_PLUGIN(LURE, "Lure of the Temptress Engine", "Lure of the Temptress (C) Revolution");
