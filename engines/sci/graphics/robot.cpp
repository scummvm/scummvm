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
#include "sci/graphics/palette.h"
#include "sci/sound/audio.h"

#include "graphics/surface.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "common/file.h"
#include "common/system.h"
#include "common/memstream.h"

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
GfxRobot::GfxRobot(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceId(resourceId) {
	assert(resourceId != -1);
	initData(resourceId);
}

GfxRobot::~GfxRobot() {
	delete[] _resourceData;
	delete[] _imageStart;
	delete[] _audioStart;
	delete[] _audioLen;
}

void GfxRobot::initData(GuiResourceId resourceId) {
	char fileName[10];
	uint32 fileSize;

	sprintf(fileName, "%d.rbt", resourceId);

	Common::File robotFile;
	if (robotFile.open(fileName)) {
		_resourceData = new byte[robotFile.size()];
		robotFile.read(_resourceData, robotFile.size());
		fileSize = robotFile.size();
		robotFile.close();
	} else {
		warning("Unable to open robot file %s", fileName);
		return;
	}

	_frameCount = READ_LE_UINT16(_resourceData + 14);

	// There is another value in the header, at offset 0x12, which
	// equals this value plus 14 (audio header size) in the cases
	// I've seen so far. Dunno which one to use, really.
	_audioSize = READ_LE_UINT16(_resourceData + 60);

	//_frameSize = READ_LE_UINT32(_resourceData + 34);
	_hasSound = (_resourceData[25] != 0);

	_palOffset = 60;
	if (_hasSound)
		_palOffset += READ_LE_UINT32(_resourceData + 60) + 14;

	getFrameOffsets();
	assert(_imageStart[_frameCount] == fileSize);

	debug("Robot %d, %d frames, sound: %s\n", resourceId, _frameCount, _hasSound ? "yes" : "no");
}

void GfxRobot::draw(int x, int y) {

  	return; // TODO: Remove once done
	// Play the audio of the robot file (for debugging)
#if 0
	if (_hasSound) {
		Audio::SoundHandle _audioHandle;
		Audio::AudioStream *audioStream = g_sci->_audio->getRobotAudioStream(_resourceData);
		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_audioHandle, audioStream);
	}
#endif

	byte *paletteData = _hasSound ? 
		_resourceData + 60 + 14 + _audioSize : 
		_resourceData + 60;
	uint16 paletteSize = READ_LE_UINT16(_resourceData + 16);

	Palette resourcePal;

	byte robotPal[256 * 4];
	byte savePal[256 * 4];
	int startIndex = READ_LE_UINT16(paletteData + 25);
	int colorCount = READ_LE_UINT16(paletteData + 29);

	warning("%d palette entries starting at %d", colorCount, startIndex);
	
	_palette->createFromData(paletteData, paletteSize, &resourcePal);

	for (int i = 0; i < 256; ++i) {
		savePal[i * 4 + 0] = _palette->_sysPalette.colors[i].r;
		savePal[i * 4 + 1] = _palette->_sysPalette.colors[i].g;
		savePal[i * 4 + 2] = _palette->_sysPalette.colors[i].b;
		savePal[i * 4 + 3] = 0;
	}
	
	memcpy(robotPal, savePal, sizeof(savePal));

	for (int i = 0; i < colorCount; ++i) {
		int index = i + startIndex;
		robotPal[index * 4 + 0] = resourcePal.colors[index].r;
		robotPal[index * 4 + 1] = resourcePal.colors[index].g;
		robotPal[index * 4 + 2] = resourcePal.colors[index].b;
		robotPal[index * 4 + 3] = 0;
	}

	g_system->setPalette(robotPal, 0, 256);

	for (int i = 0; i < _frameCount; ++i) {
		int width, height;

		byte *pixels = assembleVideoFrame(i);
		getFrameDimensions(i, width, height);
		g_system->copyRectToScreen(pixels, width, x, y, width, (int) (height * getFrameScaleFactor(i)));
		g_system->updateScreen();
		g_system->delayMillis(100);
		delete[] pixels;
	}

	g_system->setPalette(savePal, 0, 256);
}

