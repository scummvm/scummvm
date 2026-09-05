
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

constexpr int COLOR_TEXT = 11; // light cyan

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

// Tavern tip/rumor flavor texts. Index 0 is the drunk penalty message
// (no header); indices 1-8 are prefixed with "Thou had best know " when
// shown. Index 2 is sex-dependent and filled in at the point of use
static const char *const TAVERN_TIPS[9] = {
	"Thou hast been seduced!\nAfter a long night,\nthou art back.",
	"about space travel!\nThou must destroy at least 20 enemy vessels\nto become an ace!",
	nullptr,
	"that the princess will give great reward to the one who\nrescues her, and an extra gift to an 8th level ace!",
	"thou must go back in time.",
	"thou should destroy the evil gem!",
	"that many lakes and ponds have strong magical powers!",
	"this is a great game!",
	"that over 1000 years ago, Mondain the Wizard created an evil gem.\n"
	"With this gem, he is immortal and cannot be defeated.\n\n"
	"The quest of --Ultima-- is to traverse the lands in search\n"
	"of a time machine.  Upon finding such a device, thou\n"
	"should go back in time to the days before Mondain created\n"
	"the evil gem and destroy him!"
};

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
	if (_mode != SHOP_BUY && _mode != SHOP_SELL)
		return;

	auto s = getSurface();
	s.writeString(Point(15, 0), "\x10 Transact \x11");
	s.fillRect(TextRect(1, 1, 38, 18), 0);

	if (_mode == SHOP_BUY) {
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

void Merchant::drawShopName() {
	static const ShopName *const TABLES[6] = {
		ARMORY_NAMES, WEAPON_SHOP_NAMES, MAGIC_SHOP_NAMES,
		GROCER_SHOP_NAMES, TAVERN_SHOP_NAMES, TRANSPORT_SHOP_NAMES
	};

	const ShopName &name = TABLES[_shopType][shopVariant()];
	if (!name._line1)
		return;

	auto s = getSurface();
	s.setColor(COLOR_TEXT);
	s.writeString(Point(19 - (int)Common::String(name._line1).size() / 2, 2), "%s", name._line1);
	if (name._line2)
		s.writeString(Point(19 - (int)Common::String(name._line2).size() / 2, 3), "%s", name._line2);
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

void Merchant::chooseBuy() {
	switch (_shopType) {
	case SHOP_ARMORY:
	case SHOP_WEAPONS:
	case SHOP_MAGIC:
		_mode = SHOP_BUY;
		setBounds(TextRect(0, 0, 39, 19));
		redraw();
		break;

	case SHOP_GROCER:
		groceryPrompt();
		break;

	case SHOP_TAVERN:
		tavernBuy();
		break;

	case SHOP_TRANSPORT: {
		auto &map = _G(map);
		auto &sg = _G(savegame);
		bool fleetFull = sg._transports[Data::TRANSPORT_SHUTTLE] == 0 &&
			overworldTransportCount() >= 15;

		for (int i = 0; i < 6; ++i)
			_transportIds[i] = 0;

		if ((map._woodsCount || map._grassCount) && !fleetFull) {
			_transportIds[0] = Data::TRANSPORT_HORSE;
			_transportIds[1] = Data::TRANSPORT_CART;
		}
		if (map._waterCount && !fleetFull) {
			_transportIds[2] = Data::TRANSPORT_RAFT;
			_transportIds[3] = Data::TRANSPORT_FRIGATE;
		}
		if (_G(moveCtr) > 3000) {
			if (map._grassCount && !fleetFull)
				_transportIds[4] = Data::TRANSPORT_AIRCAR;
			if (sg._transports[Data::TRANSPORT_SHUTTLE] == 0)
				_transportIds[5] = Data::TRANSPORT_SHUTTLE;
		}

		bool any = false;
		for (int i = 0; i < 6 && !any; ++i)
			any = _transportIds[i] != 0;

		if (!any) {
			writeString("\n");
			writeString("Closed for the day.\n");
			finish();
			break;
		}

		_mode = SHOP_BUY;
		setBounds(TextRect(0, 0, 39, 19));
		redraw();
		break;
	}

	default:
		break;
	}
}

void Merchant::chooseSell() {
	switch (_shopType) {
	case SHOP_ARMORY: {
		bool anyOwned = false;
		for (int idx = 1; idx < Data::ARMOR_COUNT && !anyOwned; ++idx)
			anyOwned = _G(savegame)._armor[idx] != 0;

		if (!anyOwned) {
			writeString("\n");
			writeString("Thou hast no armour to sell!\n");
			nothing();
		} else {
			_mode = SHOP_SELL;
			setBounds(TextRect(0, 0, 39, 19));
			redraw();
		}
		break;
	}

	case SHOP_WEAPONS: {
		bool anyOwned = false;
		for (int idx = 1; idx < Data::WEAPON_COUNT && !anyOwned; ++idx)
			anyOwned = _G(savegame)._weapons[idx] != 0;

		if (!anyOwned) {
			writeString("\n");
			writeString("Thou hast no weaponry to sell!\n");
			nothing();
		} else {
			_mode = SHOP_SELL;
			setBounds(TextRect(0, 0, 39, 19));
			redraw();
		}
		break;
	}

	case SHOP_MAGIC:
		writeString("\n");
		writeString("Sorry, we don't buy spells!\n");
		nothing();
		break;

	case SHOP_GROCER:
		writeString("\n");
		writeString("Used food?  No thanks!\n");
		nothing();
		break;

	case SHOP_TAVERN:
		writeString("\n");
		writeString("We have plenty of booze already!\n");
		nothing();
		break;

	case SHOP_TRANSPORT:
		writeString("\n");
		writeString("Sorry, we don't deal in used stuff.\n");
		nothing();
		break;
	}
}

void Merchant::drawArmoryBuy() {
	drawShopName();
	auto s = getSurface();
	s.setColor(COLOR_TEXT);

	int maxIdx = (_G(moveCtr) > 3000) ? 5 : 3;
	int y = 8;
	for (int idx = 1; idx <= maxIdx; ++idx, ++y) {
		int price = ((200 - _G(savegame)._intelligence) / 4) * idx;
		s.writeString(Point(10, y), "%c) %s", 'a' + idx, Data::ARMOR_NAMES[idx]);
		s.writeString(Point(27, y), "-%3d", price);
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
	writeString("\n");
	writeString("Sold!\n");
	finish();
}

void Merchant::drawArmorySell() {
	drawShopName();
	auto s = getSurface();
	s.setColor(COLOR_TEXT);

	int y = 8;
	for (int idx = 1; idx < Data::ARMOR_COUNT; ++idx) {
		if (_G(savegame)._armor[idx]) {
			int price = (_G(savegame)._charisma + 40) * idx * idx / 256 + 1;
			s.writeString(Point(10, y), "%c) %s", 'a' + idx, Data::ARMOR_NAMES[idx]);
			s.writeString(Point(27, y), "-%3d", price);
			++y;
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
	writeString("\n");
	writeString("Done!\n");
	finish();
}

void Merchant::weaponsRange(int &minIdx, int &maxIdx) const {
	int base = (((_G(map)._mapStyle - 1) & 1) == 0) ? 1 : 2;
	uint32 moveCtr = _G(moveCtr);
	int tier = (moveCtr <= 3000) ? MIN<uint32>(moveCtr % 1500, 3) : 3;

	minIdx = base;
	maxIdx = base + (tier + 1) * 2;
}

void Merchant::drawWeaponsBuy() {
	drawShopName();
	auto s = getSurface();
	s.setColor(COLOR_TEXT);

	int minIdx, maxIdx;
	weaponsRange(minIdx, maxIdx);

	int y = 8;
	for (int idx = minIdx; idx <= maxIdx; idx += 2, ++y) {
		int price = (255 - _G(savegame)._intelligence) * idx * idx / 256 + 5;
		s.writeString(Point(9, y), "%c) %s", 'a' + idx, Data::WEAPON_NAMES[idx]);
		s.writeString(Point(26, y), "-%3d", price);
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
	writeString("\n");
	writeString("Sold!\n");
	finish();
}

void Merchant::drawWeaponsSell() {
	drawShopName();
	auto s = getSurface();
	s.setColor(COLOR_TEXT);

	int y = 6;
	for (int idx = 1; idx < Data::WEAPON_COUNT; ++idx) {
		if (_G(savegame)._weapons[idx]) {
			int price = (_G(savegame)._charisma + 40) * idx * idx / 256 + 1;
			s.writeString(Point(9, y), "%c) %s", 'a' + idx, Data::WEAPON_NAMES[idx]);
			s.writeString(Point(26, y), "-%3d", price);
			++y;
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
	writeString("\n");
	writeString("Done!\n");
	finish();
}

void Merchant::magicRange(int &minIdx, int &maxIdx) const {
	int base = ((_G(map)._mapStyle & 1) == 0) ? 1 : 2;
	int span = (_G(savegame)._class == Data::CLASS_CLERIC) ? 8 : 4;

	minIdx = base;
	maxIdx = base + span;
}

void Merchant::drawMagicBuy() {
	drawShopName();
	auto s = getSurface();
	s.setColor(COLOR_TEXT);

	int minIdx, maxIdx;
	magicRange(minIdx, maxIdx);

	int y = 8;
	for (int idx = minIdx; idx <= maxIdx; idx += 2) {
		if (idx == Data::SPELL_STEAL)
			continue;

		int price = ((200 - _G(savegame)._wisdom) / 32) * idx;
		s.writeString(Point(9, y), "%c) %s", 'a' + idx, Data::SPELL_NAMES[idx]);
		s.writeString(Point(26, y), "-%3d", price);
		++y;
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
	writeString("\n");
	writeString("Sold!\n");
	finish();
}

void Merchant::groceryPrompt() {
	_mode = SHOP_QTY;
	_amount.clear();

	int pricePerPack = 5 - _G(savegame)._intelligence / 20;
	writeString("\n");
	writeString(Common::String::format("Packs of 10 food cost %d pence each.\n", pricePerPack));
	resetLine();
	writeString("\x10""How many dost thou wish to purchase? ");
	showCursor();
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
			writeString(Common::String::format("\x10""How many dost thou wish to purchase? %s", _amount.c_str()));
			showCursor();
		}

	} else {
		int qty = _amount.empty() ? 0 : atoi(_amount.c_str());
		auto &sg = _G(savegame);
		int pricePerPack = 5 - sg._intelligence / 20;
		int total = pricePerPack * qty;

		writeString("\n");
		if (qty == 0) {
			nothing();
		} else if (total > sg._coins) {
			cantAfford();
		} else {
			sg._coins -= total;
			sg._food += qty * 10;
			redrawStats();
			writeString(Common::String::format("%d packs food\n", qty));
			writeString("Sold!\n");
			finish();
		}
	}
}

void Merchant::tavernBuy() {
	auto &sg = _G(savegame);

	if (sg._coins == 0) {
		writeString("\n");
		writeString("Thou art broke!\n");
		writeString("Come back when thou hast some money to spend.\n");
		nothing();
		return;
	}

	--sg._coins;
	redrawStats();
	writeString("\n");
	writeString("The tavern keeper sayeth:\n");
	writeString("Here, have a cold one!\n");
	writeString("ale");
	++_G(map)._tavernCtr;

	if (_G(map)._tavernCtr > sg._stamina / 4 && isWenchNearby()) {
		writeString("\n");
		writeString(TAVERN_TIPS[0]);
		writeString("\n");

		sg._coins /= 2;
		redrawStats();
		--sg._wisdom;
		if (sg._wisdom == 4)
			++sg._wisdom;
	}

	if (g_engine->getRandomNumber(1, 255) < 75) {
		int tipIdx = g_engine->getRandomNumber(11, 89) / 10;
		writeString("\n");
		writeString("Thou had best know ");

		if (tipIdx == 2)
			writeString(Common::String::format("to watch the %s.", sg._sex == Data::SEX_MALE ? "wench" : "lecher"));
		else
			writeString(TAVERN_TIPS[tipIdx]);
		writeString("\n");
	}

	finish();
}

void Merchant::drawTransportBuy() {
	drawShopName();
	auto s = getSurface();
	s.setColor(COLOR_TEXT);

	int y = 8;
	for (int i = 0; i < 6; ++i, ++y) {
		if (_transportIds[i] == 0)
			continue;

		int transportId = _transportIds[i];
		int price = ((200 - _G(savegame)._intelligence) / 5) * transportId * transportId;
		s.writeString(Point(11, y), "%c) %s", 'a' + i, Data::TRANSPORT_NAMES[transportId]);
		s.writeString(Point(22, y), "-%4d", price);
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

	writeString("\n");
	writeString("Sold!\n");
	finish();
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
		neededTile = (_G(map)._woodsCount > 0) ? Data::TILE_WOODS : Data::TILE_GRASS;
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
	writeString("nothing");
	finish();
}

void Merchant::cantAfford() {
	writeString("nothing");
	writeString("\n");
	writeString("Thou canst not afford it!\n");
	g_engine->playFX(1);
	finish();
}

void Merchant::finish() {
	writeString("\n");
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
