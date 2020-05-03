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

#ifndef ULTIMA4_VIEWS_STATS_H
#define ULTIMA4_VIEWS_STATS_H

#include "ultima/ultima4/core/observable.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/views/menu.h"
#include "ultima/ultima4/views/textview.h"

namespace Ultima {
namespace Ultima4 {

struct SaveGame;
class Aura;
class Ingredients;
class Menu;
class MenuEvent;
class Party;
class PartyEvent;

#define STATS_AREA_WIDTH 15
#define STATS_AREA_HEIGHT 8
#define STATS_AREA_X TEXT_AREA_X
#define STATS_AREA_Y 1

enum StatsView {
	STATS_PARTY_OVERVIEW,
	STATS_CHAR1,
	STATS_CHAR2,
	STATS_CHAR3,
	STATS_CHAR4,
	STATS_CHAR5,
	STATS_CHAR6,
	STATS_CHAR7,
	STATS_CHAR8,
	STATS_WEAPONS,
	STATS_ARMOR,
	STATS_EQUIPMENT,
	STATS_ITEMS,
	STATS_REAGENTS,
	STATS_MIXTURES,
	MIX_REAGENTS
};

class StatsArea : public Observer<Aura *>, public Observer<Party *, PartyEvent &>,
		public Observer<Menu *, MenuEvent &>, public Observable<StatsArea *, Common::String> {
public:
	StatsArea();

	void setView(StatsView view);

	void clear();

	/**
	 * Sets the stats item to the previous in sequence.
	 */
	void prevItem();

	/**
	 * Sets the stats item to the next in sequence.
	 */
	void nextItem();

	/**
	 * Update the stats (ztats) box on the upper right of the screen.
	 */
	virtual void update(bool avatarOnly = false);
	void update(Aura *observable, NoArg *arg) override;
	void update(Aura *aura) override;
	void update(Party *party, PartyEvent &event) override;
	void update(Menu *menu, MenuEvent &event) override;

	void highlightPlayer(int player);

	/**
	 * Redraws the entire stats area
	 */
	void redraw();

	TextView *getMainArea() {
		return &_mainArea;
	}

	void resetReagentsMenu();
	Menu *getReagentsMenu() {
		return &_reagentsMixMenu;
	}

private:
	/**
	 * The basic party view.
	 */
	void showPartyView(bool avatarOnly);

	/**
	 * The individual character view.
	 */
	void showPlayerDetails();

	/**
	 * Weapons in inventory.
	 */
	void showWeapons();

	/**
	 * Armor in inventory.
	 */
	void showArmor();

	/**
	 * Equipment: touches, gems, keys, and sextants.
	 */
	void showEquipment();

	/**
	 * Items: runes, stones, and other miscellaneous quest items.
	 */
	void showItems();

	/**
	 * Unmixed reagents in inventory.
	 */
	void showReagents(bool active = false);

	/**
	 * Mixed reagents in inventory.
	 */
	void showMixtures();

	/**
	 * Sets the title of the stats area.
	 */
	void setTitle(const Common::String &s);

	TextView _title;
	TextView _mainArea;
	TextView _summary;

	StatsView _view;

	Menu _reagentsMixMenu;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
