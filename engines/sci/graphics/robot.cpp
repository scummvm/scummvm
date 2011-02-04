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
#include "common/stream.h"
#include "common/substream.h"

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
	: _resMan(resMan), _screen(screen), _palette(palette), _outputBuffer(0),
	_outputBufferSize(0), _audioStream(0) {
	_resourceId = -1;
	_x = _y = 0;
}

GfxRobot::~GfxRobot() {
	freeData();
}

void GfxRobot::init(GuiResourceId resourceId, uint16 x, uint16 y) {
	_resourceId = resourceId;
	//_resourceId = 1305;	// debug
	_x = x;
	_y = y;
	_curFrame = 0;

	char fileName[10];
	sprintf(fileName, "%d.rbt", _resourceId);

	if (!_robotFile.open(fileName)) {
		warning("Unable to open robot file %s", fileName);
		return;
	}

	readHeaderChunk();

	// There are several versions of robot files, ranging from 3 to 6.
	// v3: no known examples
	// v4: PQ:SWAT
	// v5: SCI2.1 and SCI3 games
	// v6: SCI3 games
	switch (_header.version) {
	case 4:	// used in PQ:SWAT
		// Unsupported
		warning("TODO: add support for v4 robot videos");
		_curFrame = _header.frameCount;	// jump to the last frame
		return;
	case 5:	// used in most SCI2.1 games and in some SCI3 robots
	case 6:	// used in SCI3 games
		// Supported
		break;
	default:
		// Unsupported, error out so that we find out where this is used
		error("Unknown robot version: %d", _header.version);
	}

	_frameTotalSize = new uint32[_header.frameCount];
#if 0
	if (_header.hasSound) {
		_audioStream = Audio::makeQueuingAudioStream(22050, true);
		g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_audioHandle, _audioStream);
	}
#endif

	readPaletteChunk();
	readFrameSizesChunk();

	debug("Robot %d, %d frames, sound: %s\n", resourceId, _header.frameCount, _header.hasSound ? "yes" : "no");
}

void GfxRobot::readHeaderChunk() {
	// Header (60 bytes)
	_robotFile.skip(6);
	_header.version = _robotFile.readUint16LE();
	_robotFile.skip(2);
	_header.audioSilenceSize = _robotFile.readUint16LE();
	_robotFile.skip(2);
	_header.frameCount = _robotFile.readUint16LE();
	_header.paletteDataSize = _robotFile.readUint16LE();
	_robotFile.skip(7);
	_header.hasSound = _robotFile.readByte();
	_robotFile.skip(34);

	// Some robot files have sound, which doesn't start from frame 0
	// (e.g. Phantasmagoria, robot 1305). In this case, there won't
	// be audio silence in the header, but there will be an extra audio
	// preload chunk before the palette chunk. Skip past it and its 
	// 14-byte header.
	if (_header.hasSound && !_header.audioSilenceSize) {
		// The header is 14 bytes: the chunk size + 10 more
		uint32 preloadChunkSize = _robotFile.readUint32LE();
		_robotFile.skip(preloadChunkSize + 10);
	}
}

