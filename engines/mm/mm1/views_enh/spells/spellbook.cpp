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

#include "mm/mm1/views_enh/spells/spellbook.h"
#include "common/config-manager.h"
#include "mm/mm1/data/text.h"
#include "mm/mm1/game/spells_party.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

Spellbook::Spellbook() : PartyView("Spellbook") {
	_bounds = Common::Rect(27, 6, 208, 142);
	addButtons();
}

void Spellbook::addButtons() {
	_scrollSprites.load("scroll.icn");
	addButton(&g_globals->_mainIcons, Common::Point(187, 26), 0, Common::KEYCODE_UP);
	addButton(&g_globals->_mainIcons, Common::Point(187, 111), 2, Common::KEYCODE_DOWN);
	addButton(&_scrollSprites, Common::Point(100, 109), 5, KEYBIND_SELECT);

	addButton(Common::Rect(5, 14, 152, 22), Common::KEYCODE_1);
	addButton(Common::Rect(5, 23, 152, 31), Common::KEYCODE_2);
	addButton(Common::Rect(5, 32, 152, 40), Common::KEYCODE_3);
	addButton(Common::Rect(5, 41, 152, 49), Common::KEYCODE_4);
	addButton(Common::Rect(5, 50, 152, 58), Common::KEYCODE_5);
	addButton(Common::Rect(5, 59, 152, 67), Common::KEYCODE_6);
	addButton(Common::Rect(5, 68, 152, 76), Common::KEYCODE_7);
	addButton(Common::Rect(5, 77, 152, 85), Common::KEYCODE_8);
	addButton(Common::Rect(5, 86, 152, 94), Common::KEYCODE_9);
	addButton(Common::Rect(5, 95, 152, 103), Common::KEYCODE_0);

	addButton(Common::Rect(139, 109, 163, 119), KEYBIND_ESCAPE);
	addButton(Common::Rect(152, 21, 163, 59), Common::KEYCODE_PAGEUP);
	addButton(Common::Rect(152, 60, 163, 98), Common::KEYCODE_PAGEDOWN);
	addButton(Common::Rect(97, 109, 163, 119), KEYBIND_SELECT);
}

bool Spellbook::msgFocus(const FocusMessage &msg) {
	if (!isInCombat())
		PartyView::msgFocus(msg);

	// In this view we don't want 1 to 6 mapping to char selection
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	updateChar();
	return true;
}

bool Spellbook::msgUnfocus(const UnfocusMessage &msg) {
	if (!isInCombat())
		PartyView::msgUnfocus(msg);
	return true;
}

bool Spellbook::canSwitchChar() {
	return !g_events->isInCombat();
}

void Spellbook::draw() {
	if (isInCombat()) {
		ScrollView::draw();
	} else {
		PartyView::draw();
	}

	Graphics::ManagedSurface s = getSurface();
	const Character &c = *g_globals->_currCharacter;

	// Draw the scrolling area frame
	_scrollSprites.draw(&s, 4, Common::Point(14, 20));
	_scrollSprites.draw(&s, 0, Common::Point(162, 20));
	_scrollSprites.draw(&s, 2, Common::Point(162, 105));

	// Title line
	_fontReduced = true;
	Common::String title = Common::String::format("Spells for %s",
		c._name
	);
	title = truncateString(title, _innerBounds.width());
	writeString(0, 0, title, ALIGN_MIDDLE);

	// Write current spell points
	Common::String sp = Common::String::format("Spell Pts - %d", c._sp._current);
	writeString(7, 111, sp);

	// Iterate over the lines
	for (int i = 0; i < 10; ++i) {
		// Left gutter row number
		setTextColor(0);
		const int yp = 15 + 9 * i;
		writeString(0, yp, Common::String::format("%c", (i == 9) ? '0' : '1' + i));

		const int displayIndex = _topIndex + i;

		if (_count == 0) {
			if (i == 0) {
				setTextColor(37);
				writeString(12, yp, "Not a spell caster...");
			}
		} else if (displayIndex < _count) {
			const int spellIndex = _spellOrder[displayIndex];

			// Spell requirements
			int lvl, num;
			getSpellLevelNum(CATEGORY_SPELLS_COUNT * (_isWizard ? 1 : 0) + spellIndex, lvl, num);
			setSpell(g_globals->_currCharacter, lvl, num);

			const int textColor = (displayIndex == _selectedIndex) ? 15 :
				(canCast() ? 37 : 1);
			setTextColor(textColor);

			// Spell name and requirements
			Common::String spellName = _isWizard ?
				WIZARD_SPELL_NAMES[spellIndex] : CLERIC_SPELL_NAMES[spellIndex];

			// MM1 Cleric 2-5 is Protection From Fire.
			if (!_isWizard && spellIndex == 12)
				spellName = "Protection From Fire";

			// Mark the spell being assigned, and existing favorites
			if (_favoriteAssignMode && spellIndex == _favoriteAssignSpell) {
				setTextColor(15);
				writeString(7, yp, "X");
				setTextColor(textColor);
			} else if (isFavorite(spellIndex)) {
				setTextColor(15);
				writeString(7, yp, "*");
				setTextColor(textColor);
			}

			writeString(14, yp, spellName);
			writeString(152, yp, Common::String::format("%d/%d",
				_requiredSp, _requiredGems), ALIGN_RIGHT);
		}
	}
}

