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

#include "engines/advancedDetector.h"
#include "vcruise/vcruise.h"

namespace Graphics {

struct Surface;

} // End of namespace Graphics

namespace VCruise {

static const ADExtraGuiOptionsMap optionsList[] = {
	{GAMEOPTION_LAUNCH_DEBUG,
	 {_s("Start with debugger"),
	  _s("Starts with the debugger dashboard active."),
	  "vcruise_debug",
	  false,
	  0,
	  0}},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR};

} // End of namespace VCruise

class VCruiseMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "vcruise";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return VCruise::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

bool VCruiseMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f);
}

Common::Error VCruiseMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new VCruise::VCruiseEngine(syst, reinterpret_cast<const VCruise::VCruiseGameDescription *>(desc));
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(VCRUISE)
REGISTER_PLUGIN_DYNAMIC(VCRUISE, PLUGIN_TYPE_ENGINE, VCruiseMetaEngine);
#else
REGISTER_PLUGIN_STATIC(VCRUISE, PLUGIN_TYPE_ENGINE, VCruiseMetaEngine);
#endif
