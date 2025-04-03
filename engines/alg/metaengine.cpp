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

#include "alg/alg.h"
#include "alg/detection.h"

namespace Alg {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_SINGLE_SPEED_VERSION,
		{
			_s("Use lower quality single speed CD-ROM video"),
		 	_s("These videos are of lower quality, the default version uses double speed CD-ROM videos which are of better quality"),
		 	"single_speed_videos",
		 	false,
		 	0,
		 	0
		},
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

Common::Platform AlgEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

bool AlgEngine::isDemo() const {
	return (bool)(_gameDescription->desc.flags & ADGF_DEMO);
}

} // namespace Alg

class AlgMetaEngine : public AdvancedMetaEngine<Alg::AlgGameDescription> {
public:
	const char *getName() const override {
		return "alg";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return Alg::optionsList;
	}

	bool hasFeature(MetaEngineFeature f) const override {
		return
			(f == kSimpleSavesNames);
	}
	
	Common::Error createInstance(OSystem *syst, Engine **engine, const Alg::AlgGameDescription *gd) const override;
};

Common::Error AlgMetaEngine::createInstance(OSystem *syst, Engine **engine, const Alg::AlgGameDescription *gd) const {
	*engine = new Alg::AlgEngine(syst, gd);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(ALG)
REGISTER_PLUGIN_DYNAMIC(ALG, PLUGIN_TYPE_ENGINE, AlgMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ALG, PLUGIN_TYPE_ENGINE, AlgMetaEngine);
#endif
