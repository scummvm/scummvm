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

#ifndef AWE_RESOURCE_NTH_H
#define AWE_RESOURCE_NTH_H

#include "awe/intern.h"

namespace Awe {

struct ResourceNth {
	virtual ~ResourceNth() {
	}

	virtual bool init() = 0;
	virtual uint8 *load(const char *name) = 0;
	virtual uint8 *loadBmp(int num) = 0;
	virtual void preloadDat(int part, int type, int num) {}
	virtual uint8 *loadDat(int num, uint8 *dst, uint32 *size) = 0;
	virtual uint8 *loadWav(int num, uint8 *dst, uint32 *size) = 0;
	virtual const char *getString(Language lang, int num) = 0;
	virtual const char *getMusicName(int num) = 0;
	virtual void getBitmapSize(int *w, int *h) = 0;

	static ResourceNth *create(int edition);
};

} // namespace Awe

#endif
