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

#include "objects.h"

Darkseed::Objects::Objects() {
	_objectVar.resize(MAX_OBJECTS);
	_objectRunningCode.resize(MAX_OBJECTS);
	_moveObjectXY.resize(MAX_OBJECTS);
	_moveObjectRoom.resize(MAX_OBJECTS); // The original only allocates 42 entries here but writes 199 in the save file!
	reset();
}

void Darkseed::Objects::reset() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		_objectVar[i] = 0;
		_objectRunningCode[i] = 0;
		_moveObjectXY[i].x = 0; // TODO verify this is the correct reset state for these XY vars.
		_moveObjectXY[i].y = 0;
		_moveObjectRoom[i] = i < 42 ? 0xff : 0; // Hack for weird behaviour in original engine.
	}
	// Initial object state.
	setVar(52, 1);
	setVar(112, 0);
	setVar(62, 0);
}

void Darkseed::Objects::setVar(uint16 varIdx, int16 newValue) {
	if (varIdx >= MAX_OBJECTS) {
		error("setVar: Object Index out of range! %d", varIdx);
	}
	_objectVar[varIdx] = newValue;
}

int16 Darkseed::Objects::getVar(uint16 varIdx) {
	if (varIdx >= MAX_OBJECTS) {
		error("getVar: Object Index out of range! %d", varIdx);
	}
	return _objectVar[varIdx];
}

Common::Point Darkseed::Objects::getMoveObjectPosition(uint8 objIdx) {
	if (objIdx >= MAX_OBJECTS) {
		error("getMoveObjectPosition: Object Index out of range! %d", objIdx);
	}
	return _moveObjectXY[objIdx];
}

void Darkseed::Objects::setMoveObjectPosition(uint8 objIdx, const Common::Point &newPoint) {
	if (objIdx >= MAX_OBJECTS) {
		error("setMoveObjectPosition: Object Index out of range! %d", objIdx);
	}
	_moveObjectXY[objIdx] = newPoint;
}

void Darkseed::Objects::setMoveObjectX(uint8 objIdx, int16 xPos) {
	if (objIdx >= MAX_OBJECTS) {
		error("setMoveObjectX: Object Index out of range! %d", objIdx);
	}
	_moveObjectXY[objIdx].x = xPos;
}

int16 &Darkseed::Objects::operator[](uint16 varIdx) {
	if (varIdx >= MAX_OBJECTS) {
		error("getVar: Object Index out of range! %d", varIdx);
	}
	return _objectVar[varIdx];
}
const int16 &Darkseed::Objects::operator[](uint16 varIdx) const {
	if (varIdx >= MAX_OBJECTS) {
		error("getVar: Object Index out of range! %d", varIdx);
	}
	return _objectVar[varIdx];
}

static constexpr uint16 eyeDescriptionsTbl[] = {
	0, 0, 0, 0,
	0, 0, 0, 513,
	0, 421, 0, 521,
	0, 0, 713, 0,
	791, 812, 0, 743,
	0, 0, 661, 166,
	0, 0, 0, 0,
	500, 0, 0, 423,
	425, 427, 418, 667,
	503, 505, 507, 509,
	511, 755, 652, 0,
	728, 0, 0, 43,
	0, 0, 0, 0,
	192, 0, 0, 0,
	0, 893, 845, 0,
	0, 452, 721, 0,
	483, 0, 466, 466,
	466, 0, 0, 705,
	0, 0, 0, 0,
	0, 0, 0, 829,
	552, 0, 0, 0,
	0, 0, 711, 608,
	610, 606, 604, 602,
	600, 598, 596, 594,
	592, 590, 588, 0,
	0, 732, 166, 0,
	0, 0, 0, 0,
	0, 0, 0, 241,
	231, 750, 815, 826,
	838, 794, 797, 806,
	802, 440, 448, 117,
	259, 271, 305, 90,
	161, 136, 376, 398,
	414, 474, 477, 480,
	0, 0, 999, 252,
	0, 0, 170, 182,
	212, 219, 284, 315,
	328, 337, 346, 356,
	515, 526, 533, 0,
	547, 561, 570, 575,
	613, 615, 620, 624,
	636, 638, 641, 643,
	645, 0, 673, 677,
	680, 683, 688, 717,
	726, 746, 0, 759,
	765, 780, 787, 818,
	822, 824, 0, 855,
	862, 0, 880, 887,
	891, 900, 0, 724,
	671, 321, 163
};

