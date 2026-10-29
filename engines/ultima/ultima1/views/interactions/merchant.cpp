
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

#include "ultima/ultima1/views/interactions/merchant.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {


// Same generic dialog box geometry the Lose and Win views use
constexpr int DIALOG_X1 = 31, DIALOG_Y1 = 23, DIALOG_X2 = 287, DIALOG_Y2 = 127;
constexpr int BORDER_X1 = 34, BORDER_Y1 = 26, BORDER_X2 = 284, BORDER_Y2 = 124;

struct ShopName {
	const char *_line1;
	const char *_line2;
};

// Per city (map style 2-9, i.e. variant 0-7) shop proprietor flavor names.
// A null line1 means that particular city doesn't have this kind of shop
static const ShopName ARMORY_NAMES[8] = {
	{ "The Armour Shop of", "Lord Eldric D'Charbonneux" }, { "Max's Armoury", nullptr },
	{ "The Iron Fist", nullptr }, { "The Hammer -n- Anvil", nullptr },
	{ nullptr, nullptr }, { "Defense Specialties Unlimited", nullptr },
	{ nullptr, nullptr }, { "Custom Armor Works", nullptr }
};
static const ShopName WEAPON_SHOP_NAMES[8] = {
	{ "The Weapon Shop of", "Lord Eldric D'Charbonneux" }, { "The Tempered Steel", nullptr },
	{ "The Razor's Edge", nullptr }, { "Cold Steel Creations", nullptr },
	{ "The Bloody Blade", nullptr }, { "The Duelo Shop", nullptr },
	{ "Weaponry Supply", nullptr }, { "Cold Steel Creations", nullptr }
};
static const ShopName MAGIC_SHOP_NAMES[8] = {
	{ "Mad Chad's Magic Shoppe", nullptr }, { "Mystic Melinda's", nullptr },
	{ "Words of Power", nullptr }, { "Psychic Sam's Magical Emporium", nullptr },
	{ nullptr, nullptr }, { nullptr, nullptr },
	{ nullptr, nullptr }, { "The Unleashed Spell", nullptr }
};
static const ShopName GROCER_SHOP_NAMES[8] = {
	{ "Li'l Karelia's Finnish Grocery", nullptr }, { "Adventurer's Supply Post", nullptr },
	{ nullptr, nullptr }, { "The Brown Bag", nullptr },
	{ "Fresh Food Marketplace", nullptr }, { "Rations Unlimited", nullptr },
	{ "Fastest Freshest Food Market", nullptr }, { "Exploration Provisioners", nullptr }
};
static const ShopName TAVERN_SHOP_NAMES[8] = {
	{ "Pub de Varg", nullptr }, { "Ye Olde Local Pub", nullptr },
	{ "Dr. Cat's Lair", nullptr }, { "Teaser's House", nullptr },
	{ nullptr, nullptr }, { "Mary's Midway", nullptr },
	{ "Kurtstable's Korner", nullptr }, { "Dav's House-o-Suds", nullptr }
};
static const ShopName TRANSPORT_SHOP_NAMES[8] = {
	{ "Scooter's Super Duper", "Transport, Inc." }, { nullptr, nullptr },
	{ nullptr, nullptr }, { "Quality Transport Ltd.", nullptr },
	{ nullptr, nullptr }, { "O.K. New and Used Transport", nullptr },
	{ nullptr, nullptr }, { "Sly Sam's Transportation", "Specialists" }
};

// Text column of each shop's first name line, in the same order and per
// variant as the name tables above. Their second lines (only the
// armoury/weapon shops' first variant and the transport shops' first and
// last variants have one) are at a fixed column
static const byte NAME_X[6][8] = {
	{ 11, 15, 13, 10, 0, 5, 0, 11 },
	{ 11, 11, 12, 10, 12, 13, 12, 10 },
	{ 9, 12, 13, 5, 0, 0, 0, 11 },
	{ 5, 8, 0, 13, 9, 11, 6, 8 },
	{ 14, 11, 13, 13, 0, 12, 10, 11 },
	{ 10, 0, 0, 9, 0, 7, 0, 8 }
};

// Looks up the proprietor name for a shop, which varies per city
static const ShopName &shopNameFor(int shopType, int variant) {
	static const ShopName *const TABLES[6] = {
		ARMORY_NAMES, WEAPON_SHOP_NAMES, MAGIC_SHOP_NAMES,
		GROCER_SHOP_NAMES, TAVERN_SHOP_NAMES, TRANSPORT_SHOP_NAMES
	};

	return TABLES[shopType][variant];
}

