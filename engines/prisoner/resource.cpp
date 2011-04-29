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

#include "common/file.h"
#include "common/textconsole.h"

#include "common/stream.h"
#include "common/zlib.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/wave.h"

#include "prisoner/resource.h"

namespace Prisoner {

Common::String readString(Common::ReadStream &source) {
	char tempString[256], *p = tempString;
	while((*p++ = source.readByte()) != 0);
	return tempString;
}

/* PictureResource */

PictureResource::PictureResource() : _picture(NULL) {
}

PictureResource::~PictureResource() {
	delete _picture;
}

void PictureResource::load(Common::MemoryReadStream &stream) {
	uint32 width = stream.readUint16LE();
	uint32 height = stream.readUint16LE();
	debug(1, "PictureResource::load() width = %d; height = %d", width, height);
	_picture = new Graphics::Surface();
	_picture->create(width, height, 1);
	stream.read(_picture->pixels, width * height);
}

/* PaletteResource */

PaletteResource::PaletteResource() {
}

PaletteResource::~PaletteResource() {
}

void PaletteResource::load(Common::MemoryReadStream &stream) {
	stream.read(_palette, 768);
}

/* FontResource */

FontResource::FontResource() : _charData(NULL) {
}

FontResource::~FontResource() {
	delete[] _charData;
}

void FontResource::load(Common::MemoryReadStream &stream) {
	byte charCount;
	uint32 charDataSize;
	_flags = stream.readByte();
	stream.readByte();
	_firstChar = stream.readByte();
	_lastChar = stream.readByte();
	charCount = _lastChar - _firstChar + 1;
	_height = stream.readByte();
	_unk2 = stream.readByte();
	_unk1 = _height + _unk2;
	stream.readUint16LE();
	_chars.reserve(charCount);
	for (uint i = 0; i < charCount; i++) {
		FontChar fontChar;
		fontChar.offset = stream.readUint32LE();
		fontChar.width = stream.readByte();
		fontChar.height = stream.readByte();
		fontChar.xoffs = stream.readByte();
		fontChar.yoffs = stream.readByte();
		fontChar.interletter = stream.readByte();
		stream.readByte(); // unknown

		debug(8, "%08X; %d,%d %d,%d %d", fontChar.offset, fontChar.width, fontChar.height,
			fontChar.xoffs, fontChar.yoffs, fontChar.interletter);

		_chars.push_back(fontChar);
	}
	charDataSize = stream.size() - stream.pos();
	_charData = new byte[charDataSize];
	stream.read(_charData, charDataSize);
	_baseOffset = charCount * 10 + 8;
}

/* ScriptResource */

ScriptResource::ScriptResource() : _program(NULL) {
}

ScriptResource::~ScriptResource() {
	delete[] _program;
}

void ScriptResource::load(Common::MemoryReadStream &stream) {

	Common::Array<int32> offsets;
	uint32 programSize;

	do {
		offsets.push_back(stream.readUint16LE() * 2);
	} while (stream.pos() < offsets[0]);

	programSize = stream.size() - offsets[0];
	_program = new byte[programSize];
	stream.read(_program, programSize);

	for (uint i = 0; i < offsets.size(); i++)
		_scripts.push_back(_program + offsets[i] - offsets[0]);

	debug(8, "script count = %d", _scripts.size());

}

/* TextResource */

TextResource::TextResource() {
}

TextResource::~TextResource() {
	// TODO: Free
}

void TextResource::load(Common::MemoryReadStream &stream) {

	char identifier[9];
	Common::Array<int32> offsets;

	do {
		TextItem textItem;
		int32 offset;
		stream.read(identifier, 7);
		offset = stream.readUint32LE() - 1;
		debug(8, "[%s] -> %08X", identifier, offset);
		textItem.identifier = identifier;
		_items.push_back(textItem);
		offsets.push_back(offset);
	} while (stream.pos() != offsets[0] - 9);

	stream.read(identifier, 9);
	_soundPakName = identifier;
	debug(8, "_soundPakName = %s", _soundPakName.c_str());

	for (uint i = 0; i < _items.size(); i++) {
		TextItem *textItem = &_items[i];
		uint16 chunkCount;
		stream.seek(offsets[i]);
		textItem->hasSpeech = stream.readByte() != 0;
		textItem->pakSlot = stream.readUint16LE();
		debug(8, "hasSpeech = %d; pakSlot = %d", textItem->hasSpeech, textItem->pakSlot);
		chunkCount = stream.readUint16LE() / 2;
		stream.skip((chunkCount - 1) * 2);
		while (chunkCount--) {
			Common::StringArray *lines = new Common::StringArray();
			byte lineCount = stream.readByte();
			while (lineCount--)
				lines->push_back(readString(stream));
			textItem->chunks.push_back(lines);
		}
	}

}

const TextItem *TextResource::getText(Common::String &identifier) {
	return getText(getIndex(identifier));
}

const TextItem *TextResource::getText(uint index) {
	return &_items[index];
}

int32 TextResource::getIndex(Common::String &identifier) {
	for (uint i = 0; i < _items.size(); i++) {
		if (_items[i].identifier == identifier)
			return i;
	}
	return -1;
}

/* AnimationResource */

AnimationElement::~AnimationElement() {
	for (Common::Array<AnimationCommand*>::iterator iter = commands.begin(); iter != commands.end(); iter++)
		delete (*iter);
}

AnimationCel::~AnimationCel() {
	delete[] data;
}

AnimationFrameList::~AnimationFrameList() {
	for (Common::Array<AnimationFrame*>::iterator iter = frames.begin(); iter != frames.end(); iter++)
		delete (*iter);
}

AnimationResource::AnimationResource() {
}

AnimationResource::~AnimationResource() {

	for (Common::Array<AnimationElement*>::iterator iter = _elements.begin(); iter != _elements.end(); iter++)
		delete (*iter);

	for (Common::Array<AnimationCel*>::iterator iter = _cels.begin(); iter != _cels.end(); iter++)
		delete (*iter);

	for (Common::Array<AnimationFrameList*>::iterator iter = _anims.begin(); iter != _anims.end(); iter++)
		delete (*iter);

}

void AnimationResource::load(Common::MemoryReadStream &stream) {

	OffsetArray sectionOffsets, offsets;

	loadOffsets(stream, sectionOffsets);

	if (sectionOffsets.size() < 4)
		error("AnimationResource::load() Unexpected section count");

	// Load animation elements
	stream.seek(sectionOffsets[0]);
	loadOffsets(stream, offsets);
	for (uint i = 0; i < offsets.size(); i++) {
		if (sectionOffsets[0] + offsets[i] < (uint32)stream.size()) {
			stream.seek(sectionOffsets[0] + offsets[i]);
			AnimationElement *animationElement = loadAnimationElement(stream);
			_elements.push_back(animationElement);
		}
	}

	// Load animation cels
	uint32 sectionEndOfs = sectionOffsets[2];
	stream.seek(sectionOffsets[1]);
	loadOffsets(stream, offsets);
	for (uint i = 3; i < sectionOffsets.size(); i++)
		if (sectionEndOfs == 0 || (sectionOffsets[i] < sectionEndOfs && sectionOffsets[i] > sectionOffsets[1]))
			sectionEndOfs = sectionOffsets[i];
	offsets.push_back(sectionEndOfs - sectionOffsets[1] + 2);
	for (uint i = 0; i < offsets.size() - 1; i++) {
		if (sectionOffsets[1] + offsets[i] - 2 < (uint32)stream.size()) {
			stream.seek(sectionOffsets[1] + offsets[i] - 2);
			AnimationCel *animationCel = new AnimationCel();
			animationCel->flags = stream.readUint16LE();
			animationCel->width = stream.readUint16LE();
			animationCel->height = stream.readUint16LE();
			animationCel->dataSize = offsets[i + 1] - offsets[i] - 6;
			animationCel->data = new byte[animationCel->dataSize];
			animationCel->scale = 100;
			debug(8, "AnimationResource::load() cel width = %d; height = %d; dataSize = %d", animationCel->width, animationCel->height, animationCel->dataSize);
			stream.read(animationCel->data, animationCel->dataSize);
			_cels.push_back(animationCel);
		}
	}

	// Load animation frames
	if (sectionOffsets[2] > 0) {
		stream.seek(sectionOffsets[2]);
		loadOffsets(stream, offsets);
		for (uint i = 0; i < offsets.size(); i++) {
			if (sectionOffsets[2] + offsets[i] < (uint32)stream.size()) {
				stream.seek(sectionOffsets[2] + offsets[i]);
				AnimationFrameList *animationFrameList = loadAnimationFrameList(stream);
				_anims.push_back(animationFrameList);
			}
		}
	}

	// TODO: Load section 4 data
	// CHECKME: Doesn't seem to be used in Prisoner

}

void AnimationResource::loadOffsets(Common::SeekableReadStream &stream, OffsetArray &offsets) {
	offsets.clear();
	uint32 offset = stream.readUint32LE();
	uint count = offset / 4;
	debug(8, "AnimationResource::loadOffsets() count = %d", count);
	offsets.reserve(count);
	while (count--) {
		offsets.push_back(offset);
		offset = stream.readUint32LE();
	}
}

AnimationElement *AnimationResource::loadAnimationElement(Common::SeekableReadStream &stream) {
	AnimationElement *animationElement = new AnimationElement();
	animationElement->width = 0;//stream.readByte();
	animationElement->height = 0;//stream.readByte();
	animationElement->flags = stream.readByte();
	byte cmdCount = stream.readByte();
	debug(8, "AnimationResource::loadAnimationElement() cmdCount = %d; flags = %02X", cmdCount, animationElement->flags);
	while (cmdCount--) {
		AnimationCommand *animationCommand = loadAnimationCommand(stream);
		animationElement->commands.push_back(animationCommand);
	}
	return animationElement;
}

AnimationCommand *AnimationResource::loadAnimationCommand(Common::SeekableReadStream &stream) {
	AnimationCommand *animationCommand = new AnimationCommand();
	animationCommand->cmd = stream.readByte();
	byte pointsCount = stream.readByte();
	animationCommand->arg1 = stream.readByte();
	animationCommand->arg2 = stream.readByte();
	debug(8, "AnimationResource::loadAnimationCommand() cmd = %d; pointsCount = %d; arg1 = %d; arg2 = %d",
		animationCommand->cmd, pointsCount, animationCommand->arg1, animationCommand->arg2);
	animationCommand->points.reserve(pointsCount);
	while (pointsCount--) {
		Point pt;
		pt.x = (int16)stream.readUint16LE();
		pt.y = (int16)stream.readUint16LE();
		debug(8, "AnimationResource::loadAnimationCommand()	 x = %d; y = %d", pt.x, pt.y);
		animationCommand->points.push_back(pt);
	}
	return animationCommand;
}

AnimationFrameList *AnimationResource::loadAnimationFrameList(Common::SeekableReadStream &stream) {
	AnimationFrameList *animationFrameList = new AnimationFrameList();
	animationFrameList->priority = stream.readByte();
	byte frameCount = stream.readByte();
	debug(8, "AnimationResource::loadAnimationFrameList() frameCount = %d", frameCount);
	while (frameCount--) {
		AnimationFrame *animationFrame = new AnimationFrame();
		animationFrame->elementIndex = stream.readUint16LE();
		animationFrame->ticks = stream.readUint16LE();
		animationFrame->xOffs = (int16)stream.readUint16LE();
		animationFrame->yOffs = (int16)stream.readUint16LE();
		debug(8, "AnimationResource::loadAnimationFrameList() elementIndex = %d; ticks = %04X; xOffs = %d; yOffs = %d",
			animationFrame->elementIndex, animationFrame->ticks, animationFrame->xOffs, animationFrame->yOffs);
		animationFrameList->frames.push_back(animationFrame);
	}
	return animationFrameList;
}

/* BaseSoundResource */

BaseSoundResource::BaseSoundResource() : _data(NULL), _size(0), _duration(0) {
}

BaseSoundResource::~BaseSoundResource() {
	delete _data;
}

Audio::AudioStream *BaseSoundResource::createAudioStream() {
	Common::MemoryReadStream *wavStream = new Common::MemoryReadStream(_data, _size);
	return Audio::makeWAVStream(wavStream, DisposeAfterUse::YES);
}

Audio::AudioStream *BaseSoundResource::createLoopingAudioStream(uint loops) {
	Common::MemoryReadStream *wavStream = new Common::MemoryReadStream(_data, _size);
	return makeLoopingAudioStream(Audio::makeWAVStream(wavStream, DisposeAfterUse::YES), loops);
}

void BaseSoundResource::loadWaveData(Common::MemoryReadStream &stream, int size) {
	_size = size;
	_data = new byte[_size];
	stream.read(_data, _size);
	_duration = (stream.size() + 22049) * 100 / 22050;
}

/* SoundResource */

void SoundResource::load(Common::MemoryReadStream &stream) {
	loadWaveData(stream, stream.size());
}

/* LipSyncSoundResource */

void LipSyncSoundResource::load(Common::MemoryReadStream &stream) {

	while (stream.readByte() != 0);

	uint32 wavDataSize = stream.readUint32LE();
	uint32 channelOffset = stream.pos() + wavDataSize - 4;

    loadWaveData(stream, wavDataSize);

	stream.seek(channelOffset);

	_ticksScale = stream.readUint32LE();
	uint32 channelCount = stream.readUint32LE() / 4;

	debug(8, "LipSyncSoundResource::load() channelCount = %d", channelCount);

	_channels.reserve(channelCount);

	while (channelCount--) {
		LipSyncChannel channel;

		uint32 itemCount = stream.readUint32LE() / 2;

		channel.flag = stream.readByte() != 0;
		channel.parentChannel = stream.readUint16LE();
		channel.value3 = stream.readUint16LE();

		debug(8, "LipSyncSoundResource::load() channel: flag = %d; parentChannel = %d; value3 = %d",
			channel.flag, channel.parentChannel, channel.value3);

		channel.items.reserve(itemCount);
		while (itemCount--) {
			LipSyncItem item;
			item.index = stream.readByte();
			item.duration = stream.readByte();
			debug(8, "LipSyncSoundResource::load()   item: index = %d; duration = %d", item.index, item.duration);
			channel.items.push_back(item);
		}

		_channels.push_back(channel);
	}

}

/* MidiResource */

MidiResource::MidiResource() : _midiData(NULL), _midiSize(0) {
}

MidiResource::~MidiResource() {
	free(_midiData);
}

void MidiResource::load(Common::MemoryReadStream &stream) {
	convertHMPtoSMF(stream);
}

void MidiResource::convertHMPtoSMF(Common::MemoryReadStream &stream) {

	Common::MemoryWriteStreamDynamic writeS;

	byte buf[8];

	stream.read(buf, sizeof(buf));
	if (memcmp(buf, "HMIMIDIP", 8) != 0) {
		warning("convertHMPtoSMF: Invalid HMP header");
		return;
	}

	// Read the number of tracks. Note that all the tracks are still part
	// of the same song, just like in type 1 SMF files.

	stream.seek(0x30);

	uint32 numTracks = stream.readUint32LE();

	// The first track starts on offset 0x300. It's currently unknown what
	// the skipped data is for.

	stream.seek(0x300);

	// For some reason, we skip the first track entirely.

	byte a = stream.readByte();
	byte b = stream.readByte();
	byte c = stream.readByte();

	while (a != 0xFF || b != 0x2F || c != 0x00) {
		a = b;
		b = c;
		c = stream.readByte();
	}

	// The beginning of the MIDI header
	static const byte midiHeader1[] = { 'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 1 };
	// The last 2 bytes of the midi header and track 0
	static const byte midiHeader2[] = { 0, 0xC0, 'M', 'T', 'r', 'k', 0, 0, 0, 0x0B, 0, 0xFF, 0x51, 0x03, 0x18, 0x80, 0, 0, 0xFF, 0x2F, 0 };


	// Write the MIDI header
	writeS.write(midiHeader1, sizeof(midiHeader1));

	// Write the number of tracks
	writeS.writeUint16BE(numTracks);

	// Write the rest of the MIDI header and track 0.
	writeS.write(midiHeader2, sizeof(midiHeader2));

	// Read and convert all the tracks
	for (uint i = 1; i < numTracks; i++) {
		if (stream.readUint32LE() != i) {
			warning("convertHMPtoSMF: Invalid HMP track number");
			free(writeS.getData());
			return;
		}

		uint32 trackLength = stream.readUint32LE() - 12;
		stream.readUint32LE();	// Unused?

		// Write the track header
		writeS.write("MTrk", 4);

		// This is where we will write the length of the track.
		uint32 trackLengthPos = writeS.pos();
		writeS.writeUint32LE(0);

		// In the original, this is cleared once at the beginning of
		// the function, but surely the last command does not carry
		// over to the next track?

		byte lastCmd = 0;

		// Now we can finally convert the track
		int32 endPos = stream.pos() + trackLength;
		while (stream.pos() < endPos) {
			// Convert the VLQ
			byte vlq[4];
			int j = -1;

			do {
				j++;
				vlq[j] = stream.readByte();
			} while (!(vlq[j] & 0x80));

			for (int k = 0; k <= j; k++) {
				a = vlq[j - k] & 0x7F;
				if (k != j)
					a |= 0x80;
				writeS.writeByte(a);
			}

			a = stream.readByte();

			if (a == 0xFF) {
				// META event
				b = stream.readByte();
				c = stream.readByte();

				writeS.writeByte(a);
				writeS.writeByte(b);
				writeS.writeByte(c);

				if (c > 0) {
					byte *metaBuf = new byte[c];
					stream.read(metaBuf, c);
					writeS.write(metaBuf, c);
					delete[] metaBuf;
				}

				if (b == 0x2F) {
					if (c != 0x00) {
						warning("convertHMPtoSMF: End of track with non-zero size");
						free(writeS.getData());
						return;
					}
					break;
				}
			} else {
				if (a != lastCmd)
					writeS.writeByte(a);

				switch (a & 0xF0) {
				case 0x80:
				case 0x90:
				case 0xA0:
				case 0xB0:
				case 0xE0:
					b = stream.readByte();
					c = stream.readByte();
					writeS.writeByte(b);
					writeS.writeByte(c);
					break;
				case 0xC0:
				case 0xD0:
					b = stream.readByte();
					writeS.writeByte(b);
					break;
				default:
					warning("convertHMPtoSMF: Invalid HMP command %02X", a);
					free(writeS.getData());
					return;
				}

				lastCmd = a;
			}
		}

		if (stream.pos() != endPos) {
			warning("convertHMPtoSMF: Invalid track length");
			free(writeS.getData());
			return;
		}

		WRITE_BE_UINT32(writeS.getData() + trackLengthPos, writeS.pos() - trackLengthPos - 4);
	}

	_midiSize = writeS.size();
	_midiData = writeS.getData();

}

} // End of namespace Prisoner
