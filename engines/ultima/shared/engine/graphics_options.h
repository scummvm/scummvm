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

#ifndef ULTIMA_SHARED_ENGINE_GRAPHICS_OPTIONS_H
#define ULTIMA_SHARED_ENGINE_GRAPHICS_OPTIONS_H

#include "gui/widget.h"

namespace GUI {
class PopUpWidget;
}

namespace Ultima {
namespace Shared {

/**
 * ConfMan key holding the graphics mode (an index into the game's own list
 * of modes) for the games that have alternate renderers, such as the ones
 * from the Ultima II Upgrade patch. 0 is always the original graphics
 */
constexpr const char *GRAPHICS_MODE_KEY = "graphics_mode";

/**
 * Game options widget with a drop-down for picking the graphics mode. It's
 * given the (untranslated, marked with _s) labels of the game's modes, in
 * the order of the game's own mode enum - the selected entry's index is what
 * gets stored under GRAPHICS_MODE_KEY, for the game to read when it starts
 * or when it's told the game options have changed
 */
class GraphicsOptionsWidget : public GUI::OptionsContainerWidget {
private:
	const char *const *_modes;
	int _modeCount;
	GUI::PopUpWidget *_modePopUp;

protected:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName,
		const Common::String &overlayedLayout) const override;

public:
	GraphicsOptionsWidget(GUI::GuiObject *boss, const Common::String &name,
		const Common::String &domain, const char *const *modes, int modeCount);
	~GraphicsOptionsWidget() override {
	}

	// OptionsContainerWidget API
	void load() override;
	bool save() override;
};

} // namespace Shared
} // namespace Ultima

#endif
