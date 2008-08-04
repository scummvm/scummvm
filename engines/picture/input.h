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

#ifndef PICTURE_INPUT_H
#define PICTURE_INPUT_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/array.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Picture {

class Input {
public:
    Input(PictureEngine *vm);
    ~Input();

	void update();
	
	void enableMouse();
	void disableMouse();
	
	int16 getMouseDeltaStuff(int16 divisor);

//protected:
public:
    PictureEngine *_vm;

	int16 _mouseX, _mouseY;
	int16 _mousePosDelta;
	int16 _mouseCounter;
	bool _mouseButtonPressedFlag;
	byte _mouseButton;
	int16 _mouseDisabled;
	
	bool _leftButtonDown, _rightButtonDown;

};

} // End of namespace Picture

#endif /* PICTURE_INPUT_H */
