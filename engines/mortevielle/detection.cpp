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
 */

#include "base/plugins.h"
#include "engines/advancedDetector.h"

#include "mortevielle/mortevielle.h"
#include "mortevielle/detection_tables.h"

namespace Mortevielle {
uint32 MortevielleEngine::getGameFlags() const { return _gameDescription->flags; }
}

static const PlainGameDescriptor MortevielleGame[] = {
	{"mortevielle", "Mortville Manor"},
	{0, 0}
};

static const ADGameDescription MortevielleGameDescriptions[] = {
	{
		"mortevielle",
		"",
		{
			{"menufr.mor", 0, "e413f36b9e14eef16130adc347a9391f", 144},
			{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{
		"mortevielle",
		"",
		{
			{"menual.mor", 0, "792aea282b07a1d74c4a4abeabc90c19", 144},
			{"dxx.mor", 0, "949e68e829ecd5ad29e36a00347a9e7e", 207744},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

class MortevielleMetaEngine : public AdvancedMetaEngine {
public:
	MortevielleMetaEngine() : AdvancedMetaEngine(MortevielleGameDescriptions, sizeof(ADGameDescription), 
		MortevielleGame) {
		_md5Bytes = 512;
		_singleid = "mortevielle";
	}
	virtual const char *getName() const {
		return "Mortevielle";
	}

	virtual const char *getOriginalCopyright() const {
		return "Mortville Manor (C) 1988-89 Lankhor";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
};

bool MortevielleMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Mortevielle::MortevielleEngine(syst, desc);
	}
	return desc != 0;
}

bool MortevielleMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false; 
}

#if PLUGIN_ENABLED_DYNAMIC(MORTEVIELLE)
	REGISTER_PLUGIN_DYNAMIC(MORTEVIELLE, PLUGIN_TYPE_ENGINE, MortevielleMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MORTEVIELLE, PLUGIN_TYPE_ENGINE, MortevielleMetaEngine);
#endif
