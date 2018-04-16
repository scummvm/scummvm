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

#ifndef GNAP_GAMESYS_H
#define GNAP_GAMESYS_H

#include "gnap/gnap.h"
#include "gnap/resource.h"
#include "common/array.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Gnap {

const int kMaxSequenceItems = 40;
const int kMaxSpriteDrawItems = 30;
const int kMaxSoundIds = 50;
const int kMaxSeqItems = 50;
const int kMaxUpdRects = 20;
const int kMaxGfxItems = 50;
const int kMaxAnimations = 12;

enum {
	kSeqNone		= 0x00,
	kSeqScale		= 0x01, // Enable scaling
	kSeqLoop		= 0x02, // Loop
	kSeqUnk			= 0x04,	// Unknown
	kSeqSyncWait	= 0x08,	// Start if other sequence is done
	kSeqSyncExists	= 0x20	// Start if other sequence exists
};

struct Sequence {
	int32 _sequenceId;
	int32 _id;
	int32 _sequenceId2;
	int32 _id2;
	uint32 _flags;
	int32 _totalDuration;
	int16 _x, _y;
};

struct SpriteDrawItem {
	int _id;
	Common::Rect _rect;
	Graphics::Surface *_surface;
};

struct RemoveSequenceItem {
	int _sequenceId;
	int _id;
	bool _forceFrameReset;
};

struct RemoveSpriteDrawItem {
	int _id;
	Graphics::Surface *_surface;
};

struct GfxItem {
	int _sequenceId;
	int _id;
	int _flags;
	SequenceAnimation *_animation;
	int _currFrameNum;
	int _delayTicks;
	bool _updFlag;
	int _updRectsCount;
	Graphics::Surface *_surface;
	Common::Rect _updRects[kMaxUpdRects];
	SequenceFrame _prevFrame;
	SequenceFrame _currFrame;
	void testUpdRect(const Common::Rect &updRect);
};

struct Animation {
	int _sequenceId;
	int _id;
	int _status;
};

class GameSys {
public:
	GameSys(GnapEngine *vm);
	~GameSys();
	void insertSequence(int sequenceId, int id, int sequenceId2, int id2, int flags, int totalDuration, int16 x, int16 y);
	void insertDirtyRect(const Common::Rect &rect);
	void removeSequence(int sequenceId, int id, bool resetFl);
	void invalidateGrabCursorSprite(int id, Common::Rect &rect, Graphics::Surface *surface1, Graphics::Surface *surface2);
	void requestClear2(bool resetFl);
	void requestClear1();
	void requestRemoveSequence(int sequenceId, int id);
	void waitForUpdate();
	int isSequenceActive(int sequenceId, int id);
	void setBackgroundSurface(Graphics::Surface *surface, int a4, int a5, int a6, int a7);
	void setScaleValues(int a1, int a2, int a3, int a4);
	void insertSpriteDrawItem(Graphics::Surface *surface, int x, int y, int id);
	void removeSpriteDrawItem(Graphics::Surface *surface, int id);
	void drawSpriteToBackground(int x, int y, int resourceId);
	Graphics::Surface *allocSurface(int width, int height);
	Graphics::Surface *createSurface(int resourceId);
	void drawSpriteToSurface(Graphics::Surface *surface, int x, int y, int resourceId);
	void drawTextToSurface(Graphics::Surface *surface, int x, int y, byte r, byte g, byte b, const char *text);
	int getTextHeight(const char *text);
	int getTextWidth(const char *text);
	void fillSurface(Graphics::Surface *surface, int x, int y, int width, int height, byte r, byte g, byte b);
	void setAnimation(int sequenceId, int id, int animationIndex);
	int getAnimationStatus(int animationIndex);
	int getSpriteWidthById(int resourceId);
	int getSpriteHeightById(int resourceId);
	Graphics::Surface *loadBitmap(int resourceId);
	void drawBitmap(int resourceId);
public:
	GnapEngine *_vm;

	Common::Array<Common::Rect> _dirtyRects;

	SpriteDrawItem _newSpriteDrawItems[kMaxSpriteDrawItems];
	int _newSpriteDrawItemsCount;

	RemoveSequenceItem _removeSequenceItems[kMaxSequenceItems];
	int _removeSequenceItemsCount;

	RemoveSpriteDrawItem _removeSpriteDrawItems[kMaxSpriteDrawItems];
	int _removeSpriteDrawItemsCount;

	int _grabSpriteId;
	Common::Rect _grabSpriteRect;
	bool _grabSpriteChanged;
	Graphics::Surface *_grabSpriteSurface1, *_grabSpriteSurface2;

	bool _reqRemoveSequenceItem;
	int _removeSequenceItemSequenceId, _removeSequenceItemValue;

	Common::Array<int> _soundIds;

	////////////////////////////////////////////////////////////////////////////

	Common::Array<Sequence> _seqItems;
	Common::Array<Sequence> _fatSequenceItems;

	GfxItem _gfxItems[kMaxGfxItems];
	int _gfxItemsCount;

	Animation _animations[kMaxAnimations];
	int _animationsCount;

	int _backgroundImageValue3, _backgroundImageValue1;
	int _backgroundImageValue4, _backgroundImageValue2;

	int32 _gameSysClock, _lastUpdateClock;
	bool _animationsDone;


	Graphics::Surface *_backgroundSurface;
	Graphics::Surface *_frontSurface;
	Common::Rect _screenRect;

	Sequence *seqFind(int sequenceId, int id, int *outIndex);
	int seqLocateGfx(int sequenceId, int id, int *outGfxIndex);
	void seqInsertGfx(int index, int duration);
	void seqRemoveGfx(int sequenceId, int id);
	bool updateSequenceDuration(int sequenceId, int id, int *outDuration);
	void updateAnimationsStatus(int sequenceId, int id);

	void restoreBackgroundRect(const Common::Rect &rect);

	void blitSurface32(Graphics::Surface *destSurface, int x, int y, Graphics::Surface *sourceSurface,
		Common::Rect &sourceRect, bool transparent);
	void blitSprite32(Graphics::Surface *destSurface, int x, int y, byte *sourcePixels,
		int sourceWidth, Common::Rect &sourceRect, uint32 *sourcePalette, bool transparent);
	void blitSpriteScaled32(Graphics::Surface *destSurface, Common::Rect &frameRect,
		Common::Rect &destRect, byte *sourcePixels, int sourceWidth, Common::Rect &sourceRect, uint32 *sourcePalette);

	void seqDrawStaticFrame(Graphics::Surface *surface, SequenceFrame &frame, Common::Rect *subRect);
	void seqDrawSpriteFrame(SpriteResource *spriteResource, SequenceFrame &frame, Common::Rect *subRect);

	void drawSprites();
	void updateRect(const Common::Rect &r);
	void updateScreen();

	void handleReqRemoveSequenceItem();
	void handleReqRemoveSequenceItems();
	void handleReqRemoveSpriteDrawItems();
	void fatUpdateFrame();
	void fatUpdate();
	void updatePlaySounds();

};

bool intersectRect(Common::Rect &intersectingRect, const Common::Rect &r1, const Common::Rect &r2);

} // End of namespace Gnap

#endif // GNAP_GAMESYS_H
