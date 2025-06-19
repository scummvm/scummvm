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

#include "tetraedge/metaengine.h"
#include "tetraedge/detection.h"
#include "tetraedge/tetraedge.h"
#include "common/str-array.h"
#include "common/translation.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_CORRECT_MOVIE_ASPECT,
		{
			_s("Correct movie aspect ratio"),
			_s("Play Syberia cutscenes in 16:9, rather than stretching to full screen"),
			"correct_movie_aspect",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_RESTORE_SCENES,
		{
			_s("Restore missing scenes"),
			_s("Restore some scenes originally in the Windows edition"),
			"restore_scenes",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const ADExtraGuiOptionsMap *TetraedgeMetaEngine::getAdvancedExtraGuiOptions() const {
	return optionsList;
}

const char *TetraedgeMetaEngine::getName() const {
	return "tetraedge";
}

Common::Error TetraedgeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Tetraedge::TetraedgeEngine(syst, desc);
	return Common::kNoError;
}

bool TetraedgeMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSavesUseExtendedFormat) ||
		(f == kSimpleSavesNames) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSupportsLoadingDuringStartup);
}

void TetraedgeMetaEngine::getSavegameThumbnail(Graphics::Surface &thumb) {
	Tetraedge::TetraedgeEngine::getSavegameThumbnail(thumb);
}

Common::KeymapArray TetraedgeMetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	using namespace Tetraedge;

	Keymap *engineKeyMap = new Keymap(Keymap::kKeymapTypeGame, "tetraedge-default", _("Default keymappings"));

	Common::Action *act;

	Common::String gameId = ConfMan.get("gameid", target);

	act = new Common::Action(kStandardActionLeftClick, _("Move / Interact / Select"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	engineKeyMap->addAction(act);

	act = new Common::Action("SKIP", _("Skip dialog"));
	act->setCustomEngineActionEvent(kActionSkip);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	engineKeyMap->addAction(act);

	act = new Common::Action("LOAD", _("Load game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("l");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	engineKeyMap->addAction(act);

	act = new Common::Action("SAVE", _("Save game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("s");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);

	return Common::Keymap::arrayOf(engineKeyMap);

}


#if PLUGIN_ENABLED_DYNAMIC(TETRAEDGE)
REGISTER_PLUGIN_DYNAMIC(TETRAEDGE, PLUGIN_TYPE_ENGINE, TetraedgeMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TETRAEDGE, PLUGIN_TYPE_ENGINE, TetraedgeMetaEngine);
#endif
