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

#ifndef REMAP_DIALOG_H
#define REMAP_DIALOG_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "gui/dialog.h"

namespace GUI {
class ButtonWidget;
class PopUpWidget;
class ScrollContainerWidget;
class StaticTextWidget;
}

namespace Common {

class Action;
class Keymap;
class Keymapper;
class InputWatcher;

class RemapDialog : public GUI::Dialog {
public:
	RemapDialog();
	virtual ~RemapDialog();
	virtual void open();
	virtual void close();
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleTickle();

protected:
	struct ActionRow {
		Common::Action *action;

		GUI::StaticTextWidget *actionText;
		GUI::ButtonWidget *keyButton;
		GUI::ButtonWidget *clearButton;
		GUI::ButtonWidget *resetButton;

		ActionRow() : action(nullptr), actionText(nullptr), keyButton(nullptr), clearButton(nullptr), resetButton(nullptr) { }
	};

	void loadKeymap();
	void refreshKeymap();
	void clearKeymap();
	void reflowActionWidgets();
	void clearMapping(uint i);
	void resetMapping(uint i);
	void startRemapping(uint i);
	void stopRemapping();

	Keymapper *_keymapper;
	Common::Array<Keymap *> _keymapTable;

	InputWatcher *_remapInputWatcher;
	Action *_remapAction;
	uint32 _remapTimeout;

	GUI::StaticTextWidget *_kmPopUpDesc;
	GUI::PopUpWidget *_kmPopUp;
	GUI::ScrollContainerWidget *_scrollContainer;

	static const uint32 kRemapTimeoutDelay = 3000;

	bool _changes;

	Array<ActionRow> _actions;
};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef REMAP_DIALOG_H