Merchant::Merchant() : Interaction("Merchant") {
}

bool Merchant::msgFocus(const FocusMessage &msg) {
	_mode = SHOP_PROMPT;
	setBounds(Common::Rect(0, 0, 0, 0));

	const auto &pos = _G(savegame)._locationPosition;
	switch (_G(map).getTileAt(pos.x, pos.y)) {
	case Data::CTILE_ARMORY:
	case Data::CTILE_STEAL_ARMOR:
		_shopType = SHOP_ARMORY;
		break;
	case Data::CTILE_GROCER:
	case Data::CTILE_STEAL_FOOD:
		_shopType = SHOP_GROCER;
		break;
	case Data::CTILE_WEAPONS:
	case Data::CTILE_STEAL_WEAPON:
		_shopType = SHOP_WEAPONS;
		break;
	case Data::CTILE_MAGIC:
		_shopType = SHOP_MAGIC;
		break;
	case Data::CTILE_TAVERN:
		_shopType = SHOP_TAVERN;
		break;
	default:
		_shopType = SHOP_TRANSPORT;
		break;
	}

	writeString("-Buy, Sell: ");
	showCursor();
	return true;
}

bool Merchant::msgAction(const ActionMessage &msg) {
	if (_mode == SHOP_MESSAGE)
		nextScreen();
	else
		nothing();
	return true;
}

bool Merchant::msgKeypress(const KeypressMessage &msg) {
	switch (_mode) {
	case SHOP_PROMPT:
		if (msg.keycode == Common::KEYCODE_b)
			chooseBuy();
		else if (msg.keycode == Common::KEYCODE_s)
			chooseSell();
		else
			nothing();
		break;

	case SHOP_MESSAGE:
		nextScreen();
		break;

	case SHOP_BUY:
		switch (_shopType) {
		case SHOP_ARMORY:
			armoryBuyKeypress(msg.keycode);
			break;
		case SHOP_WEAPONS:
			weaponsBuyKeypress(msg.keycode);
			break;
		case SHOP_MAGIC:
			magicBuyKeypress(msg.keycode);
			break;
		case SHOP_TRANSPORT:
			transportBuyKeypress(msg.keycode);
			break;
		default:
			break;
		}
		break;

	case SHOP_SELL:
		switch (_shopType) {
		case SHOP_ARMORY:
			armorySellKeypress(msg.keycode);
			break;
		case SHOP_WEAPONS:
			weaponsSellKeypress(msg.keycode);
			break;
		default:
			break;
		}
		break;

	case SHOP_QTY:
		groceryKeypress(msg);
		break;
	}

	return true;
}

void Merchant::draw() {
	if (_mode == SHOP_PROMPT)
		return;

	drawFrame();

	if (_mode == SHOP_MESSAGE) {
		const Screen &screen = _screens[0];
		auto s = getSurface();
		if (screen._clear)
			s.fillRect(Common::Rect(39, 48, 280, 120), 0);

		s.setColor(_G(colors)._text);
		for (const ScreenLine &line : screen._lines)
			s.writeString(Common::Point(line._x, line._y), "%s", line._text);

	} else if (_mode == SHOP_QTY) {
		drawGroceryPrompt();

	} else if (_mode == SHOP_BUY) {
		switch (_shopType) {
		case SHOP_ARMORY:
			drawArmoryBuy();
			break;
		case SHOP_WEAPONS:
			drawWeaponsBuy();
			break;
		case SHOP_MAGIC:
			drawMagicBuy();
			break;
		case SHOP_TRANSPORT:
			drawTransportBuy();
			break;
		default:
			break;
		}
	} else {
		switch (_shopType) {
		case SHOP_ARMORY:
			drawArmorySell();
			break;
		case SHOP_WEAPONS:
			drawWeaponsSell();
			break;
		default:
			break;
		}
	}
}

int Merchant::shopVariant() const {
	return _G(map)._mapStyle - 2;
}

