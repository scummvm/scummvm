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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_SHOPS_H
#define ULTIMA3_VIEWS_INTERACTIONS_SHOPS_H

#include "ultima/ultima3/views/interactions/log_input.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * What the shops, inns and temples of a town have in common: the party
 * member who deals with them is the one who pays
 */
class Shop : public Interaction {
protected:
	int _payer;

	Data::RosterEntry &payer() const;

	/**
	 * Returns true if the payer has at least an amount of gold
	 */
	bool canAfford(int gold) const;

	/**
	 * Takes gold from the payer
	 */
	void spend(int gold);

	/**
	 * Writes some text, followed by a sound if one is given
	 */
	void say(const char *text, int sound = 0);

public:
	Shop(int payer) : _payer(payer) {}
};

/**
 * The pub, where the more is paid for a drink the more is learnt
 */
class Tavern : public Shop {
private:
	LogNumber _number;
	LogYesNo _another;
	bool _asking = false;

	void askForPayment();

public:
	Tavern(int payer);
	bool keypress(const KeypressMessage &msg) override;
};

/**
 * Rations of food are sold at the grocer, a gold piece each
 */
class Grocer : public Shop {
private:
	LogNumber _number;

public:
	Grocer(int payer);
	bool keypress(const KeypressMessage &msg) override;
};

/**
 * The temple cures, heals, resurrects and recalls for a price
 */
class Temple : public Shop {
private:
	enum Stage {
		CHOOSE_SERVICE,
		CONFIRM,
		CHOOSE_PLAYER
	};

	Stage _stage = CHOOSE_SERVICE;
	int _service = 0;
	LogMenu _menu;
	LogYesNo _confirm;
	PlayerChooser _chooser;

	void farewell();
	bool serve(int slot);

public:
	Temple(int payer);
	bool keypress(const KeypressMessage &msg) override;
};

/**
 * Weapons and armour are bought and sold at the shops for each
 */
class EquipmentShop : public Shop {
private:
	enum Stage {
		ASK_LIST,
		LIST_PAGE,
		ASK_ACTION,
		CHOOSE_ITEM
	};

	bool _weapons;
	Stage _stage = ASK_LIST;
	int _page = 0;
	bool _buying = false;
	LogYesNo _list;
	LogMenu _action;
	LogMenu _item;
	char _keys[20];
	const char *_words[20];

	/**
	 * Returns true if the shop is the one holding the best equipment
	 */
	bool isBestShop() const;

	/**
	 * Returns how many different things the shop deals in
	 */
	int itemCount() const;

	void showPage();
	void askAction();
	void askItem();
	bool buy(int item);
	bool sell(int item);

public:
	EquipmentShop(int payer, bool weapons);
	bool keypress(const KeypressMessage &msg) override;
};

/**
 * The guild sells keys, torches, powder and gems
 */
class Guild : public Shop {
private:
	enum Stage {
		CHOOSE_GOODS,
		CHOOSE_AMOUNT,
		ANYTHING_ELSE
	};

	Stage _stage = CHOOSE_GOODS;
	int _goods = 0;
	LogMenu _menu;
	LogNumber _number;
	LogYesNo _more;

	void showGoods();

public:
	Guild(int payer);
	bool keypress(const KeypressMessage &msg) override;
};

/**
 * The oracle gives clues in return for gold
 */
class Oracle : public Shop {
private:
	LogNumber _number;
	LogYesNo _more;
	bool _asking = false;

	void askForOffering();

public:
	Oracle(int payer);
	bool keypress(const KeypressMessage &msg) override;
};

/**
 * Horses are sold at the stables, for the whole party
 */
class Stable : public Shop {
private:
	LogYesNo _buy;

public:
	Stable(int payer);
	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
