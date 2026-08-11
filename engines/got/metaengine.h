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

#ifndef GOT_METAENGINE_H
#define GOT_METAENGINE_H

#include "engines/advancedDetector.h"

namespace Got {

enum KeybindingAction {
	KEYBIND_NONE,
	KEYBIND_UP,
	KEYBIND_DOWN,
	KEYBIND_LEFT,
	KEYBIND_RIGHT,
	KEYBIND_SELECT,
	KEYBIND_FIRE,
	KEYBIND_MAGIC,
	KEYBIND_ESCAPE,
	KEYBIND_THOR_DIES
};

} // namespace Got

class GotMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
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

	Common::Array<Common::Keymap *> initKeymaps(const char *target) const override;
};

#endif // GOT_METAENGINE_H
