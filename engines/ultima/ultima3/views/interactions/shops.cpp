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

#include "ultima/ultima3/views/interactions/shops.h"
#include "ultima/ultima3/logic/location_logic.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr char ESCAPE_KEY = 0x1B;
constexpr int MAX_GOLD = 9999;
constexpr int BEST_SHOP_X = 0x25;
constexpr byte MAX_OWNED = 0x99;

static bool isContinueKey(const KeypressMessage &msg) {
	return msg.keycode == Common::KEYCODE_RETURN || msg.keycode == Common::KEYCODE_KP_ENTER ||
		msg.keycode == Common::KEYCODE_DOWN || msg.keycode == Common::KEYCODE_SPACE;
}

Data::RosterEntry &Shop::payer() const {
	return _G(savegame).partyMember(_payer);
}

bool Shop::canAfford(int gold) const {
	return Data::fromBcdWord(payer()._gold) >= gold;
}

void Shop::spend(int gold) {
	Data::RosterEntry &e = payer();
	e._gold = Data::toBcdWord(Data::fromBcdWord(e._gold) - gold);
}

void Shop::say(const char *text, int sound) {
	_G(messages).print(text);
	if (sound)
		g_engine->playSoundEffect(sound);
}

/*------------------------------------------------------------------------*/

static const char *const PUB_TALK[10] = {
	"\n\nThank you,\nkindly!\n",
	"\nAmbrosia,\never heard\nof it?\n",
	"\nDawn,\nthe city of\nmyths & magic!\n",
	"\nThe conjunction\nof the moons\nfinds link!\n",
	"Nasty creatures,\n  nasty dark,\nsure thee ready,\nfore thee embark\n",
	"None return or\n so I'm told,\nfrom the pool,\ndark and cold!\n",
	"  Shrines of\n  knowledge,\n  shrines of\n   strength,\n all are lost\ninto the brink!",
	" Fountains fair\n       &\n fountains foul\nall are found in\n dungeons bowel\n",
	"     EXODUS:\n   Ultima ]I[\n which is next?\nNow could it be.\n",
	"Seek ye out the\n Lord of Time,\nand the one way\nis a sure find!\n"
};

static const char *const CANT_PAY = "\nWhat? Can't pay!\nOut you scum!\n";

Tavern::Tavern(int payer) : Shop(payer) {
	_G(messages).print("\n   Welcome to\n    the Pub!\n");
	askForPayment();
}

void Tavern::askForPayment() {
	_asking = false;
	_G(messages).print("\nHere, friend,\nhave a drink!\nIt costs 7 g.p.\nYou pay? ");
	_number.setup(2);
}

bool Tavern::keypress(const KeypressMessage &msg) {
	if (_asking) {
		if (!_another.handleKey(msg))
			return false;

		_G(messages).print("\n\n");
		if (_another.yes()) {
			_another = LogYesNo();
			askForPayment();
			return false;
		}

		say("It's been a\npleasure!!\n");
		return true;
	}

	if (!_number.handleKey(msg))
		return false;

	if (_number.hasLetters()) {
		say("\n<-What?\n", 0xFE);
		return true;
	}

	_G(messages).print("\n");
	int amount = _number.value();

	if (_number.bcd() < 7) {
		say("\n Leave my shop!\n   You scum!!\n", 0xFF);
		return true;
	}

	if (amount >= Data::fromBcdWord(payer()._gold)) {
		say(CANT_PAY, 0xFE);
		return true;
	}

	_G(messages).print("\n");
	spend(amount);
	say(PUB_TALK[_number.bcd() >> 4]);
	say("\nAnother? ");
	_asking = true;
	return false;
}

/*------------------------------------------------------------------------*/

Grocer::Grocer(int payer) : Shop(payer) {
	_G(messages).print("    Ye local\n     Grocer\n\nRations:\n1 g.p. each.\nHow many would\nyou like? ");
	_number.setup(4, true);
}

