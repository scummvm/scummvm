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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/


#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "dm/detection.h"

namespace DM {

static const PlainGameDescriptor DMGames[] = {
	{"dm", "Dungeon Master"},
	{0, 0}
};

static const DMADGameDescription gameDescriptions[] = {
	{
		{"dm", "Amiga v2.0 English",
			AD_ENTRY2s("graphics.dat",	"c2205f6225bde728417de29394f97d55", 411960,
					   "Dungeon.dat",	"43a213da8eda413541dd12f90ce202f6", 25006),
			Common::EN_ANY, Common::kPlatformAmiga, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI)
		},
	    kDMSaveTargetDM21, kDMSaveFormatAmigaPC98FmTowns, kDMSavePlatformAmiga,
		{ kDMSaveTargetDM21, kDMSaveTargetEndOfList },
		{ kDMSaveFormatAmigaPC98FmTowns, kDMSaveFormatEndOfList},
		{ kDMSavePlatformAcceptAny}
	},
	{
		{"dm", "Atari v??? English",
			AD_ENTRY2s("graphics.dat",	"6ffff2a17e2df0effa9a12fb4b1bf6b6", 271911,
					   "Dungeon.dat",	"be9468b460515741babec9a70501e2e9", 33286),
	    	Common::EN_ANY, Common::kPlatformAtariST, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI),
	    },
	    kDMSaveTargetDM21, kDMSaveFormatAmigaPC98FmTowns, kDMSavePlatformAtariSt,
	    { kDMSaveTargetDM21, kDMSaveTargetEndOfList},
	    { kDMSaveFormatAmigaPC98FmTowns, kDMSaveFormatEndOfList},
	    { kDMSavePlatformAcceptAny }
	},
	{
		// Added by Strangerke
		{"dm", "Amiga Demo v2.0 English",
			AD_ENTRY2s("graphics.dat",	"3932c8359bb36c24291b09e915114d38", 192421,
					   "DemoDun.dat",	"78848e1a2d3d5a11e5954deb8c7b772b", 1209),
			Common::EN_ANY, Common::kPlatformAmiga, ADGF_DEMO, GUIO1(GUIO_NOMIDI),
		},
		kDMSaveTargetDM21, kDMSaveFormatAmigaPC98FmTowns, kDMSavePlatformAtariSt,
		{ kDMSaveTargetDM21, kDMSaveTargetEndOfList},
		{ kDMSaveFormatAmigaPC98FmTowns, kDMSaveFormatEndOfList},
		{ kDMSavePlatformAcceptAny }
	},
	{
		AD_TABLE_END_MARKER, kDMSaveTargetNone, kDMSaveFormatNone, kDMSavePlatformNone,
		{kDMSaveTargetNone}, {kDMSaveFormatNone}, {kDMSavePlatformNone}
	}
};

static const ADExtraGuiOptionsMap optionsList[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class DMMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	DMMetaEngineDetection() : AdvancedMetaEngineDetection(DM::gameDescriptions, sizeof(DMADGameDescription), DMGames, optionsList) {
	}

	const char *getEngineId() const override {
		return "dm";
	}

	const char *getName() const override {
		return "Dungeon Master";
	}

	const char *getOriginalCopyright() const override {
		return "Dungeon Master (C) 1987 FTL Games";
	}


};

} // End of namespace DM

REGISTER_PLUGIN_STATIC(DM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DM::DMMetaEngineDetection);
