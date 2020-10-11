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
#include "agds/agds.h"
#include "agds/object.h"

namespace AGDS {

class AGDSMetaEngine : public AdvancedMetaEngine {
public:
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsLoadingDuringStartup:
		case kSupportsDeleteSave:
		case kSavesSupportMetaInfo:
		case kSavesSupportThumbnail:
		case kSimpleSavesNames:
			return true;
		default:
			return AdvancedMetaEngine::hasFeature(f);
		}
	}

	const char *getName() const override {
		return "agds";
	}

	int getMaximumSaveSlot() const override {
		return 99;
	}
};

bool AGDSMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new AGDS::AGDSEngine(syst, desc);
	}
	return *engine;
}

}

#if PLUGIN_ENABLED_DYNAMIC(AGDS)
	REGISTER_PLUGIN_DYNAMIC(AGDS, PLUGIN_TYPE_ENGINE, AGDS::AGDSMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(AGDS, PLUGIN_TYPE_ENGINE, AGDS::AGDSMetaEngine);
#endif