bool Grocer::keypress(const KeypressMessage &msg) {
	if (!_number.handleKey(msg))
		return false;

	Data::RosterEntry &e = payer();
	int quantity = _number.value();

	if (quantity != 0) {
		if (quantity > Data::fromBcdWord(e._gold)) {
			say(CANT_PAY, 0xFF);
			return true;
		}

		int food = Data::fromBcdWord(e._food) + quantity;
		if (food > MAX_GOLD) {
			say("\nToo much to\ncarry!\n", 0xFF);
			return true;
		}

		e._food = Data::toBcdWord(food);
		spend(quantity);
	}

	say("\n\nThank you,\nCome again!\n");
	return true;
}

/*------------------------------------------------------------------------*/

static const int TEMPLE_COSTS[4] = { 100, 200, 500, 900 };

static const char *const TEMPLE_OFFERS[4] = {
	"\nA curing will\ncost 100 g.p.\nWilt thou pay?\n",
	"\nHealings cost\n200 g.p.  Wilt\nthou pay? ",
	"\nResurrections\ncost 500 g.p.\nWilt thou pay?\n",
	"Recallings\ncost 900 g.p.\nWilt thou pay?\n"
};

static const char *const TEMPLE_WHOM[4] = {
	"Cure whom? ", "Heal whom? ", "Resurrect whom? ", "Recall whom? "
};

Temple::Temple(int payer) : Shop(payer) {
	static const char *const WORDS[6] = { "1", "2", "3", "4", "Q", "Q" };

	_G(messages).print("\nClerical Healing\nSacraments:\n 1-Curing,\n 2-Healing,\n 3-Resurrection,\n 4-Recallings.\nYour needs: ");
	_menu.setup("1234Q\033", WORDS, 6);
}

void Temple::farewell() {
	say("\nFare thee well\nmy children.\n");
}

bool Temple::serve(int slot) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);
	bool restored = false;

	// A flash of light goes over everything
	auto flash = [&]() {
		_G(effects).flashViewport();
		_G(effects).flashSlot(slot);
		g_engine->playSoundEffect(0xFD, 0xC0, 0x80);
	};

	switch (_service) {
	case 0:
		if (e._status == Data::STATUS_GOOD) {
			say("Not injured!\n", 0xFF);
			return false;
		}

		flash();
		if (e._status != Data::STATUS_POISONED) {
			say("Failed!\n", 0xFF);
			return false;
		}

		e._status = Data::STATUS_GOOD;
		restored = true;
		break;

	case 1:
		if (Data::fromBcdWord(e._maxHitPoints) < Data::fromBcdWord(e._hitPoints)) {
			say("Not injured!\n", 0xFF);
			return false;
		}

		e._hitPoints = e._maxHitPoints;
		flash();
		restored = true;
		break;

	case 2:
		if (e._status == Data::STATUS_GOOD) {
			say("Not injured!\n", 0xFF);
			return false;
		}
		if (e._status == Data::STATUS_POISONED) {
			say("Not dead!\n", 0xFF);
			return false;
		}

		flash();
		if (e._status != Data::STATUS_DEAD) {
			say("Failed!\n", 0xFF);
			return false;
		}

		e._status = Data::STATUS_GOOD;
		restored = true;
		break;

	default:
		if (e._status == Data::STATUS_GOOD) {
			say("Not injured!\n", 0xFF);
			return false;
		}
		if (e._status != Data::STATUS_ASHES) {
			say("Not ashes!\n", 0xFF);
			return false;
		}

		e._status = Data::STATUS_GOOD;
		flash();
		restored = true;
		break;
	}

	if (restored)
		spend(TEMPLE_COSTS[_service]);

	return restored;
}

bool Temple::keypress(const KeypressMessage &msg) {
	switch (_stage) {
	case CHOOSE_SERVICE: {
		if (!_menu.handleKey(msg))
			return false;

		char key = _menu.key();
		if (key == 'Q' || key == ESCAPE_KEY) {
			farewell();
			return true;
		}

		_service = key - '1';
		say(TEMPLE_OFFERS[_service]);
		_stage = CONFIRM;
		return false;
	}

	case CONFIRM:
		if (!_confirm.handleKey(msg))
			return false;

		_G(messages).print("\n");
		if (!_confirm.yes()) {
			say("\nWithout proper\nofferings I\ncannot help!\n");
			return true;
		}

		if (!canAfford(TEMPLE_COSTS[_service])) {
			say("I'm sorry, but\nthou hast not\ngold enough.\n", 0xFF);
			return true;
		}

		say(TEMPLE_WHOM[_service]);
		_stage = CHOOSE_PLAYER;
		return false;

	default: {
		PlayerChooser::Result result = _chooser.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		if (serve(_chooser.slot()))
			farewell();
		return true;
	}
	}
}

