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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/user_interface.h"

namespace MADS {

void SceneNode::load(Common::SeekableReadStream *f) {
	_walkPos.x = f->readSint16LE();
	_walkPos.y = f->readSint16LE();
	for (int i = 0; i < MAX_ROUTE_NODES; ++i)
		_indexes[i] = f->readUint16LE();
}

/*------------------------------------------------------------------------*/

UserInterface::UserInterface(MADSEngine *vm) : _vm(vm) {
	_category = CAT_NONE;
	_screenObjectsCount = 0;
	_inventoryTopIndex = 0;
	_objectY = 0;

	byte *pData = _vm->_screen.getBasePtr(0, MADS_SCREEN_HEIGHT - MADS_INTERFACE_HEIGHT);
	setPixels(pData, MADS_SCREEN_WIDTH, MADS_INTERFACE_HEIGHT);
}

void UserInterface::load(const Common::String &resName) {
	File f(resName);
	MadsPack madsPack(&f);

	// Load in the palette
	Common::SeekableReadStream *palStream = madsPack.getItemStream(0);

	uint32 *gamePalP = &_vm->_palette->_palFlags[0];
	byte *palP = &_vm->_palette->_mainPalette[0];

	for (int i = 0; i < 16; ++i, gamePalP++, palP += 3) {
		palP[0] = VGA_COLOR_TRANS(palStream->readByte());
		palP[1] = VGA_COLOR_TRANS(palStream->readByte());
		palP[2] = VGA_COLOR_TRANS(palStream->readByte());
		*gamePalP |= 1;
		palStream->skip(3);
	}
	delete palStream;

	// set the size for the interface
	Common::SeekableReadStream *pixelsStream = madsPack.getItemStream(1);
	pixelsStream->read(getData(), MADS_SCREEN_WIDTH * MADS_INTERFACE_HEIGHT);
	delete pixelsStream;
}

void UserInterface::setup(int id) {
	Scene &scene = _vm->_game->_scene;

	if (scene._screenObjects._v832EC != id) {
		Common::String resName = _vm->_game->_aaName;

		// Strip off any extension
		const char *p = strchr(resName.c_str(), '.');
		if (p) {
			resName = Common::String(resName.c_str(), p);
		}

		// Add on suffix if necessary
		if (id)
			resName += "A";

		resName += ".INT";
		load(resName);
	}
	scene._screenObjects._v832EC = id;

	scene._imageInterEntries.clear();
	scene._imageInterEntries.add(-2, 0xff);
	_vm->_game->_ticksExpiry = _vm->_events->getFrameCounter();
	scene._v1A = true;
	_vm->_game->_objectHiliteVocabIdx = -1;
	scene._v1C = -1;

	// Make a copy of the surface
	copyTo(&_surface);

	if (_vm->_game->_v1 == 5)
		scene._imageInterEntries.call(0, 0);

	scene._action.clear();
	writeText();
	loadElements();
	scene._dynamicHotspots.refresh();
}

void UserInterface::elementHighlighted() {
	warning("TODO: UserInterface::elementHighlighted");
}

void UserInterface::writeText() {
	warning("TODO: UserInterface::writeText");
}


void UserInterface::setBounds(const Common::Rect &r) {
	_bounds = r;
}

void UserInterface::loadElements() {
	Scene &scene = _vm->_game->_scene;
	Common::Rect bounds;
	scene._screenObjects.clear();

	if (!scene._screenObjects._v832EC) {
		// Set up screen objects for the inventory scroller
		for (int idx = 1; idx <= 3; ++idx) {
			getBounds(CAT_INV_SCROLLER, idx, bounds);
			moveRect(bounds);

			scene._screenObjects.add(bounds, LAYER_GUI, CAT_INV_SCROLLER, idx);
		}

		// Set up actions
		for (int idx = 0; idx < 10; ++idx) {
			getBounds(CAT_ACTION, idx, bounds);
			moveRect(bounds);

			scene._screenObjects.add(bounds, LAYER_GUI, CAT_ACTION, idx);
		}

		// Set up inventory list
		for (int idx = 0; idx < 5; ++idx) {
			getBounds(CAT_INV_LIST, idx, bounds);
			moveRect(bounds);

			scene._screenObjects.add(bounds, LAYER_GUI, CAT_INV_LIST, idx);
		}

		// Set up the inventory vocab list
		for (int idx = 0; idx < 5; ++idx) {
			getBounds(CAT_INV_VOCAB, idx, bounds);
			moveRect(bounds);

			scene._screenObjects.add(bounds, LAYER_GUI, CAT_INV_VOCAB, idx);
		}

		// Set up the inventory item picture
		scene._screenObjects.add(Common::Rect(160, 159, 231, 194), LAYER_GUI,
			CAT_INV_ANIM, 0);
	}

	if (!scene._screenObjects._v832EC || scene._screenObjects._v832EC == 2) {
		for (int hotspotIdx = scene._hotspots.size() - 1; hotspotIdx >= 0; --hotspotIdx) {
			Hotspot &hs = scene._hotspots[hotspotIdx];
			scene._screenObjects.add(hs._bounds, LAYER_GUI, CAT_HOTSPOT, hotspotIdx);
		}
	}

	if (scene._screenObjects._v832EC == 1) {
		for (int idx = 0; idx < 5; ++idx) {
			getBounds(CAT_6, idx, bounds);
			moveRect(bounds);

			scene._screenObjects.add(bounds, LAYER_GUI, CAT_6, idx);
		}
	}
}

bool UserInterface::getBounds(ScrCategory category, int v, Common::Rect &bounds) {
	Common::Rect result;
	int heightMultiplier, widthMultiplier;
	int leftStart, yOffset, widthAmt;

	switch (category) {
	case CAT_ACTION:
		heightMultiplier = v / 5;
		widthMultiplier = v / 5;
		leftStart = 2;
		yOffset = 3;
		widthAmt = 32;
		break;

	case CAT_INV_LIST:
		if (v < _inventoryTopIndex || v > (_inventoryTopIndex + 5))
			return false;

		heightMultiplier = v - _inventoryTopIndex;
		widthMultiplier = 0;
		leftStart = 90;
		yOffset = 3;
		widthAmt = 69;
		break;

	case CAT_6:
		heightMultiplier = v;
		widthMultiplier = 0;
		leftStart = 2;
		yOffset = 3;
		widthAmt = 310;
		break;

	case CAT_INV_SCROLLER:
		heightMultiplier = 0;
		widthMultiplier = 73;
		yOffset = 0;
		widthAmt = 9;
		leftStart = (v != 73) ? 73 : 75;
		break;

	default:
		heightMultiplier = v;
		widthMultiplier = 0;
		leftStart = 240;
		yOffset = 3;
		widthAmt = 80;
		break;
	}

	result.left = (widthMultiplier > 0) ? widthMultiplier * widthAmt + leftStart : leftStart;
	result.setWidth(widthAmt);
	result.top = heightMultiplier * 3 + yOffset;
	result.setHeight(8);

	if (category == CAT_INV_SCROLLER) {
		switch (v) {
		case 1:
			// Arrow up
			result.top = 4;
			result.setHeight(7);
			break;
		case 2:
			// Arrow down
			result.top = 35;
			result.setHeight(7);
			break;
		case 3:
			// Scroller
			result.top = 12;
			result.setHeight(22);
			break;
		case 4:
			// Thumb
			result.top = _objectY + 14;
			result.setHeight(1);
			break;
		default:
			break;
		}
	}

	return true;
}

void UserInterface::moveRect(Common::Rect &bounds) {
	bounds.translate(0, MADS_SCREEN_HEIGHT - MADS_INTERFACE_HEIGHT);
}

} // End of namespace MADS
