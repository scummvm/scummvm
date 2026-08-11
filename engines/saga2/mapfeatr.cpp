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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/mapfeatr.h"
#include "saga2/automap.h"
#include "saga2/tile.h"

namespace Saga2 {

/* ===================================================================== *
   Constants
 * ===================================================================== */

#define STARGATE_COLOR  (164+9)
#define VILLAGE_COLOR   (66+9)
#define CAVE_COLOR      (10+9)

/* ===================================================================== *
   Prototypes and inlines
 * ===================================================================== */

#define MAP_VILLAGE(u,v,n)  g_vm->_mapFeatures.push_back(\
        new CStaticMapFeature(TilePoint(u,v,0),0,n,VILLAGE_COLOR))
#define MAP_CAVE(u,v,n)  g_vm->_mapFeatures.push_back(\
        new CStaticMapFeature(TilePoint(u,v,0),0,n,CAVE_COLOR))
#define MAP_STARGATE(u,v,n) g_vm->_mapFeatures.push_back(\
        new CStaticMapFeature(TilePoint(u,v,0),0,n,STARGATE_COLOR))

/* ===================================================================== *
   Map feature list maintenance
 * ===================================================================== */

// ------------------------------------------------------------------------
// init

void initMapFeatures() {
	//MAP_VILLAGE(13000,12535,"DummyVillage");
	//MAP_STARGATE(1,2,"DummyGate");

//Padavis
	MAP_VILLAGE(13368, 12712, "Padavis");
//	MAP_VILLAGE(13464, 12072,"Singing Hammer Smith Shop");
//	MAP_VILLAGE(13368,12712,"The Glass Sparrow Inn");
//	MAP_VILLAGE(13512,12232,"The Third Eye");
//	MAP_VILLAGE(12712,12008,"Caladrin's General Store");
	MAP_VILLAGE(12904, 12328, "Royal Coach Service");
//	MAP_VILLAGE(13252,12520,"Horse Statue");

	MAP_CAVE(10296, 9176, "Cave");      //  Sorcerer's Lair

	MAP_VILLAGE(9693, 10633, "Goblin Village");     //  Goblin Village
//	MAP_VILLAGE(13128,12216,"Birket's Birch");
	MAP_VILLAGE(14456, 9544, "Castle Jovanc");
//	MAP_VILLAGE(13894,12498,"Apple Grove");

	//Hatak
	MAP_VILLAGE(5096, 4351, "Gleng'l Zur");     //  Front
//	MAP_VILLAGE(4842,3839,"Hatak Arena");
	MAP_VILLAGE(8208, 3120, "Royal Coach Service");

	//Karminac
	//  Darnoc
	MAP_VILLAGE(13384, 18544, "Darnoc");
//	MAP_VILLAGE(13430,18770,"Guard Barrack");
//	MAP_VILLAGE(13260,18740,"Bull Dog's Tail");
//	MAP_VILLAGE(13064,19208,"Sorcerer's Hall");
//	MAP_VILLAGE(12860,19076,"Warrior's Boon");
//	MAP_VILLAGE(12516,19122,"Mercenaries Guild");
//	MAP_VILLAGE(12231,19464,"Blacksmith");
//	MAP_VILLAGE(12155,19622,"Warehouse");
//	MAP_VILLAGE(12488,19705,"Dragon's Gamble");
//	MAP_VILLAGE(12992,19590,"Armor Shop");
//	MAP_VILLAGE(13190,19922,"Fish Market");
//	MAP_VILLAGE(13260,20518,"Ghost Boat");
//	MAP_VILLAGE(13530,19717,"Gallows");
//	MAP_VILLAGE(13805,19523,"Palace");
//	MAP_VILLAGE(13430,19120,"Weapon Exchange");
//	MAP_VILLAGE(13380,19190,"Pawn Shop");
	MAP_VILLAGE(13240, 18104, "Royal Coach Service");

	//Roska's Citadel
	MAP_VILLAGE(12550, 23096, "Roska's Citadel");

	//Pentere
	MAP_VILLAGE(21608, 11512, "Bilton");
//	MAP_VILLAGE(21120,10156,"Bilton main gate");
//	MAP_VILLAGE(21760,10284,"Bilton Slaughter Yard");
	MAP_VILLAGE(22217, 11365, "Royal Coach Service");
//	MAP_VILLAGE(22424,11269,"Orsolo's Cupboard");
//	MAP_VILLAGE(22616,11016,"Alchemist Hollow");
//	MAP_VILLAGE(22394,10703,"Weapon Shack");
//	MAP_VILLAGE(22392,10536,"Palmer's Pawn Shop");
//	MAP_VILLAGE(22408,10440,"Gully's Bakery");
//	MAP_VILLAGE(22525,10321,"Whale of a Tale");
//	MAP_VILLAGE(22602,10104,"Elegant Ornament");
//	MAP_VILLAGE(22392,9832,"Swines Pride");
//	MAP_VILLAGE(21960,11016,"Wizards Guild");
//	MAP_VILLAGE(22104,11000,"Weapon Exchange");
//	MAP_VILLAGE(22065,10613,"Guard Barracks");
//	MAP_VILLAGE(22209,10621,"Draigon the Armorer");
//	MAP_VILLAGE(22340,9976,"Fighter's Guild");
//	MAP_VILLAGE(21960,9992,"Key Figures Locksmith");
//	MAP_VILLAGE(22021,9801,"Brewery");
	MAP_VILLAGE(21608, 11512, "Bilton Noble Park");
//	MAP_VILLAGE(22056,11960,"Bilton Archives");
//	MAP_VILLAGE(22120,12164,"Gold Tree");
//	MAP_VILLAGE(21316,10376,"Weapon Buyer");
//	MAP_VILLAGE(21636,9672,"Blacksmith");
//	MAP_VILLAGE(21306,9970,"Tavern");
//	MAP_VILLAGE(21685,14791,"Cottage");          //  Brythe's Cottage

	//Hethrallin
	MAP_VILLAGE(4012, 12268, "Ta-Taavan");
//	MAP_VILLAGE(3624,11960,"Craft corner");
//	MAP_VILLAGE(4012,12268,"Royal corner");
//	MAP_VILLAGE(3660,12332,"Healer's corner");
//	MAP_VILLAGE(4008,11944,"Mage corner");
//	MAP_VILLAGE(4024,11752,"Weaver");
	MAP_VILLAGE(4752, 10544, "Royal Coach Service");

	//Maldavith
	MAP_VILLAGE(6520, 17176, "Maldavith");
//	MAP_VILLAGE(6648,17688,"Stockade");
//	MAP_VILLAGE(6520,17176,"House of Brindav");
//	MAP_VILLAGE(6328,17688,"Skull & Cleaver");
//	MAP_VILLAGE(6464,17472,"Filmore Town Square");
//	MAP_VILLAGE(6615,17031,"Brindav Court");
//	MAP_VILLAGE(6076,17031,"Sailmaker");
//	MAP_VILLAGE(6136,17352,"Import/Export office");
//	MAP_VILLAGE(6262,17195,"Curious Cutlass");
//	MAP_VILLAGE(5738,17205,"Scurvy Dogs Pawn Shop");
//	MAP_VILLAGE(5816,17000,"Royal Coach Service");
//	MAP_VILLAGE(6000,17080,"Filmore Marketplace");
//	MAP_VILLAGE(5404,17160,"Warehouse");
//	MAP_VILLAGE(5160,17032,"Droll Parrot");
//	MAP_VILLAGE(5189,17689,"Dock");
//	MAP_VILLAGE(5680,16688,"South West Gate");
//	MAP_VILLAGE(6945,17468,"North East Gate");
	MAP_VILLAGE(1016, 19272, "Light Tower");
	MAP_VILLAGE(5035, 19002, "Tamnath Ruins");

	//Mons
	MAP_VILLAGE(20496, 17456, "Royal Coach Service");

	//Aroblin
	MAP_VILLAGE(14152, 4360, "Amber Castle");
//	MAP_CAVE(14920,4952,"Dwarven Mine");
	MAP_VILLAGE(12560, 1968, "Royal Coach Service");

	//STARGATES
	MAP_STARGATE(14792, 12856,   "Obelisks");   // Wildevarr Stargate 1
	MAP_STARGATE(10296, 9416,    "Obelisks");   // Wildevarr Stargate 2
	MAP_STARGATE(21576, 12984,   "Obelisks");   // Pentere Stargate 1
	MAP_STARGATE(17976, 8520,    "Obelisks");   // Pentere Stargate 2
	MAP_STARGATE(22328, 21832,   "Obelisks");   // Mons Stargate 1
	MAP_STARGATE(21816, 19784,   "Obelisks");   // Mons Stargate 2
	MAP_STARGATE(12216, 23624,   "Obelisks");   // Karminac Stargate 1
	MAP_STARGATE(14904, 17224,   "Obelisks");   // Karminac Stargate 2
	MAP_STARGATE(6344, 21944,    "Obelisks");   // Maldavith Stargate 1
	MAP_STARGATE(4152, 16200,    "Obelisks");   // Maldavith Stargate 2
	MAP_STARGATE(7112, 13368,    "Obelisks");   // Hethrallin Stargate 1
	MAP_STARGATE(3656, 9528,     "Obelisks");   // Hethrallin Stargate 2
	MAP_STARGATE(5944, 5192,     "Obelisks");   // Hatak Stargate 1
	MAP_STARGATE(1720, 968,      "Obelisks");   // Hatak Stargate 2
	MAP_STARGATE(13256, 4664,    "Obelisks");   // Aroblin Stargate 1
	MAP_STARGATE(15544, 1608,    "Obelisks");   // Aroblin Stargate 2

	MAP_CAVE(10457, 14145, "Cave"); //Wildevarr West Cave 1
	MAP_CAVE(11996, 16310, "Cave"); //Wildevarr West Cave 2
	MAP_CAVE(11224, 14920, "Cave"); //Wildevarr West Cave 3
	MAP_CAVE(10181, 14664, "Cave"); //Wildevarr West Cave 4
	MAP_CAVE(9814, 12870, "Cave");  //Wildevarr West Cave 5
	MAP_CAVE(10280, 9160, "Cave");  //Wizard's Lair
	MAP_CAVE(12104, 8920, "Cave");  //Wildevarr Cave
	MAP_CAVE(13893, 13655, "Cave"); //Wildevarr Bandit Cave
	MAP_CAVE(14808, 15172, "Cave"); //Wildevarr Cave
	MAP_CAVE(9560, 20161, "Cave");  //Karminac Cave
	MAP_CAVE(8772, 18776, "Cave");  //Karminac Cave
	MAP_CAVE(14560, 18763, "Cave"); //Karminac Cave
	MAP_CAVE(18510, 15297, "Cave"); //Pentere Cave
	MAP_CAVE(20568, 15284, "Cave"); //Pentere Cave
	MAP_CAVE(16856, 9540, "Cave");  //Pentere Cave
	MAP_CAVE(4408, 9816, "Cave"); //Dark Elf UG
	MAP_CAVE(7368, 12888, "Cave");  //Hethrallin Cave
	MAP_CAVE(7128, 14664, "Cave");  //Hethrallin Cave
	MAP_CAVE(2648, 21185, "Cave");  //Maldavith Cave
	MAP_CAVE(1624, 17988, "Cave");  //Gretmar Island
	MAP_CAVE(6084, 22360, "Cave");  //Smuggler's Cave
	MAP_CAVE(1848, 15704, "Cave");  //Pirate Cave
	MAP_CAVE(7368, 22613, "Cave");  //Maldavith Cave
	MAP_CAVE(17608, 23368, "Cave"); //Mons false cairn
	MAP_CAVE(20420, 23368, "Cave"); //Frostwing
	MAP_CAVE(18852, 19768, "Cave"); //Rock Giant
	MAP_CAVE(21560, 19000, "Cave"); //WhiteDeep
	MAP_CAVE(22324, 18504, "Cave"); //Snowmen
	MAP_CAVE(22984, 21172, "Cave"); //Wolf Cave 1
	MAP_CAVE(23188, 21064, "Cave"); //Wolf Cave 2
	MAP_CAVE(23220, 20936, "Cave"); //Wolf Cave 3
	MAP_CAVE(22180, 22456, "Cave"); //IceWind
	MAP_CAVE(10200, 2484, "Cave");  //Aroblin Cave
	MAP_CAVE(14900, 4952, "Cave");  //Aroblin Mine
}

