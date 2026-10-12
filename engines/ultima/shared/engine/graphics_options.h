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

#include "engines/metaengine.h"
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
 * Returns the graphics mode of the game being run, or the given one if it
 * has never been chosen. Enhanced versions of games default to a better one
 */
extern int configuredGraphicsMode(int defaultMode);

/**
 * Returns a checkbox option of the game being run, or the given state if it
 * has never been set. Enhanced versions of games default to having them on
 */
extern bool configuredOption(const char *key, bool defaultState);

/**
 * Game options widget with a drop-down for picking the graphics mode. It's
 * given the (untranslated, marked with _s) labels of the game's modes, in
 * the order of the game's own mode enum - the selected entry's index is what
 * gets stored under GRAPHICS_MODE_KEY, for the game to read when it starts
 * or when it's told the game options have changed. Any of the game's other
 * options that are checkboxes can be shown below it
 */
class GraphicsOptionsWidget : public GUI::OptionsContainerWidget {
private:
	const char *const *_modes;
	int _modeCount;
	GUI::PopUpWidget *_modePopUp;
	ExtraGuiOptions _options;
	int _defaultMode;
	Common::Array<GUI::CheckboxWidget *> _checkboxes;

protected:
	// OptionsContainerWidget API
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName,
		const Common::String &overlayedLayout) const override;

public:
	/**
	 * Constructor
	 * @param modes				The labels of the modes
	 * @param modeCount			How many modes there are
	 * @param options			The game's other options to show
	 * @param onTitleScreen		True if the mode can be changed from the game's title screen
	 * @param defaultMode		The mode that's selected until one has been chosen
	 */
	GraphicsOptionsWidget(GUI::GuiObject *boss, const Common::String &name,
		const Common::String &domain, const char *const *modes, int modeCount,
		const ExtraGuiOptions &options = ExtraGuiOptions(), bool onTitleScreen = true,
		int defaultMode = 0);
	~GraphicsOptionsWidget() override {
	}

	// OptionsContainerWidget API
	void load() override;
	bool save() override;
};

} // namespace Shared
} // namespace Ultima

#endif
