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

#include "common/events.h"
#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/advancedDetector.h"

#include "harold/bcb/bcb.h"
#include "harold/detection.h"

class HaroldMetaEngine : public AdvancedMetaEngine<BCB::HaroldGameDescription> {
public:
	const char *getName() const override {
		return "harold";
	}

	int getMaximumSaveSlot() const override { return 8; }
	Common::Error createInstance(OSystem *syst, Engine **engine, const BCB::HaroldGameDescription *gd) const override;
};

Common::Error HaroldMetaEngine::createInstance(OSystem *syst, Engine **engine, const BCB::HaroldGameDescription *gd) const {
	switch (gd->gameType) {
	case BCB::kHaroldGameBCB:
		*engine = new BCB::BCBEngine(syst, gd);
		return Common::kNoError;
	default:
		return Common::kUnsupportedGameidError;
	}
}

#if PLUGIN_ENABLED_DYNAMIC(HAROLD)
REGISTER_PLUGIN_DYNAMIC(HAROLD, PLUGIN_TYPE_ENGINE, HaroldMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HAROLD, PLUGIN_TYPE_ENGINE, HaroldMetaEngine);
#endif
