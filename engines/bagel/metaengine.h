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

#ifndef BAGEL_METAENGINE_H
#define BAGEL_METAENGINE_H

#include "common/system.h"
#include "engines/advancedDetector.h"

namespace Bagel {
namespace HodjNPodj {
extern Common::KeyCode KeybindToKeycode(int key);
} // namespace HodjNPodj

enum KeybindingAction {
	KEYBIND_NONE, KEYBIND_WAIT, KEYBIND_CHEAT714,
	KEYBIND_UP, KEYBIND_DOWN, KEYBIND_LEFT, KEYBIND_RIGHT,
	KEYBIND_SELECT, KEYBIND_ESCAPE, KEYBIND_PAGEUP,
	KEYBIND_PAGEDOWN, KEYBIND_HOME, KEYBIND_END
};


enum KeybindingMode {
	KBMODE_NORMAL,		///< Keys available when normal in-game
	KBMODE_MINIMAL,		///< Keys when in a textbox,
	KBMODE_ALL
};

} // namespace Bagel

class BagelMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
private:
	static Common::String getGameId(const Common::String &target);

public:
	const char *getName() const override;

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	/**
	 * Determine whether the engine supports the specified MetaEngine feature.
	 *
	 * Used by e.g. the launcher to determine whether to enable the Load button.
	 */
	bool hasFeature(MetaEngineFeature f) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override;

	void getSavegameThumbnail(Graphics::Surface &thumb) override;

	/**
	 * Initialize keymaps
	 */
	static Common::KeymapArray initKeymaps(Bagel::KeybindingMode mode, bool isSpacebar);

	Common::KeymapArray initKeymaps(const char *target) const override;

	/**
	 * Set the keybinding mode
	 */
	static void setKeybindingMode(Bagel::KeybindingMode mode);
};

#endif // BAGEL_METAENGINE_H
