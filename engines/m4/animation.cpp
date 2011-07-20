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

#include "common/textconsole.h"

#include "m4/assets.h"
#include "m4/animation.h"
#include "m4/compression.h"
#include "m4/mads_scene.h"

namespace M4 {

// TODO: this code needs cleanup

MadsAnimation::MadsAnimation(MadsM4Engine *vm, MadsView *view): Animation(vm), _view(view) {
	_font = NULL;
	_resetFlag = false;
	_freeFlag = false;
	_skipLoad = false;
	_unkIndex = -1;
	_messageCtr= 0;
	_field12 = 0;

	_currentFrame = 0;
	_oldFrameEntry = 0;
	_nextFrameTimer = _madsVm->_currentTimer;
	_nextScrollTimer = 0;
}

MadsAnimation::~MadsAnimation() {
	for (uint i = 0; i < _messages.size(); ++i) {
		if (_messages[i].kernelMsgIndex >= 0)
			_view->_kernelMessages.remove(_messages[i].kernelMsgIndex);
	}

	// Further deletion logic
	if (_field12) {
		_view->_spriteSlots.deleteSprites(_spriteListIndexes[_spriteListIndex]);
	}
}

#define FILENAME_SIZE 13

/**
 * Initializes and loads the data of an animation
 */
void MadsAnimation::initialize(const Common::String &filename, uint16 flags, M4Surface *surface, M4Surface *depthSurface) {
	MadsPack anim(filename.c_str(), _vm);
	bool madsRes = filename[0] == '*';
	char buffer[20];
	int streamIndex = 1;

	// Chunk 1: header
	// header

	Common::SeekableReadStream *animStream = anim.getItemStream(0);

	int spriteListCount = animStream->readUint16LE();
	int miscEntriesCount = animStream->readUint16LE();
	int frameEntryCount = animStream->readUint16LE();
	int messagesCount = animStream->readUint16LE();
	animStream->skip(1);
	_flags = animStream->readByte();

	animStream->skip(2);
	_animMode = animStream->readUint16LE();
	_roomNumber = animStream->readUint16LE();
	animStream->skip(2);
	_field12 = animStream->readUint16LE() != 0;
	_spriteListIndex = animStream->readUint16LE();
	_scrollX = animStream->readSint16LE();
	_scrollY = animStream->readSint16LE();
	_scrollTicks = animStream->readUint16LE();
	animStream->skip(8);

	animStream->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE] = '\0';
	_interfaceFile = Common::String(buffer);

	for (int i = 0; i < 10; ++i) {
		animStream->read(buffer, FILENAME_SIZE);
		buffer[FILENAME_SIZE] = '\0';
		_spriteSetNames[i] = Common::String(buffer);
	}