void GfxRobot::readPaletteChunk() {
	byte *paletteChunk = new byte[_header.paletteDataSize];
	_robotFile.read(paletteChunk, _header.paletteDataSize);
	int startIndex = READ_LE_UINT16(paletteChunk + 25);
	int colorCount = READ_LE_UINT16(paletteChunk + 29);
	Palette resourcePal;
	_palette->createFromData(paletteChunk, _header.paletteDataSize, &resourcePal);
	delete[] paletteChunk;

	byte robotPal[256 * 4];

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


void GfxRobot::readFrameSizesChunk() {
	switch (_header.version) {
	case 5:		// sizes are 16-bit integers
		// Skip table with frame image sizes, as we don't need it
		_robotFile.skip(_header.frameCount * 2);
		for (int i = 0; i < _header.frameCount; ++i)
			_frameTotalSize[i] = _robotFile.readUint16LE();
		break;
	case 6:		// sizes are 32-bit integers
		// Skip table with frame image sizes, as we don't need it
		_robotFile.skip(_header.frameCount * 4);
		for (int i = 0; i < _header.frameCount; ++i)
			_frameTotalSize[i] = _robotFile.readUint32LE();
		break;
	default:
		error("Can't yet handle index table for robot version %d", _header.version);
	}

	_robotFile.skip(1024 + 512);	// Skip unknown tables 1 and 2

	// Pad to nearest 2 kilobytes
	uint32 curPos = _robotFile.pos();
	if (curPos & 0x7ff) {
		curPos = (curPos & ~0x7ff) + 2048;
		_robotFile.seek(curPos);
	}
}

void GfxRobot::processNextFrame() {
	// Make sure that we haven't reached the end of the video already
	if (_curFrame == _header.frameCount)
		return;

	// Read frame header (24 bytes)
	_robotFile.skip(3);
	byte frameScale = _robotFile.readByte();
	uint16 frameWidth = _robotFile.readUint16LE();
	uint16 frameHeight = _robotFile.readUint16LE();
	_robotFile.skip(8);		// x, y, width and height of the frame
	uint16 compressedSize = _robotFile.readUint16LE();
	uint16 frameFragments = _robotFile.readUint16LE();
	_robotFile.skip(4);		// unknown

	uint32 decompressedSize = frameWidth * (frameHeight * frameScale / 100);

	// Reallocate the output buffer, if its size has increased
	if (decompressedSize > _outputBufferSize) {
		delete[] _outputBuffer;
		_outputBuffer = new byte[decompressedSize];
	}
	
	_outputBufferSize = decompressedSize;

	DecompressorLZS lzs;
	byte *outPtr = _outputBuffer;

	for (uint16 i = 0; i < frameFragments; ++i) {
		uint32 compressedFragmentSize = _robotFile.readUint32LE();
		uint32 decompressedFragmentSize = _robotFile.readUint32LE();
		uint16 compressionType = _robotFile.readUint16LE();

		if (compressionType == 0) {
			Common::SeekableSubReadStream fragmentStream(&_robotFile, _robotFile.pos(), _robotFile.pos() + compressedFragmentSize);
			lzs.unpack(&fragmentStream, outPtr, compressedFragmentSize, decompressedFragmentSize);
		} else if (compressionType == 2) {	// untested
			_robotFile.read(outPtr, compressedFragmentSize);
		} else {
			error("Unknown frame compression found: %d", compressionType);
		}

		outPtr += decompressedFragmentSize;
	}

	uint32 audioChunkSize = _frameTotalSize[_curFrame] - (24 + compressedSize);

	// TODO: Audio
#if 0
	// Queue the next audio frame
	if (_header.hasSound) {
		uint16 decodedSize = _robotFile.readUint16LE();
		_robotFile.skip(2);	// skip audio buffer position

		byte *audioFrame = g_sci->_audio->getDecodedRobotAudioFrame(&_robotFile, audioChunkSize - 4);
		_audioStream->queueBuffer(audioFrame, decodedSize, DisposeAfterUse::NO, Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO);
	} else {
		_robotFile.skip(audioChunkSize);
	}
#else
	_robotFile.skip(audioChunkSize);
#endif

	// Show frame
	g_system->copyRectToScreen(_outputBuffer, frameWidth, _x, _y, frameWidth, frameHeight * frameScale / 100);
	g_system->updateScreen();
	g_system->delayMillis(100);

	_curFrame++;

	if (_curFrame == _header.frameCount) {
		// End of robot video, restore palette
		g_system->setPalette(_savedPal, 0, 256);
		_resourceId = -1;
		freeData();
	}
}

void GfxRobot::freeData() {
#if 0
	if (_header.hasSound) {
		g_system->getMixer()->stopHandle(_audioHandle);
		//delete _audioStream; _audioStream = 0;
	}
#endif
	delete[] _frameTotalSize; _frameTotalSize = 0;
	delete[] _outputBuffer; _outputBuffer = 0;
	_outputBufferSize = 0;
	_robotFile.close();
}	

#endif

} // End of namespace Sci