// ------------------------------------------------------------------------
// load (derive from map data)

void updateMapFeatures(int16 cWorld) {
	extern WorldMapData         *mapList;
	extern GameWorld            *currentWorld;
	WorldMapData    *wMap = &mapList[currentWorld->_mapNum];

	uint16          *mapData = wMap->map->mapData;

	for (uint i = 0; i < g_vm->_mapFeatures.size(); i++) {
		if (g_vm->_mapFeatures[i]->getWorld() == cWorld) {
			uint16   *mapRow;
			mapRow = &mapData[(g_vm->_mapFeatures[i]->getU() >> (kTileUVShift + kPlatShift)) * wMap->mapSize];
			uint16   mtile = mapRow[(g_vm->_mapFeatures[i]->getV() >> (kTileUVShift + kPlatShift))];
			g_vm->_mapFeatures[i]->expose(mtile & kMetaTileVisited);
		}
	}
}

// ------------------------------------------------------------------------
// draw

void drawMapFeatures(TileRegion viewRegion,
                     int16 inWorld,
                     TilePoint baseCoords,
                     gPort &tPort) {
	for (uint i = 0; i < g_vm->_mapFeatures.size(); i++) {
		g_vm->_mapFeatures[i]->draw(viewRegion, inWorld, baseCoords, tPort);
	}

}