void Merchant::drawFrame() {
	auto s = getSurface();
	s.fillRect(Common::Rect(DIALOG_X1, DIALOG_Y1, DIALOG_X2, DIALOG_Y2), 0);
	s.frameRect(Common::Rect(BORDER_X1, BORDER_Y1, BORDER_X2, BORDER_Y2), _G(colors)._border);
	s.setColor(_G(colors)._text);

	const int variant = shopVariant();
	const ShopName &name = shopNameFor(_shopType, variant);

	if (name._line1)
		s.writeString(Common::Point(NAME_X[_shopType][variant], 4), "%s", name._line1);
	if (name._line2) {
		int x = (_shopType == SHOP_TRANSPORT) ? (variant == 0 ? 13 : 15) : 8;
		s.writeString(Common::Point(x, 5), "%s", name._line2);
	}
}

int Merchant::listY() const {
	return shopNameFor(_shopType, shopVariant())._line2 ? 7 : 6;
}

void Merchant::drawShopLine(int y, int nameX, int priceX, int letterIdx, const char *name, int price, int digits) {
	auto s = getSurface();
	s.setColor(_G(colors)._text);
	s.writeString(Common::Point(nameX, y), "%c) %s", 'a' + letterIdx, name);
	s.writeString(Common::Point(priceX, y), digits == 4 ? "- %4d" : "- %3d", price);
}

bool Merchant::isWenchNearby() const {
	const auto &pos = _G(savegame)._locationPosition;

	for (int dy = -1; dy <= 1; ++dy) {
		for (int dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0)
				continue;

			int idx = _G(savegame).getLocationEntityAt(pos.x + dx, pos.y + dy);
			if (idx != -1 && _G(savegame)._locationEntities[idx]._type == 50)
				return true;
		}
	}

	return false;
}

int Merchant::overworldTransportCount() const {
	int count = 0;
	for (int idx = 1; idx <= _G(overworldEntityCount); ++idx) {
		int type = _G(savegame)._overworldEntities[idx]._type;
		if (type >= Data::TILE_HORSE && type <= Data::TILE_SHUTTLE)
			++count;
	}

	return count;
}

void Merchant::addScreen(bool clear, std::initializer_list<ScreenLine> lines) {
	Screen screen;
	screen._clear = clear;
	for (const ScreenLine &line : lines)
		screen._lines.push_back(line);

	_screens.push_back(screen);
}

void Merchant::showScreens(bool nothingAfter) {
	_nothingAfter = nothingAfter;
	_mode = SHOP_MESSAGE;
	setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
	redraw();
	showCursor();
}

void Merchant::showText(bool nothingAfter, bool clear, std::initializer_list<ScreenLine> lines) {
	_screens.clear();
	addScreen(clear, lines);
	showScreens(nothingAfter);
}

void Merchant::nextScreen() {
	_screens.remove_at(0);

	if (!_screens.empty()) {
		redraw();
	} else if (_nothingAfter) {
		nothing();
	} else {
		writeString("\n");
		finish();
	}
}

void Merchant::addTip(int tipIdx) {
	switch (tipIdx) {
	case 0:
		addScreen(true, { { 9, 8, "Thou hast been seduced!" }, { 11, 10, "After a long night," },
			{ 13, 11, "thou art back." } });
		break;
	case 1:
		addScreen(true, { { 11, 7, "Thou had best know" }, { 10, 8, "about space travel!" },
			{ 10, 9, "Thou must destroy at" }, { 9, 10, "least 20 enemy vessels" },
			{ 11, 11, "to become an ace!" } });
		break;
	case 2:
		addScreen(true, { { 11, 7, "Thou had best know" },
			{ 11, 8, _G(savegame)._sex == Data::SEX_MALE ? "to watch the wench." : "to watch the lecher." } });
		break;
	case 3:
		addScreen(true, { { 11, 7, "Thou had best know" }, { 5, 8, " that the princess will give" },
			{ 5, 9, " great reward to the one who" }, { 5, 10, "rescues her, and an extra gift" },
			{ 10, 11, "to an 8th level ace!" } });
		break;
	case 4:
		addScreen(true, { { 11, 7, "Thou had best know" }, { 7, 8, "thou must go back in time." } });
		break;
	case 5:
		addScreen(true, { { 11, 7, "Thou had best know" }, { 10, 8, "thou should destroy" },
			{ 13, 9, "the evil gem!" } });
		break;
	case 6:
		addScreen(true, { { 11, 7, "Thou had best know" }, { 5, 8, "   that many lakes and ponds" },
			{ 5, 9, "  have strong magical powers!" } });
		break;
	case 7:
		addScreen(true, { { 11, 7, "Thou had best know" }, { 10, 8, "this is a great game!" } });
		break;
	default:
		addScreen(true, { { 11, 7, "Thou had best know" }, { 5, 8, "   that over 1000 years ago," },
			{ 5, 9, "Mondain the Wizard created an" }, { 5, 10, " evil gem.  With this gem, he" },
			{ 5, 11, "   is immortal and cannot be" }, { 15, 12, "defeated." } });
		addScreen(true, { { 5, 7, "The quest of --Ultima-- is to" }, { 5, 8, " traverse the lands in search" },
			{ 8, 9, "of a time machine.  Upon" }, { 5, 10, " finding such a device, thou" },
			{ 5, 11, "should go back in time to the" }, { 5, 12, " days before Mondain created" },
			{ 5, 13, "the evil gem and destroy him!" } });
		break;
	}
}

