/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/meta_engine.h"
#include "ultima/ultima8/misc/debugger.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"

namespace Ultima {
namespace Ultima8 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_method;
	const char *_key;
};

static const KeybindingRecord KEYS[] = {
	{ ACTION_COMBAT, "COMBAT", "Combat", "MainActor::toggleCombat", "c" },
	{ ACTION_NONE, nullptr, nullptr, nullptr, nullptr }
};

Common::KeymapArray MetaEngine::initKeymaps() {
	Common::Keymap *keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame, "ultima8", _("Ultima VIII"));

	for (const KeybindingRecord *r = KEYS; r->_id; ++r) {
		Common::Action *act = new Common::Action(r->_id, _(r->_desc));
		act->setCustomEngineActionEvent(r->_action);
		act->addDefaultInputMapping(r->_key);
		keyMap->addAction(act);
	}

	return Common::Keymap::arrayOf(keyMap);
}

void MetaEngine::executeAction(KeybindingAction keyAction) {
	for (const KeybindingRecord *r = KEYS; r->_id; ++r) {
		if (r->_action == keyAction) {
			g_debugger->executeCommand(r->_method);
			break;
		}
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
