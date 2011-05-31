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

#ifndef M4_ANIMATION_H
#define M4_ANIMATION_H

#include "m4/m4.h"
#include "m4/graphics.h"
#include "m4/assets.h"
#include "m4/mads_views.h"
#include "common/array.h"

namespace M4 {

class MadsView;
class SpriteSlotSubset;

class AnimMessage {
public:
	int16 soundId;
	char msg[64];
	Common::Point pos;
	RGB8 rgb1, rgb2;
	uint16 flags;
	int startFrame, endFrame;
	int kernelMsgIndex;
};

class AnimFrameEntry {
public:
	int frameNumber;
	int seqIndex;
	SpriteSlotSubset spriteSlot;
};

class AnimMiscEntry {
public:
	int soundNum;
	int msgIndex;
	int numTicks;
	Common::Point posAdjust;
};

#define ANIM_SPRITE_SET_SIZE 50

enum MadsAnimationFlags {ANIM_CUSTOM_FONT = 0x20, ANIM_HAS_SOUND = 0x8000};

class MadsAnimation: public Animation {
private:
	MadsView *_view;

	int _spriteListCount;
	Common::Array<AnimMessage> _messages;
	Common::Array<AnimFrameEntry> _frameEntries;
	Common::Array<AnimMiscEntry> _miscEntries;
	Font *_font;

	uint8 _flags;
	int _animMode;
	int _roomNumber;
	bool _field12;
	int _spriteListIndex;
	int _scrollX;
	int _scrollY;
	int _scrollTicks;
	Common::String _interfaceFile;
	Common::String _spriteSetNames[10];
	Common::String _lbmFilename;
	Common::String _spritesFilename;
	Common::String _soundName;
	Common::String _dsrName;
	Common::Array<int> _spriteListIndexes;

	int _currentFrame, _oldFrameEntry;
	bool _resetFlag;
	bool _freeFlag;
	bool _skipLoad;
	int _unkIndex;
	Common::Point _unkList[2];
	uint32 _nextFrameTimer;
	uint32 _nextScrollTimer;
	int _messageCtr;
	int _abortTimers;
	AbortTimerMode _abortMode;
	ActionDetails _actionNouns;

	void load1(int frameNumber);
	bool proc1(SpriteAsset &spriteSet, const Common::Point &pt, int frameNumber);
	void loadInterface(M4Surface *&interfaceSurface, M4Surface *&depthSurface);
	bool hasScroll() const { return (_scrollX != 0) || (_scrollY != 0); }
public:
	MadsAnimation(MadsM4Engine *vm, MadsView *view);
	virtual ~MadsAnimation();

	virtual void initialize(const Common::String &filename, uint16 flags, M4Surface *surface, M4Surface *depthSurface);
	virtual void load(const Common::String &filename, int abortTimers);
	virtual void update();
	virtual void setCurrentFrame(int frameNumber);
	virtual int getCurrentFrame();

	bool freeFlag() const { return _freeFlag; }
	bool getAnimMode() const { return _animMode; }
	int roomNumber() const { return _roomNumber; }
};

} // End of namespace M4

#endif
