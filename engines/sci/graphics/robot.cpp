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
#include "sci/sound/audio.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "common/file.h"
#include "common/system.h"

namespace Sci {

#ifdef ENABLE_SCI32
GfxRobot::GfxRobot(ResourceManager *resMan, GfxScreen *screen, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

GfxRobot::~GfxRobot() {
	delete[] _resourceData;
}

void GfxRobot::initData(GuiResourceId resourceId) {
	char fileName[10];
	sprintf(fileName, "%d.rbt", resourceId);

	Common::File robotFile;
	if (robotFile.open(fileName)) {
		_resourceData = new byte[robotFile.size()];
		robotFile.read(_resourceData, robotFile.size());
		robotFile.close();
	} else {
		warning("Unable to open robot file %s", fileName);
		return;
	}

	// The RBT video starts with a SOL audio file, followed by
	// video data which is appended after it

	_frameCount = READ_LE_UINT16(_resourceData + 14);
	_audioSize = READ_LE_UINT16(_resourceData + 15);

	//_frameSize = READ_LE_UINT32(_resourceData + 34);
	_hasSound = (_resourceData[25] != 0);

	debug("Robot %d, %d frames, sound: %s\n", resourceId, _frameCount, _hasSound ? "yes" : "no");
}

// TODO: just trying around in here...

void GfxRobot::draw() {
	byte *bitmapData = _resourceData + _audioSize;
	int x, y;
	int frame;

	// Play the audio of the robot file (for debugging)
#if 0
	if (_hasSound) {
		Audio::SoundHandle _audioHandle;
		Audio::AudioStream *audioStream = g_sci->_audio->getRobotAudioStream(_resourceData);
		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_audioHandle, audioStream);
	}
#endif

	return;

	// Each frame contains these bytes:
	// 01 00 7f 64 - always the same, perhaps resource type + extra
	// 40 01    -    total frame width (320 in this case)
	// f0 00    -    total frame height (240 in this case)
	// The total video size is calculated from the maximum width, height
	// of all the frames in the robot file
	// 4 zeroes
	// 4 bytes, perhaps frame x, y on screen?
	// 2 bytes, unknown
	// 2 bytes, a small number (e.g. 01 00 or 02 00)
	// 7f 7f    -    127x127
	// 7f 7f    -    127x127
	// 2 bytes, related to frame size?
	// 00 00
	// 00 f0
	// 4 zeroes
	// 43 e0
	// 7f ff

	// The frames themselves seem to contain a size of the actual drawn data
	// on screen. The frame data seems to be uncompressed, placed on screen
	// at appropriate x,y coordinates, and each frame can have a different size.
	// This is apparent from the fact that a 320x240 frame (e.g. in Phantasmagoria
	// demo, 91.rbt) has 4833, 4898, 5111, etc bytes, whereas a full frame would
	// be 320x240 = 76800 bytes. Thus, each frame is either somehow compressed
	// (but the data seems uncompressed?), or only the part that changes is drawn
	// on screen, something like the MPEG I-frames

	for (frame = 0; frame < _frameCount; frame++) {
		bitmapData += 4;	// skip header bytes
		_width = READ_LE_UINT16(bitmapData + 4); bitmapData += 2;
		_height = READ_LE_UINT16(bitmapData + 6); bitmapData += 2;

		for (y = 0; y < _width; y++) {
			for (x = 0; x < _height; x++) {
				_screen->putPixel(x, y, GFX_SCREEN_MASK_VISUAL, *bitmapData, 0, 0);
				bitmapData++;
			}
		}

		_screen->copyToScreen();
		// Sleep for a second
		g_sci->sleep(1000);
	}

}
#endif

} // End of namespace Sci
