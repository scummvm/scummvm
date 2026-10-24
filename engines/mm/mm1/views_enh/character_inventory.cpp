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

#include "mm/mm1/views_enh/character_inventory.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/views_enh/combat.h"
#include "mm/mm1/views_enh/game_messages.h"
#include "mm/mm1/views_enh/trade.h"
#include "mm/mm1/views_enh/which_item.h"
#include "mm/mm1/data/locations.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

// Existing inventory buttons end at x=239 and Exit begins at x=275. The G/S
// control fits in the spare gap without moving any existing control. Its icon
// is drawn from the engine's native blank-button surface with a vertical split,
// avoiding the unrelated necklace art previously borrowed from ITEMS.ICN.
static const int GS_BUTTON_X = 245;
static const int GS_BUTTON_Y = 101;
static const int GS_BUTTON_W = 24;
static const int GS_BUTTON_H = 36;
static const int GS_ICON_SIZE = 20;

CharacterInventory::CharacterInventory() : ItemsView("CharacterInventory") {
	setup();
}

CharacterInventory::CharacterInventory(const Common::String &name) : ItemsView(name) {
	setup();
}

void CharacterInventory::setup() {
	_btnSprites.load("items.icn");
	addButton(2, "\x01""37Arms", Common::KEYCODE_a);
	addButton(6, "\x01""37BkPack", Common::KEYCODE_b);
	addButton(8, "\x01""37Equip", Common::KEYCODE_e);
	addButton(10, "\x01""37Rem", Common::KEYCODE_r);
	addButton(12, "\x01""37Disc", Common::KEYCODE_d);
	addButton(6, "\x01""37Trade", Common::KEYCODE_t);
	addButton(14, "\x01""37Use", Common::KEYCODE_u);
	addButton(16, "Exit", Common::KEYCODE_ESCAPE);
}

bool CharacterInventory::msgFocus(const FocusMessage &msg) {
	ItemsView::msgFocus(msg);
	assert(g_globals->_currCharacter);

	bool inCombat = g_events->isInCombat();
	for (int i = 2; i < ((int)getButtonCount() - 1); ++i)
		setButtonEnabled(i, !inCombat);

	if (dynamic_cast<WhichItem *>(msg._priorView) == nullptr &&
		dynamic_cast<Trade *>(msg._priorView) == nullptr &&
		dynamic_cast<GameMessages *>(msg._priorView) == nullptr) {
		_mode = BACKPACK_MODE;
		_selectedButton = BTN_NONE;
	}
	populateItems();

	return true;
}

bool CharacterInventory::msgGame(const GameMessage &msg) {
	if (msg._name == "ITEM" && msg._value >= 0 &&
			msg._value < (int)_items.size()) {
		_selectedItem = msg._value;
		performAction();
		return true;
	} else if (msg._name == "TRADE") {
		_tradeMode = msg._stringValue;
		_tradeAmount = msg._value;
		addView("WhichCharacter");
		return true;
	} else if (msg._name == "TRADE_DEST") {
		if (msg._value != -1)
			trade(_tradeMode, _tradeAmount, &g_globals->_party[msg._value]);
		return true;
	} else if (msg._name == "USE") {
		// Combat use item mode
		addView();
		return true;
	}

	return ItemsView::msgGame(msg);
}

void CharacterInventory::draw() {
	ItemsView::draw();
	drawGatherShare();
	drawTitle();
}

void CharacterInventory::drawGatherShare() {
	if (g_events->isInCombat())
		return;

	Graphics::ManagedSurface s = getSurface();
	const int pressOffset = _gsPressed != GS_NONE ? 1 : 0;
	const int iconX = GS_BUTTON_X + (GS_BUTTON_W - GS_ICON_SIZE) / 2 +
		_bounds.borderSize() + pressOffset;
	const int iconY = GS_BUTTON_Y + _bounds.borderSize() + pressOffset;

	// Native blank button + a simple center divider. This reads as two halves
	// without requiring a new ICN resource and keeps the UI style consistent.
	s.blitFrom(g_globals->_blankButton, Common::Point(iconX, iconY));
	s.vLine(iconX + GS_ICON_SIZE / 2, iconY + 3,
		iconY + GS_ICON_SIZE - 4, 0);

	setReduced(true);
	writeString(GS_BUTTON_X + GS_BUTTON_W / 2, 123,
		"\x01""37G/\x01""37S", ALIGN_MIDDLE);
}

