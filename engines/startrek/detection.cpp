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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/config-manager.h"
#include "common/file.h"

#include "startrek/detection.h"

static const PlainGameDescriptor starTrekGames[] = {
	{"st25", "Star Trek: 25th Anniversary"},
	{"stjr", "Star Trek: Judgment Rites"},
	{0, 0}
};


namespace StarTrek {

static const StarTrekGameDescription gameDescriptions[] = {
	{ // ST25 DOS CD-ROM edition (EN)
		{
			"st25",
			"CD",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7793814),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		GF_CDROM,
	},

	{ // ST25 DOS CD-ROM edition (EN) #2
		{
			"st25",
			"CD",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7793808),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		GF_CDROM,
	},

	{ // ST25 DOS CD-ROM edition (FR)
		{
			"st25",
			"CD",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7815278),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		GF_CDROM,
	},

	{ // ST25 DOS CD-ROM edition (GER)
		{
			"st25",
			"CD",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7805836),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		GF_CDROM,
	},

	{ // ST25 DOS floppy edition (EN)
		{
			"st25",
			"Floppy",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7222630),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 DOS floppy edition (EN) #2
		{
			"st25",
			"Floppy",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7172238),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 DOS floppy edition (EN) #3 (Interplay's 10 Year Anthology: Classic Collection)
		{
			"st25",
			"Floppy",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7222652),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 DOS floppy edition (GER)
		{
			"st25",
			"Floppy",
			AD_ENTRY1s("data.001", "57040928a0f374281aa86ba4e7db8444", 7181890),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 Mac floppy edition (EN)
		{
			"st25",
			"Floppy",
			AD_ENTRY1s("Star Trek Data", "d95eb00532b7082d53862c906c7ac3dc", 39032),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 DOS floppy demo (EN)
		{
			"st25",
			"Demo",
			AD_ENTRY1s("data.001", "093909a9d3dd3afd207c085a24baf9cf", 2598376),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		GF_DEMO,
	},

	// These are commented until future notice because I don't have complete information
	// on them (the file size for data.000 / data.001). Will be relying on people to
	// report the missing data.
	/*
	{ // ST25 Amiga ? (EN)
		{
			"st25",
			"Floppy",
			AD_ENTRY1("data.000", "f0918b6d096455ce2ae6dd5ef973292e"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 Amiga ? (GER)
		{
			"st25",
			"Floppy",
			AD_ENTRY1("data.000", "70d0e374d5fa973e536dba0f42310672"),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 Amiga ? (FR)
		{
			"st25",
			"Floppy",
			AD_ENTRY1("data.000", "d0299af1385edd7c7612ed453e417dd8"),
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 Mac ? (EN)
		{
			"st25",
			"Floppy",
			AD_ENTRY1("Star Trek Data", "871fa51c7680c0a43df9622128f1569f"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		0,
	},

	{ // ST25 DOS demo ? (EN)
		{
			"st25",
			"Demo",
			AD_ENTRY1("data.001", "f68126e7e36ce6286c4c8575f8b594f5"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		GF_DEMO,
	},

	{ // ST25 MAC demo ? (EN)
		{
			"st25",
			"Demo",
			AD_ENTRY1("Star Trek Data", "88f88b81d389a3217fc7efd6ee631c42"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_DEMO | ADGF_UNSTABLE,
			GUIO0()
		},
		GType_ST25,
		GF_DEMO,
	},
	*/

	{ // STJR DOS CD-ROM edition (EN)
		{
			"stjr",
			"CD",
			AD_ENTRY1s("data.001", "1c8de3c02f69c07c582d59d3c29e4dd9", 3318644),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_STJR,
		GF_CDROM,
	},

	{ // STJR DOS CD-ROM edition (GER)
		{
			"stjr",
			"CD",
			AD_ENTRY1s("data.001", "1c8de3c02f69c07c582d59d3c29e4dd9", 2958818),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_STJR,
		GF_CDROM,
	},

	{ // STJR DOS Floppy edition (ENG) (The White Label - Doubles)
		{
			"stjr",
			"Floppy",
			AD_ENTRY1s("data.001", "1c8de3c02f69c07c582d59d3c29e4dd9", 2955146),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_STJR,
		0
	},

	{ // STJR DOS Floppy edition (FRA) (The White Label - Doubles)
		{
			"stjr",
			"Floppy",
			AD_ENTRY1s("data.001", "1c8de3c02f69c07c582d59d3c29e4dd9", 2966180),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GType_STJR,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace StarTrek

class StarTrekMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	StarTrekMetaEngineDetection() : AdvancedMetaEngineDetection(StarTrek::gameDescriptions, sizeof(StarTrek::StarTrekGameDescription), starTrekGames) {
	}

	const char *getEngineId() const override {
		return "startrek";
	}

	const char *getName() const override {
		return "Star Trek";
	}

	const char *getOriginalCopyright() const override {
		return "Star Trek: 25th Anniversary, Star Trek: Judgment Rites (C) Interplay";
	}
};



REGISTER_PLUGIN_STATIC(STARTREK_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, StarTrekMetaEngineDetection);
