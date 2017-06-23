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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "macventure/sound.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace MacVenture {

// SoundManager
SoundManager::SoundManager(MacVentureEngine *engine, Audio::Mixer *mixer) {
	_container = NULL;
	Common::String filename = engine->getFilePath(kSoundPathID);
	_container = new Container(filename);
	_mixer = mixer;
	debugC(1, kMVDebugSound, "Created sound manager with file %s", filename.c_str());
}

SoundManager::~SoundManager() {
	if (_container)
		delete _container;

	Common::HashMap<ObjID, SoundAsset*>::iterator it;
	Common::HashMap<ObjID, SoundAsset*>::iterator end = _assets.end();
	for (it = _assets.begin(); it != end; it++) {
		delete it->_value;
	}
}

uint32 SoundManager::playSound(ObjID sound) {
	ensureLoaded(sound);
	_assets[sound]->play(_mixer, &_handle);
	return _assets[sound]->getPlayLength();
}

void SoundManager::ensureLoaded(ObjID sound) {
	if (!_assets.contains(sound))
		_assets[sound] = new SoundAsset(_container, sound);
}

SoundAsset::SoundAsset(Container *container, ObjID id) :
	_container(container), _id(id), _length(0), _frequency(1) {
	if (_container->getItemByteSize(_id) == 0)
		warning("Trying to load an empty sound asset (%d).", _id);

	Common::SeekableReadStream *stream = _container->getItem(_id);

	stream->seek(5, SEEK_SET);
	SoundType type = (SoundType)stream->readByte();
	debugC(2, kMVDebugSound, "Decoding sound of type %x", type);
	switch(type) {
	case kSound10:
		decode10(stream);
		break;
	case kSound12:
		decode12(stream);
		break;
	case kSound18:
		decode18(stream);
		break;
	case kSound1a:
		decode1a(stream);
		break;
	case kSound44:
		decode44(stream);
		break;
	case kSound78:
		decode78(stream);
		break;
	case kSound7e:
		decode7e(stream);
		break;
	default:
		warning("Unrecognized sound type: %x", type);
		break;
	}

	delete stream;
}

SoundAsset::~SoundAsset() {
	debugC(3, kMVDebugSound, "~SoundAsset(%d)", _id);
}

void SoundAsset::play(Audio::Mixer *mixer, Audio::SoundHandle *soundHandle) {
	if (_data.size() == 0) {
		return;
	}
	Audio::AudioStream *sound = Audio::makeRawStream(&_data.front(), _length, _frequency, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
	mixer->playStream(Audio::Mixer::kPlainSoundType, soundHandle, sound);
}

uint32 SoundAsset::getPlayLength() {
	// Transform to milliseconds
	return _length * 1000 / _frequency;
}

void SoundAsset::decode10(Common::SeekableReadStream *stream) {
	warning("Decode sound 0x10 untested");
	Common::Array<byte> wavtable;
	stream->seek(0x198, SEEK_SET);
	for (uint i = 0; i < 16; i++) {
		wavtable.push_back(stream->readByte());
	}
	_length = stream->readUint32BE() * 2;
	//Unused
	stream->readUint16BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000);
	byte ch = 0;
	for (uint i = 0; i < _length; i++) {
		if (i & 1) {
			ch >>= 4;
		} else {
			ch = stream->readByte();
		}
		_data.push_back(wavtable[ch & 0xf]);
	}
}

