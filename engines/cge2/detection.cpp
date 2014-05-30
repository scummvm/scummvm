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

#include "cge2/cge2.h"
#include "engines/advancedDetector.h"

namespace CGE2 {

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
		Common::PL_POL, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"sfinx", "Sfinx English Alfa v0.1",
		{
			{"vol.cat", 0, "41a61030468e33828f3531fd53ef0319", 129024},
			{"vol.dat", 0, "978ebe56442c111d193fdb4d960cc58d", 34382498},	
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS, GUIO0()
	},
	AD_TABLE_END_MARKER
};

class CGE2MetaEngine : public AdvancedMetaEngine {
public:
	CGE2MetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(ADGameDescription), CGE2Games) {
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
};

bool CGE2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new CGE2::CGE2Engine(syst, desc);
	}
	return desc != 0;
}

bool CGE2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

} // End of namespace CGE2

#if PLUGIN_ENABLED_DYNAMIC(CGE2)
	REGISTER_PLUGIN_DYNAMIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CGE2, PLUGIN_TYPE_ENGINE, CGE2::CGE2MetaEngine);
#endif