void Merchant::chooseBuy() {
	resetLine();
	writeString("Transact-Buy: ");
	showCursor();

	switch (_shopType) {
	case SHOP_ARMORY:
	case SHOP_WEAPONS:
	case SHOP_MAGIC:
		_mode = SHOP_BUY;
		setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
		redraw();
		break;

	case SHOP_GROCER:
		groceryPrompt();
		break;

	case SHOP_TAVERN:
		tavernBuy();
		break;

	case SHOP_TRANSPORT: {
		auto &sg = _G(savegame);
		bool fleetFull = sg._transports[Data::TRANSPORT_SHUTTLE] == 0 &&
			overworldTransportCount() >= 15;

		for (int i = 0; i < 6; ++i)
			_transportIds[i] = 0;

		if ((sg._woodsCount || sg._grassCount) && !fleetFull) {
			_transportIds[0] = Data::TRANSPORT_HORSE;
			_transportIds[1] = Data::TRANSPORT_CART;
		}
		if (sg._waterCount && !fleetFull) {
			_transportIds[2] = Data::TRANSPORT_RAFT;
			_transportIds[3] = Data::TRANSPORT_FRIGATE;
		}
		if (_G(moveCtr) > 3000) {
			if (sg._grassCount && !fleetFull)
				_transportIds[4] = Data::TRANSPORT_AIRCAR;
			if (sg._transports[Data::TRANSPORT_SHUTTLE] == 0)
				_transportIds[5] = Data::TRANSPORT_SHUTTLE;
		}

		bool any = false;
		for (int i = 0; i < 6 && !any; ++i)
			any = _transportIds[i] != 0;

		if (!any) {
			showText(false, false, { { 11, listY() + 2, "Closed for the day." } });
			break;
		}

		_mode = SHOP_BUY;
		setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
		redraw();
		break;
	}

	default:
		break;
	}
}

void Merchant::chooseSell() {
	resetLine();
	writeString("Transact-Sell: ");
	showCursor();

	switch (_shopType) {
	case SHOP_ARMORY: {
		bool anyOwned = false;
		for (int idx = 1; idx < Data::ARMOR_COUNT && !anyOwned; ++idx)
			anyOwned = _G(savegame)._armor[idx] != 0;

		if (!anyOwned) {
			showText(true, false, { { 5, listY() + 1, "  Thou hast no armour to sell!" } });
		} else {
			_mode = SHOP_SELL;
			setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
			redraw();
		}
		break;
	}

	case SHOP_WEAPONS: {
		bool anyOwned = false;
		for (int idx = 1; idx < Data::WEAPON_COUNT && !anyOwned; ++idx)
			anyOwned = _G(savegame)._weapons[idx] != 0;

		if (!anyOwned) {
			showText(true, false, { { 5, listY() + 1, "Thou hast no weaponry to sell!" } });
		} else {
			_mode = SHOP_SELL;
			setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
			redraw();
		}
		break;
	}

	case SHOP_MAGIC:
		showText(true, false, { { 5, listY() + 2, "  Sorry, we don't buy spells!" } });
		break;

	case SHOP_GROCER:
		showText(true, false, { { 9, listY() + 2, "Used food?  No thanks!" } });
		break;

	case SHOP_TAVERN:
		showText(true, false, { { 8, listY() + 2, "We have plenty of booze" }, { 17, listY() + 3, "already!" } });
		break;

	case SHOP_TRANSPORT:
		showText(true, false, { { 9, listY() + 2, "Sorry, we don't deal in" }, { 15, listY() + 3, "used stuff." } });
		break;
	}
}