void SoundAsset::decode12(Common::SeekableReadStream *stream) {
	warning("Decode sound 0x12 untested");
	stream->seek(0xc, SEEK_SET);
	uint32 repeat = stream->readUint16BE();
	stream->seek(0x34, SEEK_SET);
	uint32 base = stream->readUint16BE() + 0x34;
	stream->seek(base, SEEK_SET);
	_length = stream->readUint32BE() - 6;
	stream->readUint16BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000);
	stream->seek(0xe2, SEEK_SET);
	// TODO: Possible source of bugs, the original just assigns the seek to the scales
	uint32 scales = stream->pos() + 0xe2;
	for (uint i = 0; i < repeat; i++) {
		stream->seek(scales + i * 2, SEEK_SET);
		uint32 scale = stream->readUint16BE();
		stream->seek(base + 0xa, SEEK_SET);
		for (uint j = 0; j < _length; j++) {
			byte ch = stream->readByte();
			if (ch & 0x80) {
				ch -= 0x80;
				uint32 env = ch * scale;
				ch = (env >> 8) & 0xff;
				if (ch & 0x80) {
					ch = 0x7f;
				}
				ch += 0x80;
			} else {
				ch = (ch ^ 0xff) + 1;
				ch -= 0x80;
				uint32 env = ch * scale;
				ch = (env >> 8) & 0xff;
				if (ch & 0x80) {
					ch = 0x7f;
				}
				ch += 0x80;
				ch = (ch ^ 0xff) + 1;
			}
			_data.push_back(ch);
		}
	}
}

void SoundAsset::decode18(Common::SeekableReadStream *stream) {
	warning("Decode sound 0x18 untested");
	Common::Array<byte> wavtable;
	stream->seek(0x252, SEEK_SET);
	for (uint i = 0; i < 16; i++) {
		wavtable.push_back(stream->readByte());
	}
	_length = stream->readUint32BE() * 2;
	//Unused
	stream->readUint16BE();
	// TODO: It had `| 0` at the end of this line, possible source of bugs.
	_frequency = (stream->readUint32BE() * 22100 / 0x10000);
	byte ch = 0;
	for (uint i = 0; i < _length; i++) {
		if (i & 1) {
			ch >>= 4;
		} else {
			ch = stream->readByte();
		}
		_data.push_back(wavtable[ch & 0xf]);
	}
}

void SoundAsset::decode1a(Common::SeekableReadStream *stream) {
	warning("Decode sound 0x1a untested");
	Common::Array<byte> wavtable;
	stream->seek(0x220, SEEK_SET);
	for (uint i = 0; i < 16; i++) {
		wavtable.push_back(stream->readByte());
	}
	_length = stream->readUint32BE();
	//Unused
	stream->readUint16BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000);
	byte ch = 0;
	for (uint i = 0; i < _length; i++) {
		if (i & 1) {
			ch >>= 4;
		} else {
			ch = stream->readByte();
		}
		_data.push_back(wavtable[ch & 0xf]);
	}
}

void SoundAsset::decode44(Common::SeekableReadStream *stream) {
	stream->seek(0x5e, SEEK_SET);
	_length = stream->readUint32BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000);
	for (uint i = 0; i < _length; i++) {
		_data.push_back(stream->readByte());
	}
}

void SoundAsset::decode78(Common::SeekableReadStream *stream) {
	Common::Array<byte> wavtable;
	stream->seek(0xba, SEEK_SET);
	for (uint i = 0; i < 16; i++) {
		wavtable.push_back(stream->readByte());
	}
	//Unused
	stream->readUint32BE();
	_length = stream->readUint32BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000);
	byte ch = 0;
	for (uint i = 0; i < _length; i++) {
		if (i & 1) {
			ch <<= 4;
		} else {
			ch = stream->readByte();
		}
		_data.push_back(wavtable[(ch >> 4) & 0xf]);
	}
}

void SoundAsset::decode7e(Common::SeekableReadStream *stream) {
	Common::Array<byte> wavtable;
	stream->seek(0xc2, SEEK_SET);
	for (uint i = 0; i < 16; i++) {
		wavtable.push_back(stream->readByte());
	}
	//Unused
	stream->readUint32BE();
	_length = stream->readUint32BE();
	_frequency = (stream->readUint32BE() * 22100 / 0x10000);
	uint32 last = 0x80;
	byte ch = 0;
	for (uint i = 0; i < _length; i++) {
		if (i & 1) {
			ch <<= 4;
		} else {
			ch = stream->readByte();
		}
		last += wavtable[(ch >> 4) & 0xf];
		_data.push_back(last & 0xff);
	}
}

} //End of namespace MacVenture
