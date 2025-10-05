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


#ifndef GAMOS_MOVIE_H
#define GAMOS_MOVIE_H

#include "common/file.h"

namespace Gamos
{

class GamosEngine;

class MoviePlayer {
    public:

    bool playMovie(Common::File *file, uint32 offset, GamosEngine *gamos);


    private:

    bool init(Common::File *file, uint32 offset, GamosEngine *gamos);
	bool deinit();
	bool error();

	int processControlChunk();
	int processImageChunk();
	int processPaletteChunk();
	int processSoundChunk();
	int proccessMidiChunk();

	bool readHdr();
	bool readCompressed(int32_t count, Common::Array<byte> *buf);

	uint8 processMessages(bool keepAct, uint32 *msecs);

	static byte* blit0(Common::Rect rect, byte *in, Graphics::Surface *surface);
	static byte* blit1(Common::Rect rect, byte *in, Graphics::Surface *surface);
	static byte* blit2(Common::Rect rect, byte *in, Graphics::Surface *surface);
	static byte* blit3(Common::Rect rect, byte *in, Graphics::Surface *surface);



    private:

    GamosEngine *_gamos = nullptr;
    Graphics::Screen *_screen = nullptr;
    SystemProc *_messageProc = nullptr;


    bool _doUpdateScreen = false;
	uint32 _skippedFrames = 0;
	uint32 _currentFrame = 0;
	uint32 _firstFrameTime = 0;

	bool _forceStopMidi = false;

	int _loopCount = 1;
    int _loopPoint = 0;

	Common::Point _pos; /* Movie frame leftup corner */
	Common::Point _frameSize; /* Sizes of movie frame */

    int _midiBufferSize = 0;
    int _soundBufferSize = 0;
    int _paletteBufferSize = 0;
    int _bufferSize = 0;
    int _packedBufferSize = 0;
    int _frameTime = 0;

    Common::Array<byte> _midiBuffer;
    Common::Array<byte> _soundBuffer;
    Common::Array<byte> _paletteBuffer;
    Common::Array<byte> _buffer;
    Common::Array<byte> _packedBuffer;

    bool _midiStarted = false;
    bool _soundPlaying = false;

	Common::File *_file = nullptr;

	byte _hdrBytes[4];
	int32_t _hdrValue1 = 0;
	int32_t _hdrValue2 = 0;
};

}

#endif //GAMOS_MOVIE_H