void Merchant::drawArmoryBuy() {
	int maxIdx = (_G(moveCtr) > 3000) ? 5 : 3;
	for (int idx = 1; idx <= maxIdx; ++idx) {
		int price = ((200 - _G(savegame)._intelligence) / 4) * idx;
		drawShopLine(listY() + idx, 9, 26, idx, Data::ARMOR_NAMES[idx], price, 3);
	}
}

void Merchant::armoryBuyKeypress(Common::KeyCode keycode) {
	int maxIdx = (_G(moveCtr) > 3000) ? 5 : 3;
	int idx = keycode - Common::KEYCODE_a;

	if (idx < 1 || idx > maxIdx) {
		nothing();
		return;
	}

	auto &sg = _G(savegame);
	int price = ((200 - sg._intelligence) / 4) * idx;
	if (price > sg._coins) {
		cantAfford();
		return;
	}

	sg._coins -= price;
	if (sg._armor[idx] < 255)
		++sg._armor[idx];

	writeString(Data::ARMOR_NAMES[idx]);
	redrawStats();
	showText(false, true, { { 18, 9, "Sold!" } });
}

void Merchant::drawArmorySell() {
	int y = listY() + 1;
	for (int idx = 1; idx < Data::ARMOR_COUNT; ++idx) {
		if (_G(savegame)._armor[idx]) {
			int price = (_G(savegame)._charisma + 40) * idx * idx / 256 + 1;
			drawShopLine(y++, 9, 26, idx, Data::ARMOR_NAMES[idx], price, 3);
		}
	}
}

void Merchant::armorySellKeypress(Common::KeyCode keycode) {
	int idx = keycode - Common::KEYCODE_a;
	auto &sg = _G(savegame);

	if (idx < 1 || idx >= Data::ARMOR_COUNT || !sg._armor[idx]) {
		nothing();
		return;
	}

	int price = (sg._charisma + 40) * idx * idx / 256 + 1;
	sg._coins += price;
	--sg._armor[idx];
	if (idx == sg._equippedArmor && sg._armor[idx] == 0)
		sg._equippedArmor = 0;

	writeString(Data::ARMOR_NAMES[idx]);
	redrawStats();
	showText(false, true, { { 18, 9, "Done!" } });
}

void Merchant::weaponsRange(int &minIdx, int &maxIdx) const {
	int base = (((_G(map)._mapStyle - 1) & 1) == 0) ? 1 : 2;

	// The stock widens as the game goes on: one more weapon offered per 1500
	// moves, up to a cap of 3 extra tiers (then a 4th once past 3000 moves).
	// The original takes moveCtr % 0x7FFF before the /1500, but with the
	// > 3000 branch forcing tier 3 that modulo never changes the result
	uint32 moveCtr = _G(moveCtr);
	int tier = (moveCtr <= 3000) ? MIN<uint32>((moveCtr % 0x7FFF) / 1500, 3) : 3;

	minIdx = base;
	maxIdx = base + (tier + 1) * 2;
}

void Merchant::drawWeaponsBuy() {
	int minIdx, maxIdx;
	weaponsRange(minIdx, maxIdx);

	int y = listY();
	for (int idx = minIdx; idx <= maxIdx; idx += 2) {
		int price = (255 - _G(savegame)._intelligence) * idx * idx / 256 + 5;
		drawShopLine(y++, 9, 26, idx, Data::WEAPON_NAMES[idx], price, 3);
	}
}

void Merchant::weaponsBuyKeypress(Common::KeyCode keycode) {
	int minIdx, maxIdx;
	weaponsRange(minIdx, maxIdx);
	int idx = keycode - Common::KEYCODE_a;

	if (idx < minIdx || idx > maxIdx || (idx - minIdx) % 2 != 0) {
		nothing();
		return;
	}

	auto &sg = _G(savegame);
	int price = (255 - sg._intelligence) * idx * idx / 256 + 5;
	if (price > sg._coins) {
		cantAfford();
		return;
	}

	sg._coins -= price;
	if (sg._weapons[idx] < 255)
		++sg._weapons[idx];

	writeString(Data::WEAPON_NAMES[idx]);
	redrawStats();
	showText(false, true, { { 18, 9, "Sold!" } });
}

void Merchant::drawWeaponsSell() {
	int y = listY();
	for (int idx = 1; idx < Data::WEAPON_COUNT; ++idx) {
		if (_G(savegame)._weapons[idx]) {
			int price = (_G(savegame)._charisma + 40) * idx * idx / 256 + 1;
			drawShopLine(y++, 9, 26, idx, Data::WEAPON_NAMES[idx], price, 3);
		}
	}
}

