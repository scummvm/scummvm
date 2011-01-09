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

#include "graphics/surface.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "common/file.h"
#include "common/system.h"

namespace Sci {

// Some non technical information on robot files, from an interview with
// Greg Tomko-Pavia of Sierra On-Line
// Taken from http://anthonylarme.tripod.com/phantas/phintgtp.html
//
// (...) What we needed was a way of playing video, but have it blend into
// normal room art instead of occupying its own rectangular area. Room art 
// consists of a background pic overlaid with various animating cels 
// (traditional lingo: sprites). The cels each have a priority that determines 
// who is on top and who is behind in the drawing order. Cels are read from 
// *.v56 files (another proprietary format). A Robot is video frames with 
// transparent background including priority and x,y information. Thus, it is 
// like a cel, except it comes from an RBT - not a v56. Because it blends into
// our graphics engine, it looks just like a part of the room. A RBT can move 
// around the screen and go behind other objects. (...)

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
	// For some weird reason, the audio size seems to ALWAYS
	// be 0xB000, padded with 0x20 and 0x00 at the end
	_audioSize = READ_LE_UINT16(_resourceData + 15);

	//_frameSize = READ_LE_UINT32(_resourceData + 34);
	_hasSound = (_resourceData[25] != 0);

	debug("Robot %d, %d frames, sound: %s\n", resourceId, _frameCount, _hasSound ? "yes" : "no");
}

void GfxRobot::draw() {
	// Play the audio of the robot file (for debugging)
#if 0
	if (_hasSound) {
		Audio::SoundHandle _audioHandle;
		Audio::AudioStream *audioStream = g_sci->_audio->getRobotAudioStream(_resourceData);
		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_audioHandle, audioStream);
	}
#endif

	return;	// TODO: Remove once done

	byte *frameData = _resourceData + _audioSize;
	uint16 frameWidth, frameHeight;
	uint16 maskBlockSize, frameBlockSize;

	// TODO: Palette! Where is it?
#if 0
	byte dummyPal[256 * 4];
	for (int i = 0; i < 256; i++) {
		dummyPal[i * 4 + 0] = i;
		dummyPal[i * 4 + 1] = i;
		dummyPal[i * 4 + 2] = i;
		dummyPal[i * 4 + 3] = 0;
	}
	memset(dummyPal, 255, 256 * 4);
	g_system->setPalette(dummyPal, 0, 256);
#endif

	// Both the frame and the mask data are uncompressed
	// TODO: How are the rectangle dimensions calculated from the block sizes?

	Graphics::Surface *screen;
	for (uint16 frame = 0; frame < _frameCount; frame++) {
		// Read 20 byte header
		// First 4 bytes are always 01 00 7f 64
		assert(*(frameData + 0) == 0x01);
		assert(*(frameData + 1) == 0x00);
		assert(*(frameData + 2) == 0x7f);
		assert(*(frameData + 3) == 0x64);
		frameWidth = READ_LE_UINT16(frameData + 4);
		frameHeight = READ_LE_UINT16(frameData + 6);
		// 4 bytes, always 0
		assert(*(frameData +  8) == 0x00);
		assert(*(frameData +  9) == 0x00);
		assert(*(frameData + 10) == 0x00);
		assert(*(frameData + 11) == 0x00);
		// 2 bytes: a small number (x, perhaps? But it doesn't match with the movie dimensions shown by SV...)
		// 2 bytes: a small number (y, perhaps? But it doesn't match with the movie dimensions shown by SV...)
		// 2 bytes: a small number, 1 or 2 (type, perhaps?)
		maskBlockSize = READ_LE_UINT16(frameData + 16);
		assert(*(frameData +  18) == 0x01 || *(frameData +  18) == 0x02);
		assert(*(frameData + 20) == 0x00);
		frameData += 20;

		// Process mask(?) data
		// FIXME: This isn't correct, we need to use copyRectToScreen() instead
		// (but we need to figure out the rectangle dimensions, see TODO above)
		screen = g_system->lockScreen();
		memcpy(frameData, screen->pixels, maskBlockSize);
		g_system->unlockScreen();
		g_system->updateScreen();
		// Sleep for a second
		g_sci->sleep(1000);
		warning("Mask %d", frame);

		frameData += maskBlockSize;

		// Read 12 byte frame data header
		frameBlockSize = READ_LE_UINT16(frameData + 8);
		frameData += 12;

		// Process frame(?) data
		// FIXME: This isn't correct, we need to use copyRectToScreen() instead
		// (but we need to figure out the rectangle dimensions, see TODO above)
		screen = g_system->lockScreen();
		memcpy(frameData, screen->pixels, frameBlockSize);
		g_system->unlockScreen();
		g_system->updateScreen();
		// Sleep for a second
		g_sci->sleep(1000);
		warning("Frame %d", frame);

		frameData += frameBlockSize;
	}

}
#endif

} // End of namespace Sci
