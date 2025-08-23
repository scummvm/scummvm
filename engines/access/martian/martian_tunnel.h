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

#ifndef ACCESS_MARTIAN_MARTIAN_TUNNEL_H
#define ACCESS_MARTIAN_MARTIAN_TUNNEL_H

#include "common/scummsys.h"

namespace Access {

namespace Martian {

class MartianEngine;

class MartianTunnel {
public:
	MartianTunnel(MartianEngine *vm);
	~MartianTunnel();

	void tunnel2();
	void tunnel4();

private:
	void doTunnel();
	void drawArrowSprites();
	void drawArrowSprites2();
	void clearWorkScreenArea();
	void copyBufBlockToScreen();
	void tunnel_doloop_8c65();
	void tunnel_17f5c();
	void tunnel_1888a();
	void tunnel_18985();

	MartianEngine *_vm;
	int16 _tunnelParam_ca42;
	int16 _tunnelParam_ca44;
	int16 _tunnelParam_ca46;
	uint16 _tunnel_ca20;
	byte _tunnelMoveFlag;
	int16 _crawlFrame;
	byte _tunnelStopLoop_ca27;
};

}

} // end namespace Access

#endif // ACCESS_MARTIAN_MARTIAN_TUNNEL_H