void Merchant::weaponsSellKeypress(Common::KeyCode keycode) {
	int idx = keycode - Common::KEYCODE_a;
	auto &sg = _G(savegame);

	if (idx < 1 || idx >= Data::WEAPON_COUNT || !sg._weapons[idx]) {
		nothing();
		return;
	}

	int price = (sg._charisma + 40) * idx * idx / 256 + 1;
	sg._coins += price;
	--sg._weapons[idx];
	if (idx == sg._equippedWeapon && sg._weapons[idx] == 0)
		sg._equippedWeapon = 0;

	writeString(Data::WEAPON_NAMES[idx]);
	redrawStats();
	showText(false, true, { { 18, 9, "Done!" } });
}

void Merchant::magicRange(int &minIdx, int &maxIdx) const {
	int base = ((_G(map)._mapStyle & 1) == 0) ? 1 : 2;
	int span = (_G(savegame)._class == Data::CLASS_CLERIC) ? 8 : 4;

	minIdx = base;
	maxIdx = base + span;
}

void Merchant::drawMagicBuy() {
	int minIdx, maxIdx;
	magicRange(minIdx, maxIdx);

	int y = listY();
	for (int idx = minIdx; idx <= maxIdx; idx += 2) {
		if (idx == Data::SPELL_STEAL)
			continue;

		int price = ((200 - _G(savegame)._wisdom) / 32) * idx;
		drawShopLine(y++, 9, 26, idx, Data::SPELL_NAMES[idx], price, 3);
	}
}

void Merchant::magicBuyKeypress(Common::KeyCode keycode) {
	int minIdx, maxIdx;
	magicRange(minIdx, maxIdx);
	int idx = keycode - Common::KEYCODE_a;

	if (idx < minIdx || idx > maxIdx || (idx - minIdx) % 2 != 0 || idx == Data::SPELL_STEAL) {
		nothing();
		return;
	}

	auto &sg = _G(savegame);
	int price = ((200 - sg._wisdom) / 32) * idx;
	if (price > sg._coins) {
		cantAfford();
		return;
	}

	sg._coins -= price;
	if (sg._spells[idx] < 255)
		++sg._spells[idx];

	writeString(Data::SPELL_NAMES[idx]);
	redrawStats();
	showText(false, true, { { 18, 9, "Sold!" } });
}

void Merchant::groceryPrompt() {
	_mode = SHOP_QTY;
	_amount.clear();
	setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
	redraw();
}

void Merchant::drawGroceryPrompt() {
	auto s = getSurface();
	s.setColor(_G(colors)._text);

	int y = listY();
	s.writeString(Common::Point(5, y + 2), " Packs of 10 food cost %d pence", 5 - _G(savegame)._intelligence / 20);
	s.writeString(Common::Point(8, y + 3), "each.  How many dost thou");
	s.writeString(Common::Point(11, y + 4), "wish to purchase?");
}

void Merchant::groceryKeypress(const KeypressMessage &msg) {
	if (msg.ascii >= '0' && msg.ascii <= '9') {
		if ((_amount.empty() && msg.ascii == '0') || _amount.size() >= 4)
			return;

		_amount += (char)msg.ascii;
		writeString(Common::String(1, (char)msg.ascii));

	} else if (msg.keycode == Common::KEYCODE_BACKSPACE) {
		if (!_amount.empty()) {
			_amount.deleteLastChar();
			resetLine();
			writeString(Common::String::format("Transact-Buy: %s", _amount.c_str()));
			showCursor();
		}

	} else {
		int qty = _amount.empty() ? 0 : atoi(_amount.c_str());
		auto &sg = _G(savegame);
		int pricePerPack = 5 - sg._intelligence / 20;
		int total = pricePerPack * qty;

		if (qty == 0 || total > sg._coins) {
			// The typed amount is replaced by "nothing"
			resetLine();
			writeString("Transact-Buy: ");

			if (qty == 0)
				nothing();
			else
				cantAfford();
		} else {
			sg._coins -= total;
			sg._food += qty * 10;
			redrawStats();
			writeString(" packs food");
			showText(false, true, { { 18, 9, "Sold!" } });
		}
	}
}

