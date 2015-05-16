/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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
 *
 */

#include "common/str.h"

#include "engines/grim/emi/layer.h"

#include "engines/grim/bitmap.h"

namespace Grim {

Layer::Layer() : _filename(""), _sortOrder(0), _frame(0), _bitmap(nullptr) {
}

Layer::Layer(const Common::String &filename, int sortOrder) : _filename(filename), _sortOrder(sortOrder), _frame(0) {
	_bitmap = Bitmap::create(filename);
}

Layer::~Layer() {
	delete _bitmap;
}

void Layer::draw() {
	_bitmap->drawLayer(_frame);
}

void Layer::setFrame(int frame) {
	int numframes = _bitmap->getNumLayers();
	if (frame >= numframes || frame < 0) {
		warning("Layer::setFrame: invalid frame number: %d, numLayers: %d", frame, numframes);
		return;
	}
	_frame = frame;
}

void Layer::setSortOrder(int order) {
	_sortOrder = order;
}

void Layer::advanceFrame(int num) {
	_frame += num;
	int numframes = _bitmap->getNumLayers();
	_frame %= numframes;
}

void Layer::saveState(SaveGame *state) {
	if (_bitmap) {
		state->writeBool(true);
		state->writeString(_bitmap->getFilename());
	} else {
		state->writeBool(false);
	}
	state->writeLESint32(_frame);
	state->writeLESint32(_sortOrder);
}

void Layer::restoreState(SaveGame *state) {
	bool hasBitmap = state->readBool();
	if (hasBitmap)
		_bitmap = Bitmap::create(state->readString());
	_frame = state->readLESint32();
	_sortOrder = state->readLESint32();
}
}
