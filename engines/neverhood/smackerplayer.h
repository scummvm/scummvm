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

#ifndef NEVERHOOD_SMACKERPLAYER_H
#define NEVERHOOD_SMACKERPLAYER_H

#include "video/smk_decoder.h"
#include "neverhood/neverhood.h"
#include "neverhood/entity.h"
#include "neverhood/subtitles.h"

namespace Neverhood {

class Scene;
class Palette;
class SmackerPlayer;

class SmackerSurface : public BaseSurface {
public:
	SmackerSurface(NeverhoodEngine *vm);
	void draw() override;
	void setSmackerFrame(const Graphics::Surface *smackerFrame);
	void unsetSmackerFrame();
	void renderSubtitles();
protected:
	const Graphics::Surface *_smackerFrame;
	Common::ScopedPtr<SubtitlePlayer> _subtitles;
	uint32 frameNumber;
	friend class SmackerPlayer;
};

class SmackerDoubleSurface : public SmackerSurface {
public:
	SmackerDoubleSurface(NeverhoodEngine *vm);
	void draw() override;
};

class NeverhoodSmackerDecoder : public Video::SmackerDecoder {
public:
	void forceSeekToFrame(uint frame);
};

class SmackerPlayer : public Entity {
public:
	SmackerPlayer(NeverhoodEngine *vm, Scene *scene, uint32 fileHash, bool doubleSurface, bool flag, bool paused = false);
	~SmackerPlayer() override;
        Common::SharedPtr<BaseSurface> getSurface() { return _smackerSurface; }
	void open(uint32 fileHash, bool keepLastFrame);
	void close();
	void gotoFrame(int frameNumber);
	uint32 getFrameCount();
	uint32 getFrameNumber();
	uint getStatus();
	void setDrawPos(int16 x, int16 y);
	void rewind();
	bool isDone() { return getFrameNumber() + 1 == getFrameCount(); }
	NeverhoodSmackerDecoder *getSmackerDecoder() const { return _smackerDecoder; }
protected:
	Scene *_scene;
	Palette *_palette;
	NeverhoodSmackerDecoder *_smackerDecoder;
	Common::SharedPtr<SmackerSurface> _smackerSurface;
	uint32 _fileHash;
	bool _smackerFirst;
	bool _doubleSurface;
	Common::SeekableReadStream *_stream;
	bool _keepLastFrame;
	bool _videoDone;
	bool _paused;
	int _drawX, _drawY;
	void update();
	void updateFrame();
	void updatePalette();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SMACKERPLAYER_H */
