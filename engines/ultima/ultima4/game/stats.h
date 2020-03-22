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

#ifndef ULTIMA4_STATS_H
#define ULTIMA4_STATS_H

#include "ultima/ultima4/core/observable.h"
#include "ultima/ultima4/core/observer.h"
#include "ultima/ultima4/game/menu.h"
#include "ultima/ultima4/game/textview.h"

namespace Ultima {
namespace Ultima4 {

struct SaveGame;
class Aura;
class Ingredients;
class Menu;
class MenuEvent;
class Party;
class PartyEvent;

using Common::String;

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

class StatsArea : public Observer<Aura *>, public Observer<Party *, PartyEvent &>, public Observer<Menu *, MenuEvent &>, public Observable<StatsArea *, Common::String> {
public:
	StatsArea();

	void setView(StatsView view);

	void clear();
	void prevItem();
	void nextItem();
	void update(bool avatarOnly = false);
	virtual void update(Aura *aura);
	virtual void update(Party *party, PartyEvent &event)    {
		update(); /* do a full update */
	}
	virtual void update(Menu *menu, MenuEvent &event)       {
		update(); /* do a full update */
	}
	void highlightPlayer(int player);
	void redraw();

	TextView *getMainArea() {
		return &_mainArea;
	}

	void resetReagentsMenu();
	Menu *getReagentsMenu() {
		return &_reagentsMixMenu;
	}

private:
	void showPartyView(bool avatarOnly);
	void showPlayerDetails();
	void showWeapons();
	void showArmor();
	void showEquipment();
	void showItems();
	void showReagents(bool active = false);
	void showMixtures();
	void setTitle(const Common::String &s);

	TextView _title;
	TextView _mainArea;
	TextView _summary;

	StatsView _view;

	Menu _reagentsMixMenu;
};

/**
 * Controller for the reagents menu used when mixing spells.  Fills
 * the passed in Ingredients with the selected reagents.
 */
class ReagentsMenuController : public MenuController {
public:
	ReagentsMenuController(Menu *menu, Ingredients *i, TextView *view) : MenuController(menu, view), _ingredients(i) { }

	bool keyPressed(int key);

private:
	Ingredients *_ingredients;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