	animStream->skip(81);
	animStream->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE] = '\0';
	_lbmFilename = Common::String(buffer);

	animStream->skip(365);
	animStream->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE] = '\0';
	_spritesFilename = Common::String(buffer);

	animStream->skip(48);
	animStream->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE] = '\0';
	_soundName = Common::String(buffer);

	animStream->skip(13);
	animStream->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE] = '\0';
	_dsrName = Common::String(buffer);

	animStream->read(buffer, FILENAME_SIZE);
	buffer[FILENAME_SIZE] = '\0';
	Common::String fontResource(buffer);

	if (_animMode == 4)
		flags |= 0x4000;
	if (flags & 0x100)
		loadInterface(surface, depthSurface);

	// Initialize the reference list
	for (int i = 0; i < spriteListCount; ++i)
		_spriteListIndexes.push_back(-1);

	delete animStream;

	if (messagesCount > 0) {
		// Chunk 2
		// Following is a list of any messages for the animation

		animStream = anim.getItemStream(streamIndex++);

		for (int i = 0; i < messagesCount; ++i) {
			AnimMessage rec;
			rec.soundId = animStream->readSint16LE();
			animStream->read(rec.msg, 64);
			animStream->skip(4);
			rec.pos.x = animStream->readSint16LE();
			rec.pos.y = animStream->readSint16LE();
			rec.flags = animStream->readUint16LE();
			rec.rgb1.r = animStream->readByte() << 2;
			rec.rgb1.g = animStream->readByte() << 2;
			rec.rgb1.b = animStream->readByte() << 2;
			rec.rgb2.r = animStream->readByte() << 2;
			rec.rgb2.g = animStream->readByte() << 2;
			rec.rgb2.b = animStream->readByte() << 2;
			animStream->skip(2);	// Space for kernelMsgIndex
			rec.kernelMsgIndex = -1;
			animStream->skip(6);
			rec.startFrame = animStream->readUint16LE();
			rec.endFrame = animStream->readUint16LE();
			animStream->skip(2);

			_messages.push_back(rec);
		}

		delete animStream;
	}

	if (frameEntryCount > 0) {
		// Chunk 3: animation frame info
		animStream = anim.getItemStream(streamIndex++);

		for (int i = 0; i < frameEntryCount; i++) {
			AnimFrameEntry rec;
			rec.frameNumber = animStream->readUint16LE();
			rec.seqIndex = animStream->readByte();
			rec.spriteSlot.spriteListIndex = animStream->readByte();
			rec.spriteSlot.frameNumber = animStream->readUint16LE();
			rec.spriteSlot.xp = animStream->readSint16LE();
			rec.spriteSlot.yp = animStream->readSint16LE();
			rec.spriteSlot.depth = animStream->readSByte();
			rec.spriteSlot.scale = (int8)animStream->readByte();

			_frameEntries.push_back(rec);
		}

		delete animStream;
	}

	if (miscEntriesCount > 0) {
		// Chunk 4: Misc Data
		animStream = anim.getItemStream(streamIndex);

		for (int i = 0; i < miscEntriesCount; ++i) {
			AnimMiscEntry rec;
			rec.soundNum = animStream->readByte();
			rec.msgIndex = animStream->readSByte();
			rec.numTicks = animStream->readUint16LE();
			rec.posAdjust.x = animStream->readUint16LE();
			rec.posAdjust.y = animStream->readUint16LE();
			animStream->readUint16LE();

			_miscEntries.push_back(rec);
		}

		delete animStream;
	}

	// If the animation specifies a font, then load it for access
	if (_flags & ANIM_CUSTOM_FONT) {
		Common::String fontName;
		if (madsRes)
			fontName += "*";
		fontName += fontResource;

		if (fontName != "")
			_font = _vm->_font->getFont(fontName.c_str());
		else
			warning("Attempted to set a font with an empty name");
	}

	// If a speech file is specified, then load it
	if (!_dsrName.empty())
		_vm->_sound->loadDSRFile(_dsrName.c_str());

	// Load all the sprite sets for the animation
	for (int i = 0; i < spriteListCount; ++i) {
		if (_field12 && (i == _spriteListIndex))
			// Skip over field, since it's manually loaded
			continue;

		_spriteListIndexes[i] = _view->_spriteSlots.addSprites(_spriteSetNames[i].c_str());
	}


	if (_field12) {
		Common::String resName;
		if (madsRes)
			resName += "*";
		resName += _spriteSetNames[_spriteListIndex];

		_spriteListIndexes[_spriteListIndex] = _view->_spriteSlots.addSprites(resName.c_str());
	}

	// TODO: Unknown section about handling sprite set list combined with messages size

	// TODO: The original has two separate loops for the loop below based on _animMode == 4. Is it
	// perhaps that in that mode the sprite frames has a different format..?

	// Remap the sprite list index fields from the initial value to the indexes of the loaded
	// sprite sets for the animation
	for (uint i = 0; i < _frameEntries.size(); ++i)  {
		int idx = _frameEntries[i].spriteSlot.spriteListIndex;
		_frameEntries[i].spriteSlot.spriteListIndex = _spriteListIndexes[idx];
	}

	if (hasScroll())
		_nextScrollTimer = _madsVm->_currentTimer + _scrollTicks;
}

/**
 * Loads an animation file for display
 */
void MadsAnimation::load(const Common::String &filename, int abortTimers) {
	initialize(filename, 0, NULL, NULL);
	_messageCtr = 0;
	_skipLoad = true;

/* TODO:  figure out extra stuff in this routine
	if (_field12) {
		_unkIndex = -1;
		int listIndex = _spriteListIndexes[_spriteListIndex];
		SpriteAsset &spriteSet = _view->_spriteSlots.getSprite(listIndex);
		..?..
	}
*/

	// Initialize miscellaneous fields
	_currentFrame = 0;
	_oldFrameEntry = 0;
	_nextFrameTimer = _madsVm->_currentTimer;
	_abortTimers = abortTimers;
	_abortMode = _madsVm->scene()->_abortTimersMode2;

	if (_madsVm->_scene)
		_actionNouns = _madsVm->scene()->_action._action;

	// Initialize kernel message list
	for (uint i = 0; i < _messages.size(); ++i)
		_messages[i].kernelMsgIndex = -1;
}