/*------------------------------------------------------------------------*/

// What each kind of weapon and armour costs, in the order the shops list them
static const int WEAPON_PRICES[14] = {
	5, 30, 60, 125, 350, 200, 250, 400, 1050, 800, 1200, 2700, 6550, 4550
};
static const int ARMOUR_PRICES[6] = { 75, 195, 575, 2500, 6130, 8250 };

EquipmentShop::EquipmentShop(int payer, bool weapons) : Shop(payer), _weapons(weapons) {
	_G(messages).print(weapons ? "\nWelcome to the\nWeapons Shop!\n\nList? " :
		"\nWelcome to the\nArmour Shop!\n\nList? ");
}

bool EquipmentShop::isBestShop() const {
	return _G(savegame)._worldX == BEST_SHOP_X;
}

int EquipmentShop::itemCount() const {
	const Data::Savegame &save = _G(savegame);

	if (_weapons)
		return (isBestShop() && save._plusTwoWeapons) ? 14 : 7;
	else
		return (isBestShop() && save._plusTwoArmour) ? 6 : 4;
}

void EquipmentShop::showPage() {
	if (_weapons) {
		if (_page == 0) {
			_G(messages).print("\nAvailable:\nB:Dagger    5gp\nC:Mace     30gp\nD:Sling    60gp\n"
				"E:Axe     125gp\nF:Bow     350gp\nG:Sword   200gp");
		} else {
			_G(messages).print("I:+2 Axe  400gp\nJ:+2 Bow 1050gp\nK:+2 Swd  800gp\nL:Gloves 1200gp\n"
				"M:+4 Axe 2700gp\nN:+4 Bow 6550gp");
		}
	} else {
		_G(messages).print("\nAvailable:\nB:Cloth     75gp\nC:Leather  195gp\nD:Chain    575gp\nE:Plate   2500gp");
	}

	_stage = LIST_PAGE;
}

void EquipmentShop::askAction() {
	static const char *const WORDS[4] = { "Buy", "Sell", "Quit", "Quit" };

	_G(messages).print("\nBuy or sell?\n");
	_action.setup("BSQ\033", WORDS, 4);
	_stage = ASK_ACTION;
}

void EquipmentShop::askItem() {
	_G(messages).print(_buying ? "Your interest?\n" : "For sale?\n");

	int count = itemCount();
	_keys[0] = 'Q';
	_keys[1] = ESCAPE_KEY;
	_words[0] = _words[1] = "Quit";

	for (int i = 0; i < count; ++i) {
		_keys[2 + i] = 'B' + i;
		_words[2 + i] = _weapons ? Data::WEAPON_NAMES[1 + i] : Data::ARMOUR_NAMES[1 + i];
	}

	_item.setup(_keys, _words, 2 + count);
	_stage = CHOOSE_ITEM;
}

bool EquipmentShop::buy(int item) {
	Data::RosterEntry &e = payer();
	byte &owned = _weapons ? e._weaponOwned[item] : e._armourOwned[item];

	if (owned == MAX_OWNED) {
		say("\nNo more room!\n", 0xFE);
		return false;
	}

	int price = _weapons ? WEAPON_PRICES[item] : ARMOUR_PRICES[item];
	if (!canAfford(price)) {
		say("\nI'm very sorry,\nbut you haven't\nthe gold!\n", 0xFE);
		return false;
	}

	spend(price);
	owned = Data::toBcd(Data::fromBcd(owned) + 1);
	say("\nHere you are.\nMay it serve\nyou well.\n");
	return true;
}