void GfxRobot::getFrameOffsets() {
	int *audioEnd = new int[_frameCount];
	int *videoEnd = new int[_frameCount];

	for (int i = 0; i < _frameCount; ++i) {
		videoEnd[i] = READ_LE_UINT16(_resourceData + _palOffset + 1200 + i * 2);
		audioEnd[i] = READ_LE_UINT16(_resourceData + _palOffset + 1200 + _frameCount * 2 + i * 2);
	}

	int frameDataOffset = _palOffset + 0x4b0 + 0x400 + 0x200 + _frameCount * 4;
	
	// Pad to nearest 2 kilobytes
	if (frameDataOffset & 0x7ff)
		frameDataOffset = (frameDataOffset & ~0x7ff)+0x800;

	_imageStart = new uint32[_frameCount+1];
	_audioStart = new uint32[_frameCount];
	_audioLen = new uint32[_frameCount];

	_imageStart[0] = frameDataOffset;

	// Plus one so we can assert on this in the calling routine
	// The last one should point to end-of-file, unless I'm misunderstanding something
	for (int i = 1; i < _frameCount + 1; ++i) 
		_imageStart[i] = _imageStart[i - 1] + audioEnd[i - 1];
	for (int i = 0; i < _frameCount; ++i)
		_audioStart[i] = _imageStart[i] + videoEnd[i - 1];
	for (int i = 0; i < _frameCount; ++i)
		_audioLen[i] = _imageStart[i + 1] - _audioStart[i];

	delete[] audioEnd;
	delete[] videoEnd;
}

byte *GfxRobot::assembleVideoFrame(int frame) {
	byte *videoData = _resourceData + _imageStart[frame];
	int frameWidth = READ_LE_UINT16(videoData + 4);
	int frameHeight = READ_LE_UINT16(videoData + 6);
	int frameFragments = READ_LE_UINT16(videoData + 18);

	int decompressedSize = 0;

	DecompressorLZS lzs;

	videoData = _resourceData + _imageStart[frame] + 24;
	
	for (int i = 0; i < frameFragments; ++i) {
		int fragmentCompressed = READ_LE_UINT32(videoData);
		int fragmentUncompressed = READ_LE_UINT32(videoData + 4);

		decompressedSize += fragmentUncompressed;
		videoData += 10 + fragmentCompressed;
	}

	assert(decompressedSize == (frameWidth * frameHeight) * getFrameScaleFactor(frame));

	byte *output = new byte[decompressedSize];
	int assemblePtr = 0;

	videoData = _resourceData + _imageStart[frame] + 24;

	for (int i = 0; i < frameFragments; ++i) {
		int fragmentCompressed = READ_LE_UINT32(videoData);
		int fragmentUncompressed = READ_LE_UINT32(videoData + 4);
		uint16 compressionType = READ_LE_UINT16(videoData + 8);

		switch (compressionType) {
		case 0: {
			Common::MemoryReadStream compressedFrame(videoData + 10, fragmentCompressed, DisposeAfterUse::NO);

			lzs.unpack(&compressedFrame, output + assemblePtr, fragmentCompressed, fragmentUncompressed);
			assemblePtr += fragmentUncompressed;
			break;
		}
		case 2: // Untested
			memcpy(output + assemblePtr, videoData + 10, fragmentCompressed);
			assemblePtr += fragmentUncompressed;
			break;
		}
		
		videoData += 10 + fragmentCompressed;
	}

	assert(assemblePtr == decompressedSize);

	return output;
}

void GfxRobot::getFrameDimensions(int frame, int &width, int &height) {
	byte *videoData = _resourceData + _imageStart[frame];

	width = READ_LE_UINT16(videoData + 4);
	height = READ_LE_UINT16(videoData + 6);
}

void GfxRobot::getFrameRect(int frame, Common::Rect &rect) {
	byte *videoData = _resourceData + _imageStart[frame];

	int x = READ_LE_UINT16(videoData + 8);
	int y = READ_LE_UINT16(videoData + 10);
	int w = READ_LE_UINT16(videoData + 12);
	int h = READ_LE_UINT16(videoData + 14);

	rect = Common::Rect(x, y, x + w, y + h); 
}

float GfxRobot::getFrameScaleFactor(int frame) {
	byte *videoData = _resourceData + _imageStart[frame];
	byte percentage = videoData[3];

	return (float) percentage / 100.0;
}
	
#endif

} // End of namespace Sci
