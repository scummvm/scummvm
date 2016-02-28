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

#ifndef ADL_ADL_V1_H
#define ADL_ADL_V1_H

#include "adl/adl.h"

namespace Common {
class ReadStream;
}

namespace Adl {

enum {
	IDI_HR1_MSG_
};

class HiRes1Engine : public AdlEngine {
public:
	HiRes1Engine(OSystem *syst, const AdlGameDescription *gd);

protected:
	void runGame();

private:
	enum {
		MH_ROOMS = 42,
		MH_PICS = 98,
		MH_ITEM_OFFSETS = 21
	};

	void printMessage(uint idx, bool wait = true);
	uint getEngineMessage(EngineMessage msg);

	void runIntro();
	void drawPic(Common::ReadStream &stream, byte xOffset, byte yOffset);
	void showRoom();
	void drawItems();
	void drawPic(byte pic, byte xOffset, byte yOffset);
};

} // End of namespace Adl

#endif