static constexpr uint16 handDescriptionsTbl[] = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	791, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 652, 0,
	729, 0, 0, 0,
	0, 0, 0, 852,
	0, 0, 0, 772,
	0, 0, 846, 0,
	0, 453, 0, 0,
	484, 0, 0, 0,
	0, 0, 0, 706,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 609,
	611, 607, 605, 603,
	601, 599, 597, 595,
	593, 591, 589, 0,
	0, 0, 0, 355,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 827,
	839, 0, 798, 807,
	803, 441, 449, 118,
	260, 272, 306, 91,
	162, 137, 377, 399,
	415, 475, 478, 481,
	0, 0, 999, 253,
	0, 0, 171, 183,
	213, 220, 285, 316,
	329, 338, 338, 357,
	516, 527, 534, 0,
	548, 562, 571, 576,
	614, 616, 621, 625,
	637, 0, 642, 644,
	646, 0, 674, 678,
	681, 684, 689, 0,
	726, 747, 0, 760,
	766, 781, 788, 819,
	823, 825, 0, 0,
	863, 0, 881, 888,
	892, 901, 0, 0,
	672, 322, 164
};

static constexpr int16 glovesTextTbl[199] = {
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 525,
	999, 999, 999, 999,
	999, 84, 999, 745,
	999, 999, 84, 999,
	999, 84, 84, 999,
	999, 999, 999, 999,
	999, 999, 999, 84,
	999, 999, 999, 999,
	999, 999, 84, 999,
	84, 999, 84, 49,
	999, 84, 999, 999,
	198, 487, 999, 773,
	999, 0, 999, 84,
	999, 84, 84, 999,
	999, 999, 84, 84,
	84, 999, 999, 84,
	999, 999, 999, 999,
	999, 999, 35, 999,
	84, 999, 999, 999,
	84, 84, 84, 84,
	84, 84, 84, 84,
	84, 84, 84, 84,
	84, 84, 84, 999,
	84, 84, 999, 999,
	999, 999, 999, 999,
	386, 84, 392, 84,
	996, 84, 999, 981,
	999, 796, 999, 808,
	804, 84, 84, 84,
	999, 84, 84, 84,
	999, 998, 991, 84,
	84, 999, 84, 84,
	999, 84, 999, 84,
	999, 999, 84, 185,
	84, 84, 84, 84,
	84, 84, 84, 999,
	999, 84, 84, 999,
	84, 999, 84, 84,
	84, 84, 999, 999,
	999, 999, 999, 999,
	999, 999, 84, 84,
	84, 84, 84, 84,
	84, 84, 999, 999,
	768, 999, 999, 820,
	84, 84, 999, 999,
	999, 999, 999, 84,
	84, 999, 84, 84,
	84, 84, 999
};

static constexpr int16 moneyTextTbl[199] = {
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 169,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	961, 961, 961, 961,
	961, 999, 999, 999,
	999, 999, 543, 58,
	872, 999, 999, 999,
	206, 999, 999, 999,
	999, 999, 999, 999,
	999, 461, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 987,
	999, 999, 999, 999,
	999, 999, 999, 999,
	560, 999, 999, 999,
	999, 436, 999, 586,
	586, 586, 586, 586,
	586, 586, 586, 586,
	586, 586, 586, 999,
	156, 986, 999, 999,
	999, 999, 999, 999,
	386, 992, 392, 999,
	996, 0, 999, 999,
	999, 999, 999, 999,
	804, 990, 999, 132,
	999, 281, 86, 111,
	999, 998, 991, 402,
	417, 999, 999, 999,
	999, 303, 0, 999,
	999, 999, 999, 190,
	217, 227, 290, 999,
	999, 999, 999, 999,
	906, 531, 999, 999,
	550, 999, 573, 577,
	999, 999, 999, 633,
	633, 633, 633, 633,
	633, 999, 999, 999,
	988, 687, 999, 999,
	999, 999, 999, 999,
	999, 785, 999, 999,
	980, 982, 999, 999,
	865, 999, 885, 999,
	999, 999, 989, 999,
	999, 326, 999
};

