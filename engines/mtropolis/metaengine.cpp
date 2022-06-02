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

#include "engines/advancedDetector.h"

#include "mtropolis/actions.h"
#include "mtropolis/debug.h"
#include "mtropolis/detection.h"

#include "mtropolis/mtropolis.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"

#include "common/translation.h"

namespace MTropolis {

uint32 MTropolisEngine::getGameID() const {
	return _gameDescription->gameID;
}

Common::Platform MTropolisEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 MTropolisEngine::getVersion() const {
	return _gameDescription->version;
}

} // End of namespace MTropolis

class MTropolisMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "mtropolis";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;
};

bool MTropolisMetaEngine::hasFeature(MetaEngineFeature f) const {
	switch (f) {
	case kSupportsListSaves:
	case kSupportsDeleteSave:
	case kSavesSupportMetaInfo:
	case kSavesSupportThumbnail:
	case kSavesSupportCreationDate:
	case kSavesSupportPlayTime:
	case kSimpleSavesNames:
	case kSavesUseExtendedFormat:
		return true;
	default:
		return false;
	}
}

bool MTropolis::MTropolisEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsReturnToLauncher:
		return true;
	default:
		return false;
	};
}

Common::Error MTropolisMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new MTropolis::MTropolisEngine(syst, (const MTropolis::MTropolisGameDescription *)desc);
	return Common::kNoError;
}


Common::Array<Common::Keymap *> MTropolisMetaEngine::initKeymaps(const char *target) const {
	Common::Keymap *keymap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "mtropolis", "mTropolis");

	Common::Action *act;
	act = new Common::Action("DEBUG_TOGGLE_OVERLAY", _("Toggle debug overlay"));
	act->setCustomEngineActionEvent(MTropolis::Actions::kDebugToggleOverlay);
	act->addDefaultInputMapping("F10");
	keymap->addAction(act);

	return Common::Keymap::arrayOf(keymap);
}

#if PLUGIN_ENABLED_DYNAMIC(MTROPOLIS)
REGISTER_PLUGIN_DYNAMIC(MTROPOLIS, PLUGIN_TYPE_ENGINE, MTropolisMetaEngine);
#else
REGISTER_PLUGIN_STATIC(MTROPOLIS, PLUGIN_TYPE_ENGINE, MTropolisMetaEngine);
#endif
