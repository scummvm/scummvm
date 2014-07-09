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
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_DETECTION_H
#define CGE2_DETECTION_H

#include "cge2/cge2.h"
#include "engines/advancedDetector.h"
#include "common/translation.h"

namespace CGE2 {

#define GAMEOPTION_COLOR_BLIND_DEFAULT_OFF  GUIO_GAMEOPTIONS1

static const PlainGameDescriptor CGE2Games[] = {
	{ "sfinx", "Sfinx" },
	{ 0, 0 }
};

static const ADGameDescription gameDescriptions[] = {
	{
		"sfinx", "Sfinx Freeware",
		{
			{ "vol.cat", 0, "21197b287d397c53261b6616bf0dd880", 129024 },
			{ "vol.dat", 0, "de14291869a8eb7c2732ab783c7542ef", 34180844 },
			AD_LISTEND
		},
		Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO1(GAMEOPTION_COLOR_BLIND_DEFAULT_OFF)
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

class CGE2MetaEngine : public AdvancedMetaEngine {
public:
	CGE2MetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ADGameDescription), CGE2Games, optionsList) {
		_singleid = "sfinx";
	}

	virtual const char *getName() const {
		return "CGE2";
	}

	virtual const char *getOriginalCopyright() const {
		return "Sfinx (c) 1994-1997 Janus B. Wisniewski and L.K. Avalon";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;

	const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;
};

} // End of namespace CGE2

#endif // CGE2_DETECTION_H
