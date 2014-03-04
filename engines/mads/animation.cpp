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
 */

#include "mads/animation.h"
#include "mads/compression.h"

#define FILENAME_SIZE 13

namespace MADS {
	
AAHeader::AAHeader(Common::SeekableReadStream *f) {
	_spriteSetsCount = f->readUint16LE();
	_miscEntriesCount = f->readUint16LE();
	_frameEntriesCount = f->readUint16LE();
	_messagesCount = f->readUint16LE();
	f->skip(1);
	_flags = f->readByte();

	f->skip(2);
	_animMode = f->readUint16LE();
	_roomNumber = f->readUint16LE();
	f->skip(2);
	_manualFlag = f->readUint16LE() != 0;
	_spriteListIndex = f->readUint16LE();
	_scrollPosition.x = f->readSint16LE();
	_scrollPosition.y = f->readSint16LE();
	_scrollTicks = f->readUint32LE();
	f->skip(6);

	char buffer[FILENAME_SIZE];
	f->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE - 1] = '\0';
	_interfaceFile = Common::String(buffer);

	for (int i = 0; i < 10; ++i) {
		f->read(buffer, FILENAME_SIZE);
		buffer[FILENAME_SIZE - 1] = '\0';
		_spriteSetNames.push_back(Common::String(buffer));
	}

	f->skip(81);
	f->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE - 1] = '\0';
	_lbmFilename = Common::String(buffer);

	f->skip(365);
	f->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE - 1] = '\0';
	_spritesFilename = Common::String(buffer);

	f->skip(48);
	f->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE - 1] = '\0';
	_soundName = Common::String(buffer);

	f->skip(13);
	f->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE - 1] = '\0';
	_dsrName = Common::String(buffer);

	f->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE - 1] = '\0';
	_fontResource = Common::String(buffer);
}

/*------------------------------------------------------------------------*/

void AnimMessage::load(Common::SeekableReadStream *f) {
	_soundId = f->readSint16LE();

	char buffer[64];
	f->read(&buffer[0], 64);
	_msg = Common::String(buffer);
	f->skip(4);
	_pos.x = f->readSint16LE();
	_pos.y = f->readSint16LE();
	_flags = f->readUint16LE();
	_rgb1[0] = f->readByte() << 2;
	_rgb1[1] = f->readByte() << 2;
	_rgb1[2] = f->readByte() << 2;
	_rgb2[0] = f->readByte() << 2;
	_rgb2[1] = f->readByte() << 2;
	_rgb2[2] = f->readByte() << 2;
	f->skip(2);	// Space for kernelMsgIndex
	_kernelMsgIndex = -1;
	f->skip(6);
	_startFrame = f->readUint16LE();
	_endFrame = f->readUint16LE();
	f->skip(2);
}

void AnimFrameEntry::load(Common::SeekableReadStream *f) {
	_frameNumber = f->readUint16LE();
	_seqIndex = f->readByte();
	_spriteSlot._spritesIndex = f->readByte();
	_spriteSlot._frameNumber = f->readUint16LE();
	_spriteSlot._position.x = f->readSint16LE();
	_spriteSlot._position.y = f->readSint16LE();
	_spriteSlot._depth = f->readSByte();
	_spriteSlot._scale = (int8)f->readByte();
}

/*------------------------------------------------------------------------*/

void AnimMiscEntry::load(Common::SeekableReadStream *f) {
	_soundId = f->readByte();
	_msgIndex = f->readSByte();
	_numTicks = f->readUint16LE();
	_posAdjust.x = f->readSint16LE();
	_posAdjust.y = f->readSint16LE();
	_scrollPos.x = f->readSint16LE();
	_scrollPos.y = f->readSint16LE();
}

/*------------------------------------------------------------------------*/

Animation *Animation::init(MADSEngine *vm, Scene *scene) {
	return new Animation(vm, scene);
}

Animation::Animation(MADSEngine *vm, Scene *scene) : _vm(vm), _scene(scene) {
	_font = nullptr;
}

Animation::~Animation() {
	delete _font;
	for (uint i = 0; i < _spriteSets.size(); ++i)
		delete _spriteSets[i];
}

