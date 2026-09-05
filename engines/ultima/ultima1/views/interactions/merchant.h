
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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_MERCHANT_H
#define ULTIMA1_VIEWS_INTERACTIONS_MERCHANT_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * The kind of shop found at the counter tile the player is standing on
 */
enum ShopType {
	SHOP_ARMORY, SHOP_WEAPONS, SHOP_MAGIC, SHOP_GROCER, SHOP_TAVERN, SHOP_TRANSPORT
};

/**
 * Handler for the buy/sell side of the Transact command within cities.
 * Only usable when standing at a shop counter tile - the specific shop is
 * determined from the tile at focus time
 */
class Merchant : public Interaction {
	enum Mode {
		SHOP_PROMPT, SHOP_BUY, SHOP_SELL, SHOP_QTY
	};
private:
	ShopType _shopType = SHOP_ARMORY;
	Mode _mode = SHOP_PROMPT;
	Common::String _amount;
	// Which of the 6 transports (Horse..Shuttle) are offered this visit,
	// indexed by the letter (a-f) used to select them; 0 means unavailable
	int _transportIds[6] = { 0, 0, 0, 0, 0, 0 };

	void nothing();
	void cantAfford();
	void finish();
	void redrawStats();

	/**
	 * Returns which of the 8 flavor variants of the current shop type is
	 * showing, based on the current city's map style
	 */
	int shopVariant() const;

	/**
	 * Draws the current shop's proprietor name(s), if any, centered near
	 * the top of the dialog
	 */
	void drawShopName();

	/**
	 * Returns true if a wench (or, for a female player, a lecher) NPC is
	 * standing next to the player - used by the tavern's drunk penalty
	 */
	bool isWenchNearby() const;

	/**
	 * Returns the number of vehicle entities currently out on the
	 * overworld map, used to cap the transport shop's available fleet
	 */
	int overworldTransportCount() const;

	void chooseBuy();
	void chooseSell();

	void drawArmoryBuy();
	void drawArmorySell();
	void armoryBuyKeypress(Common::KeyCode keycode);
	void armorySellKeypress(Common::KeyCode keycode);

	/**
	 * Works out the inclusive min/max weapon indexes this shop currently
	 * offers - every second index starting from a base that alternates by
	 * map style, extended over time by the move counter
	 */
	void weaponsRange(int &minIdx, int &maxIdx) const;
	void drawWeaponsBuy();
	void drawWeaponsSell();
	void weaponsBuyKeypress(Common::KeyCode keycode);
	void weaponsSellKeypress(Common::KeyCode keycode);

	/**
	 * Works out the inclusive min/max spell indexes this shop currently
	 * offers - every second index starting from a base that alternates by
	 * map style, extended for Clerics. The Steal spell is never offered
	 */
	void magicRange(int &minIdx, int &maxIdx) const;
	void drawMagicBuy();
	void magicBuyKeypress(Common::KeyCode keycode);

	void groceryPrompt();
	void groceryKeypress(const KeypressMessage &msg);

	void tavernBuy();

	void drawTransportBuy();
	void transportBuyKeypress(Common::KeyCode keycode);

	/**
	 * Places a newly bought vehicle on the overworld next to the city,
	 * on whichever adjacent tile matches the terrain it needs
	 */
	void spawnTransport(int transportId);

public:
	Merchant();
	~Merchant() override {
	}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
