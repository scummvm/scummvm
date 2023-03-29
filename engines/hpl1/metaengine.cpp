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

#include "hpl1/metaengine.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "hpl1/detection.h"
#include "hpl1/graphics.h"
#include "hpl1/hpl1.h"

const char *Hpl1MetaEngine::getName() const {
	return "hpl1";
}

Common::Error Hpl1MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Hpl1::Hpl1Engine(syst, desc);
	return Common::kNoError;
}

bool Hpl1MetaEngine::hasFeature(MetaEngineFeature f) const {
	return (f == kSavesUseExtendedFormat) ||
		   (f == kSimpleSavesNames) ||
		   (f == kSupportsListSaves) ||
		   (f == kSupportsDeleteSave) ||
		   (f == kSavesSupportMetaInfo) ||
		   (f == kSavesSupportThumbnail) ||
		   (f == kSupportsLoadingDuringStartup);
}

void Hpl1MetaEngine::getSavegameThumbnail(Graphics::Surface &thumbnail) {
	Common::ScopedPtr<Graphics::Surface> screen = Hpl1::createViewportScreenshot();
	Common::ScopedPtr<Graphics::Surface> scaledScreen(screen->scale(kThumbnailWidth, kThumbnailHeight2));
	scaledScreen->convertToInPlace(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	thumbnail.copyFrom(*scaledScreen);
	screen->free();
	scaledScreen->free();
}

Common::Action *createKeyBoardAction(const char *id, const Common::U32String &desc, const char *defaultMap, const Common::KeyState &key) {
	Common::Action *act = new Common::Action(id, desc);
	act->setKeyEvent(key);
	act->addDefaultInputMapping(defaultMap);
	return act;
}

Common::Action *createMouseAction(const char *id, const Common::U32String &desc, const char *defaultMap, const Common::EventType type) {
	Common::Action *act = new Common::Action(id, desc);
	act->setEvent(type);
	act->addDefaultInputMapping(defaultMap);
	return act;
}

Common::Array<Common::Keymap *> Hpl1MetaEngine::initKeymaps(const char *target) const {
	using Common::Keymap;
	using namespace Hpl1;

	Keymap *movement = new Keymap(Keymap::kKeymapTypeGame, "HPL1_MOVEMENT", "Movement");
	movement->addAction(createKeyBoardAction("FORWARD", _("Forward"), "w", Common::KEYCODE_w));
	movement->addAction(createKeyBoardAction("BACKWARD", _("Backward"), "s", Common::KEYCODE_s));
	movement->addAction(createKeyBoardAction("LEFT", _("Strafe Left"), "a", Common::KEYCODE_a));
	movement->addAction(createKeyBoardAction("RIGHT", _("Strafe Right"), "d", Common::KEYCODE_d));
	movement->addAction(createKeyBoardAction("LEAN_LEFT", _("Lean Left"), "q", Common::KEYCODE_q));
	movement->addAction(createKeyBoardAction("LEAN_RIGHT", _("Lean Right"), "e", Common::KEYCODE_e));
	movement->addAction(createKeyBoardAction("RUN", _("Run"), "LSHIFT", Common::KEYCODE_LSHIFT));
	movement->addAction(createKeyBoardAction("JUMP", _("Jump"), "SPACE", Common::KEYCODE_SPACE));
	movement->addAction(createKeyBoardAction("CROUCH", _("Crouch"), "LCTRL", Common::KEYCODE_LCTRL));

	Keymap *actions = new Keymap(Keymap::kKeymapTypeGame, "HPL1_ACTIONS", "Actions");
	actions->addAction(createKeyBoardAction("INTERACTMODE", _("Interact Mode"), "r", Common::KEYCODE_r));
	actions->addAction(createMouseAction("LOOK_MODE", _("Look Mode"), "MOUSE_MIDDLE", Common::EVENT_MBUTTONDOWN));
	actions->addAction(createKeyBoardAction("HOLSTER", _("Holster"), "x", Common::KEYCODE_x));
	actions->addAction(createMouseAction("EXAMINE", _("Examine"), "MOUSE_LEFT", Common::EVENT_LBUTTONDOWN));
	actions->addAction(createMouseAction("INTERACT", _("Interact"), "MOUSE_RIGHT", Common::EVENT_RBUTTONDOWN));

	Keymap *misc = new Keymap(Keymap::kKeymapTypeGame, "HPL1_MISC", "Misc");
	misc->addAction(createKeyBoardAction("INVENTORY", _("Inventory"), "TAB", Common::KEYCODE_TAB));
	misc->addAction(createKeyBoardAction("NOTEBOOK", _("Notebook"), "n", Common::KEYCODE_n));
	misc->addAction(createKeyBoardAction("PERSONAL_NOTES", _("Personal Notes"), "p", Common::KEYCODE_p));
	misc->addAction(createKeyBoardAction("FLASHLIGHT", _("Flashlight"), "f", Common::KEYCODE_f));
	misc->addAction(createKeyBoardAction("GLOWSTICK", _("Glowstick"), "g", Common::KEYCODE_f));

	Common::Array<Common::Keymap *> keymaps(3);
	keymaps[0] = movement;
	keymaps[1] = actions;
	keymaps[2] = misc;
	return keymaps;
}

#if PLUGIN_ENABLED_DYNAMIC(HPL1)
REGISTER_PLUGIN_DYNAMIC(HPL1, PLUGIN_TYPE_ENGINE, Hpl1MetaEngine);
#else
REGISTER_PLUGIN_STATIC(HPL1, PLUGIN_TYPE_ENGINE, Hpl1MetaEngine);
#endif