void Merchant::tavernBuy() {
	auto &sg = _G(savegame);
	const int y = listY();

	if (sg._coins == 0) {
		showText(true, false, { { 12, y + 1, "Thou art broke!" }, { 8, y + 4, "Come back when thou hast" },
			{ 10, y + 5, "some money to spend." } });
		return;
	}

	--sg._coins;
	redrawStats();
	writeString("ale");
	++_G(map)._tavernCtr;

	_screens.clear();
	addScreen(false, { { 8, y + 1, "The tavern keeper sayeth:" }, { 9, y + 4, "Here, have a cold one!" } });

	if (_G(map)._tavernCtr > sg._stamina / 4 && isWenchNearby()) {
		addTip(0);

		sg._coins /= 2;
		redrawStats();
		--sg._wisdom;
		if (sg._wisdom == 4)
			++sg._wisdom;
	}

	if (g_engine->getRandomNumber(1, 255) < 75)
		addTip(g_engine->getRandomNumber(11, 89) / 10);

	showScreens(false);
}

void Merchant::drawTransportBuy() {
	int y = listY();
	for (int i = 0; i < 6; ++i) {
		if (_transportIds[i] == 0)
			continue;

		int transportId = _transportIds[i];
		int price = ((200 - _G(savegame)._intelligence) / 5) * transportId * transportId;
		drawShopLine(y++, 12, 23, i, Data::TRANSPORT_NAMES[transportId], price, 4);
	}
}

void Merchant::transportBuyKeypress(Common::KeyCode keycode) {
	int letterIdx = keycode - Common::KEYCODE_a;
	if (letterIdx < 0 || letterIdx > 5 || _transportIds[letterIdx] == 0) {
		nothing();
		return;
	}

	int transportId = _transportIds[letterIdx];
	auto &sg = _G(savegame);
	int price = ((200 - sg._intelligence) / 5) * transportId * transportId;

	if (price > sg._coins) {
		cantAfford();
		return;
	}

	writeString(Data::TRANSPORT_NAMES[transportId]);
	sg._coins -= price;
	redrawStats();

	if (sg._transports[transportId] < 255)
		++sg._transports[transportId];

	if (transportId == Data::TRANSPORT_SHUTTLE) {
		sg._shipFuel = 1000;
		sg._shipShield = 1000;
	}

	spawnTransport(transportId);

	showText(false, true, { { 18, 9, "Sold!" } });
}

void Merchant::spawnTransport(int transportId) {
	static const int TRANSPORT_TILES[8] = {
		0, Data::TILE_HORSE, Data::TILE_CART, Data::TILE_RAFT, Data::TILE_FRIGATE1,
		Data::TILE_AIRCAR, Data::TILE_SHUTTLE, 0
	};

	int neededTile;
	switch (transportId) {
	case Data::TRANSPORT_HORSE:
	case Data::TRANSPORT_CART:
		neededTile = (_G(savegame)._woodsCount > 0) ? Data::TILE_WOODS : Data::TILE_GRASS;
		break;
	case Data::TRANSPORT_RAFT:
	case Data::TRANSPORT_FRIGATE:
		neededTile = Data::TILE_OCEAN;
		break;
	default:
		neededTile = Data::TILE_GRASS;
		break;
	}

	const Common::Point &pos = _G(savegame)._overworldPos;
	int dx = 1, dy = 1;
	bool found = false;

	for (int y = -1; y <= 1 && !found; ++y) {
		for (int x = -1; x <= 1 && !found; ++x) {
			if (_G(map).getTileAt(pos.x + x, pos.y + y) == neededTile) {
				dx = x;
				dy = y;
				found = true;
			}
		}
	}

	if (_G(overworldEntityCount) < Data::OVERWORLD_ENTITY_COUNT - 1) {
		++_G(overworldEntityCount);
		auto &e = _G(savegame)._overworldEntities[_G(overworldEntityCount)];
		e._type = TRANSPORT_TILES[transportId];
		e._data = neededTile;
		e._x = pos.x + dx;
		e._y = pos.y + dy;
		e._hits = 0;
	}
}

void Merchant::nothing() {
	writeString("nothing\n");
	finish();
}

void Merchant::cantAfford() {
	g_engine->playFX(1);
	showText(true, true, { { 8, 9, "Thou canst not afford it!" } });
}

void Merchant::finish() {
	prompt();
	close();
}

void Merchant::redrawStats() {
	g_engine->findView("Stats")->draw();
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima
