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

#include "common/archive.h"
#include "common/system.h"
#include "common/stream.h"
#include "common/substream.h"

namespace Sci {

// TODO:
// - v4 robot support (used in PQ:SWAT)
// - Positioning
// - Proper handling of frame scaling - scaled frames look squashed
//   (probably because both dimensions should be scaled)
// - Timing - the arbitrary 100ms delay between each frame is not quite right
// - Proper handling of sound chunks in some cases, so that the frame size
//   table can be ignored (it's only used to determine the correct sound chunk
//   size at the moment, cause it can be wrong in some cases)
// - Fix audio "hiccups" - probably data that shouldn't be in the audio frames


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
	_outputBufferSize(0), _audioStream(0), _frameTotalSize(0), _robotFile(0) {
	_x = _y = 0;
}

GfxRobot::~GfxRobot() {
	freeData();
}

void GfxRobot::init(GuiResourceId resourceId, uint16 x, uint16 y) {
	_x = x;
	_y = y;
	_curFrame = 0;

	Common::String fileName = Common::String::format("%d.rbt", resourceId);
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);

	if (!stream) {
		warning("Unable to open robot file %s", fileName.c_str());
		return;
	}

	_robotFile = new Common::SeekableSubReadStreamEndian(stream, 0, stream->size(),
			g_sci->getPlatform() == Common::kPlatformMacintosh, DisposeAfterUse::YES);

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
		freeData();
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

	if (_header.hasSound) {
		_audioStream = Audio::makeQueuingAudioStream(11025, false);
		g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_audioHandle, _audioStream);
	}

	readPaletteChunk();
	readFrameSizesChunk();

	debug("Robot %d, %d frames, sound: %s\n", resourceId, _header.frameCount, _header.hasSound ? "yes" : "no");
}

void GfxRobot::readHeaderChunk() {
	// Header (60 bytes)
	_robotFile->skip(6);
	_header.version = _robotFile->readUint16();
	_header.audioChunkSize = _robotFile->readUint16();
	_header.audioSilenceSize = _robotFile->readUint16();
	_robotFile->skip(2);
	_header.frameCount = _robotFile->readUint16();
	_header.paletteDataSize = _robotFile->readUint16();
	_header.unkChunkDataSize = _robotFile->readUint16();
	_robotFile->skip(5);
	_header.hasSound = _robotFile->readByte();
	_robotFile->skip(34);

	// Some videos (e.g. robot 1305 in Phantasmagoria and
	// robot 184 in Lighthouse) have an unknown chunk before
	// the palette chunk (probably used for sound preloading).
	// Skip it here.
	if (_header.unkChunkDataSize)
		_robotFile->skip(_header.unkChunkDataSize);
}

void GfxRobot::readPaletteChunk() {
	byte *paletteChunk = new byte[_header.paletteDataSize];
	_robotFile->read(paletteChunk, _header.paletteDataSize);

	int startIndex = paletteChunk[25];
	int colorCount = READ_SCI11ENDIAN_UINT16(paletteChunk + 29);

	if (colorCount > 256)
		error("Invalid color count: %d", colorCount);

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
	// The robot video file contains 2 tables, with one entry for each frame:
	// - A table containing the size of the image in each video frame
	// - A table containing the total size of each video frame.
	// In v5 robots, the tables contain 16-bit integers, whereas in v6 robots,
	// they contain 32-bit integers.

	// TODO: The table reading code can probably be removed once the
	// audio chunk size is figured out (check the TODO inside processNextFrame())
#if 0
	// We don't need any of the two tables to play the video, so we ignore
	// both of them.
	uint16 wordSize = _header.version == 6 ? 4 : 2;
	_robotFile->skip(_header.frameCount * wordSize * 2);
#else
	switch (_header.version) {
	case 5:		// sizes are 16-bit integers
		// Skip table with frame image sizes, as we don't need it
		_robotFile->skip(_header.frameCount * 2);
		for (int i = 0; i < _header.frameCount; ++i)
			_frameTotalSize[i] = _robotFile->readUint16();
		break;
	case 6:		// sizes are 32-bit integers
		// Skip table with frame image sizes, as we don't need it
		_robotFile->skip(_header.frameCount * 4);
		for (int i = 0; i < _header.frameCount; ++i)
			_frameTotalSize[i] = _robotFile->readUint32();
		break;
	default:
		error("Can't yet handle index table for robot version %d", _header.version);
	}
#endif

	// 2 more unknown tables
	_robotFile->skip(1024 + 512);

	// Pad to nearest 2 kilobytes
	uint32 curPos = _robotFile->pos();
	if (curPos & 0x7ff)
		_robotFile->seek((curPos & ~0x7ff) + 2048);
}

