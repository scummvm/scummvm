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

#ifndef TITANIC_CONTINUE_SAVE_DIALOG_H
#define TITANIC_CONTINUE_SAVE_DIALOG_H

#include "common/array.h"
#include "titanic/events.h"
#include "titanic/support/image.h"
#include "titanic/support/rect.h"
#include "titanic/support/string.h"
#include "titanic/gfx/text_control.h"

namespace Titanic {

#define EXIT_GAME -2

class CContinueSaveDialog : public CEventTarget {
	struct SaveEntry {
		int _slot;
		CString _name;
		SaveEntry() : _slot(0) {}
		SaveEntry(int slot, const CString &name) : _slot(slot), _name(name) {}
	};
private:
	Common::Array<SaveEntry> _saves;
	CTextControl _slotNames[5];
	int _highlightedSlot, _selectedSlot;
	Point _mousePos;
	bool _evilTwinShown;
	bool _mouseDown;
	int _restoreState, _startState;
	Image _backdrop;
	Image _evilTwin;
	Image _restoreD, _restoreU, _restoreF;
	Image _startD, _startU, _startF;
private:
	/**
	 * Load the images
	 */
	void loadImages();

	/**
	 * Render the dialog
	 */
	void render();

	/**
	 * Render the buttons
	 */
	void renderButtons();

	/**
	 * Render the slots
	 */
	void renderSlots();

	/**
	 * Get the area to draw a slot name in
	 */
	Rect getSlotBounds(int index);
public:
	CContinueSaveDialog();
	~CContinueSaveDialog() override;

	void mouseMove(const Point &mousePos) override;
	void leftButtonDown(const Point &mousePos) override;
	void leftButtonUp(const Point &mousePos) override;
	void keyDown(Common::KeyState keyState) override;

	/**
	 * Add a savegame to the list to be displayed
	 */
	void addSavegame(int slot, const CString &name);

	/**
	 * Show the dialog and wait for a slot to be selected
	 */
	int show();
};

} // End of namespace Titanic

#endif /* TITANIC_CONTINUE_SAVE_DIALOG_H */
