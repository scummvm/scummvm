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

#ifndef MADS_NEBULAR_MAC_DIALOGS_H
#define MADS_NEBULAR_MAC_DIALOGS_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str-array.h"

namespace Graphics {
class Font;
class MacWindowManager;
class ManagedSurface;
}

namespace MADS {
namespace RexNebular {

class MacResourceProvider;
class MacNebularMenu;
class RexNebularEngine;

enum MacDialogEditCommand {
	kMacDialogUndo,
	kMacDialogCut,
	kMacDialogCopy,
	kMacDialogPaste,
	kMacDialogClear
};

class MacNebularDialog {
private:
	enum ItemType {
		kUserItem = 0,
		kButton = 4,
		kCheckbox = 5,
		kRadioButton = 6,
		kStaticText = 8,
		kEditableText = 16
	};

	struct Item {
		Common::Rect bounds;
		Common::String text;
		Common::String undoText;
		Common::StringArray list;
		byte type;
		bool enabled;
		bool checked;
		bool listItem;
		bool hasUndo;
		int listSelection;
		int listTop;
		uint caret;
		uint selectionStart;
		uint selectionEnd;
		uint maxLength;

		Item();
	};

	RexNebularEngine &_engine;
	MacResourceProvider &_resources;
	Graphics::ManagedSurface &_screen;
	Graphics::MacWindowManager &_windowManager;
	MacNebularMenu *_menus;
	const Graphics::Font *_font;
	Common::Rect _bounds;
	Common::Array<Item> _items;
	int _focusItem;
	int _pressedItem;
	int _defaultItem;
	int _cancelItem;
	bool _redraw;
	bool _inlineEditable;

	Item *getItem(int itemNumber);
	const Item *getItem(int itemNumber) const;
	Common::Rect getItemBounds(const Item &item) const;
	bool loadDialogResource(uint16 resourceID, uint16 &itemResourceID);
	bool loadDialogItems(uint16 resourceID);
	void draw();
	void drawItem(Item &item, int itemNumber);
	void drawButton(const Item &item, const Common::Rect &bounds,
		bool pressed);
	void drawCheckbox(const Item &item, const Common::Rect &bounds);
	void drawRadioButton(const Item &item, const Common::Rect &bounds);
	void drawEditable(Item &item, const Common::Rect &bounds,
		bool focused);
	void drawList(const Item &item, const Common::Rect &bounds,
		bool focused);
	void setFocus(int itemNumber);
	void focusNext(bool backwards);
	void selectRadio(int itemNumber);
	void selectList(Item &item, int selection);
	void ensureListSelectionVisible(Item &item);
	void moveCaret(Item &item, int position, bool extend);
	void deleteSelection(Item &item);
	void rememberUndo(Item &item);
	void insertCharacter(Item &item, char character);
	void handleKey(const Common::Event &event, int &result, bool &done);
	void handleMouseDown(const Common::Point &point);
	void handleMouseUp(const Common::Point &point, int &result, bool &done);
	void restore(const Graphics::ManagedSurface &saved, bool cursorWasVisible);

public:
	MacNebularDialog(RexNebularEngine &engine,
		MacResourceProvider &resources, Graphics::ManagedSurface &screen,
		Graphics::MacWindowManager &windowManager,
		MacNebularMenu *menus = nullptr);

	bool load(uint16 resourceID);
	void configureInlineEditable(const Common::Rect &bounds,
		const Common::String &text, uint maxLength);
	void center();
	void setItemEnabled(int itemNumber, bool enabled);
	void setItemChecked(int itemNumber, bool checked);
	bool isItemChecked(int itemNumber) const;
	void setItemText(int itemNumber, const Common::String &text);
	void setItemMaxLength(int itemNumber, uint maxLength);
	Common::String getItemText(int itemNumber) const;
	void setList(int itemNumber, const Common::StringArray &entries,
		int selection);
	int getListSelection(int itemNumber) const;
	bool hasEditableFocus() const;
	bool isEditCommandEnabled(MacDialogEditCommand commandId) const;
	bool handleEditCommand(MacDialogEditCommand commandId);
	int runModal(int defaultItem, int cancelItem);
};

} // namespace RexNebular
} // namespace MADS

#endif