void GfxRobot::processNextFrame() {
	// Make sure that we haven't reached the end of the video already
	if (_curFrame == _header.frameCount)
		return;

	// Read frame image header (24 bytes)
	_robotFile->skip(3);
	byte frameScale = _robotFile->readByte();
	uint16 frameWidth = _robotFile->readUint16();
	uint16 frameHeight = _robotFile->readUint16();
	_robotFile->skip(8); // x, y, width and height of the frame
	uint16 compressedSize = _robotFile->readUint16();
	uint16 frameFragments = _robotFile->readUint16();
	_robotFile->skip(4); // unknown

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
		uint32 compressedFragmentSize = _robotFile->readUint32();
		uint32 decompressedFragmentSize = _robotFile->readUint32();
		uint16 compressionType = _robotFile->readUint16();

		if (compressionType == 0) {
			Common::SeekableSubReadStream fragmentStream(_robotFile, _robotFile->pos(), _robotFile->pos() + compressedFragmentSize);
			lzs.unpack(&fragmentStream, outPtr, compressedFragmentSize, decompressedFragmentSize);
		} else if (compressionType == 2) {	// untested
			_robotFile->read(outPtr, compressedFragmentSize);
		} else {
			error("Unknown frame compression found: %d", compressionType);
		}

		outPtr += decompressedFragmentSize;
	}

	uint32 audioChunkSize = _frameTotalSize[_curFrame] - (24 + compressedSize);

// TODO: The audio chunk size below is usually correct, but there are some
// exceptions (e.g. robot 4902 in Phantasmagoria, towards its end)
#if 0
	// Read frame audio header (14 bytes)
	_robotFile->skip(2); // buffer position
	_robotFile->skip(2); // unknown (usually 1)
	_robotFile->skip(2); /*uint16 audioChunkSize = _robotFile->readUint16() + 8;*/
	_robotFile->skip(2);
#endif

	// Queue the next audio frame
	// FIXME: For some reason, there are audio hiccups/gaps
	if (_header.hasSound) {
		_robotFile->skip(8);	// header
		_audioStream->queueBuffer(g_sci->_audio->getDecodedRobotAudioFrame(_robotFile, audioChunkSize - 8), 
									(audioChunkSize - 8) * 2, DisposeAfterUse::NO, 
									Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN);
	} else {
		_robotFile->skip(audioChunkSize);
	}

	// Show frame
	g_system->copyRectToScreen(_outputBuffer, frameWidth, _x, _y, frameWidth, frameHeight * frameScale / 100);
	g_system->updateScreen();
	g_system->delayMillis(100);	// TODO: This isn't quite right

	_curFrame++;

	if (_curFrame == _header.frameCount) {
		// End of robot video, restore palette
		g_system->setPalette(_savedPal, 0, 256);
		freeData();
	}
}

void GfxRobot::freeData() {
	if (_header.hasSound) {
		g_system->getMixer()->stopHandle(_audioHandle);
		//delete _audioStream; _audioStream = 0;
	}
	delete[] _frameTotalSize; _frameTotalSize = 0;
	delete[] _outputBuffer; _outputBuffer = 0;
	_outputBufferSize = 0;
	delete _robotFile; _robotFile = 0;
}	

#endif

} // End of namespace Sci
