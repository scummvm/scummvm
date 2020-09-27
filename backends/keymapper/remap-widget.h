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

#ifndef REMAP_WIDGET_H
#define REMAP_WIDGET_H

#include "common/scummsys.h"
#include "common/hash-ptr.h"

#include "gui/widget.h"

namespace GUI {
class ButtonWidget;
class DropdownButtonWidget;
class PopUpWidget;
class ScrollContainerWidget;
class StaticTextWidget;
}

namespace Common {

struct Action;
class Keymap;
class Keymapper;
class InputWatcher;

class RemapWidget : public GUI::OptionsContainerWidget {
public:
	typedef Common::Array<Keymap *> KeymapArray;

	RemapWidget(GuiObject *boss, const Common::String &name, const KeymapArray &keymaps);
	~RemapWidget() override;
	void load() override;
	bool save() override;
	void handleInputChanged();
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;
	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleTickle() override;

protected:
	struct ActionRow {
		Keymap *keymap;
		Common::Action *action;

		GUI::StaticTextWidget *actionText;
		GUI::DropdownButtonWidget *keyButton;

		ActionRow() : keymap(nullptr), action(nullptr), actionText(nullptr), keyButton(nullptr) {}
	};

	struct KeymapTitleRow {
		GUI::StaticTextWidget *descriptionText;
		GUI::ButtonWidget *resetButton;

		KeymapTitleRow() : descriptionText(nullptr), resetButton(nullptr) {}
	};

	void loadKeymap();
	void refreshKeymap();
	void reflowActionWidgets();
	void clearMapping(uint actionIndex);
	void resetMapping(uint actionIndex);
	void resetKeymap(uint actionIndex);
	void startRemapping(uint actionIndex);
	void stopRemapping();

	KeymapArray _keymapTable;

	InputWatcher *_remapInputWatcher;
	Keymap *_remapKeymap;
	Action *_remapAction;
	uint32 _remapTimeout;

	static const uint32 kRemapMinTimeoutDelay = 3000;

	bool _changes;

	Array<ActionRow> _actions;
	HashMap<Keymap *, KeymapTitleRow> _keymapSeparators;
};

} // End of namespace Common

#endif // #ifndef REMAP_WIDGET_H
