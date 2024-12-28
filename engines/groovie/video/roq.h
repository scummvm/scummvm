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

#ifndef GROOVIE_VIDEO_ROQ_H
#define GROOVIE_VIDEO_ROQ_H

#include "groovie/video/player.h"
#include "audio/mixer.h"

namespace Video {
class VideoDecoder;
}

namespace Groovie {

class GroovieEngine;

struct ROQBlockHeader {
	uint16 type;
	uint32 size;
	uint16 param;
};

class ROQPlayer : public VideoPlayer {
public:
	ROQPlayer(GroovieEngine *vm);
	~ROQPlayer();
	void setOrigin(int16 x, int16 y) override;

	Audio::SoundHandle getSoundHandle() {
		return _soundHandle;
	}

	void drawString(Graphics::Surface *surface, const Common::String text, int posx, int posy, uint32 color, bool blackBackground) override;
	void copyfgtobg(uint8 arg) override;

	bool isFileHandled() override { return _isFileHandled; }

protected:
	void waitFrame() override;
	uint16 loadInternal() override;
	bool playFrameInternal() override;
	void stopAudioStream() override;
	virtual void createAudioStream(bool stereo);

	Audio::SoundHandle _soundHandle;
	Graphics::Surface *_bg, *_screen, *_overBuf;
	Graphics::Surface *_currBuf, *_prevBuf;

private:
	bool readBlockHeader(ROQBlockHeader &blockHeader);

	bool processBlock();
	bool processBlockInfo(ROQBlockHeader &blockHeader);
	bool processBlockQuadCodebook(ROQBlockHeader &blockHeader);
	bool processBlockQuadVector(ROQBlockHeader &blockHeader);
	void processBlockQuadVectorBlock(int baseX, int baseY);
	void processBlockQuadVectorBlockSub(int baseX, int baseY);
	bool processBlockStill(ROQBlockHeader &blockHeader);
	bool processBlockSoundMono(ROQBlockHeader &blockHeader);
	bool processBlockSoundStereo(ROQBlockHeader &blockHeader);
	bool processBlockAudioContainer(ROQBlockHeader &blockHeader);
	bool playFirstFrame() { return _flagNoPlay; }; // _alpha && !_flagOverlay; }
	void clearOverlay();
	void dumpAllSurfaces(const Common::String funcname);

	void paint2(byte i, int destx, int desty);
	void paint4(byte i, int destx, int desty);
	void paint8(byte i, int destx, int desty);
	void copy(byte size, int destx, int desty, int dx, int dy);

	// Origin
	int16 _origX, _origY;
	//int16 _screenOffset;
	void calcStartStop(int &start, int &stop, int origin, int length);

	// Block coding type
	byte getCodingType();
	uint16 _codingType;
	byte _codingTypeCount;

	// Codebooks
	uint16 _num2blocks;
	uint16 _num4blocks;
	uint32 _codebook2[256 * 4];
	byte _codebook4[256 * 4];

	// Flags
	bool _flagNoPlay;	 //!< Play only first frame and do not print the image to the screen
	bool _flagOverlay;	 //!< If _flagNoPlay is set. Copy frame to the foreground otherwise to the background
	bool _altMotionDecoder; // Some ROQ vids use a variation on the copy codeblock
	bool _flagMasked; //!< Clear the video instead of play it, used in pente

	// Buffers
	void redrawRestoreArea(int screenOffset, bool force);
	void buildShowBuf();
	byte _scaleX, _scaleY;
	byte _offScale;
	int8 _motionOffX, _motionOffY;
	bool _interlacedVideo;
	bool _dirty;
	byte _alpha;
	bool _firstFrame;
	Common::Rect *_restoreArea;	// Area to be repainted by foreground

	Video::VideoDecoder *_videoDecoder;
	bool _isFileHandled;
};

class ROQSoundPlayer : public ROQPlayer {
public:
	ROQSoundPlayer(GroovieEngine *vm);
	~ROQSoundPlayer();
	void createAudioStream(bool stereo) override;
};

} // End of Groovie namespace

#endif // GROOVIE_VIDEO_ROQ_H
