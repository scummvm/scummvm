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
 *
 */


#include "bladerunner/text_resource.h"

#include "bladerunner/bladerunner.h"

#include "common/debug.h"
#include "common/util.h"

namespace BladeRunner {

TextResource::TextResource(BladeRunnerEngine *vm) : _vm(vm) {
	_count   = 0;
	_ids     = nullptr;
	_offsets = nullptr;
	_strings = nullptr;
}

TextResource::~TextResource() {
	delete[] _ids;
	delete[] _offsets;
	delete[] _strings;
}

bool TextResource::open(const char *name) {
	assert(strlen(name) <= 8);

	char resName[13];
	sprintf(resName, "%s.TRE", name);
	Common::ScopedPtr<Common::SeekableReadStream> s(_vm->getResourceStream(resName));
	if (!s)
		return false;

	_count = s->readUint32LE();

	_ids = new uint32[_count];
	_offsets = new uint32[_count + 1];

	for (uint32 i = 0; i != _count; ++i) {
		_ids[i] = s->readUint32LE();
	}

	for (uint32 i = 0; i != _count + 1; ++i) {
		_offsets[i] = s->readUint32LE();
	}

	uint32 stringsStart = s->pos() - 4;

	for (uint32 i = 0; i != _count + 1; ++i) {
		_offsets[i] -= stringsStart;
	}

	uint32 remain = s->size() - s->pos();
	_strings = new char[remain];

	assert(remain >= _offsets[_count]);

	s->read(_strings, remain);

#if 0
	debug("\n%s\n----------------", resName);
	for (uint32 i = 0; i != (uint32)_count; ++i) {
		debug("%3d: %s", i, getText(i));
	}
#endif

	return true;
}

const char *TextResource::getText(uint32 id) {
	for (uint32 i = 0; i != _count; ++i) {
		if (_ids[i] == id) {
			return _strings + _offsets[i];
		}
	}

	return "";
}

} // End of namespace BladeRunner
