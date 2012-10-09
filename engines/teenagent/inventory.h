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
 */

#ifndef TEENAGENT_INVENTORY_H
#define TEENAGENT_INVENTORY_H

#include "teenagent/surface.h"
#include "teenagent/animation.h"
#include "common/events.h"
#include "common/array.h"
#include "teenagent/objects.h"

namespace TeenAgent {

struct InventoryObject;
class TeenAgentEngine;

// Maximum number of items found within game
const uint8 kNumInventoryItems = 92;

// Inventory Item Ids
const uint8 kInvItemNoItem = 0; // No item i.e. empty inventory slot
const uint8 kInvItemFeather = 1;
const uint8 kInvItemShotgun = 2;
const uint8 kInvItemToolboxFull = 3; // Contains Car Jack and Spanner
const uint8 kInvItemToolboxHalfEmpty = 4; // Contains Spanner
const uint8 kInvItemSpanner = 5;
const uint8 kInvItemComb = 6;
const uint8 kInvItemFan = 7;
const uint8 kInvItemBrokenPaddle = 8;
const uint8 kInvItemPaddle = 9; // Repaired - BrokenPaddle combined with Branch (with Glue)
const uint8 kInvItemFirstFlower = 10; // Smells nice
const uint8 kInvItemSecondFlower = 11; // Really beautiful
const uint8 kInvItemFeatherDusterClean = 12;
const uint8 kInvItemChainsaw = 13; // Unfueled
const uint8 kInvItemDrunkenChainsaw = 14; // Fueled with Whisky (Chainsaw combined with Whiskey)
const uint8 kInvItemBranch = 15;
const uint8 kInvItemWhisky = 16;
const uint8 kInvItemNeedle = 17;
const uint8 kInvItemWrapper = 18;
const uint8 kInvItemChocCandy = 19;
const uint8 kInvItemPotato = 20;
const uint8 kInvItemRakeBroken = 21;
const uint8 kInvItemHeartShapedCandy = 22;
const uint8 kInvItemWrappedCandy = 23; // HeartShapedCandy combined with Wrapper
const uint8 kInvItemRibbon = 24;
const uint8 kInvItemRakeFixed = 25; // Rake combined with Ribbon
const uint8 kInvItemNut = 26;
const uint8 kInvItemPlasticApple = 27;
const uint8 kInvItemCone = 28;
const uint8 kInvItemSuperGlue = 29;
const uint8 kInvItemConeAndNeedle = 30; // Cone combined with Needle
const uint8 kInvItemConeAndFeather = 31; // Cone combined with Feather
const uint8 kInvItemDart = 32; // Needle combined with ConeAndFeather or Feather combined with ConeAndNeedle
const uint8 kInvItemFeatherDusterDirty = 33;
const uint8 kInvItemPaintedPotato = 34; // Potato combined with Dirty Feather Duster (Soot)
const uint8 kInvItemCarJack = 35;
const uint8 kInvItemBone = 36;
const uint8 kInvItemShovelAct2 = 37;
const uint8 kInvItemRopeAct2 = 38;
const uint8 kInvItemMask = 39;
const uint8 kInvItemFins = 40;
const uint8 kInvItemDiveEquipment = 41; // Mask combined with Fins
const uint8 kInvItemAnchor = 42;
const uint8 kInvItemGrapplingHook = 43;
const uint8 kInvItemSickleBlunt = 44;
const uint8 kInvItemCheese = 45;
const uint8 kInvItemSickleSharp = 46;
const uint8 kInvItemHandkerchief = 47;
const uint8 kInvItemMouse = 48;
const uint8 kInvItemRock = 49;
const uint8 kInvItemNugget = 50;
const uint8 kInvItemBanknote = 51;
const uint8 kInvItemDictaphoneNoBatteries = 52;
const uint8 kInvItemPolaroidCamera = 53;
const uint8 kInvItemVideoTape = 54;
const uint8 kInvItemSheetOfPaper = 55;
const uint8 kInvItemCognac = 56;
const uint8 kInvItemRemoteControl = 57;
const uint8 kInvItemIceTongs = 58;
const uint8 kInvItemCork = 59;
const uint8 kInvItemWrappedCork = 60; // Cork combined with Sheet Of Paper
const uint8 kInvItemPhoto = 61;
const uint8 kInvItemChilliWithLabel = 62;
const uint8 kInvItemPastryRoller = 63;
const uint8 kInvItemFakeChilli = 64;
const uint8 kInvItemLabel = 65;
const uint8 kInvItemBatteries = 66;
const uint8 kInvItemDictaphoneWithBatteries = 67; // Dictaphone combined with Batteries
const uint8 kInvItemBurningPaper = 68;
const uint8 kInvItemMeat = 69;
const uint8 kInvItemPlasticBag = 70;
const uint8 kInvItemSocks = 71;
const uint8 kInvItemTimePills = 72;
const uint8 kInvItemHandle = 73;
const uint8 kInvItemChilliNoLabel = 74;
const uint8 kInvItemPass = 75;
const uint8 kInvItemBulb = 76;
const uint8 kInvItemJailKey = 77;
const uint8 kInvItemDelicatePlant = 78;
const uint8 kInvItemSwissArmyKnife = 79;
const uint8 kInvItemSpring = 80;
const uint8 kInvItemShovelAct1 = 81;
const uint8 kInvItemKaleidoscope = 82;
const uint8 kInvItemSoldierNews = 83;
const uint8 kInvItemGrenade = 84;
const uint8 kInvItemMug = 85; // Empty
const uint8 kInvItemMugOfMud = 86; // Full of mud
const uint8 kInvItemCrumbs = 87;
const uint8 kInvItemRopeAct1 = 88;
const uint8 kInvItemRopeAndGrenade = 89; // Rope combined with Grenade
const uint8 kInvItemMedicine = 90;
const uint8 kInvItemDruggedFood = 91; // Crumbs combined with Medicine
const uint8 kInvItemBird = 92;

// Maximum number of inventory items held by Ego (Mark)
const uint8 kInventorySize = 24;

class Inventory {
public:
	Inventory(TeenAgentEngine *vm);
	~Inventory();

	void render(Graphics::Surface *surface, int delta);

	void clear();
	void reload();
	void add(byte item);
	bool has(byte item) const;
	void remove(byte item);

	void activate(bool a) { _active = a; }
	bool active() const { return _active; }

	bool processEvent(const Common::Event &event);

	InventoryObject *selectedObject() { return _selectedObj; }
	void resetSelectedObject() { _selectedObj = NULL; }

private:
	TeenAgentEngine *_vm;
	Surface _background;
	byte *_items;
	uint _offset[kNumInventoryItems+1];

	Common::Array<InventoryObject> _objects;
	byte *_inventory;

	struct Item {
		Animation _animation;
		Surface _surface;
		Rect _rect;
		bool _hovered;

		Item() : _hovered(false) {}
		void free();
		void load(Inventory *inventory, uint itemId);
		void backgroundEffect(Graphics::Surface *s);
		void render(Inventory *inventory, uint itemId, Graphics::Surface *surface, int delta);
	};

	Item _graphics[kInventorySize];

	bool _active;
	Common::Point _mouse;

	bool tryObjectCallback(InventoryObject *obj);

	InventoryObject *_hoveredObj;
	InventoryObject *_selectedObj;
};

} // End of namespace TeenAgent

#endif