CharacterInventory::GatherShareButton CharacterInventory::gatherShareAt(
		const Common::Point &pos) const {
	if (g_events->isInCombat())
		return GS_NONE;

	Common::Rect r(GS_BUTTON_X, GS_BUTTON_Y,
		GS_BUTTON_X + GS_BUTTON_W, GS_BUTTON_Y + GS_BUTTON_H);
	r.translate(_innerBounds.left, _innerBounds.top);
	if (!r.contains(pos))
		return GS_NONE;

	return pos.x < r.left + GS_BUTTON_W / 2 ? GS_GATHER : GS_SHARE;
}

void CharacterInventory::gatherShare(GatherShareButton action) {
	if (action == GS_GATHER) {
		InfoMessage msg(Common::String::format(
			"Gather all gold, gems and food onto %s?",
			g_globals->_currCharacter->_name));
		msg._fontReduced = true;
		msg._callback = []() {
			g_globals->_currCharacter->gatherResources();
			g_events->redraw();
		};
		send(msg);
	} else if (action == GS_SHARE) {
		addView("Trade");
		send("Trade", GameMessage("SHARE"));
	}
}

void CharacterInventory::drawTitle() {
	const Character &c = *g_globals->_currCharacter;
	const Common::String fmt = ((_mode == ARMS_MODE) ? "Equipment for %s the %s" : "Backpack for %s the %s");

	const Common::String title = Common::String::format(fmt.c_str(),
		c._name,
		CLASS_NAMES[c._class - 1]
	);

	setReduced(false);
	writeLine(0, title, ALIGN_MIDDLE);
}

bool CharacterInventory::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	switch (msg.keycode) {
	case Common::KEYCODE_a:
		_mode = ARMS_MODE;
		populateItems();
		redraw();
		return true;
	case Common::KEYCODE_b:
		_mode = BACKPACK_MODE;
		populateItems();
		redraw();
		return true;
	default:
		break;
	}

	if (!g_events->isInCombat()) {
		switch (msg.keycode) {
		case Common::KEYCODE_e:
			selectButton(BTN_EQUIP);
			return true;
		case Common::KEYCODE_r:
			selectButton(BTN_REMOVE);
			return true;
		case Common::KEYCODE_d:
			selectButton(BTN_DISCARD);
			return true;
		case Common::KEYCODE_t:
			addView("Trade");
			return true;
		case Common::KEYCODE_u:
			selectButton(BTN_USE);
			return true;
		case Common::KEYCODE_g:
			gatherShare(GS_GATHER);
			return true;
		case Common::KEYCODE_s:
			gatherShare(GS_SHARE);
			return true;
		default:
			break;
		}
	}

	return ItemsView::msgKeypress(msg);
}

bool CharacterInventory::msgMouseDown(const MouseDownMessage &msg) {
	if (msg._button == MouseMessage::MB_LEFT) {
		const GatherShareButton action = gatherShareAt(msg._pos);
		if (action != GS_NONE) {
			_gsPressed = action;
			draw();
			return true;
		}
	}

	return ItemsView::msgMouseDown(msg);
}

bool CharacterInventory::msgMouseUp(const MouseUpMessage &msg) {
	if (_gsPressed != GS_NONE) {
		const GatherShareButton pressed = _gsPressed;
		const GatherShareButton released =
			msg._button == MouseMessage::MB_LEFT ? gatherShareAt(msg._pos) : GS_NONE;

		_gsPressed = GS_NONE;
		draw();

		if (released == pressed)
			gatherShare(pressed);
		return true;
	}

	return ItemsView::msgMouseUp(msg);
}

bool CharacterInventory::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	return ItemsView::msgAction(msg);
}

void CharacterInventory::populateItems() {
	_items.clear();
	_selectedItem = -1;

	const Character &c = *g_globals->_currCharacter;
	const Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;

	for (uint i = 0; i < inv.size(); ++i)
		_items.push_back(inv[i]._id);
}

bool CharacterInventory::canSwitchChar() {
	// When in combat, the current character can't be changed
	return !g_events->isInCombat();
}

bool CharacterInventory::canSwitchToChar(Character *dst) {
	if (_selectedItem != -1) {
		tradeItem(dst);
		return false;
	}

	return true;
}

void CharacterInventory::charSwitched(Character *priorChar) {
	PartyView::charSwitched(priorChar);
	populateItems();
	redraw();
}

void CharacterInventory::itemSelected() {
	if (g_events->isInCombat() && dynamic_cast<Combat *>(g_events->priorView()) != nullptr) {
		useItem();
	}
}

