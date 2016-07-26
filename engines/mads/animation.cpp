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

#include "mads/animation.h"
#include "mads/compression.h"

#define FILENAME_SIZE 13

namespace MADS {

void AAHeader::load(Common::SeekableReadStream *f) {
	_spriteSetsCount = f->readUint16LE();
	_miscEntriesCount = f->readUint16LE();
	_frameEntriesCount = f->readUint16LE();
	_messagesCount = f->readUint16LE();
	_loadFlags = f->readUint16LE();
	_charSpacing = f->readSint16LE();
	_bgType = (AnimBgType)f->readUint16LE();
	_roomNumber = f->readUint16LE();
	f->skip(2);
	_manualFlag = f->readUint16LE() != 0;
	_spritesIndex = f->readUint16LE();
	_scrollPosition.x = f->readSint16LE();
	_scrollPosition.y = f->readSint16LE();
	_scrollTicks = f->readUint32LE() & 0xffff;
	f->skip(6);

	char buffer[FILENAME_SIZE];
	f->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE - 1] = '\0';
	_backgroundFile = Common::String(buffer);

	for (int i = 0; i < 50; ++i) {
		f->read(buffer, FILENAME_SIZE);
		buffer[FILENAME_SIZE - 1] = '\0';
		if (i < _spriteSetsCount)
			_spriteSetNames.push_back(Common::String(buffer));
	}

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

void AnimFrameEntry::load(Common::SeekableReadStream *f, bool uiFlag) {
	if (uiFlag) {
		f->skip(2);
		_frameNumber = -1;		// Unused
		_seqIndex = f->readByte();
		_spriteSlot._spritesIndex = f->readByte();
		_spriteSlot._frameNumber = (int8)f->readByte();
		f->skip(1);
		_spriteSlot._position.x = f->readSint16LE();
		_spriteSlot._position.y = f->readSint16LE();
	} else {
		_frameNumber = f->readUint16LE();
		if (_frameNumber & 0x8000)
			_frameNumber = -(_frameNumber & 0x7fff);

		_seqIndex = f->readByte();
		_spriteSlot._spritesIndex = f->readByte();
		_spriteSlot._frameNumber = f->readUint16LE();
		if (_spriteSlot._frameNumber & 0x8000)
			_spriteSlot._frameNumber = -(_spriteSlot._frameNumber & 0x7fff);

		_spriteSlot._position.x = f->readSint16LE();
		_spriteSlot._position.y = f->readSint16LE();
		_spriteSlot._depth = f->readSByte();
		_spriteSlot._scale = (int8)f->readByte();
	}
}

/*------------------------------------------------------------------------*/

void AnimMiscEntry::load(Common::SeekableReadStream *f) {
	_soundId = f->readByte();
	_msgIndex = f->readSByte();
	_numTicks = f->readUint16LE();
	_posAdjust.x = f->readSint16LE();
	_posAdjust.y = f->readSint16LE();
	_scroll.x = f->readSByte();
	_scroll.y = f->readSByte();
}

/*------------------------------------------------------------------------*/

void AnimUIEntry::load(Common::SeekableReadStream *f) {
	_probability = f->readUint16LE();
	_imageCount = f->readUint16LE();
	_firstImage = f->readUint16LE();
	_lastImage = f->readUint16LE();
	_counter = f->readSint16LE();
	for (int i = 0; i < ANIM_SPAWN_COUNT; ++i)
		_spawn[i] = f->readByte();
	for (int i = 0; i < ANIM_SPAWN_COUNT; ++i)
		_spawnFrame[i] = f->readUint16LE();
	_sound = f->readUint16LE() & 0xFF;
	_soundFrame = f->readUint16LE();
}

/*------------------------------------------------------------------------*/

Animation *Animation::init(MADSEngine *vm, Scene *scene) {
	return new Animation(vm, scene);
}

Animation::Animation(MADSEngine *vm, Scene *scene) : _vm(vm), _scene(scene) {
	_flags = 0;
	_font = nullptr;
	_resetFlag = false;
	_canChangeView = false;
	_messageCtr = 0;
	_skipLoad = false;
	_freeFlag = false;
	_unkIndex = -1;
	_nextFrameTimer = 0;
	_nextScrollTimer = 0;
	_trigger = 0;
	_triggerMode = SEQUENCE_TRIGGER_PREPARE;
	_actionDetails._verbId = VERB_NONE;
	_actionDetails._objectNameId = -1;
	_actionDetails._indirectObjectId = -1;
	_currentFrame = 0;
	_oldFrameEntry = 0;
	_rgbResult = -1;
	_palIndex1 = _palIndex2 = -1;
	_dynamicHotspotIndex = -1;
}

Animation::~Animation() {
	Scene &scene = _vm->_game->_scene;

	if (_header._manualFlag)
		scene._sprites.remove(_spriteListIndexes[_header._spritesIndex]);

	for (int idx = 0; idx < _header._spriteSetsCount; ++idx) {
		if (!_header._manualFlag || _header._spritesIndex != idx)
			scene._sprites.remove(_spriteListIndexes[idx]);
	}
}

void Animation::load(MSurface &backSurface, DepthSurface &depthSurface,
		const Common::String &resName, int flags, Common::Array<PaletteCycle> *palCycles,
		SceneInfo *sceneInfo) {
	Common::String resourceName = resName;
	if (!resourceName.contains("."))
		resourceName += ".AA";

	File f(resourceName);
	MadsPack madsPack(&f);

	Common::SeekableReadStream *stream = madsPack.getItemStream(0);
	_header.load(stream);
	delete stream;

	if (_header._bgType == ANIMBG_INTERFACE)
		flags |= PALFLAG_RESERVED;
	_flags = flags;

	if (flags & ANIMFLAG_LOAD_BACKGROUND) {
		loadBackground(backSurface, depthSurface, _header, flags, palCycles, sceneInfo);
	}
	if (flags & ANIMFLAG_LOAD_BACKGROUND_ONLY) {
		// No data
		_header._messagesCount = 0;
		_header._frameEntriesCount = 0;
		_header._miscEntriesCount = 0;
	}

	// Initialize the reference list
	_spriteListIndexes.clear();
	for (int i = 0; i < _header._spriteSetsCount; ++i)
		_spriteListIndexes.push_back(-1);

	int streamIndex = 1;
	_messages.clear();
	if (_header._messagesCount > 0) {
		// Chunk 2: Following is a list of any messages for the animation
		Common::SeekableReadStream *msgStream = madsPack.getItemStream(streamIndex++);

		for (int i = 0; i < _header._messagesCount; ++i) {
			AnimMessage rec;
			rec.load(msgStream);
			_messages.push_back(rec);
		}

		delete msgStream;
	}

	_frameEntries.clear();
	if (_header._frameEntriesCount > 0) {
		// Chunk 3: animation frame info
		Common::SeekableReadStream *frameStream = madsPack.getItemStream(streamIndex++);

		for (int i = 0; i < _header._frameEntriesCount; i++) {
			AnimFrameEntry rec;
			rec.load(frameStream, _header._bgType == ANIMBG_INTERFACE);
			_frameEntries.push_back(rec);
		}

		delete frameStream;
	}

	_miscEntries.clear();
	_uiEntries.clear();
	if (_header._miscEntriesCount > 0) {
		// Chunk 4: Misc Data
		Common::SeekableReadStream *miscStream = madsPack.getItemStream(streamIndex++);

		if (_header._bgType == ANIMBG_INTERFACE) {
			for (int i = 0; i < _header._miscEntriesCount; ++i) {
				AnimUIEntry rec;
				rec.load(miscStream);
				_uiEntries.push_back(rec);
			}
		} else {
			for (int i = 0; i < _header._miscEntriesCount; ++i) {
				AnimMiscEntry rec;
				rec.load(miscStream);
				_miscEntries.push_back(rec);
			}
		}

		delete miscStream;
	}

	// If the animation specifies a font, then load it for access
	delete _font;
	if (_header._loadFlags & ANIMFLAG_CUSTOM_FONT) {
		Common::String fontName = "*" + _header._fontResource;
		_font = _vm->_font->getFont(fontName.c_str());
	} else {
		_font = nullptr;
	}

	// Load all the sprite sets for the animation
	for (uint i = 0; i < _spriteSets.size(); ++i)
		delete _spriteSets[i];
	_spriteSets.clear();
	_spriteSets.resize(_header._spriteSetsCount);

	for (int i = 0; i < _header._spriteSetsCount; ++i) {
		if (_header._manualFlag && (i == _header._spritesIndex)) {
			// Skip over field, since it's manually loaded
			_spriteSets[i] = nullptr;
		} else {
			_spriteSets[i] = new SpriteAsset(_vm, _header._spriteSetNames[i], flags);
			_spriteListIndexes[i] = _vm->_game->_scene._sprites.add(_spriteSets[i]);
		}
	}

	if (_header._manualFlag) {
		Common::String assetResName = "*" + _header._spriteSetNames[_header._spritesIndex];
		SpriteAsset *sprites = new SpriteAsset(_vm, assetResName, flags);
		_spriteSets[_header._spritesIndex] = sprites;

		_spriteListIndexes[_header._spritesIndex] = _scene->_sprites.add(sprites);
	}

	Common::Array<int> usageList;
	for (int idx = 0; idx < _header._spriteSetsCount; ++idx)
		usageList.push_back(_spriteSets[idx]->_usageIndex);

	if (usageList.size() > 0) {
		int spritesUsageIndex = _spriteSets[0]->_usageIndex;
		_vm->_palette->_paletteUsage.updateUsage(usageList, spritesUsageIndex);
	}

	// Remaps the sprite list indexes for frames to the loaded sprite list indexes
	for (uint i = 0; i < _frameEntries.size(); ++i) {
		int spriteListIndex = _frameEntries[i]._spriteSlot._spritesIndex;
		_frameEntries[i]._spriteSlot._spritesIndex = _spriteListIndexes[spriteListIndex];
	}

	f.close();
}

void Animation::preLoad(const Common::String &resName, int level) {
	// No implementation in ScummVM, since access is fast enough that data
	// doesn't need to be preloaded
}

void Animation::startAnimation(int endTrigger) {
	_messageCtr = 0;
	_skipLoad = true;

	if (_header._manualFlag) {
		_unkIndex = -1;
		//SpriteAsset *asset = _scene->_sprites[_spriteListIndexes[_header._spritesIndex]];

		loadFrame(1);
	}

	if (_vm->_game->_kernelMode == KERNEL_ACTIVE_CODE)
		_vm->_palette->refreshSceneColors();

	_currentFrame = 0;
	_oldFrameEntry = 0;
	_nextFrameTimer = _vm->_game->_scene._frameStartTime;
	_trigger = endTrigger;
	_triggerMode = _vm->_game->_triggerSetupMode;
	_actionDetails = _vm->_game->_scene._action._activeAction;

	for (int idx = 0; idx < _header._messagesCount; ++idx) {
		_messages[idx]._kernelMsgIndex = -1;
	}
}

void Animation::loadFrame(int frameNumber) {
	Scene &scene = _vm->_game->_scene;
	if (_skipLoad)
		return;

	Common::Point pt;
	int spriteListIndex = _spriteListIndexes[_header._spritesIndex];
	SpriteAsset &spriteSet = *scene._sprites[spriteListIndex];

	if (_unkIndex < 0) {
		MSurface *frame = spriteSet.getFrame(0);
		pt.x = frame->getBounds().left;
		pt.y = frame->getBounds().top;
	} else {
		pt.x = _unkList[_unkIndex].x;
		pt.y = _unkList[_unkIndex].y;
		_unkIndex = 1 - _unkIndex;
		warning("LoadFrame - Using unknown array");
	}

	if (drawFrame(spriteSet, pt, frameNumber))
		error("drawFrame failure");
}

bool Animation::drawFrame(SpriteAsset &spriteSet, const Common::Point &pt, int frameNumber) {
	return 0;
}

void Animation::loadBackground(MSurface &backSurface, DepthSurface &depthSurface,
		AAHeader &header, int flags, Common::Array<PaletteCycle> *palCycles, SceneInfo *sceneInfo) {
	_scene->_depthStyle = 0;
	if (header._bgType <= ANIMBG_FULL_SIZE) {
		_vm->_palette->_paletteUsage.setEmpty();
		sceneInfo->load(header._roomNumber, 0, header._backgroundFile, flags, depthSurface, backSurface);
		_scene->_depthStyle = sceneInfo->_depthStyle == 2 ? 1 : 0;
		if (palCycles) {
			palCycles->clear();
			for (uint i = 0; i < sceneInfo->_paletteCycles.size(); ++i)
				palCycles->push_back(sceneInfo->_paletteCycles[i]);
		}
	} else if (header._bgType == ANIMBG_INTERFACE) {
		// Load a scene interface
		Common::String resourceName = "*" + header._backgroundFile;
		backSurface.load(resourceName);

		if (palCycles)
			palCycles->clear();
	} else {
		// Original has useless code here
	}
}

bool Animation::hasScroll() const {
	return (_header._scrollPosition.x != 0) || (_header._scrollPosition.y != 0);
}

void Animation::update() {
	Scene &scene = _vm->_game->_scene;
	Palette &palette = *_vm->_palette;

	if (_header._manualFlag) {
		int spriteListIndex = _spriteListIndexes[_header._spritesIndex];
		int newIndex = -1;

		for (uint idx = _oldFrameEntry; idx < _frameEntries.size(); ++idx) {
			if (_frameEntries[idx]._frameNumber > _currentFrame)
				break;
			if (_frameEntries[idx]._spriteSlot._spritesIndex == spriteListIndex)
				newIndex = _frameEntries[idx]._spriteSlot._frameNumber;
		}

		if (newIndex >= 0)
			loadFrame(newIndex);
	}

	// If it's not time for the next frame, then exit
	if (_vm->_game->_scene._frameStartTime < _nextFrameTimer)
		return;

	// Erase any active sprites
	eraseSprites();

	// Validate the current frame
	if (_currentFrame >= (int)_miscEntries.size()) {
		// Is the animation allowed to be repeated?
		if (_resetFlag) {
			_currentFrame = 0;
			_oldFrameEntry = 0;
		} else {
			_freeFlag = true;
			return;
		}
	}

	// Handle executing any sound command for this frame
	AnimMiscEntry &misc = _miscEntries[_currentFrame];
	if (misc._soundId)
		_vm->_sound->command(misc._soundId);

	// Handle any screen scrolling
	if (hasScroll()) {
		scene._backgroundSurface.scrollX(_header._scrollPosition.x);
		scene._backgroundSurface.scrollY(_header._scrollPosition.y);
		scene._spriteSlots.fullRefresh();
	}

	bool isV2 = (_vm->getGameID() != GType_RexNebular);
	if (isV2 && _canChangeView) {
		// Handle any offset adjustment for sprites as of this frame
		bool paChanged = false;
		if (getFramePosAdjust(_currentFrame).x != scene._posAdjust.x) {
			scene._posAdjust.x = getFramePosAdjust(_currentFrame).x;
			paChanged = true;
		}

		if (getFramePosAdjust(_currentFrame).y != scene._posAdjust.y) {
			scene._posAdjust.y = getFramePosAdjust(_currentFrame).y;
			paChanged = true;
		}

		if (paChanged) {
			int newIndex = scene._spriteSlots.add();
			scene._spriteSlots[newIndex]._seqIndex = -1;
			scene._spriteSlots[newIndex]._flags = IMG_REFRESH;
		}
	}

	// Main frame animation loop - frames get animated by being placed, as necessary, into the
	// main sprite slot array
	while ((uint)_oldFrameEntry < _frameEntries.size()) {
		if (_frameEntries[_oldFrameEntry]._frameNumber > _currentFrame)
			break;
		else if (_frameEntries[_oldFrameEntry]._frameNumber == _currentFrame) {
			// Found the correct frame
			int spriteSlotIndex = 0;
			int index = 0;

			for (;;) {
				if ((spriteSlotIndex == 0) && (index < (int)scene._spriteSlots.size())) {
					int seqIndex = _frameEntries[_oldFrameEntry]._seqIndex - scene._spriteSlots[index]._seqIndex;
					if (seqIndex == 0x80) {
						if (scene._spriteSlots[index] == _frameEntries[_oldFrameEntry]._spriteSlot) {
							scene._spriteSlots[index]._flags = IMG_STATIC;
							spriteSlotIndex = -1;
						}
					}
					++index;
					continue;
				}

				if (spriteSlotIndex == 0) {
					int slotIndex = scene._spriteSlots.add();
					SpriteSlot &slot = scene._spriteSlots[slotIndex];
					slot.copy(_frameEntries[_oldFrameEntry]._spriteSlot);
					slot._seqIndex = _frameEntries[_oldFrameEntry]._seqIndex + 0x80;

					SpriteAsset &spriteSet = *scene._sprites[
						scene._spriteSlots[slotIndex]._spritesIndex];
					slot._flags = spriteSet.isBackground() ? IMG_DELTA : IMG_UPDATE;
				}
				break;
			}
		}

		++_oldFrameEntry;
	}

	// Handle the display of any messages
	for (uint idx = 0; idx < _messages.size(); ++idx) {
		if (_messages[idx]._kernelMsgIndex >= 0) {
			// Handle currently active message
			if ((_currentFrame < _messages[idx]._startFrame) || (_currentFrame > _messages[idx]._endFrame)) {
				scene._kernelMessages.remove(_messages[idx]._kernelMsgIndex);
				_messages[idx]._kernelMsgIndex = -1;
				--_messageCtr;
			}
		} else if ((_currentFrame >= _messages[idx]._startFrame) && (_currentFrame <= _messages[idx]._endFrame)) {
			// Start displaying the message
			AnimMessage &me = _messages[idx];

			if (_flags & ANIMFLAG_ANIMVIEW) {
				_rgbResult = palette._paletteUsage.checkRGB(me._rgb1, -1, true, &_palIndex1);
				_rgbResult = palette._paletteUsage.checkRGB(me._rgb2, _rgbResult, true, &_palIndex2);

				// Update the palette with the two needed colors
				int palStart = MIN(_palIndex1, _palIndex2);
				int palCount = ABS(_palIndex2 - _palIndex1) + 1;
				palette.setPalette(&palette._mainPalette[palStart * 3], palStart, palCount);
			} else {
				// The color index to use is dependant on how many messages are currently on-screen
				switch (_messageCtr) {
				case 1:
					_palIndex1 = 252;
					break;
				case 2:
					_palIndex1 = 16;
					break;
				default:
					_palIndex1 = 250;
					break;
				}
				_palIndex2 = _palIndex1 + 1;

				_vm->_palette->setEntry(_palIndex1, me._rgb1[0], me._rgb1[1], me._rgb1[2]);
				_vm->_palette->setEntry(_palIndex2, me._rgb2[0], me._rgb2[1], me._rgb2[2]);
			}

			// Add a kernel message to display the given text
			me._kernelMsgIndex = scene._kernelMessages.add(me._pos,
				_palIndex1 | (_palIndex2 << 8),
				0, 0, INDEFINITE_TIMEOUT, me._msg);
			assert(me._kernelMsgIndex >= 0);
			++_messageCtr;

			// If there's an accompanying sound, also play it
			if (me._soundId > 0)
				_vm->_audio->playSound(me._soundId - 1);
		}
	}

	// Move to the next frame
	_currentFrame++;
	if (_currentFrame >= (int)_miscEntries.size()) {
		// Animation is complete
		if (_trigger != 0) {
			_vm->_game->_trigger = _trigger;
			_vm->_game->_triggerMode = _triggerMode;

			if (_triggerMode != SEQUENCE_TRIGGER_DAEMON) {
				// Copy the noun list
				scene._action._activeAction = _actionDetails;
			}
		}
	}

	int frameNum = MIN(_currentFrame, (int)_miscEntries.size() - 1);
	_nextFrameTimer = _vm->_game->_scene._frameStartTime + _miscEntries[frameNum]._numTicks;
}

void Animation::setCurrentFrame(int frameNumber) {
	_currentFrame = frameNumber;
	_oldFrameEntry = 0;
	_freeFlag = false;
}

void Animation::setNextFrameTimer(uint32 newTimer) {
	_nextFrameTimer = newTimer;
}

void Animation::eraseSprites() {
	Scene &scene = _vm->_game->_scene;

	for (uint idx = 0; idx < scene._spriteSlots.size(); ++idx) {
		if (scene._spriteSlots[idx]._seqIndex >= 0x80)
			scene._spriteSlots[idx]._flags = IMG_ERASE;
	}
}

Common::Point Animation::getFramePosAdjust(int idx) {
	warning("TODO: Implement getFramePosAdjust");

	return Common::Point(0, 0);
}
} // End of namespace MADS
