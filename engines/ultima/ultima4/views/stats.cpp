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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/core/config.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/views/stats.h"
#include "ultima/ultima4/game/armor.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/game/names.h"
#include "ultima/ultima4/game/player.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/spell.h"
#include "ultima/ultima4/map/tile.h"
#include "ultima/ultima4/game/weapon.h"

namespace Ultima {
namespace Ultima4 {

/**
 * StatsArea class implementation
 */
StatsArea::StatsArea() :
	_title(STATS_AREA_X * CHAR_WIDTH, 0 * CHAR_HEIGHT, STATS_AREA_WIDTH, 1),
	_mainArea(STATS_AREA_X * CHAR_WIDTH, STATS_AREA_Y * CHAR_HEIGHT, STATS_AREA_WIDTH, STATS_AREA_HEIGHT),
	_summary(STATS_AREA_X * CHAR_WIDTH, (STATS_AREA_Y + STATS_AREA_HEIGHT + 1) * CHAR_HEIGHT, STATS_AREA_WIDTH, 1),
	_view(STATS_PARTY_OVERVIEW) {
	// Generate a formatted Common::String for each menu item,
	// and then add the item to the menu.  The Y value
	// for each menu item will be filled in later.
	for (int count = 0; count < 8; count++) {
		char outputBuffer[16];
		snprintf(outputBuffer, sizeof(outputBuffer), "-%-11s%%s", getReagentName((Reagent)count));
		_reagentsMixMenu.add(count, new IntMenuItem(outputBuffer, 1, 0, -1, (int *)g_context->_party->getReagentPtr((Reagent)count), 0, 99, 1, MENU_OUTPUT_REAGENT));
	}

	_reagentsMixMenu.addObserver(this);
}

void StatsArea::setView(StatsView view) {
	this->_view = view;
	update();
}

void StatsArea::prevItem() {
	_view = (StatsView)(_view - 1);
	if (_view < STATS_CHAR1)
		_view = STATS_MIXTURES;
	if (_view <= STATS_CHAR8 && (_view - STATS_CHAR1 + 1) > g_context->_party->size())
		_view = (StatsView)(STATS_CHAR1 - 1 + g_context->_party->size());
	update();
}

void StatsArea::nextItem() {
	_view = (StatsView)(_view + 1);
	if (_view > STATS_MIXTURES)
		_view = STATS_CHAR1;
	if (_view <= STATS_CHAR8 && (_view - STATS_CHAR1 + 1) > g_context->_party->size())
		_view = STATS_WEAPONS;
	update();
}

void StatsArea::update(bool avatarOnly) {
	clear();

	/*
	 * update the upper stats box
	 */
	switch (_view) {
	case STATS_PARTY_OVERVIEW:
		showPartyView(avatarOnly);
		break;
	case STATS_CHAR1:
	case STATS_CHAR2:
	case STATS_CHAR3:
	case STATS_CHAR4:
	case STATS_CHAR5:
	case STATS_CHAR6:
	case STATS_CHAR7:
	case STATS_CHAR8:
		showPlayerDetails();
		break;
	case STATS_WEAPONS:
		showWeapons();
		break;
	case STATS_ARMOR:
		showArmor();
		break;
	case STATS_EQUIPMENT:
		showEquipment();
		break;
	case STATS_ITEMS:
		showItems();
		break;
	case STATS_REAGENTS:
		showReagents();
		break;
	case STATS_MIXTURES:
		showMixtures();
		break;
	case MIX_REAGENTS:
		showReagents(true);
		break;
	}

	/*
	 * update the lower stats box (food, gold, etc.)
	 */
	if (g_context->_transportContext == TRANSPORT_SHIP)
		_summary.textAt(0, 0, "F:%04d   SHP:%02d", g_ultima->_saveGame->_food / 100, g_ultima->_saveGame->_shipHull);
	else
		_summary.textAt(0, 0, "F:%04d   G:%04d", g_ultima->_saveGame->_food / 100, g_ultima->_saveGame->_gold);

	update(g_context->_aura);

	redraw();
}

void StatsArea::update(Aura *observable, NoArg *arg) {
	Observer<Aura *>::update(observable, arg);
}

void StatsArea::update(Aura *aura) {
	byte mask = 0xff;
	for (int i = 0; i < VIRT_MAX; i++) {
		if (g_ultima->_saveGame->_karma[i] == 0)
			mask &= ~(1 << i);
	}

	switch (aura->getType()) {
	case Aura::NONE:
		_summary.drawCharMasked(0, STATS_AREA_WIDTH / 2, 0, mask);
		break;
	case Aura::HORN:
		_summary.drawChar(CHARSET_REDDOT, STATS_AREA_WIDTH / 2, 0);
		break;
	case Aura::JINX:
		_summary.drawChar('J', STATS_AREA_WIDTH / 2, 0);
		break;
	case Aura::NEGATE:
		_summary.drawChar('N', STATS_AREA_WIDTH / 2, 0);
		break;
	case Aura::PROTECTION:
		_summary.drawChar('P', STATS_AREA_WIDTH / 2, 0);
		break;
	case Aura::QUICKNESS:
		_summary.drawChar('Q', STATS_AREA_WIDTH / 2, 0);
		break;
	}

	_summary.update();
}

void StatsArea::update(Party *party, PartyEvent &event) {
	update();	// Do a full update
}

void StatsArea::update(Menu *menu, MenuEvent &event) {
	update();	// Do a full update
}

void StatsArea::highlightPlayer(int player) {
	assertMsg(player < g_context->_party->size(), "player number out of range: %d", player);
	_mainArea.highlight(0, player * CHAR_HEIGHT, STATS_AREA_WIDTH * CHAR_WIDTH, CHAR_HEIGHT);
#ifdef IOS_ULTIMA4
	U4IOS::updateActivePartyMember(player);
#endif
}

void StatsArea::clear() {
	for (int i = 0; i < STATS_AREA_WIDTH; i++)
		_title.drawChar(CHARSET_HORIZBAR, i, 0);

	_mainArea.clear();
	_summary.clear();
}

void StatsArea::redraw() {
	_title.update();
	_mainArea.update();
	_summary.update();
}

void StatsArea::setTitle(const Common::String &s) {
	int titleStart = (STATS_AREA_WIDTH / 2) - ((s.size() + 2) / 2);
	_title.textAt(titleStart, 0, "%c%s%c", 16, s.c_str(), 17);
}

void StatsArea::showPartyView(bool avatarOnly) {
	const char *format = "%d%c%-9.8s%3d%s";

	PartyMember *p = nullptr;
	int activePlayer = g_context->_party->getActivePlayer();

	assertMsg(g_context->_party->size() <= 8, "party members out of range: %d", g_context->_party->size());

	if (!avatarOnly) {
		for (int i = 0; i < g_context->_party->size(); i++) {
			p = g_context->_party->member(i);
			_mainArea.textAt(0, i, format, i + 1, (i == activePlayer) ? CHARSET_BULLET : '-', p->getName().c_str(), p->getHp(), _mainArea.colorizeStatus(p->getStatus()).c_str());
		}
	} else {
		p = g_context->_party->member(0);
		_mainArea.textAt(0, 0, format, 1, (activePlayer == 0) ? CHARSET_BULLET : '-', p->getName().c_str(), p->getHp(), _mainArea.colorizeStatus(p->getStatus()).c_str());
	}
}

void StatsArea::showPlayerDetails() {
	int player = _view - STATS_CHAR1;

	assertMsg(player < 8, "character number out of range: %d", player);

	PartyMember *p = g_context->_party->member(player);
	setTitle(p->getName());
	_mainArea.textAt(0, 0, "%c             %c", p->getSex(), p->getStatus());
	Common::String classStr = getClassName(p->getClass());
	int classStart = (STATS_AREA_WIDTH / 2) - (classStr.size() / 2);
	_mainArea.textAt(classStart, 0, "%s", classStr.c_str());
	_mainArea.textAt(0, 2, " MP:%02d  LV:%d", p->getMp(), p->getRealLevel());
	_mainArea.textAt(0, 3, "STR:%02d  HP:%04d", p->getStr(), p->getHp());
	_mainArea.textAt(0, 4, "DEX:%02d  HM:%04d", p->getDex(), p->getMaxHp());
	_mainArea.textAt(0, 5, "INT:%02d  EX:%04d", p->getInt(), p->getExp());
	_mainArea.textAt(0, 6, "W:%s", p->getWeapon()->getName().c_str());
	_mainArea.textAt(0, 7, "A:%s", p->getArmor()->getName().c_str());
}

void StatsArea::showWeapons() {
	setTitle("Weapons");

	int line = 0;
	int col = 0;
	_mainArea.textAt(0, line++, "A-%s", g_weapons->get(WEAP_HANDS)->getName().c_str());
	for (int w = WEAP_HANDS + 1; w < WEAP_MAX; w++) {
		int n = g_ultima->_saveGame->_weapons[w];
		if (n >= 100)
			n = 99;
		if (n >= 1) {
			const char *format = (n >= 10) ? "%c%d-%s" : "%c-%d-%s";

			_mainArea.textAt(col, line++, format, w - WEAP_HANDS + 'A', n, g_weapons->get((WeaponType) w)->getAbbrev().c_str());
			if (line >= (STATS_AREA_HEIGHT)) {
				line = 0;
				col += 8;
			}
		}
	}
}

void StatsArea::showArmor() {
	setTitle("Armour");

	int line = 0;
	_mainArea.textAt(0, line++, "A  -No Armour");
	for (int a = ARMR_NONE + 1; a < ARMR_MAX; a++) {
		if (g_ultima->_saveGame->_armor[a] > 0) {
			const char *format = (g_ultima->_saveGame->_armor[a] >= 10) ? "%c%d-%s" : "%c-%d-%s";

			_mainArea.textAt(0, line++, format, a - ARMR_NONE + 'A', g_ultima->_saveGame->_armor[a], g_armors->get((ArmorType) a)->getName().c_str());
		}
	}
}

void StatsArea::showEquipment() {
	setTitle("Equipment");

	int line = 0;
	_mainArea.textAt(0, line++, "%2d Torches", g_ultima->_saveGame->_torches);
	_mainArea.textAt(0, line++, "%2d Gems", g_ultima->_saveGame->_gems);
	_mainArea.textAt(0, line++, "%2d Keys", g_ultima->_saveGame->_keys);
	if (g_ultima->_saveGame->_sextants > 0)
		_mainArea.textAt(0, line++, "%2d Sextants", g_ultima->_saveGame->_sextants);
}

void StatsArea::showItems() {
	int i, j;
	char buffer[17];

	setTitle("Items");

	int line = 0;
	if (g_ultima->_saveGame->_stones != 0) {
		j = 0;
		for (i = 0; i < 8; i++) {
			if (g_ultima->_saveGame->_stones & (1 << i))
				buffer[j++] = getStoneName((Virtue) i)[0];
		}
		buffer[j] = '\0';
		_mainArea.textAt(0, line++, "Stones:%s", buffer);
	}
	if (g_ultima->_saveGame->_runes != 0) {
		j = 0;
		for (i = 0; i < 8; i++) {
			if (g_ultima->_saveGame->_runes & (1 << i))
				buffer[j++] = getVirtueName((Virtue) i)[0];
		}
		buffer[j] = '\0';
		_mainArea.textAt(0, line++, "Runes:%s", buffer);
	}
	if (g_ultima->_saveGame->_items & (ITEM_CANDLE | ITEM_BOOK | ITEM_BELL)) {
		buffer[0] = '\0';
		if (g_ultima->_saveGame->_items & ITEM_BELL) {
			strcat(buffer, getItemName(ITEM_BELL));
			strcat(buffer, " ");
		}
		if (g_ultima->_saveGame->_items & ITEM_BOOK) {
			strcat(buffer, getItemName(ITEM_BOOK));
			strcat(buffer, " ");
		}
		if (g_ultima->_saveGame->_items & ITEM_CANDLE) {
			strcat(buffer, getItemName(ITEM_CANDLE));
			buffer[15] = '\0';
		}
		_mainArea.textAt(0, line++, "%s", buffer);
	}
	if (g_ultima->_saveGame->_items & (ITEM_KEY_C | ITEM_KEY_L | ITEM_KEY_T)) {
		j = 0;
		if (g_ultima->_saveGame->_items & ITEM_KEY_T)
			buffer[j++] = getItemName(ITEM_KEY_T)[0];
		if (g_ultima->_saveGame->_items & ITEM_KEY_L)
			buffer[j++] = getItemName(ITEM_KEY_L)[0];
		if (g_ultima->_saveGame->_items & ITEM_KEY_C)
			buffer[j++] = getItemName(ITEM_KEY_C)[0];
		buffer[j] = '\0';
		_mainArea.textAt(0, line++, "3 Part Key:%s", buffer);
	}
	if (g_ultima->_saveGame->_items & ITEM_HORN)
		_mainArea.textAt(0, line++, "%s", getItemName(ITEM_HORN));
	if (g_ultima->_saveGame->_items & ITEM_WHEEL)
		_mainArea.textAt(0, line++, "%s", getItemName(ITEM_WHEEL));
	if (g_ultima->_saveGame->_items & ITEM_SKULL)
		_mainArea.textAt(0, line++, "%s", getItemName(ITEM_SKULL));
}

void StatsArea::showReagents(bool active) {
	setTitle("Reagents");

	Menu::MenuItemList::iterator i;
	int line = 0,
	    r = REAG_ASH;
	Common::String shortcut("A");

	_reagentsMixMenu.show(&_mainArea);

	for (i = _reagentsMixMenu.begin(); i != _reagentsMixMenu.end(); i++, r++) {
		if ((*i)->isVisible()) {
			// Insert the reagent menu item shortcut character
			shortcut.setChar('A' + r, 0);
			if (active)
				_mainArea.textAt(0, line++, "%s", _mainArea.colorizeString(shortcut, FG_YELLOW, 0, 1).c_str());
			else
				_mainArea.textAt(0, line++, "%s", shortcut.c_str());
		}
	}
}

void StatsArea::showMixtures() {
	setTitle("Mixtures");

	int line = 0;
	int col = 0;
	for (int s = 0; s < SPELL_MAX; s++) {
		int n = g_ultima->_saveGame->_mixtures[s];
		if (n >= 100)
			n = 99;
		if (n >= 1) {
			_mainArea.textAt(col, line++, "%c-%02d", s + 'A', n);
			if (line >= (STATS_AREA_HEIGHT)) {
				if (col >= 10)
					break;
				line = 0;
				col += 5;
			}
		}
	}
}

void StatsArea::resetReagentsMenu() {
	Menu::MenuItemList::iterator current;
	int i = 0,
	    row = 0;

	for (current = _reagentsMixMenu.begin(); current != _reagentsMixMenu.end(); current++) {
		if (g_ultima->_saveGame->_reagents[i++] > 0) {
			(*current)->setVisible(true);
			(*current)->setY(row++);
		} else (*current)->setVisible(false);
	}

	_reagentsMixMenu.reset(false);
}

} // End of namespace Ultima4
} // End of namespace Ultima
