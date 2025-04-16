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

#include "awe/metaengine.h"
#include "awe/detection.h"
#include "awe/awe.h"

namespace Awe {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Enable copy protection"),
			_s("Enable any copy protection that would otherwise be bypassed by default."),
			"copy_protection",
			false,
			0,
			0
		},
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Awe

const char *AweMetaEngine::getName() const {
	return "awe";
}

const ADExtraGuiOptionsMap *AweMetaEngine::getAdvancedExtraGuiOptions() const {
	return Awe::optionsList;
}

Common::Error AweMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Awe::AweEngine(syst, desc);
	return Common::kNoError;
}

bool AweMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(AWE)
REGISTER_PLUGIN_DYNAMIC(AWE, PLUGIN_TYPE_ENGINE, AweMetaEngine);
#else
REGISTER_PLUGIN_STATIC(AWE, PLUGIN_TYPE_ENGINE, AweMetaEngine);
#endif