void MadsAnimation::update() {
	if (_field12) {
		int spriteListIndex = _spriteListIndexes[_spriteListIndex];
		int newIndex = -1;

		for (uint idx = _oldFrameEntry; idx < _frameEntries.size(); ++idx) {
			if (_frameEntries[idx].frameNumber > _currentFrame)
				break;
			if (_frameEntries[idx].spriteSlot.spriteListIndex == spriteListIndex)
				newIndex = _frameEntries[idx].spriteSlot.frameNumber;
		}

		if (newIndex >= 0)
			load1(newIndex);
	}

	// Check for scroll change
	bool screenChanged = false;

	// Handle any scrolling of the screen surface
	if (hasScroll() && (_madsVm->_currentTimer >= _nextScrollTimer)) {
		_view->_bgSurface->scrollX(_scrollX);
		_view->_bgSurface->scrollY(_scrollY);

		_nextScrollTimer = _madsVm->_currentTimer + _scrollTicks;
		screenChanged = true;
	}

	// If it's not time for the next frame, then exit
	if (_madsVm->_currentTimer < _nextFrameTimer) {
		if (screenChanged)
			_view->_spriteSlots.fullRefresh();
		return;
	}

	// Loop checks for any prior animation sprite slots to be expired
	for (int slotIndex = 0; slotIndex < _view->_spriteSlots.startIndex; ++slotIndex) {
		if (_view->_spriteSlots[slotIndex].seqIndex >= 0x80) {
			// Flag the frame as animation sprite slot
			_view->_spriteSlots[slotIndex].spriteType = EXPIRED_SPRITE;
		}
	}

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

	// Handle starting any sound for this frame
	AnimMiscEntry &misc = _miscEntries[_currentFrame];
	if (misc.soundNum)
		_vm->_sound->playSound(misc.soundNum);

	// Handle any offset adjustment for sprites as of this frame
	if (_view->_posAdjust.x != misc.posAdjust.x) {
		_view->_posAdjust.x = misc.posAdjust.x;
		screenChanged = true;
	}
	if (_view->_posAdjust.y != misc.posAdjust.y) {
		_view->_posAdjust.y = misc.posAdjust.y;
		screenChanged = true;
	}


	if (screenChanged) {
		// Signal the entire screen needs refreshing
		_view->_spriteSlots.fullRefresh();
	}

	int spriteSlotsMax = _view->_spriteSlots.startIndex;

	// Main frame animation loop - frames get animated by being placed, as necessary, into the
	// main sprite slot array
	while ((uint)_oldFrameEntry < _frameEntries.size()) {
		if (_frameEntries[_oldFrameEntry].frameNumber > _currentFrame)
			break;
		else if (_frameEntries[_oldFrameEntry].frameNumber == _currentFrame) {
			// Found the correct frame
			int spriteSlotIndex = 0;
			int index = 0;

			for (;;) {
				if ((spriteSlotIndex == 0) && (index < spriteSlotsMax)) {
					int seqIndex = _frameEntries[_oldFrameEntry].seqIndex - _view->_spriteSlots[index].seqIndex;
					if (seqIndex == 0x80) {
						if (_view->_spriteSlots[index] == _frameEntries[_oldFrameEntry].spriteSlot) {
							_view->_spriteSlots[index].spriteType = SPRITE_ZERO;
							spriteSlotIndex = -1;
						}
					}
					++index;
					continue;
				}

				if (spriteSlotIndex == 0) {
					int slotIndex = _view->_spriteSlots.getIndex();
					MadsSpriteSlot &slot = _view->_spriteSlots[slotIndex];
					slot.copy(_frameEntries[_oldFrameEntry].spriteSlot);
					slot.seqIndex = _frameEntries[_oldFrameEntry].seqIndex + 0x80;

					SpriteAsset &spriteSet = _view->_spriteSlots.getSprite(
						_view->_spriteSlots[slotIndex].spriteListIndex);
					slot.spriteType = spriteSet.isBackground() ? BACKGROUND_SPRITE : FOREGROUND_SPRITE;
				}
				break;
			}
		}

		++_oldFrameEntry;
	}

	// Handle the display of any messages
	for (uint idx = 0; idx < _messages.size(); ++idx) {
		if (_messages[idx].kernelMsgIndex >= 0) {
			// Handle currently active message
			if ((_currentFrame < _messages[idx].startFrame) || (_currentFrame > _messages[idx].endFrame)) {
				_view->_kernelMessages.remove(_messages[idx].kernelMsgIndex);
				_messages[idx].kernelMsgIndex = -1;
				--_messageCtr;
			}
		} else if ((_currentFrame >= _messages[idx].startFrame) && (_currentFrame <= _messages[idx].endFrame)) {
			// Start displaying the message
			AnimMessage &me = _messages[idx];

			// The color index to use is dependant on how many messages are currently on-screen
			uint8 colIndex;
			switch (_messageCtr) {
			case 1:
				colIndex = 252;
				break;
			case 2:
				colIndex = 16;
				break;
			default:
				colIndex = 250;
				break;
			}

			_vm->_palette->setEntry(colIndex, me.rgb1.r, me.rgb1.g, me.rgb1.b);
			_vm->_palette->setEntry(colIndex + 1, me.rgb2.r, me.rgb2.g, me.rgb2.b);

			// Add a kernel message to display the given text
			me.kernelMsgIndex = _view->_kernelMessages.add(me.pos, colIndex * 0x101 + 0x100, 0, 0, INDEFINITE_TIMEOUT, me.msg);
			assert(me.kernelMsgIndex >= 0);

			// Play the associated sound, if it exists
			if (me.soundId > 0)
				_vm->_sound->playDSRSound(me.soundId - 1, 255, false);
			++_messageCtr;
		}
	}

	// Move to the next frame
	_currentFrame++;
	if (_currentFrame >= (int)_miscEntries.size()) {
		// Animation is complete
		if (_abortTimers != 0) {
			_view->_abortTimers = _abortTimers;
			_view->_abortTimersMode = _abortMode;

			if (_abortMode != ABORTMODE_1) {
				// Copy the noun list
				if (_madsVm->_scene)
					_madsVm->scene()->_action._action = _actionNouns;
			}
		}
	}

	int frameNum = MIN(_currentFrame, (int)_miscEntries.size() - 1);
	_nextFrameTimer = _madsVm->_currentTimer + _miscEntries[frameNum].numTicks;
}

