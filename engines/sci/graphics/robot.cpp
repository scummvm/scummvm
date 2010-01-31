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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/robot.h"

namespace Sci {

#ifdef ENABLE_SCI32
Robot::Robot(ResourceManager *resMan, GfxScreen *screen, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

Robot::~Robot() {
	_resMan->unlockResource(_resource);
}

void Robot::initData(GuiResourceId resourceId) {
	_resource = _resMan->findResource(ResourceId(kResourceTypeRobot, resourceId), true);
	if (!_resource) {
		error("robot resource %d not found", resourceId);
	}
	_resourceData = _resource->data;

// sample data:
// DWORD:Sample Size - 2 needs to be subtracted (??!!)

// 90.rbt (640x390, 22050, 1 16, ADPCM) 67 frames
// 00000000: 16 00 53 4f 4c 00 05 00-ad 08 00 00 f0 00 43 00  ..SOL.........C.
//                                                     ^ frames
// 00000010: b0 04 00 a0 00 00 00 00-01 01 00 00 0a 00 01 00  ................
// 00000020: 03 00 01 00 00 cf 03 00-00 00 00 00 00 00 00 00  ................
//                       ^ pixel count
// 00000030: 00 00 00 00 00 00 00 00-00 00 00 00 f2 9f 00 00  ................
//                                               ^ data begin (sample)
// 00000040: 00 00 d2 4d 00 00 20 52-00 00 a5 11 04 02 85 90  ...M.. R........

// 91.rbt (320x240, 22050, 1 16, ADPCM) 90 frames
// 00000000: 16 00 53 4f 4c 00 05 00-ad 08 00 00 f0 00 5a 00  ..SOL.........Z.
//                                                     ^ frames
// 00000010: b0 04 00 a0 00 00 00 00-01 01 00 00 0a 00 01 00  ................
// 00000020: 03 00 01 00 00 2c 01 00-00 00 00 00 00 00 00 00  .....,..........
//                       ^ pixel count
// 00000030: 00 00 00 00 00 00 00 00-00 00 00 00 f2 9f 00 00  ................ offset 60
//                                               ^ data begin (sample)
// 00000040: 00 00 d2 4d 00 00 20 52-00 00 82 01 00 01 00 01  ...M.. R........
// ...
// 0000a030: 8d 8d 8f 8e 8f 90 90 91-92 92 92 94 0e 00 00 00  ................ offset 41004
//                                               ^ palette start
// 0000a040: 00 00 00 00 00 00 01 00-00 09 01 00 00 00 00 00  ................
// 0000a050: 00 00 00 00 00 37 00 00-00 51 00 01 01 00 00 00  .....7...Q......
//                          ^ color start^ color count
// 0000a060: 00 58 6b 2b 4b 69 28 50-5b 24 68 50 20 5b 53 21  .Xk+Ki(P[$hP [S!
//              ^ start pal data
// [...]
// 0000a110: 24 05 41 14 04 18 25 10-64 00 00 2d 18 05 58 00  $.A...%.d..-..X.
// 0000a120: 00 16 20 07 50 00 00 20-19 01 2d 0e 00 48 00 00  .. .P.. ..-..H..
// 0000a130: 40 00 00 10 18 05 38 00-00 30 00 00 28 00 00 0b  @.....8..0..(...
// 0000a140: 0e 00 20 00 00 18 00 00-00 08 00 10 00 00 08 00  .. .............
// 0000a150: 00 00 00 00 70 70 70 70-70 70 70 70 70 70 70 70  ....pppppppppppp
// [...]
// 0000a4e0: 70 70 70 70 70 70 70 70-70 70 70 70 34 0a 75 0a  pppppppppppp4.u.
// 0000a4f0: 4a 0b c5 0b f4 0b 54 0c-bd 0c 7a 0d 91 0e 1f 10  J.....T...z.....
// 0000a500: 16 12 72 14 19 17 ef 19-9a 1c b3 1e 79 20 c1 22  ..r.........y ."
// 0000a510: 33 22 33 23 e0 25 84 26-eb 26 1a 2d 43 2d af 2d  3"3#.%.&.&.-C-.-
// [...]
// 0000aff0: 20 20 20 20 20 20 20 20-20 20 20 20 20 20 20 20                  
// 0000b000: 01 00 7f 64 40 01 f0 00-00 00 00 00 00 00 00 00  ...d@...........
//                       ^width^height
// 0000b010: 1c 0a 02 00 7f 7f 7f 7f-04 08 00 00 00 f0 00 00  ................
// 0000b020: 00 00 43 e0 7f ff ff ff-ff ff ff ff ff ff ff ff  ..C.............
// 0000b030: ff ff ff ff ff ff ff ff-ff ff ff ff ff ff ff ff  ................

// 161.rbt (112x155, 22050, 1 16, ADPCM) 29 frames
// 00000000: 16 00 53 4f 4c 00 05 00-ad 08 00 00 96 00 1d 00  ..SOL...........
//                                                     ^ frames
// 00000010: b0 04 00 a0 00 00 00 00-01 01 00 00 0a 00 01 00  ................
// 00000020: 03 00 01 00 47 3e 00 00-00 00 00 00 00 00 00 00  ....G>..........
//                       ^ pixel count
// 00000030: 00 00 00 00 00 00 00 00-00 00 00 00 f2 9f 00 00  ................
//                                               ^ data begin (sample)
// 00000040: 00 00 d2 4d 00 00 20 52-00 00 00 00 00 00 00 00  ...M.. R........
// 00000050: 00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00  ................

// 213.rbt (125x248, nosound) 30 frames
// 00000000: 16 00 53 4f 4c 00 05 00-ad 08 00 00 96 00 1e 00  ..SOL...........
//                                                     ^ frames
// 00000010: b0 04 00 00 00 00 00 00-01 00 00 00 0a 00 01 00  ................
//                    ^ ?!              ^ no sound?!
// 00000020: 03 00 01 00 82 6e 00 00-00 00 00 00 00 00 00 00  .....n..........
//                       ^ pixel count
// 00000030: 00 00 00 00 00 00 00 00-00 00 00 00 0e 00 00 00  ................
//                                               ^ data begin (palette)
// 00000040: 00 00 00 00 00 00 01 00-00 ca 00 00 00 00 00 00  ................
// 00000050: 00 00 00 00 00 37 00 00-00 3c 00 01 01 00 00 00  .....7...<......
// 00000060: 00 d0 d0 c0 d0 c0 a8 c8-b8 c0 d0 b0 a0 c0 a8 88
//              ^ palette data start
// 00000070: c0 a0 a0 c8 98 90 d0 88-60 b0 90 80 b8 88 80 a0  ........`.......
// 00000080: 90 98 b0 88 90 c0 78 60-a0 80 80 a0 80 70 c8 70  ......x`.....p.p
// [...]
// 00000110: 00 00 00 00 08 70 70 70-70 70 70 70 70 70 70 70  .....ppppppppppp
//                          ^ ??
// 00000120: 70 70 70 70 70 70 70 70-70 70 70 70 70 70 70 70  pppppppppppppppp

	_frameCount = READ_LE_UINT16(_resourceData + 12);
	_frameSize = READ_LE_UINT32(_resourceData + 34);
}

// TODO: just trying around in here...

void Robot::draw() {
	byte *bitmapData = _resourceData + ROBOT_FILE_STARTOFDATA;
	int x, y;
	//int frame;

	return;

	//for (frame = 0; frame < 30; frame++) {
	for (y = 0; y < _height; y++) {
		for (x = 0; x < _width; x++) {
			_screen->putPixel(x, y, SCI_SCREEN_MASK_VISUAL, *bitmapData, 0, 0);
			bitmapData++;
		}
	}
	//}
	_screen->copyToScreen();
}
#endif

} // End of namespace Sci
