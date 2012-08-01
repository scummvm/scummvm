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
const uint8 numInventoryItems = 92;

// Inventory Item Ids
const uint8 invItemNoItem = 0; // No item i.e. empty inventory slot
const uint8 invItemFeather = 1;
const uint8 invItemShotgun = 2;
const uint8 invItemToolboxFull = 3; // Contains Car Jack and Spanner
const uint8 invItemToolboxHalfEmpty = 4; // Contains Spanner
const uint8 invItemSpanner = 5;
const uint8 invItemComb = 6;
const uint8 invItemFan = 7;
const uint8 invItemBrokenPaddle = 8;
const uint8 invItemPaddle = 9; // Repaired - BrokenPaddle combined with Branch (with Glue)
const uint8 invItemFirstFlower = 10; // Smells nice
const uint8 invItemSecondFlower = 11; // Really beautiful
const uint8 invItemFeatherDusterClean = 12;
const uint8 invItemChainsaw = 13; // Unfueled
const uint8 invItemDrunkenChainsaw = 14; // Fueled with Whisky (Chainsaw combined with Whiskey)
const uint8 invItemBranch = 15;
const uint8 invItemWhisky = 16;
const uint8 invItemNeedle = 17;
const uint8 invItemWrapper = 18;
const uint8 invItemChocCandy = 19;
const uint8 invItemPotato = 20;
const uint8 invItemRakeBroken = 21;
const uint8 invItemHeartShapedCandy = 22;
const uint8 invItemWrappedCandy = 23; // HeartShapedCandy combined with Wrapper
const uint8 invItemRibbon = 24;
const uint8 invItemRakeFixed = 25; // Rake combined with Ribbon
const uint8 invItemNut = 26;
const uint8 invItemPlasticApple = 27;
const uint8 invItemCone = 28;
const uint8 invItemSuperGlue = 29;
const uint8 invItemConeAndNeedle = 30; // Cone combined with Needle
const uint8 invItemConeAndFeather = 31; // Cone combined with Feather
const uint8 invItemDart = 32; // Needle combined with ConeAndFeather or Feather combined with ConeAndNeedle
const uint8 invItemFeatherDusterDirty = 33;
const uint8 invItemPaintedPotato = 34; // Potato combined with Dirty Feather Duster (Soot)
const uint8 invItemCarJack = 35;
const uint8 invItemBone = 36;
const uint8 invItemShovelAct2 = 37;
const uint8 invItemRopeAct2 = 38;
const uint8 invItemMask = 39;
const uint8 invItemFins = 40;
const uint8 invItemDiveEquipment = 41; // Mask combined with Fins
const uint8 invItemAnchor = 42;
const uint8 invItemGrapplingHook = 43;
const uint8 invItemSickleBlunt = 44;
const uint8 invItemCheese = 45;
const uint8 invItemSickleSharp = 46;
const uint8 invItemHandkerchief = 47;
const uint8 invItemMouse = 48;
const uint8 invItemRock = 49;
const uint8 invItemNugget = 50;
const uint8 invItemBanknote = 51;
const uint8 invItemDictaphoneNoBatteries = 52;
const uint8 invItemPolaroidCamera = 53;
const uint8 invItemVideoTape = 54;
const uint8 invItemSheetOfPaper = 55;
const uint8 invItemCognac = 56;
const uint8 invItemRemoteControl = 57;
const uint8 invItemIceTongs = 58;
const uint8 invItemCork = 59;
const uint8 invItemWrappedCork = 60; // Cork combined with Sheet Of Paper
const uint8 invItemPhoto = 61;
const uint8 invItemChilliWithLabel = 62;
const uint8 invItemPastryRoller = 63;
const uint8 invItemFakeChilli = 64;
const uint8 invItemLabel = 65;
const uint8 invItemBatteries = 66;
const uint8 invItemDictaphoneWithBatteries = 67; // Dictaphone combined with Batteries
const uint8 invItemBurningPaper = 68;
const uint8 invItemMeat = 69;
const uint8 invItemPlasticBag = 70;
const uint8 invItemSocks = 71;
const uint8 invItemTimePills = 72;
const uint8 invItemHandle = 73;
const uint8 invItemChilliNoLabel = 74;
const uint8 invItemPass = 75;
const uint8 invItemBulb = 76;
const uint8 invItemJailKey = 77;
const uint8 invItemDelicatePlant = 78;
const uint8 invItemSwissArmyKnife = 79;
const uint8 invItemSpring = 80;
const uint8 invItemShovelAct1 = 81;
const uint8 invItemKaleidoscope = 82;
const uint8 invItemSoldierNews = 83;
const uint8 invItemGrenade = 84;
const uint8 invItemMug = 85; // Empty
const uint8 invItemMugOfMud = 86; // Full of mud
const uint8 invItemCrumbs = 87;
const uint8 invItemRopeAct1 = 88;
const uint8 invItemRopeAndGrenade = 89; // Rope combined with Grenade
const uint8 invItemMedicine = 90;
const uint8 invItemDruggedFood = 91; // Crumbs combined with Medicine
const uint8 invItemBird = 92;

// Maximum number of inventory items held by Ego (Mark)
const uint8 inventorySize = 24;

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
	uint _offset[numInventoryItems+1];

	Common::Array<InventoryObject> _objects;
	byte *_inventory;

	struct Item {
		Animation _animation;
		Surface _surface;
		Rect _rect;
		bool _hovered;

		Item() : _hovered(false) {}
		void free();
		void load(Inventory *inventory, uint item_id);
		void backgroundEffect(Graphics::Surface *s);
		void render(Inventory *inventory, uint item_id, Graphics::Surface *surface, int delta);
	};

	Item _graphics[inventorySize];

	bool _active;
	Common::Point _mouse;

	bool tryObjectCallback(InventoryObject *obj);

	InventoryObject *_hoveredObj;
	InventoryObject *_selectedObj;
};

} // End of namespace TeenAgent

#endif
