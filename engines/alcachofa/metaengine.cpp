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
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

#include "alcachofa/metaengine.h"
#include "alcachofa/alcachofa.h"

using namespace Common;
using namespace Graphics;
using namespace Alcachofa;

namespace Alcachofa {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_HIGH_QUALITY,
		{
			_s("High Quality"),
			_s("TODO: Explain what this does"),
			_s("high_quality"),
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_32BITS,
		{
			_s("32 Bits"),
			_s("TODO: Also explain this, and implement it maybe"),
			_s("32_bits"),
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Alcachofa

const char *AlcachofaMetaEngine::getName() const {
	return "alcachofa";
}

const ADExtraGuiOptionsMap *AlcachofaMetaEngine::getAdvancedExtraGuiOptions() const {
	return Alcachofa::optionsList;
}

Error AlcachofaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Alcachofa::AlcachofaEngine(syst, desc);
	return kNoError;
}

bool AlcachofaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

KeymapArray AlcachofaMetaEngine::initKeymaps(const char *target) const {
	Keymap *keymap = new Keymap(Keymap::kKeymapTypeGame, "alcachofa-default", _("Default keymappings"));

	Action *act;

	act = new Action(kStandardActionLeftClick, _("Activate"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Look at"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keymap->addAction(act);

	act = new Action("MENU", _("Menu"));
	act->setCustomEngineActionEvent((CustomEventType)EventAction::InputMenu);
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_START");
	keymap->addAction(act);

	return Keymap::arrayOf(keymap);
}

void AlcachofaMetaEngine::getSavegameThumbnail(Surface &surf) {
	if (Alcachofa::g_engine == nullptr)
		surf.create(160, 120, PixelFormat::createFormatRGBA32());
	else
		Alcachofa::g_engine->getSavegameThumbnail(surf);
}

#if PLUGIN_ENABLED_DYNAMIC(ALCACHOFA)
REGISTER_PLUGIN_DYNAMIC(ALCACHOFA, PLUGIN_TYPE_ENGINE, AlcachofaMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ALCACHOFA, PLUGIN_TYPE_ENGINE, AlcachofaMetaEngine);
#endif