void Animation::load(MSurface &depthSurface, InterfaceSurface &interfaceSurface,
	const Common::String &resName, int flags, Common::Array<RGB4> *palAnimData,
	SceneInfo *sceneInfo) {
	Common::String resourceName = resName;
	if (!resourceName.contains("."))
		resourceName += ".AA";

	File f(resourceName);
	MadsPack madsPack(&f);

	Common::SeekableReadStream *stream = madsPack.getItemStream(0);
	AAHeader aaHeader(stream);
	delete stream;

	if (aaHeader._animMode == 4)
		flags |= 0x4000;

	if (flags & 0x100) {
		loadInterface(interfaceSurface, depthSurface, aaHeader, flags, palAnimData, sceneInfo);
	}
	if (flags & 0x200) {
		// No data
		aaHeader._messagesCount = 0;
		aaHeader._frameEntriesCount = 0;
		aaHeader._miscEntriesCount = 0;
	}

	// Initialize the reference list
	_spriteListIndexes.clear();
	for (int i = 0; i < aaHeader._spriteSetsCount; ++i)
		_spriteListIndexes.push_back(-1);

	_kernelMessages.clear();
	if (aaHeader._messagesCount > 0) {
		// Chunk 2: Following is a list of any messages for the animation
		Common::SeekableReadStream *msgStream = madsPack.getItemStream(1);

		for (int i = 0; i < aaHeader._messagesCount; ++i) {
			AnimMessage rec;
			rec.load(msgStream);
			_kernelMessages.push_back(rec);
		}

		delete msgStream;
	}

	_frameEntries.clear();
	if (aaHeader._frameEntriesCount > 0) {
		// Chunk 3: animation frame info
		Common::SeekableReadStream *frameStream = madsPack.getItemStream(2);

		for (int i = 0; i < aaHeader._frameEntriesCount; i++) {
			AnimFrameEntry rec;
			rec.load(frameStream);
			_frameEntries.push_back(rec);
		}

		delete frameStream;
	}
	
	_miscEntries.clear();
	if (aaHeader._miscEntriesCount > 0) {
		// Chunk 4: Misc Data
		Common::SeekableReadStream *miscStream = madsPack.getItemStream(3);

		for (int i = 0; i < aaHeader._miscEntriesCount; ++i) {
			AnimMiscEntry rec;
			rec.load(miscStream);
			_miscEntries.push_back(rec);
		}

		delete miscStream;
	}
	
	// If the animation specifies a font, then load it for access
	delete _font;
	if (aaHeader._flags & ANIM_CUSTOM_FONT) {
		Common::String fontName = "*" + aaHeader._fontResource;
		_font = _vm->_font->getFont(fontName.c_str());
	} else {
		_font = nullptr;
	}
	
	// Load all the sprite sets for the animation
	for (uint i = 0; i < _spriteSets.size(); ++i)
		delete _spriteSets[i];
	_spriteSets.clear();
	_spriteSets.resize(aaHeader._spriteSetsCount);

	for (int i = 0; i < aaHeader._spriteSetsCount; ++i) {
		if (aaHeader._manualFlag && (i == aaHeader._spriteListIndex)) {
			// Skip over field, since it's manually loaded
			_spriteSets[i] = nullptr;
		} else {
			_spriteSets[i] = new SpriteAsset(_vm, aaHeader._spriteSetNames[i], flags);
		}
	}

	if (aaHeader._manualFlag) {
		Common::String resName = "*" + aaHeader._spriteSetNames[aaHeader._spriteListIndex];
		SpriteAsset *sprites = new SpriteAsset(_vm, resName, flags);
		_spriteSets[aaHeader._spriteListIndex] = sprites;

		_spriteListIndexes[aaHeader._spriteListIndex] = _scene->_sprites.add(sprites);
	}

	// TODO: List var_420/var_422 population that seems to overwrite other structures?

	if (aaHeader._animMode == 4) {
		// Remaps the sprite list indexes for frames to the loaded sprite list indexes
		for (uint i = 0; i < _frameEntries.size(); ++i) {
			int spriteListIndex = _frameEntries[i]._spriteSlot._spritesIndex;
			_frameEntries[i]._spriteSlot._spritesIndex = _spriteListIndexes[spriteListIndex];
		}
	} else {
		// Remaps the sprite list indexes for frames to the loaded sprite list indexes
		for (uint i = 0; i < _frameEntries.size(); ++i) {
			int spriteListIndex = _frameEntries[i]._spriteSlot._spritesIndex;
			_frameEntries[i]._spriteSlot._spritesIndex = _spriteListIndexes[spriteListIndex];
		}
	}

	f.close();
}

void Animation::loadInterface(InterfaceSurface &interfaceSurface, MSurface &depthSurface,
		AAHeader &header, int flags, Common::Array<RGB4> *palAnimData, SceneInfo *sceneInfo) {
	_scene->_depthStyle = 0;
	if (header._animMode <= 2) {
		sceneInfo->load(header._roomNumber, flags, header._interfaceFile, 0, depthSurface, interfaceSurface);
		_scene->_depthStyle = sceneInfo->_depthStyle == 2 ? 1 : 0;
		if (palAnimData) {
			palAnimData->clear();
			for (uint i = 0; i < sceneInfo->_palAnimData.size(); ++i)
				palAnimData->push_back(sceneInfo->_palAnimData[i]);
		}
	}
	else if (header._animMode == 4) {
		// Load a scene interface
		Common::String resourceName = "*" + header._interfaceFile;
		interfaceSurface.load(resourceName);

		if (palAnimData)
			palAnimData->clear();
	}
	else {
		// Original has useless code here
	}
}

void Animation::update() {
	warning("TODO: Animation::update");
}

} // End of namespace MADS
