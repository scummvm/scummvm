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

#ifndef SCI_GRAPHICS_ROBOT_H
#define SCI_GRAPHICS_ROBOT_H

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/decoders/raw.h"

namespace Common {
	class SeekableSubReadStreamEndian;
}

namespace Sci {

#ifdef ENABLE_SCI32

struct RobotHeader {
	// 6 bytes, identifier bytes
	uint16 version;
	uint16 audioChunkSize;
	uint16 audioSilenceSize;
	// 2 bytes, unknown
	uint16 frameCount;
	uint16 paletteDataSize;
	uint16 unkChunkDataSize;
	// 5 bytes, unknown
	byte hasSound;
	// 34 bytes, unknown
};

class GfxRobot {
public:
	GfxRobot(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette);
	~GfxRobot();

	void init(GuiResourceId resourceId, uint16 x, uint16 y);
	void processNextFrame();
	uint16 getCurFrame() { return _curFrame; }
	uint16 getFrameCount() { return _header.frameCount; }
	bool isPlaying() { return _robotFile != 0; }
	void playAudio();

private:
	void readHeaderChunk();
	void readPaletteChunk();
	void readFrameSizesChunk();

	void freeData();

	ResourceManager *_resMan;
	GfxScreen *_screen;
	GfxPalette *_palette;

	byte _savedPal[256 * 4];

	Common::SeekableSubReadStreamEndian *_robotFile;
	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	RobotHeader _header;

	uint16 _x;
	uint16 _y;
	uint16 _curFrame;
	uint32 *_frameTotalSize;

	byte *_outputBuffer;
	uint32 _outputBufferSize;
};
#endif

} // End of namespace Sci

#endif