void CharacterInventory::selectButton(SelectedButton btnMode) {
	if (btnMode == BTN_EQUIP && _mode == ARMS_MODE) {
		// Selecting items to equip only makes sense in BACKPACK_MODE,
		// so we switch to it:
		_mode = BACKPACK_MODE;
		populateItems();
		redraw();
		draw();
	}
	else if (btnMode == BTN_REMOVE && _mode == BACKPACK_MODE) {
		// Selecting items to unequip only makes sense in ARMS_MODE,
		// so we switch to it:
		_mode = ARMS_MODE;
		populateItems();
		redraw();
		draw();
	}
	_selectedButton = btnMode;

	if (_selectedItem != -1) {
		performAction();

	} else {
		Common::String btn = "Equip";
		if (btnMode == BTN_REMOVE)
			btn = "Remove";
		else if (btnMode == BTN_DISCARD)
			btn = "Discard";
		else if (btnMode == BTN_USE)
			btn = "Use";
		else if (btnMode == BTN_CHARGE)
			btn = "Charge";

		send("WhichItem", GameMessage("DISPLAY",
			Common::String::format("%s which item?", btn.c_str())
		));
	}
}

void CharacterInventory::performAction() {
	switch (_selectedButton) {
	case BTN_EQUIP:
		equipItem();
		break;

	case BTN_REMOVE:
		removeItem();
		break;

	case BTN_DISCARD:
		discardItem();
		break;

	case BTN_USE:
		useItem();
		break;

	default:
		error("No button selected");
		break;
	}
}

void CharacterInventory::equipItem() {
	Common::String errMsg;
	Common::Point textPos;

	if (EquipRemove::equipItem(_selectedItem, textPos, errMsg)) {
		_mode = ARMS_MODE;
		populateItems();
		redraw();
	} else {
		displayMessage(errMsg);
	}
}

void CharacterInventory::removeItem() {
	Common::String errMsg;
	Common::Point textPos;

	if (EquipRemove::removeItem(_selectedItem, textPos, errMsg)) {
		_mode = BACKPACK_MODE;
		populateItems();
		redraw();
	} else {
		displayMessage(errMsg);
	}
}

void CharacterInventory::useItem() {
	Character &c = *g_globals->_currCharacter;
	Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;
	Inventory::Entry *invEntry = &inv[_selectedItem];
	Common::String msg;

	if (g_events->isInCombat())
		msg = Game::UseItem::combatUseItem(inv, *invEntry, _mode == BACKPACK_MODE);
	else
		msg = Game::UseItem::nonCombatUseItem(inv, *invEntry, _mode == BACKPACK_MODE);

	if (!msg.empty())
		displayMessage(msg);
	else
		g_events->replaceView("Game", true);
}

void CharacterInventory::discardItem() {
	Character &c = *g_globals->_currCharacter;
	Inventory &inv = (_mode == ARMS_MODE) ? c._equipped : c._backpack;

	inv.removeAt(_selectedItem);
	populateItems();
	redraw();
}

void CharacterInventory::tradeItem(Character *dst) {
	if (dst == g_globals->_currCharacter)
		return;

	// Get source and dest inventories
	Character &cSrc = *g_globals->_currCharacter;
	Inventory &iSrc = (_mode == ARMS_MODE) ? cSrc._equipped : cSrc._backpack;
	Character &cDest = *dst;
	Inventory &iDest = cDest._backpack;

	if (iDest.full()) {
		backpackFull();

	} else {
		Inventory::Entry item = iSrc[_selectedItem];
		iSrc.removeAt(_selectedItem);
		iDest.add(item._id, item._charges);

		populateItems();
		redraw();
	}
}

void CharacterInventory::trade(const Common::String &mode, int amount, Character *destChar) {
	assert(isFocused());
	Character &src = *g_globals->_currCharacter;

	if (mode == "GEMS") {
		// Gems are serialized as uint16 (maximum 65,535).  Keep the
		// calculation wide enough for a five-digit entered amount and cap
		// the transfer to both the source balance and destination capacity.
		// This prevents truncation above 65,535 and prevents excess gems
		// from disappearing when the recipient is near the cap.
		uint32 transfer = MIN<uint32>((uint32)amount, (uint32)src._gems);
		transfer = MIN<uint32>(transfer, 0xffffU - (uint32)destChar->_gems);
		src._gems -= (uint16)transfer;
		destChar->_gems += (uint16)transfer;

	} else if (mode == "GOLD") {
		// Gold is a 24-bit value. The old 16-bit limit truncated both the
		// requested amount and the source gold above 65,535
		uint32 transfer = MIN<uint32>((uint32)amount, src._gold);
		if (destChar->_gold >= 0xffffff) {
			transfer = 0;
		} else {
			transfer = MIN<uint32>(transfer, 0xffffff - destChar->_gold);
		}
		src._gold -= transfer;
		destChar->_gold += transfer;

	} else if (mode == "FOOD") {
		amount = MIN<uint16>(amount, src._food);
		src._food -= amount;
		destChar->_food = MIN(destChar->_food + amount, MAX_FOOD);
	}

	redraw();
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
