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

#include "base/plugins.h"
#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "graphics/thumbnail.h"

#include "hollywood/monsters/hollywood.h"
#include "hollywood/monsters/metaengine.h"
#include "hollywood/runaway/engine.h"

namespace Hollywood {

class HollywoodMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "hollywood";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return getMonstersExtraGuiOptions();
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	bool hasFeature(MetaEngineFeature f) const override;
	void registerDefaultSettings(const Common::String &target) const override;
	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;

protected:
	void getSavegameThumbnail(Graphics::Surface &thumb) override;
};

Common::Error HollywoodMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (!strcmp(desc->gameId, "runaway"))
		*engine = new RunawayEngine(syst, desc->language);
	else
		*engine = new HollywoodEngine(syst, desc);
	return Common::kNoError;
}

bool HollywoodMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		f == kSupportsLoadingDuringStartup;
}

void HollywoodMetaEngine::registerDefaultSettings(const Common::String &) const {
	ConfMan.registerDefault("subtitles", true);
	registerMonstersDefaultSettings();
}

void HollywoodMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	if (g_engine && ConfMan.get("gameid") == "hollywood") {
		getMonstersSavegameThumbnail(thumb);
		return;
	}

	Graphics::createThumbnail(thumb);
}

Common::Array<Common::Keymap *> HollywoodMetaEngine::initKeymaps(const char *target) const {
	if (getGameId(target) == "runaway")
		return RunawayEngine::initKeymaps();
	return MetaEngine::initKeymaps(target);
}

} // End of namespace Hollywood

#if PLUGIN_ENABLED_DYNAMIC(HOLLYWOOD)
	REGISTER_PLUGIN_DYNAMIC(HOLLYWOOD, PLUGIN_TYPE_ENGINE, Hollywood::HollywoodMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(HOLLYWOOD, PLUGIN_TYPE_ENGINE, Hollywood::HollywoodMetaEngine);
#endif