bool EquipmentShop::sell(int item) {
	Data::RosterEntry &e = payer();
	byte &owned = _weapons ? e._weaponOwned[item] : e._armourOwned[item];

	if (owned == 0) {
		say("\nYou don't own\none of those!\n", 0xFE);
		return false;
	}

	int price = _weapons ? WEAPON_PRICES[item] : ARMOUR_PRICES[item];
	int gold = Data::fromBcdWord(e._gold) + price;
	if (gold > MAX_GOLD) {
		say("\nToo much gold!\n", 0xFE);
		return false;
	}

	e._gold = Data::toBcdWord(gold);
	owned = Data::toBcd(Data::fromBcd(owned) - 1);

	// Whatever was in use is put away
	if (_weapons)
		e._weaponIndex = 0;
	else
		e._armourIndex = 0;

	say("\nThank you!\n");
	return true;
}

bool EquipmentShop::keypress(const KeypressMessage &msg) {
	Data::Savegame &save = _G(savegame);

	switch (_stage) {
	case ASK_LIST:
		if (!_list.handleKey(msg))
			return false;

		_G(messages).print("\n");
		if (_list.yes())
			showPage();
		else
			askAction();
		return false;

	case LIST_PAGE:
		if (msg.keycode == Common::KEYCODE_ESCAPE) {
			_G(messages).print("\n");
			askAction();
			return false;
		}
		if (!isContinueKey(msg))
			return false;

		if (_weapons) {
			if (_page == 0) {
				_G(messages).print("\nH:2H Swd  250gp\n");

				// The best shop has the better weapons too
				if (isBestShop()) {
					save._plusTwoWeapons = true;
					_page = 1;
					showPage();
					return false;
				}
			} else {
				_G(messages).print("\nO:+4 Swd 4550gp\n");
			}
		} else {
			if (isBestShop()) {
				save._plusTwoArmour = true;
				_G(messages).print("\nF:+2Chain 6130gp\nG:+2Plate 8250gp");
			}

			_G(messages).print("\n");
		}

		askAction();
		return false;

	case ASK_ACTION: {
		if (!_action.handleKey(msg))
			return false;

		_G(messages).print("\n");
		char key = _action.key();
		if (key == 'Q' || key == ESCAPE_KEY)
			return true;

		_buying = key == 'B';
		askItem();
		return false;
	}

	default: {
		if (!_item.handleKey(msg))
			return false;

		char key = _item.key();
		if (key == 'Q' || key == ESCAPE_KEY) {
			_G(messages).print("\n");
			return true;
		}

		bool carryOn = _buying ? buy(key - 'B') : sell(key - 'B');
		if (!carryOn)
			return true;

		askItem();
		return false;
	}
	}
}

/*------------------------------------------------------------------------*/

static const int GUILD_PRICES[4] = { 75, 50, 90, 6 };

Guild::Guild(int payer) : Shop(payer) {
	showGoods();
}

void Guild::showGoods() {
	static const char *const WORDS[6] = { "Gems", "Keys", "Powders", "Torches", "Quit", "Quit" };

	_G(messages).print("The Guild shop:\n Keys      50gp\n Torches    6gp\n Powders   90gp\n Gems      75gp\nYour needs:\n");
	_menu.setup("GKPTQ\033", WORDS, 6);
	_stage = CHOOSE_GOODS;
}