static constexpr int16 crowBarTextTbl[199] = {
	999, 999, 999, 999,
	999, 999, 999, 518,
	999, 999, 999, 524,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	999, 999, 999, 999,
	518, 518, 518, 518,
	518, 757, 0, 999,
	999, 999, 999, 51,
	999, 249, 999, 999,
	999, 485, 999, 999,
	999, 999, 979, 411,
	999, 456, 999, 999,
	999, 999, 470, 470,
	470, 999, 999, 987,
	999, 999, 999, 999,
	999, 999, 36, 832,
	999, 999, 999, 999,
	999, 433, 999, 583,
	583, 583, 583, 583,
	583, 583, 583, 583,
	583, 583, 583, 999,
	999, 986, 999, 360,
	999, 999, 999, 999,
	386, 992, 392, 999,
	996, 753, 984, 981,
	999, 999, 999, 808,
	804, 990, 999, 999,
	22, 67, 85, 100,
	999, 998, 379, 999,
	999, 999, 479, 479,
	999, 995, 999, 999,
	999, 999, 173, 999,
	215, 224, 286, 999,
	330, 342, 349, 360,
	999, 999, 999, 999,
	550, 999, 999, 579,
	999, 999, 999, 628,
	628, 628, 628, 628,
	628, 999, 999, 999,
	988, 686, 999, 999,
	999, 999, 999, 762,
	999, 782, 999, 983,
	980, 982, 999, 999,
	999, 999, 999, 999,
	999, 999, 989, 999,
	999, 324, 999
};

static constexpr char objectNameTbl[199][21] = {
	"Nothing.",
	"Box",
	"hand",
	"eye",
	"Ctrl",
	"crowbar",
	"journal",
	"scotch",
	"money",
	"newspaper",
	"library card",
	"bobby pin",
	"journal scrap",
	"clock key",
	"gloves",
	"mirror shard",
	"binoculars",
	"shovel",
	"business card",
	"stick",
	"axe handle",
	"head band",
	"trunk",
	"rope",
	"microfiche card",
	"loose rock",
	"car keys",
	"hammer",
	"gun",
	"grave journal scrap",
	"note from movers",
	"package",
	"package",
	"package",
	"blue print",
	"watch",
	"crackers",
	"sardines",
	"insecticide",
	"soysauce",
	"olives",
	"tin cup",
	"trunk lock",
	"book 'Alien'",
	"gas cap",
	"Fido",
	"librarian",
	"telephone",
	"bone bars",
	"stairs",
	"brass urn",
	"viewscreen",
	"shower",
	"sergeant",
	"guard",
	"evil plans",
	"evil sergeant",
	"starship lever",
	"Dreketh guards",
	"secret door",
	"turbo door",
	"post",
	"radio dial",
	"exit in car",
	"guard",
	"prisoner lock",
	"tomb left",
	"tomb up",
	"tomb right",
	"front door",
	"top of stairs",
	"car ignition",
	"police at door",
	"evil mike",
	"Gcell lock",
	"tombstone",
	"ladder down",
	"ladder",
	"secret door 2",
	"evil fido",
	"microfiche",
	"box behind desk",
	"button",
	"library floor",
	"book",
	"store",
	"glove compartment",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tombstone",
	"tomb door",
	"coat",
	"car trunk",
	"rope tied to house",
	"clock",
	"cupboard",
	"cupboard",
	"cupboard",
	"cupboard",
	"stove",
	"drawer",
	"kitchen tap",
	"bathroom tap",
	"medicine chest",
	"pillow",
	"Ancient",
	"power nexus",
	"abyss",
	"observatory lever",
	"horizon",
	"force field",
	"leech victims",
	"shrubs",
	"railing",
	"bed",
	"artwork",
	"carpet",
	"chair",
	"spittoon",
	"bed",
	"bed bottom",
	"fridge",
	"books",
	"globe",
	"mug shots",
	"map",
	"desk",
	"xxx",
	"mirror",
	"clerk",
	"ladder",
	"postman",
	"Delbert",
	"cement rock",
	"towels",
	"water heater",
	"bathtub",
	"xxx",
	"couch",
	"window",
	"art",
	"art",
	"clock",
	"counter",
	"books",
	"desk",
	"xxx",
	"desk",
	"books",
	"arch",
	"ground",
	"pillars",
	"caskets",
	"horizon",
	"urn",
	"urn",
	"urn",
	"urn",
	"urn",
	"urn",
	"xxx",
	"soccer ball",
	"skis",
	"trunk",
	"bottles",
	"barrels",
	"wheel",
	"car",
	"furniture",
	"xxx",
	"cocoons",
	"wall of skulls",
	"creature",
	"glass case",
	"power cables",
	"tubes",
	"brain nexus",
	"xxx",
	"control panel",
	"desk",
	"cell bars",
	"cell bars",
	"console",
	"viewer",
	"building",
	"box",
	"tools",
	"newspaper",
	"table",
	"bed bottom",
};




