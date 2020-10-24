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

#include "dgds/dgds.h"

class DgdsMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "DGDS";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool DgdsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return (f == kSupportsListSaves) ||
	       (f == kSupportsLoadingDuringStartup) ||
	       (f == kSupportsDeleteSave) ||
	       (f == kSavesSupportMetaInfo) ||
	       (f == kSavesSupportThumbnail) ||
	       (f == kSimpleSavesNames);
}

bool DgdsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc)
		*engine = new Dgds::DgdsEngine(syst, desc);

	return desc != nullptr;
}

#if PLUGIN_ENABLED_DYNAMIC(DGDS)
	REGISTER_PLUGIN_DYNAMIC(DGDS, PLUGIN_TYPE_ENGINE, DgdsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DGDS, PLUGIN_TYPE_ENGINE, DgdsMetaEngine);
#endif