void MadsAnimation::setCurrentFrame(int frameNumber) {
	_currentFrame = frameNumber;
	_oldFrameEntry = 0;
	_freeFlag = false;

	_nextScrollTimer = _nextFrameTimer = _madsVm->_currentTimer;
}

int MadsAnimation::getCurrentFrame() {
	return _currentFrame;
}

void MadsAnimation::load1(int frameNumber) {
	if (_skipLoad)
		return;

	Common::Point pt;
	int listIndex = _spriteListIndexes[_spriteListIndex];
	SpriteAsset &spriteSet = _view->_spriteSlots.getSprite(listIndex);

	if (_unkIndex < 0) {
		M4Surface *frame = spriteSet.getFrame(0);
		pt.x = frame->bounds().left;
		pt.y = frame->bounds().top;
	} else {
		pt.x = _unkList[_unkIndex].x;
		pt.y = _unkList[_unkIndex].y;
		_unkIndex = 1 - _unkIndex;
	}

	if (proc1(spriteSet, pt, frameNumber))
		error("proc1 failure");
}

bool MadsAnimation::proc1(SpriteAsset &spriteSet, const Common::Point &pt, int frameNumber) {
	return 0;
}

void MadsAnimation::loadInterface(M4Surface *&interfaceSurface, M4Surface *&depthSurface) {
	if (_animMode <= 2) {
		MadsSceneResources sceneResources;
		sceneResources.load(_roomNumber, _interfaceFile.c_str(), 0, depthSurface, interfaceSurface);

	} else if (_animMode == 4) {
		// Load a scene interface
		interfaceSurface->madsLoadInterface(_interfaceFile);
	} else {
		// This mode allocates two large surfaces for the animation
		// TODO: Are these ever properly freed?
error("Anim mode %d - need to check free logic", _animMode);
		assert(!interfaceSurface);
		assert(!depthSurface);
		depthSurface = new M4Surface(MADS_SURFACE_WIDTH, MADS_SCREEN_HEIGHT);
		interfaceSurface = new M4Surface(MADS_SURFACE_WIDTH, MADS_SCREEN_HEIGHT);
		depthSurface->clear();
		interfaceSurface->clear();
	}
}

} // End of namespace M4
