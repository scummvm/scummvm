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
 * $URL$
 * $Id$
 *
 */

#include "sci/gfx/palette.h"
#include "sci/gfx/gfx_system.h"

// #define DEBUG_MERGE

namespace Sci {

Palette::Palette(uint s) {
	_size = s;
	_colors = new PaletteEntry[s];
	_parent = 0;
	_dirty = true;
	_refcount = 1;
	_revision = 0;
}

Palette::Palette(const PaletteEntry *colors, uint s) {
	_size = s;
	_colors = new PaletteEntry[s];
	_parent = 0;
	_dirty = true;
	_refcount = 1;
	_revision = 0;

	for (uint i = 0; i < _size; ++i) {
		_colors[i].r = colors[i].r;
		_colors[i].g = colors[i].g;
		_colors[i].b = colors[i].b;
		_colors[i].refcount = 0;
		_colors[i].parent_index = -1;
	}
}

Palette::~Palette() {
	if (_parent)
		unmerge();

	delete[] _colors;
	_colors = 0;
}

Palette *Palette::getref() {
	_refcount++;
	return this;
}

void Palette::free() {
	assert(_refcount > 0);
	_refcount--;

	if (_refcount == 0)
		delete this;
}

void Palette::resize(uint s) {
	if (s == _size)
		return;

	assert(!_parent);
	assert(_refcount == 1);
	assert(s >= _size);

	PaletteEntry *n = new PaletteEntry[s];
	for (uint i = 0; i < _size; ++i)
		n[i] = _colors[i];

	delete[] _colors;
	_colors = n;
	_size = s;
}

void Palette::unmerge() {
	assert(_parent);
#ifdef DEBUG_MERGE
	fprintf(stderr, "Unmerge %s from %s (%d colors)\n", name.c_str(), _parent->name.c_str(), _size);
#endif
	if (_parent->_revision != _revision) {
#ifdef DEBUG_MERGE
		fprintf(stderr, "NOP (revision mismatch)\n");
#endif
		return;
	}

	int count = 0;
	for (uint i = 0; i < _size; ++i) {
		if (_colors[i].refcount == PALENTRY_FREE)
			continue;

		int pi = _colors[i].parent_index;
		assert(pi >= 0);
		assert(pi < (int)_parent->_size);
		assert(_parent->_colors[pi].refcount != 0);
		assert(_parent->_colors[pi].refcount != PALENTRY_FREE);
		if (_parent->_colors[pi].refcount != PALENTRY_LOCKED)
			_parent->_colors[pi].refcount--;
		if (_parent->_colors[pi].refcount == 0) {
			_parent->_colors[pi].refcount = PALENTRY_FREE;
			count++;
		}
		_colors[i].parent_index = -1;
	}
#ifdef DEBUG_MERGE
	fprintf(stderr, "Unmerge free %d colors\n", count);
#endif

	_parent = 0;
}

void Palette::setColor(uint index, byte r, byte g, byte b, int parentIndex) {
	assert(index < _size);
	assert(!_parent);

//	FIXME: We may want to have this assert. This will require changing the
//	way loops sharing a single view's palette works.
//	assert(_refcount == 1);

	PaletteEntry& entry = _colors[index];

	assert(entry.refcount == PALENTRY_FREE || entry.refcount == 0);
	entry.refcount = 0;
	entry.r = r;
	entry.g = g;
	entry.b = b;
	entry.parent_index = parentIndex;

	_dirty = true;
}

void Palette::makeSystemColor(uint index, const PaletteEntry &color) {
	assert(index < _size);
	PaletteEntry& entry = _colors[index];
	entry.r = color.r;
	entry.g = color.g;
	entry.b = color.b;
	entry.refcount = PALENTRY_LOCKED;
}

uint Palette::findNearbyColor(byte r, byte g, byte b, bool lock) {
	int bestdelta = 1 + ((0x100 * 0x100) * 3);
	int bestcolor = -1;
	int firstfree = -1;

	assert(_size != 0);

	for (uint i = 0; i < _size; ++i) {
		PaletteEntry& entry = _colors[i];

		if (entry.refcount != PALENTRY_FREE) {
			int dr = abs(entry.r - r);
			int dg = abs(entry.g - g);
			int db = abs(entry.b - b);

			if (dr == 0 && dg == 0 && db == 0) {
				// Exact match
				//exact = true;
				if (lock && entry.refcount != PALENTRY_LOCKED)
					entry.refcount++;
				return i;
			}

			int delta = (dr * dr) + (dg * dg) + (db * db);
			if (delta < bestdelta) {
				bestdelta = delta;
				bestcolor = i;
			}
		} else {
			if (firstfree == -1)
				firstfree = i;
		}
	}

	if (firstfree != -1) {
		// TODO: mark palette as dirty
		setColor(firstfree, r, g, b);
		//exact = true;
		if (lock)
			_colors[firstfree].refcount++;
		return firstfree;
	}

	//exact = false;
	if (lock && _colors[bestcolor].refcount != PALENTRY_LOCKED) {
#if 0
			_colors[bestcolor].r = r;
			_colors[bestcolor].g = g;
			_colors[bestcolor].b = b;
			_dirty = true;
#endif
			_colors[bestcolor].refcount++;
	}
	return bestcolor;
}

bool Palette::mergeInto(Palette *parent) {
	assert(!_parent || _parent == parent);
	assert(parent != this);
#ifdef DEBUG_MERGE
	fprintf(stderr, "Merge: %s into %s (%d colors)\n", name.c_str(), parent->name.c_str(), _size);
#endif

	if (_parent == parent && parent->_revision == _revision) {
#ifdef DEBUG_MERGE
		fprintf(stderr, "NOP\n");
#endif
		return false;
	}
	_parent = parent;
	_revision = parent->_revision;

#ifdef DEBUG_MERGE
	bool *used = new bool[_parent->size()];
	for (uint i = 0; i < _parent->size(); ++i)
		used[i] = false;
	int count = 0;
	uint used_min = 1000;
	uint used_max = 0;
#endif

	for (uint i = 0; i < _size; ++i) {
		PaletteEntry& entry = _colors[i];
		if (entry.refcount == PALENTRY_FREE)
			continue;

		uint pi = _parent->findNearbyColor(entry.r, entry.g, entry.b);
#ifdef DEBUG_MERGE
		if (!used[pi]) count++;
		used[pi] = true;
		if (pi > used_max) used_max = pi;
		if (pi < used_min) used_min = pi;
#endif
		entry.parent_index = pi;
		if (_parent->_colors[pi].refcount != PALENTRY_LOCKED)
			_parent->_colors[pi].refcount++;
	}
#ifdef DEBUG_MERGE
	fprintf(stderr, "Merge used %d colours in [%d..%d]\n", count, used_min, used_max);
	delete[] used;
#endif
	return true;
}

void Palette::forceInto(Palette *parent) {
	assert(!_parent || _parent == parent);
	assert(parent != this);
#ifdef DEBUG_MERGE
	fprintf(stderr, "Merge: force %s into %s (%d colors)\n", name.c_str(), parent->name.c_str(), _size);
#endif

	_parent = parent;
	parent->_revision++;
	_revision = parent->_revision;

	for (unsigned int i = 0; i < _size; ++i) {
		// FIXME: PALENTRY_LOCKED doesn't work well with forceInto...
		if (_colors[i].refcount != PALENTRY_FREE) {
			_parent->_colors[i] = _colors[i];
			_parent->_colors[i].parent_index = -1;
			_colors[i].parent_index = i;
			if (_parent->_colors[i].refcount != PALENTRY_LOCKED)
				_parent->_colors[i].refcount = 1;
		} else {
			_parent->_colors[i].refcount = 0;
		}
	}
	_parent->_dirty = true;
}

Palette *Palette::copy() {
	assert(!_parent);
	Palette* p = new Palette(_size);
	p->name = "copy of " + name;

	for (uint i = 0; i < _size; ++i) {
		p->_colors[i] = _colors[i];
		if (p->_colors[i].refcount >= 0)
			p->_colors[i].refcount = 0;
	}
	return p;
}


} // End of namespace Sci