// ------------------------------------------------------------------------
// draw

char noMFText[] = "";

char *getMapFeaturesText(TileRegion viewRegion,
                         int16 inWorld,
                         TilePoint baseCoords,
                         Point16 mouseCoords) {
	TilePoint m2 = TilePoint(mouseCoords.x, mouseCoords.y, 0);

	for (uint i = 0; i < g_vm->_mapFeatures.size(); i++) {
		if (g_vm->_mapFeatures[i]->hitCheck(viewRegion, inWorld, baseCoords, m2))
			return g_vm->_mapFeatures[i]->getText();
	}
	return noMFText;

}

// ------------------------------------------------------------------------
// cleanup

void termMapFeatures() {
	for (uint i = 0; i < g_vm->_mapFeatures.size(); i++) {
		if (g_vm->_mapFeatures[i])
			delete g_vm->_mapFeatures[i];
	}

	g_vm->_mapFeatures.clear();
}




/* ===================================================================== *
   CMapFeatute class implementation
 * ===================================================================== */



CMapFeature::CMapFeature(TilePoint where, int16 inWorld, const char *desc) {
	_visible = false;
	_featureCoords = where;
	_world = inWorld;
	Common::strlcpy(_name, desc, MAX_MAP_FEATURE_NAME_LENGTH);
}