int Darkseed::Objects::getEyeDescriptionTosIdx(uint16 objNum) {
	if (objNum >= MAX_OBJECTS) {
		error("getEyeDescriptionTosIdx: Object Index out of range! %d", objNum);
	}
	return eyeDescriptionsTbl[objNum];
}

int Darkseed::Objects::getHandDescriptionTosIdx(uint16 objNum) {
	if (objNum >= MAX_OBJECTS) {
		error("getHandDescriptionTosIdx: Object Index out of range! %d", objNum);
	}
	return handDescriptionsTbl[objNum];
}

int16 Darkseed::Objects::getUseGlovesTosIdx(uint16 objNum) {
	if (objNum >= MAX_OBJECTS) {
		error("getUseGlovesTosIdx: Object Index out of range! %d", objNum);
	}
	return glovesTextTbl[objNum];
}

int16 Darkseed::Objects::getUseMoneyTosIdx(uint16 objNum) {
	if (objNum >= MAX_OBJECTS) {
		error("getUseMoneyTosIdx: Object Index out of range! %d", objNum);
	}
	return moneyTextTbl[objNum];
}

int16 Darkseed::Objects::getUseCrowbarTosIdx(uint16 objNum) {
	if (objNum >= MAX_OBJECTS) {
		error("getUseCrowbarTosIdx: Object Index out of range! %d", objNum);
	}
	return crowBarTextTbl[objNum];
}

int Darkseed::Objects::getMoveObjectRoom(uint16 idx) {
	if (idx >= MAX_OBJECTS) {
		error("getMoveObjectRoom: index out of range.");
	}
	return _moveObjectRoom[idx];
}

void Darkseed::Objects::setMoveObjectRoom(uint16 idx, uint8 value) {
	if (idx >= MAX_OBJECTS) {
		error("setMoveObjectRoom: index out of range.");
	}
	_moveObjectRoom[idx] = value;
}

int16 Darkseed::Objects::getObjectRunningCode(int idx) {
	if (idx >= MAX_OBJECTS) {
		error("getObjectRunningCode: index out of range.");
	}
	return _objectRunningCode[idx];
}

void Darkseed::Objects::setObjectRunningCode(int idx, int16 value) {
	if (idx >= MAX_OBJECTS) {
		error("setObjectRunningCode: index out of range.");
	}
	_objectRunningCode[idx] = value;
}

const char *Darkseed::Objects::getObjectName(int idx) {
	if (idx < 0 || idx >= MAX_OBJECTS) {
		error("getObjectName: index out of range.");
	}

	return objectNameTbl[idx];
}

static inline void syncPoint(Common::Serializer &s, Common::Point &value) {
	s.syncAsSint16LE(value.x);
	s.syncAsSint16LE(value.y);
}

Common::Error Darkseed::Objects::sync(Common::Serializer &s) {
	s.syncArray(_objectVar.data(), _objectVar.size(), Common::Serializer::Sint16LE);
	s.syncArray(_objectRunningCode.data(), _objectRunningCode.size(), Common::Serializer::Sint16LE);
	s.syncArray(_objectRunningCode.data(), _objectRunningCode.size(), Common::Serializer::Sint16LE);
	s.syncArray(_moveObjectXY.data(), _moveObjectXY.size(), syncPoint);
	s.syncArray(_moveObjectRoom.data(), _moveObjectRoom.size(), Common::Serializer::Byte);
	return Common::kNoError;
}
