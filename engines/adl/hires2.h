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

#ifndef ADL_HIRES1_H
#define ADL_HIRES1_H

#include "common/str.h"

#include "adl/adl.h"

namespace Common {
class ReadStream;
class Point;
}

namespace Adl {

#define IDS_HR2_DISK_IMAGE "WIZARD.DSK"

// Track, sector, offset
#define TSO(T, S, O) (((T) * 16 + (S)) * 256 + (O))

#define IDI_HR2_OFS_INTRO_TEXT TSO(0x00, 0xd, 0x17)

class HiRes2Engine : public AdlEngine {
public:
	HiRes2Engine(OSystem *syst, const AdlGameDescription *gd) : AdlEngine(syst, gd) { }

private:
	// AdlEngine
	void runIntro() const;
	void loadData();
	void initState();
	void restartGame();
	void drawPic(byte pic, Common::Point pos) const;
};

} // End of namespace Adl

#endif
