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

#ifndef PRISONER_RESOURCE_H
#define PRISONER_RESOURCE_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/hash-str.h"

#include "common/array.h"
#include "common/str-array.h"

#include "graphics/surface.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"

#include "prisoner/resourcemgr.h"

namespace Prisoner {

/* PictureResource */

class PictureResource : public BaseResource {
public:
	PictureResource();
	~PictureResource();
	void load(Common::MemoryReadStream &stream);
	Graphics::Surface *getSurface() const { return _picture; }
protected:
	Graphics::Surface *_picture;
};

/* PaletteResource */

class PaletteResource : public BaseResource {
public:
	PaletteResource();
	~PaletteResource();
	void load(Common::MemoryReadStream &stream);
	byte *getPalette() { return _palette; }
protected:
	byte _palette[768];
};

/* FontResource */

struct FontChar {
	byte width, height;
	byte xoffs, yoffs;
	byte interletter;
	uint32 offset;
};

class FontResource : public BaseResource {
public:
	FontResource();
	~FontResource();
	void load(Common::MemoryReadStream &stream);
	bool isValidChar(byte ch) const { return ch >= _firstChar && ch <= _lastChar; }
	const FontChar *getFontChar(byte ch) const { return &_chars[ch - _firstChar]; }
	const byte *getFontCharData(byte ch) const { return _charData + _chars[ch - _firstChar].offset - _baseOffset; }
	int16 getHeight() const { return _height; }
	int16 getUnk1() const { return _unk1; }
	int16 getUnk2() const { return _unk2; }
protected:
	byte _flags;
	byte _firstChar, _lastChar;
	Common::Array<FontChar> _chars;
	byte *_charData;
	uint32 _baseOffset;
	int16 _height, _unk1, _unk2;
};

/* ScriptResource */

class ScriptResource : public BaseResource {
public:
	ScriptResource();
	~ScriptResource();
	void load(Common::MemoryReadStream &stream);
	byte *getScript(int16 index) { return _scripts[index]; }
	int16 getCount() const { return _scripts.size(); }
protected:
	Common::Array<byte*> _scripts;
	byte *_program;
};

/* TextResource */

struct TextItem {
	Common::String identifier;
	bool hasSpeech;
	int16 pakSlot;
	Common::Array<Common::StringArray*> chunks;
	const Common::String &getChunkLineString(uint chunkIndex, uint lineIndex) const {
		return (*chunks[chunkIndex])[lineIndex];
	}
};

class TextResource : public BaseResource {
public:
	TextResource();
	~TextResource();
	void load(Common::MemoryReadStream &stream);
	const TextItem *getText(Common::String &identifier);
	const TextItem *getText(uint index);
	int32 getIndex(Common::String &identifier);
	uint getCount() const { return _items.size(); }
	const Common::String &getSoundPakName() const { return _soundPakName; }
protected:
	Common::Array<TextItem> _items;
	Common::String _soundPakName;
};

/* AnimationResource */

enum AnimationCommandType {
	kActElement			= 0,
	kActCelSprite		= 1,
	kActNop0			= 2,
	kActNop1			= 3,
	kActFilledPolygon	= 4,
	kActRectangle		= 5,
	kActPolygon			= 6,
	kActPixels			= 7,
	kActPolygon1		= 8,	// unused in Comet? / Alias for kActPolygon
	kActPolygon2		= 9,	// unused in Comet? / Alias for kActPolygon
	kActCelRle			= 10
};

struct Point {
	int16 x, y;
};

struct AnimationCommand {
	byte cmd;
	byte arg1, arg2;
	Common::Array<Point> points;
	int16 argAsInt16() const { return arg1 | (arg2 << 8); }
};

struct AnimationElement {
	byte width, height, flags;
	Common::Array<AnimationCommand*> commands;
	~AnimationElement();
};

struct AnimationCel {
	uint16 flags;
	uint16 width, height;
	uint32 dataSize;
	byte *data;
};

struct AnimationFrame {
	uint16 elementIndex;
	uint16 ticks;
	int16 xOffs, yOffs;
};

struct AnimationFrameList {
	byte priority;
	Common::Array<AnimationFrame*> frames;
	~AnimationFrameList();
};

class AnimationResource : public BaseResource {
public:
	AnimationResource();
	~AnimationResource();
	void load(Common::MemoryReadStream &stream);
	int16 getCelWidth(int16 celIndex) const { return _cels[celIndex]->width; }
	int16 getCelHeight(int16 celIndex) const { return _cels[celIndex]->height; }
//protected://all public while in progress
	typedef Common::Array<uint32> OffsetArray;
	Common::Array<AnimationElement*> _elements;
	Common::Array<AnimationCel*> _cels;
	Common::Array<AnimationFrameList*> _anims;
	void loadOffsets(Common::SeekableReadStream &stream, OffsetArray &offsets);
	AnimationElement *loadAnimationElement(Common::SeekableReadStream &stream);
	AnimationCommand *loadAnimationCommand(Common::SeekableReadStream &stream);
	AnimationFrameList *loadAnimationFrameList(Common::SeekableReadStream &stream);
};

/* BaseSoundResource */

class BaseSoundResource : public BaseResource {
public:
	BaseSoundResource();
	~BaseSoundResource();
	Audio::RewindableAudioStream *getAudioStream() { return audioStream; }
	int16 getDuration() const { return _duration; }
protected:
	Audio::RewindableAudioStream *audioStream;
	int16 _duration;
};

/* SoundResource */

class SoundResource : public BaseSoundResource {
public:
	void load(Common::MemoryReadStream &stream);
};

/* LipSyncSoundResource */

struct LipSyncItem {
	byte index;
	byte duration;
};

struct LipSyncChannel {
	bool flag;
	int16 parentChannel;
	int16 value3;
	Common::Array<LipSyncItem> items;
};

class LipSyncSoundResource : public BaseSoundResource {
public:
	void load(Common::MemoryReadStream &stream);
	uint getChannelCount() const { return _channels.size(); }
	const LipSyncChannel& getChannel(uint index) const { return _channels[index]; }
	uint32 getTicksScale() const { return _ticksScale; }
protected:
	uint32 _ticksScale;
	Common::Array<LipSyncChannel> _channels;
};

/* MidiResource */

class MidiResource : public BaseResource {
public:
	MidiResource();
	~MidiResource();
	void load(Common::MemoryReadStream &stream);
	byte *getMidiData() { return _midiData; }
	uint32 getMidiSize() { return _midiSize; }
protected:
	byte *_midiData;
	uint32 _midiSize;
	void convertHMPtoSMF(Common::MemoryReadStream &stream);
};

} // End of namespace Prisoner

#endif /* PRISONER_RESOURCE_H */