bool Guild::keypress(const KeypressMessage &msg) {
	Data::RosterEntry &e = payer();

	switch (_stage) {
	case CHOOSE_GOODS: {
		if (!_menu.handleKey(msg))
			return false;

		char key = _menu.key();
		if (key == 'Q' || key == ESCAPE_KEY) {
			say("\nThank you,\ncome again!\n");
			return true;
		}

		_goods = (key == 'G') ? 0 : (key == 'K') ? 1 : (key == 'P') ? 2 : 3;
		_G(messages).print("\nHow many? ");
		_number.setup(2);
		_stage = CHOOSE_AMOUNT;
		return false;
	}

	case CHOOSE_AMOUNT: {
		if (!_number.handleKey(msg))
			return false;

		if (_number.hasLetters()) {
			_G(messages).print("\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		byte *held = (_goods == 0) ? &e._gems : (_goods == 1) ? &e._keys : (_goods == 2) ? &e._powder : &e._torches;
		int quantity = Data::fromBcd(_number.bcd());
		int total = Data::fromBcd(*held) + quantity;

		if (total > 99) {
			say("\nToo much to\ncarry!\n", 0xFF);
			return true;
		}

		if (!canAfford(GUILD_PRICES[_goods] * quantity)) {
			say("\nI'm sorry,\nbut you have\nnot the funds!\n");
			_G(messages).print("\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		spend(GUILD_PRICES[_goods] * quantity);
		*held = Data::toBcd(total);
		say("\nAnything else?\n");
		_stage = ANYTHING_ELSE;
		return false;
	}

	default:
		if (!_more.handleKey(msg))
			return false;

		_G(messages).print("\n");
		if (_more.yes()) {
			_more = LogYesNo();
			showGoods();
			return false;
		}

		say("\nThank you,\ncome again!\n");
		return true;
	}
}

/*------------------------------------------------------------------------*/

static const char *const ORACLE_CLUES[10] = {
	"And so the sage\nsaid unto thee\n`If thou can\nsolve my rhyme\n",
	"you'll learn of\nmarks & playing\ncards & hidden\nholy shrines.\n",
	"Of marks I say\nthere are but 4\nof fire, force,\nsnake & king.\n",
	"Learn their use\nin Devil Guard,\nor death you'll\nsurely bring.\n",
	"Shrines there\nare again but 4\nto which you\ngo and pray.\n",
	"Their uses are\ninumerable and\nclues throughout\nI say.\n",
	"The cards their\nsuits do number\n4, called Sol\nMoon, Death\nand Love.",
	"Unto the Montors\nthou must go\nfor guidence\nfrom above.\n",
	"To aid thee in\nthy cryptic\nsearch, to\ndungeons thou\nmust fare.",
	"There seek out\nthe Lord of Time\nto help you\nif he cares.'\n"
};

Oracle::Oracle(int payer) : Shop(payer) {
	_G(messages).print("\n    Radrion:\nProphet of Life!\n");
	askForOffering();
}

void Oracle::askForOffering() {
	_asking = false;
	_G(messages).print("\nHow many 100gp\nis your\noffering? ");
	_number.setup(2);
}

bool Oracle::keypress(const KeypressMessage &msg) {
	if (_asking) {
		if (!_more.handleKey(msg))
			return false;

		_G(messages).print("\n");
		if (_more.yes()) {
			_more = LogYesNo();
			askForOffering();
			return false;
		}

		_G(messages).print("\n");
		say("\nFare thee well\nand good luck!\n");
		return true;
	}

	if (!_number.handleKey(msg))
		return false;

	if (_number.hasLetters()) {
		say("\n<-What?\n", 0xFE);
		return true;
	}

	_G(messages).print("\n\n");
	int cost = Data::fromBcd(_number.bcd()) * 100;
	if (!canAfford(cost)) {
		say(CANT_PAY, 0xFE);
		return true;
	}

	spend(cost);
	say(ORACLE_CLUES[MIN(_number.bcd(), 9)]);
	say("\nMore offering?\n");
	_asking = true;
	return false;
}

/*------------------------------------------------------------------------*/

Stable::Stable(int payer) : Shop(payer) {
	int size = _G(savegame)._partySize;
	_G(messages).print(Common::String::format("\n\nEquine Emporium:\n\n%X horses cost\n%X00gp. Will you\nbuy? ",
		size, size * 2).c_str());
}

bool Stable::keypress(const KeypressMessage &msg) {
	if (!_buy.handleKey(msg))
		return false;

	_G(messages).print("\n");
	if (!_buy.yes()) {
		say("Ah, too bad.\nThese are the\nbest in town!\n");
		return true;
	}

	int cost = _G(savegame)._partySize * 200;
	if (!canAfford(cost)) {
		say("I'm sorry, but\nthou hast not\ngold enough.\n", 0xFF);
		return true;
	}

	spend(cost);
	say("May you ride\nfast and true\nfriend!\n");
	_G(savegame)._transport = TRANSPORT_HORSE;
	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
