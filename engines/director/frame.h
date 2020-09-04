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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
}

namespace Director {

class Score;
class Sprite;
class TextCastMember;

enum {
	kChannelDataSize = (25 * 50)
};

struct PaletteInfo {
	int paletteId;

	byte firstColor;
	byte lastColor;
	byte flags;
	byte speed;
	uint16 frameCount;
	uint16 cycleCount;
	uint16 cycleLength;
	byte fade;
	byte delay;
	byte style;
	byte colorCode;

	PaletteInfo() {
		paletteId = 0;
		firstColor = lastColor = 0;
		flags = 0; speed = 0;
		frameCount = cycleCount = cycleLength = 0;
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

	Score *getScore() const { return _score; }

	void readChannels(Common::ReadStreamEndian *stream);
	void readChannel(Common::SeekableReadStreamEndian &stream, uint16 offset, uint16 size);

	void executeImmediateScripts();

private:

	void readPaletteInfo(Common::SeekableReadStreamEndian &stream);
	void readSprite(Common::SeekableReadStreamEndian &stream, uint16 offset, uint16 size);
	void readMainChannels(Common::SeekableReadStreamEndian &stream, uint16 offset, uint16 size);
	Image::ImageDecoder *getImageFrom(uint16 spriteId);
	Common::String readTextStream(Common::SeekableReadStreamEndian *textStream, TextCastMember *textCast);


public:
	int _numChannels;
	byte _channelData[kChannelDataSize];
	uint16 _actionId;
	uint16 _transDuration;
	uint8 _transArea; // 1 - Whole Window, 0 - Changing Area
	uint8 _transChunkSize;
	TransitionType _transType;
	PaletteInfo _palette;
	uint8 _tempo;

	uint16 _sound1;
	uint8 _soundType1;
	uint16 _sound2;
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

} // End of namespace Director

#endif
