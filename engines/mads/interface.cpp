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
#include "mads/interface.h"

namespace MADS {

void SceneNode::load(Common::SeekableReadStream *f) {
	_walkPos.x = f->readSint16LE();
	_walkPos.y = f->readSint16LE();
	for (int i = 0; i < MAX_ROUTE_NODES; ++i)
		_indexes[i] = f->readUint16LE();
}

/*------------------------------------------------------------------------*/

InterfaceSurface::InterfaceSurface(MADSEngine *vm) : _vm(vm) {
	_category = CAT_NONE;
	_screenObjectsCount = 0;
}


void InterfaceSurface::load(const Common::String &resName) {
	File f(resName);
	MadsPack madsPack(&f);

	// Load in the palette
	Common::SeekableReadStream *palStream = madsPack.getItemStream(0);

	uint32 *gamePalP = &_vm->_palette->_palFlags[0];
	byte *palP = &_vm->_palette->_mainPalette[0];

	for (int i = 0; i < 16; ++i, gamePalP++, palP += 3) {
		palP[0] = palStream->readByte();
		palP[1] = palStream->readByte();
		palP[2] = palStream->readByte();
		*gamePalP |= 1;
		palStream->skip(3);
	}
	delete palStream;

	// set the size for the interface
	setSize(MADS_SCREEN_WIDTH, MADS_INTERFACE_HEIGHT);
	Common::SeekableReadStream *pixelsStream = madsPack.getItemStream(1);
	pixelsStream->read(getData(), MADS_SCREEN_WIDTH * MADS_INTERFACE_HEIGHT);
	delete pixelsStream;
}

void InterfaceSurface::setup(int id) {
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
		free();
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

void InterfaceSurface::elementHighlighted() {
	warning("TODO: InterfaceSurface::elementHighlighted");
}

void InterfaceSurface::writeText() {
	warning("TODO: InterfaceSurface::writeText");
}


void InterfaceSurface::setBounds(const Common::Rect &r) {
	_bounds = r;
}

void InterfaceSurface::loadElements() {
	warning("TODO: InterfaceSurface::loadElements");
}

/*------------------------------------------------------------------------*/


} // End of namespace MADS
