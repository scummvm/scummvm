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

#ifndef DIRECTOR_FRAME_H
#define DIRECTOR_FRAME_H

namespace Image {
class ImageDecoder;
}

namespace Graphics {
class ManagedSurface;
struct Surface;
}

namespace Common {
class ReadStreamEndian;
class MemoryReadStreamEndian;
}

namespace Director {

class Score;
class Sprite;
class TextCastMember;

enum {
	kMainChannelSizeD2 = 32,
	kSprChannelSizeD2 = 16,

	kMainChannelSizeD4 = 40,
	kSprChannelSizeD4 = 20,

	kMainChannelSizeD5 = 48,
	kSprChannelSizeD5 = 24,

	kMainChannelSizeD6 = 48,
	kSprChannelSizeD6 = 24,
};

struct PaletteInfo {
	CastMemberID paletteId;

	byte firstColor;
	byte lastColor;
	byte flags;
	bool colorCycling;
	bool normal;
	bool fadeToWhite;
	bool fadeToBlack;
	bool autoReverse;
	bool overTime;
	byte speed;
	uint16 frameCount;
	uint16 cycleCount;
	byte fade;
	byte delay;
	byte style;
	byte colorCode;

	PaletteInfo() {
		paletteId = CastMemberID(0, 0);
		firstColor = lastColor = 0;
		flags = 0; colorCycling = false;
		normal = false; fadeToWhite = false;
		fadeToBlack = false; autoReverse = false;
		overTime = false; speed = 0;
		frameCount = cycleCount = 0;
		fade = delay = style = colorCode = 0;
	}
};

struct FrameEntity {
	uint16 spriteId;
	Common::Rect rect;
};


class Frame {
public:
	Frame(Score *score, int numChannels);
	Frame(const Frame &frame);
	~Frame();

	void reset();

	Score *getScore() const { return _score; }

	void readChannel(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size, uint16 version);

	void executeImmediateScripts();

	Common::String formatChannelInfo();

private:

	void readChannelD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readSpriteD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readMainChannelsD2(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);

	void readChannelD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readSpriteD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readMainChannelsD4(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);

	void readChannelD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readSpriteD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readMainChannelsD5(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);

	void readChannelD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readSpriteD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);
	void readMainChannelsD6(Common::MemoryReadStreamEndian &stream, uint16 offset, uint16 size);

	Image::ImageDecoder *getImageFrom(uint16 spriteId);
	Common::String readTextStream(Common::SeekableReadStreamEndian *textStream, TextCastMember *textCast);


public:
	int _numChannels;
	CastMemberID _actionId;
	uint16 _transDuration;
	uint8 _transArea; // 1 - Whole Window, 0 - Changing Area
	uint8 _transChunkSize;
	TransitionType _transType;
	CastMemberID _trans;
	PaletteInfo _palette;
	uint8 _tempo;

	uint8 _scoreCachedTempo;
	CastMemberID _scoreCachedPaletteId;

	CastMemberID _sound1;
	uint8 _soundType1;
	CastMemberID _sound2;
	uint8 _soundType2;

	byte _colorTempo;
	byte _colorSound1;
	byte _colorSound2;
	byte _colorScript;
	byte _colorTrans;

	uint8 _skipFrameFlag;
	uint8 _blend;
	Common::Array<Sprite *> _sprites;
	Score *_score;
	DirectorEngine *_vm;
};

void readSpriteDataD2(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition);
void readSpriteDataD4(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition);
void readSpriteDataD5(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition);
void readSpriteDataD6(Common::SeekableReadStreamEndian &stream, Sprite &sprite, uint32 startPosition, uint32 finishPosition);

} // End of namespace Director

#endif