void CMapFeature::draw(TileRegion viewRegion,
                       int16 inWorld,
                       TilePoint baseCoords,
                       gPort &tPort) {
	int32           x, y;

	if (_world != inWorld) return;
	update();

	//TilePoint centerCoords = _featureCoords >> (kTileUVShift + kPlatShift);
	TilePoint fCoords = _featureCoords >> (kTileUVShift + kPlatShift);
	if (_visible                               &&
	        fCoords.u >= viewRegion.min.u   &&
	        fCoords.u <= viewRegion.max.u   &&
	        fCoords.v >= viewRegion.min.v   &&
	        fCoords.v <= viewRegion.max.v) {
		TilePoint centerPt;

		//  Calculate the position of the cross-hairs showing the position of
		//  the center actor.
		centerPt = _featureCoords - (baseCoords << (kTileUVShift + kPlatShift));

		x = ((centerPt.u - centerPt.v) >> (kTileUVShift + kPlatShift - 2)) + 261 + 4;
		y = 255 + 4 - ((centerPt.u + centerPt.v) >> (kTileUVShift + kPlatShift - 1));
#ifdef DEBUG_FEATUREPOS
		WriteStatusF(12, "draw at (%d,%d)", x, y);
#endif
		blit(tPort, x, y);
	}
#ifdef DEBUG_FEATUREPOS
	else {
		char msg[256];
		Common::sprintf_s(msg, "Hide: ");
		if (!visible) Common::strcat_s(msg, "not visible");
		if (!(fCoords.u >= viewRegion.min.u)) Common::sprintf_s(msg + strlen(msg), "U lo %d,%d ", fCoords.u, viewRegion.min.u);
		if (!(fCoords.u <= viewRegion.max.u)) Common::sprintf_s(msg + strlen(msg), "U hi %d,%d ", fCoords.u, viewRegion.max.u);
		if (!(fCoords.v >= viewRegion.min.v)) Common::sprintf_s(msg + strlen(msg), "V lo %d,%d ", fCoords.v, viewRegion.min.v);
		if (!(fCoords.v <= viewRegion.max.v)) Common::sprintf_s(msg + strlen(msg), "V hi %d,%d ", fCoords.v, viewRegion.max.v);
		WriteStatusF(12, "%s", msg);
	}
#endif
}

