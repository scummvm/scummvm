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

#ifndef ACCESS_MARTIAN_RESOURCES_H
#define ACCESS_MARTIAN_RESOURCES_H

#include "common/scummsys.h"
#include "access/resources.h"
#include "access/font.h"

namespace Access {

namespace Martian {

extern const int SIDEOFFR[];
extern const int SIDEOFFL[];
extern const int SIDEOFFU[];
extern const int SIDEOFFD[];

extern const int SIDEOFFR[];
extern const int SIDEOFFL[];
extern const int SIDEOFFU[];
extern const int SIDEOFFD[];

extern const byte CREDIT_DATA[];
extern const byte ICON_PALETTE[];

extern const int RMOUSE[10][2];

extern byte HELP[];
extern const char *const ASK_TBL[];
extern const char *const TRAVDATA[];

extern const char *const SPEC7MESSAGE;

extern const byte _byte1EEB5[];
extern const int PICTURERANGE[][2];

class MartianResources : public Resources {
protected:
	/**
	 * Load data from the access.dat file
	 */
	void load(Common::SeekableReadStream &s) override;
public:
	MartianFont *_font6x6;
	MartianFont *_font3x5;
public:
	MartianResources(AccessEngine *vm) : Resources(vm), _font6x6(nullptr), _font3x5(nullptr) {}
	~MartianResources() override;
};

#define MMRES (*((Martian::MartianResources *)_vm->_res))

} // End of namespace Martian
} // End of namespace Access

#endif /* ACCESS_MARTIAN_RESOURCES_H */