bool Spellbook::msgKeypress(const KeypressMessage &msg) {
	// MM1-NUMPAD-PAGE-SCROLL: ScummVM reports the navigation-labelled
	// numpad keys as KP9/KP3 on some Windows/SDL keyboard paths. Treat them
	// as Page Up/Page Down only while Num Lock is OFF, preserving numeric
	// keypad input while Num Lock is ON.
	const bool pageUpKey = msg.keycode == Common::KEYCODE_PAGEUP ||
		(msg.keycode == Common::KEYCODE_KP9 && !(msg.flags & Common::KBD_NUM));
	const bool pageDownKey = msg.keycode == Common::KEYCODE_PAGEDOWN ||
		(msg.keycode == Common::KEYCODE_KP3 && !(msg.flags & Common::KBD_NUM));

	if (msg.keycode >= Common::KEYCODE_0 && msg.keycode <= Common::KEYCODE_9) {
		const int shortcut = (msg.keycode == Common::KEYCODE_0) ?
			9 : msg.keycode - Common::KEYCODE_1;

		if (_favoriteAssignMode) {
			assignFavoriteSlot(shortcut);
		} else {
			const int newIndex = _topIndex + shortcut;
			if (newIndex < _count) {
				_selectedIndex = newIndex;
				redraw();
			}
		}
	} else if (msg.keycode == Common::KEYCODE_x) {
		if (_favoriteAssignMode)
			removeFavorite(_favoriteAssignSpell);
		else
			beginFavoriteAssignment();

	} else if (pageUpKey) {
		if (_topIndex > 0) {
			_topIndex = MAX(_topIndex - 10, 0);
			redraw();
		}
	} else if (pageDownKey) {
		int newTopIndex = _topIndex + 10;
		if (newTopIndex < _count) {
			_topIndex = newTopIndex;
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_UP) {
		if (_topIndex > 0) {
			--_topIndex;
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_DOWN) {
		if ((_topIndex + 10) < _count) {
			++_topIndex;
			redraw();
		}
	} else if (msg.keycode == Common::KEYCODE_s) {
		// Alternate alias for Select button
		msgAction(ActionMessage(KEYBIND_SELECT));

	} else if (!isInCombat()) {
		return PartyView::msgKeypress(msg);
	}

	return true;
}

bool Spellbook::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		return true;

	case KEYBIND_SELECT:
		spellSelected();
		close();
		return true;

	default:
		break;
	}

	return false;
}

bool Spellbook::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		updateChar();
		return true;
	} else if (!isInCombat()) {
		return PartyView::msgGame(msg);
	} else {
		return true;
	}
}

Common::String Spellbook::favoritesKey(int slot) const {
	// Favorites are stored per character, identified by name and class
	const Common::String name = g_globals->_currCharacter->_name;
	uint32 hash = 2166136261U;

	for (uint i = 0; i < name.size(); ++i) {
		hash ^= (byte)name[i];
		hash *= 16777619U;
	}

	return Common::String::format(
		"mm1_spell_favorite_%08x_%d_slot_%d",
		hash, (int)g_globals->_currCharacter->_class, slot);
}

void Spellbook::loadFavorites() {
	for (int slot = 0; slot < 10; ++slot) {
		const Common::String key = favoritesKey(slot);

		// Stored value is spell index + 1; zero means unused slot.
		const int value = ConfMan.hasKey(key) ? ConfMan.getInt(key) : 0;
		_favoriteSlots[slot] = value - 1;

		if (_favoriteSlots[slot] < -1 ||
				_favoriteSlots[slot] >= CATEGORY_SPELLS_COUNT)
			_favoriteSlots[slot] = -1;
	}
}

void Spellbook::saveFavorites() const {
	for (int slot = 0; slot < 10; ++slot)
		ConfMan.setInt(favoritesKey(slot), _favoriteSlots[slot] + 1);

	ConfMan.flushToDisk();
}

int Spellbook::favoriteSlot(int spellIndex) const {
	for (int slot = 0; slot < 10; ++slot) {
		if (_favoriteSlots[slot] == spellIndex)
			return slot;
	}

	return -1;
}

bool Spellbook::isFavorite(int spellIndex) const {
	return favoriteSlot(spellIndex) != -1;
}

void Spellbook::beginFavoriteAssignment() {
	if (_selectedIndex < 0 || _selectedIndex >= _count)
		return;

	_favoriteAssignSpell = _spellOrder[_selectedIndex];
	_favoriteAssignMode = true;
	redraw();
}

