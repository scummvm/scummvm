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

#ifndef PELROCK_VIDEO_H
#define PELROCK_VIDEO_H

#include "graphics/surface.h"

#include "pelrock/events.h"
#include "pelrock/fonts/large_font.h"

namespace Pelrock {

struct ChunkHeader {
    uint32_t blockCount;      // +0x00: Number of 0x5000-byte blocks
    uint32_t dataOffset;      // +0x04: Varies by chunk type
    uint8_t  chunkType;       // +0x08: 1=RLE, 2=BlockCopy, 3=End, 4=Palette, 6=Special
    // +0x0D: Frame data begins
	byte *data;
};

struct Effect {
	uint16 startFrame;
};

struct AudioEffect : Effect {
};

struct Subtitle : Effect {
	uint16 startFrame;
	uint16 endFrame;
	uint16 x;
	uint16 y;
	Common::String text;
};

struct Voice : AudioEffect {
	char filename[12];
};

struct Sfx : AudioEffect {
	uint32 soundId;
};

struct ExtraSound : AudioEffect {
	char filename[12];
};

static const uint32 chunkSize = 0x5000;

class VideoManager {
public:
	VideoManager(
		Graphics::Screen *screen,
		PelrockEventManager *events,
		ChronoManager *chrono,
		LargeFont *largeFont,
		DialogManager *dialog
	);
	~VideoManager();
	void playIntro();

private:
	Graphics::Screen *_screen;
	PelrockEventManager *_events;
	ChronoManager *_chrono;
	LargeFont *_largeFont;
	DialogManager *_dialog;
	void loadPalette(ChunkHeader &chunk);
	byte *decodeCopyBlock(byte *data, uint32 offset);
    byte *decodeRLE(byte *data, size_t size, uint32 offset);
	void readChunk(Common::SeekableReadStream &stream, ChunkHeader &chunk);
	void processFrame(ChunkHeader &chunk, const int frameCount);
	void presentFrame();
	void initMetadata();
	Subtitle *getSubtitleForFrame(uint16 frameNumber);
	int _currentSubtitleIndex = 0;
	Graphics::Surface _videoSurface = Graphics::Surface();
	Graphics::ManagedSurface _textSurface = Graphics::ManagedSurface();
	Common::Array<ChunkHeader> _chunkBuffer;
	Common::Array<Subtitle> _subtitles;
	Common::Array<AudioEffect> _audioEffect;
};

} // End of namespace Pelrock
#endif
