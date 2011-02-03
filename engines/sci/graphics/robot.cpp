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

// fopen() and friends so we can dump frames and stuff to disk, for debugging
// #define FORBIDDEN_SYMBOL_ALLOW_ALL 

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
GfxRobot::GfxRobot(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceData(0),
	  _imageStart(0), _audioStart(0), _audioLen(0), _outputBuffer(0), _outputBufferSize(0) {
	_resourceId = -1;
	_x = _y = 0;
}

GfxRobot::~GfxRobot() {
	delete[] _resourceData;
	delete[] _imageStart;
	delete[] _audioStart;
	delete[] _audioLen;
	delete[] _outputBuffer;
}

void GfxRobot::init(GuiResourceId resourceId, uint16 x, uint16 y) {
	char fileName[10];
	uint32 fileSize;

	//	resourceId = 1305;	// debug

	_resourceId = resourceId;
	_x = x;
	_y = y;
	_curFrame = 0;
	sprintf(fileName, "%d.rbt", _resourceId);

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

	// There are several versions of robot files, ranging from 3 to 6. Version
	// 5 is the version of the majority of the robot files. PQ:SWAT is the only
	// game that uses robots of a different version (version 4), but there are
	// no known examples of other robot versions.
	_version = _resourceData[6];

	// Currently, we only support robot version 5. Robot version 
	_frameCount = READ_LE_UINT16(_resourceData + 14);

	// There is another value in the header, at offset 0x12, which
	// equals this value plus 14 (audio header size) in the cases
	// I've seen so far. Dunno which one to use, really.
	_audioSize = READ_LE_UINT16(_resourceData + 60);

	//_frameSize = READ_LE_UINT32(_resourceData + 34);
	_hasSound = (_resourceData[25] != 0);

	_palOffset = 60;

	// Some robot files have sound, which doesn't start from frame 0
	// (e.g. Phantasmagoria, robot 1305)
	// Yes, strangely, the word at offset 10 is non-zero if it DOESN'T have a preload
	// in that case, the word is equal to that value which would otherwise be stored
	// in the audio header (i.e. _audioSize above)
	if (_hasSound && READ_LE_UINT16(_resourceData + 10) == 0)
		_palOffset += READ_LE_UINT32(_resourceData + 60) + 14;

	switch (_version) {
	case 4:
		// TODO: Add support for this version (used in PQ:SWAT)
		warning("TODO: add support for v4 robot videos");
		_curFrame = _frameCount;	// jump to the last frame
		return;
	case 5:
		// Supported, the most well-known and used version
		break;
	default:
		// Unsupported, error out so that we find out where this is used
		error("Unknown robot version: %d", _version);
	}

	getFrameOffsets();
	assert(_imageStart[_frameCount] == fileSize);

	setPalette();

	debug("Robot %d, %d frames, sound: %s\n", resourceId, _frameCount, _hasSound ? "yes" : "no");
}

void GfxRobot::getFrameOffsets() {
	int *audioEnd = new int[_frameCount];
	int *videoEnd = new int[_frameCount];

	for (int i = 0; i < _frameCount; ++i) {
		videoEnd[i] = READ_LE_UINT16(_resourceData + _palOffset + 1200 + i * 2);
		audioEnd[i] = READ_LE_UINT16(_resourceData + _palOffset + 1200 + _frameCount * 2 + i * 2);
	}

	uint32 frameDataOffset = _palOffset + 0x4b0 + 0x400 + 0x200 + _frameCount * 4;
	
	// Pad to nearest 2 kilobytes
	if (frameDataOffset & 0x7ff)
		frameDataOffset = (frameDataOffset & ~0x7ff) + 0x800;

	_imageStart = new uint32[_frameCount + 1];
	_audioStart = new uint32[_frameCount];
	_audioLen = new uint32[_frameCount];

	_imageStart[0] = frameDataOffset;

	// Plus one so we can assert on this in the calling routine
	// The last one should point to end-of-file, unless I'm misunderstanding something
	for (int i = 1; i < _frameCount + 1; ++i) 
		_imageStart[i] = _imageStart[i - 1] + audioEnd[i - 1];
	for (int i = 0; i < _frameCount; ++i)
		_audioStart[i] = _imageStart[i] + videoEnd[i];
	for (int i = 0; i < _frameCount; ++i)
		_audioLen[i] = _imageStart[i + 1] - _audioStart[i];

	delete[] audioEnd;
	delete[] videoEnd;
}

void GfxRobot::setPalette() {
	byte *paletteData = _resourceData + _palOffset;
	uint16 paletteSize = READ_LE_UINT16(_resourceData + 16);

	Palette resourcePal;

	byte robotPal[256 * 4];
	int startIndex = READ_LE_UINT16(paletteData + 25);
	int colorCount = READ_LE_UINT16(paletteData + 29);

	warning("%d palette entries starting at %d", colorCount, startIndex);
	
	_palette->createFromData(paletteData, paletteSize, &resourcePal);

	for (int i = 0; i < 256; ++i) {
		_savedPal[i * 4 + 0] = _palette->_sysPalette.colors[i].r;
		_savedPal[i * 4 + 1] = _palette->_sysPalette.colors[i].g;
		_savedPal[i * 4 + 2] = _palette->_sysPalette.colors[i].b;
		_savedPal[i * 4 + 3] = 0;
	}
	
	memcpy(robotPal, _savedPal, sizeof(_savedPal));

	for (int i = 0; i < colorCount; ++i) {
		int index = i + startIndex;
		robotPal[index * 4 + 0] = resourcePal.colors[index].r;
		robotPal[index * 4 + 1] = resourcePal.colors[index].g;
		robotPal[index * 4 + 2] = resourcePal.colors[index].b;
		robotPal[index * 4 + 3] = 0;
	}

	g_system->setPalette(robotPal, 0, 256);
}

void GfxRobot::drawNextFrame() {
	uint16 width, height;

	// Make sure that we haven't reached the end of the video already
	if (_curFrame == _frameCount)
		return;

	assembleVideoFrame(_curFrame);
	getFrameDimensions(_curFrame, width, height);
	
	g_system->copyRectToScreen(_outputBuffer, width, _x, _y, width, height * getFrameScale(_curFrame) / 100);
	g_system->updateScreen();
	g_system->delayMillis(100);

	_curFrame++;

	if (_curFrame == _frameCount) {
		// End of robot video, restore palette
		g_system->setPalette(_savedPal, 0, 256);
		_resourceId = -1;
		delete[] _outputBuffer;
		_outputBuffer = 0;
	}
}

void GfxRobot::assembleVideoFrame(uint16 frame) {
	byte *videoData = _resourceData + _imageStart[frame];
	uint16 frameWidth = READ_LE_UINT16(videoData + 4);
	uint16 frameHeight = READ_LE_UINT16(videoData + 6);
	uint16 frameFragments = READ_LE_UINT16(videoData + 18);

	uint32 decompressedSize = 0;

	DecompressorLZS lzs;

	videoData = _resourceData + _imageStart[frame] + 24;
	
	for (int i = 0; i < frameFragments; ++i) {
		uint32 fragmentCompressed = READ_LE_UINT32(videoData);
		uint32 fragmentUncompressed = READ_LE_UINT32(videoData + 4);

		decompressedSize += fragmentUncompressed;
		videoData += 10 + fragmentCompressed;
	}

	assert(decompressedSize == (uint32)(frameWidth * frameHeight) * getFrameScale(frame) / 100);
	
	// Reallocate the output buffer, if its size has changed
	if (decompressedSize != _outputBufferSize) {
		delete[] _outputBuffer;
		_outputBuffer = new byte[decompressedSize];
	}

	_outputBufferSize = decompressedSize;

	uint32 assemblePtr = 0;

	videoData = _resourceData + _imageStart[frame] + 24;

	for (int i = 0; i < frameFragments; ++i) {
		uint32 fragmentCompressed = READ_LE_UINT32(videoData);
		uint32 fragmentUncompressed = READ_LE_UINT32(videoData + 4);
		uint16 compressionType = READ_LE_UINT16(videoData + 8);

		switch (compressionType) {
		case 0: {
			Common::MemoryReadStream compressedFrame(videoData + 10, fragmentCompressed, DisposeAfterUse::NO);

			lzs.unpack(&compressedFrame, _outputBuffer + assemblePtr, fragmentCompressed, fragmentUncompressed);
			assemblePtr += fragmentUncompressed;
			break;
		}
		case 2: // Untested
			memcpy(_outputBuffer + assemblePtr, videoData + 10, fragmentCompressed);
			assemblePtr += fragmentUncompressed;
			break;
		}
		
		videoData += 10 + fragmentCompressed;
	}

	assert(assemblePtr == decompressedSize);

	//	FILE *f = fopen("/tmp/flap", "w");
	//      fwrite(_outputBuffer, 1, decompressedSize, f);
	//      fclose(f);
	//      exit(1);
}

void GfxRobot::getFrameDimensions(uint16 frame, uint16 &width, uint16 &height) {
	byte *videoData = _resourceData + _imageStart[frame];

	width = READ_LE_UINT16(videoData + 4);
	height = READ_LE_UINT16(videoData + 6);
}

#if 0
Common::Rect GfxRobot::getFrameRect(uint16 frame) {
	byte *videoData = _resourceData + _imageStart[frame];

	uint16 x = READ_LE_UINT16(videoData + 8);
	uint16 y = READ_LE_UINT16(videoData + 10);
	uint16 w = READ_LE_UINT16(videoData + 12);
	uint16 h = READ_LE_UINT16(videoData + 14);

	return Common::Rect(x, y, x + w, y + h); 
}
#endif

byte GfxRobot::getFrameScale(uint16 frame) {
	byte *videoData = _resourceData + _imageStart[frame];
	return videoData[3];
}

void GfxRobot::playAudio() {
	if (_hasSound) {
		Audio::SoundHandle _audioHandle;
		Audio::AudioStream *audioStream = g_sci->_audio->getRobotAudioStream(_resourceData);
		g_system->getMixer()->playStream(Audio::Mixer::kSpeechSoundType, &_audioHandle, audioStream);
	}
}

void GfxRobot::freeData() {
	delete[] _resourceData; _resourceData = 0;
	delete[] _imageStart; _imageStart = 0;
	delete[] _audioStart; _audioStart = 0;
	delete[] _audioLen; _audioLen = 0;
}	

#endif

} // End of namespace Sci
