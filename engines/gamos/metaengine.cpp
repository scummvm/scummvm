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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/translation.h"

#include "gamos/metaengine.h"
#include "gamos/detection.h"
#include "gamos/gamos.h"

const char *GamosMetaEngine::getName() const {
	return "gamos";
}

Common::Error GamosMetaEngine::createInstance(OSystem *syst, Engine **engine, const Gamos::GamosGameDescription *desc) const {
	*engine = new Gamos::GamosEngine(syst, desc);
	return Common::kNoError;
}

bool GamosMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
	       (f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(GAMOS)
REGISTER_PLUGIN_DYNAMIC(GAMOS, PLUGIN_TYPE_ENGINE, GamosMetaEngine);
#else
REGISTER_PLUGIN_STATIC(GAMOS, PLUGIN_TYPE_ENGINE, GamosMetaEngine);
#endif
