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

#ifndef AWE_RESOURCE_3DO_H
#define AWE_RESOURCE_3DO_H

#include "awe/intern.h"
#include "awe/file.h"

namespace Awe {

struct OperaIso;

struct Resource3do {
	char _musicPath[32] = { '\0' };
	char _cpakPath[64] = { '\0' };
	OperaIso *_iso = nullptr;

	Resource3do();
	~Resource3do();

	bool readEntries();

	uint8_t *loadFile(int num, uint8_t *dst, uint32_t *size);
	uint16_t *loadShape555(const char *name, int *w, int *h);
	const char *getMusicName(int num, uint32_t *offset);
	const char *getCpak(const char *name, uint32_t *offset);
};

} // namespace Awe

#endif