bool CMapFeature::hitCheck(TileRegion viewRegion,
                           int16 inWorld,
                           TilePoint baseCoords,
                           TilePoint comparePoint) {
	int32           x, y;

	if (_world != inWorld) return false;
	TilePoint fCoords = _featureCoords >> (kTileUVShift + kPlatShift);
	if (_visible                               &&
	        fCoords.u >= viewRegion.min.u   &&
	        fCoords.u <= viewRegion.max.u   &&
	        fCoords.v >= viewRegion.min.v   &&
	        fCoords.v <= viewRegion.max.v) {
		TilePoint centerPt;

		//  Calculate the position of the cross-hairs showing the position of
		//  the center actor.
		centerPt = _featureCoords - (baseCoords << (kTileUVShift + kPlatShift));

		x = ((centerPt.u - centerPt.v) >> (kTileUVShift + kPlatShift - 2)) + 261 + 4;
		y = 255 + 4 - ((centerPt.u + centerPt.v) >> (kTileUVShift + kPlatShift - 1));

		TilePoint DisplayPoint = TilePoint(x, y, 0);
		return isHit(DisplayPoint, comparePoint);
	}
	return false;
}

/* ===================================================================== *
   CStaticMapFeatute class implementation
 * ===================================================================== */

CStaticMapFeature::CStaticMapFeature(TilePoint where, int16 inWorld, const char *desc, int16 bColor)
	: CMapFeature(where, inWorld, desc) {
	_color = bColor;
}

void CStaticMapFeature::blit(gPort &tPort, int32 x, int32 y) {
	tPort.setColor(9 + 15);          //  black
	tPort.fillRect(x - 2, y - 2, 5, 5);
	tPort.setColor(_color);       //  whatever color its supposed to be
	tPort.fillRect(x - 1, y - 1, 3, 3);
}

bool CStaticMapFeature::isHit(TilePoint disp, TilePoint mouse) {
	TilePoint diff = disp - mouse;
	WriteStatusF(14, "compare (%d,%d):(%d,%d)", disp.u, disp.v, mouse.u, mouse.v);
	return (diff.magnitude() < 4);
}

/* ===================================================================== *
   CPictureMapFeatute class implementation
 * ===================================================================== */

CPictureMapFeature::CPictureMapFeature(TilePoint where, int16 inWorld, char *desc, gPixelMap *pm)
	: CMapFeature(where, inWorld, desc) {
	_pic = pm;
}

void CPictureMapFeature::blit(gPort &tPort, int32 x, int32 y) {
	// This is dummy code - it draws crosshairs

	tPort.setColor(9 + 15);      //  black
	tPort.fillRect(x - 3, y - 1, 7, 3);
	tPort.fillRect(x - 1, y - 3, 3, 7);
	tPort.setColor(9 + 1);       //  white
	tPort.hLine(x - 2, y, 5);
	tPort.vLine(x, y - 2, 5);
}

} // end of namespace Saga2