void Spellbook::assignFavoriteSlot(int slot) {
	if (!_favoriteAssignMode || slot < 0 || slot >= 10 ||
			_favoriteAssignSpell < 0)
		return;

	const int spellIndex = _favoriteAssignSpell;
	const int knownCount = _count;

	// Moving an existing favorite removes its previous assignment.
	for (int i = 0; i < 10; ++i) {
		if (_favoriteSlots[i] == spellIndex)
			_favoriteSlots[i] = -1;
	}

	// Replacing a slot simply removes the previous spell's favorite
	// assignment and puts this spell in that shortcut position.
	_favoriteSlots[slot] = spellIndex;
	saveFavorites();

	_favoriteAssignMode = false;
	_favoriteAssignSpell = -1;
	buildSpellOrder(knownCount);

	// Keep the newly assigned spell highlighted.
	_selectedIndex = 0;
	for (int i = 0; i < _count; ++i) {
		if (_spellOrder[i] == spellIndex) {
			_selectedIndex = i;
			break;
		}
	}

	_topIndex = (_selectedIndex / 10) * 10;
	redraw();
}

void Spellbook::removeFavorite(int spellIndex) {
	const int knownCount = _count;
	bool changed = false;

	for (int slot = 0; slot < 10; ++slot) {
		if (_favoriteSlots[slot] == spellIndex) {
			_favoriteSlots[slot] = -1;
			changed = true;
		}
	}

	_favoriteAssignMode = false;
	_favoriteAssignSpell = -1;

	if (changed)
		saveFavorites();

	buildSpellOrder(knownCount);

	_selectedIndex = 0;
	for (int i = 0; i < _count; ++i) {
		if (_spellOrder[i] == spellIndex) {
			_selectedIndex = i;
			break;
		}
	}

	_topIndex = (_selectedIndex / 10) * 10;
	redraw();
}

void Spellbook::buildSpellOrder(int knownCount) {
	_spellOrder.clear();

	bool used[CATEGORY_SPELLS_COUNT] = { false };
	int nextNormal = 0;

	// The first ten visible entries correspond directly to shortcuts
	// 1-0. A favorite assigned to a slot is fixed to that exact row.
	for (int slot = 0; slot < 10 &&
			(int)_spellOrder.size() < knownCount; ++slot) {
		const int favorite = _favoriteSlots[slot];

		if (favorite >= 0 && favorite < knownCount && !used[favorite]) {
			_spellOrder.push_back(favorite);
			used[favorite] = true;
			continue;
		}

		// Fill an unused favorite slot with the next ordinary spell,
		// while reserving all explicitly assigned favorites for their
		// designated shortcut positions.
		while (nextNormal < knownCount &&
				(used[nextNormal] || isFavorite(nextNormal)))
			++nextNormal;

		if (nextNormal < knownCount) {
			_spellOrder.push_back(nextNormal);
			used[nextNormal] = true;
			++nextNormal;
		}
	}

	// Append every remaining spell in normal MM1 order.
	for (int spellIndex = 0; spellIndex < knownCount; ++spellIndex) {
		if (!used[spellIndex])
			_spellOrder.push_back(spellIndex);
	}

	_count = (int)_spellOrder.size();
}
void Spellbook::updateChar() {
	// Refresh the cast spell side dialog for new character
	send("CastSpell", GameMessage("UPDATE"));

	// Update the highlighted char in the party display
	g_events->send(GameMessage("CHAR_HIGHLIGHT", (int)true));

	// Update fields
	const Character &c = *g_globals->_currCharacter;
	_isWizard = c._class == SORCERER || c._class == ARCHER;

	int knownCount;
	if (c._spellLevel._current == 0) {
		knownCount = 0;
	} else {
		knownCount = (c._spellLevel._current < 5) ?
				c._spellLevel._current * 8 :
				32 + (c._spellLevel._current - 4) * 5;
	}

	_favoriteAssignMode = false;
	_favoriteAssignSpell = -1;
	loadFavorites();
	buildSpellOrder(knownCount);

	// Reopen on the page containing the previously selected spell
	const int selectedSpell = (g_events->isInCombat() ?
		c._combatSpell : c._nonCombatSpell) % CATEGORY_SPELLS_COUNT;

	_selectedIndex = 0;
	if (selectedSpell >= 0) {
		for (int i = 0; i < _count; ++i) {
			if (_spellOrder[i] == selectedSpell) {
				_selectedIndex = i;
				break;
			}
		}
	}

	_topIndex = (_selectedIndex / 10) * 10;
	redraw();
}

void Spellbook::spellSelected() {
	Character &c = *g_globals->_currCharacter;
	if (_selectedIndex < 0 || _selectedIndex >= _count)
		return;

	int spellIndex = (_isWizard ? CATEGORY_SPELLS_COUNT : 0) +
			_spellOrder[_selectedIndex];

	// Set the selected spell for the character
	c.setSpellNumber(spellIndex);
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
