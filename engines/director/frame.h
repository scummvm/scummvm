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

#include "graphics/managed_surface.h"

namespace Image {
	class ImageDecoder;
}

namespace Director {

class Sprite;

enum {
	kChannelDataSize = (25 * 50)
};

enum TransitionType {
	kTransNone,
	kTransWipeRight,
	kTransWipeLeft,
	kTransWipeDown,
	kTransWipeUp,
	kTransCenterOutHorizontal,
	kTransEdgesInHorizontal,
	kTransCenterOutVertical,
	kTransEdgesInVertical,
	kTransCenterOutSquare,
	kTransEdgesInSquare,
	kTransPushLeft,
	kTransPushRight,
	kTransPushDown,
	kTransPushUp,
	kTransRevealUp,
	kTransRevealUpRight,
	kTransRevealRight,
	kTransRevealDown,
	kTransRevealDownRight,
	kTransRevealDownLeft,
	kTransRevealLeft,
	kTransRevealUpLeft,
	kTransDissolvePixelsFast,
	kTransDissolveBoxyRects,
	kTransDissolveBoxySquares,
	kTransDissolvePatterns,
	kTransRandomRows,
	kTransRandomColumns,
	kTransCoverDown,
	kTransCoverDownLeft,
	kTransCoverDownRight,
	kTransCoverLeft,
	kTransCoverRight,
	kTransCoverUp,
	kTransCoverUpLeft,
	kTransCoverUpRight,
	kTransTypeVenitianBlind,
	kTransTypeCheckerboard,
	kTransTypeStripsBottomBuildLeft,
	kTransTypeStripsBottomBuildRight,
	kTransTypeStripsLeftBuildDown,
	kTransTypeStripsLeftBuildUp,
	kTransTypeStripsRightBuildDown,
	kTransTypeStripsRightBuildUp,
	kTransTypeStripsTopBuildLeft,
	kTransTypeStripsTopBuildRight,
	kTransZoomOpen,
	kTransZoomClose,
	kTransVerticalBinds,
	kTransDissolveBitsTrans,
	kTransDissolvePixels,
	kTransDissolveBits
};

struct PaletteInfo {
	uint8 firstColor;
	uint8 lastColor;
	uint8 flags;
	uint8 speed;
	uint16 frameCount;
	uint16 cycleCount;
};

struct FrameEntity {
	uint16 spriteId;
	Common::Rect rect;
};


class Frame {
public:
	Frame(DirectorEngine *vm);
	Frame(const Frame &frame);
	~Frame();
	void readChannels(Common::ReadStreamEndian *stream);
	void readChannel(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size);
	void prepareFrame(Score *score);
	uint16 getSpriteIDFromPos(Common::Point pos);
	bool checkSpriteIntersection(uint16 spriteId, Common::Point pos);

	void executeImmediateScripts();

private:
	void playTransition(Score *score);
	void playSoundChannel();
	void renderSprites(Graphics::ManagedSurface &surface, bool renderTrail);
	void renderText(Graphics::ManagedSurface &surface, uint16 spriteId, Common::Rect *textSize);
	void renderShape(Graphics::ManagedSurface &surface, uint16 spriteId);
	void renderButton(Graphics::ManagedSurface &surface, uint16 spriteId);
	void readPaletteInfo(Common::SeekableSubReadStreamEndian &stream);
	void readSprite(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size);
	void readMainChannels(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size);
	Image::ImageDecoder *getImageFrom(uint16 spriteId);
	Common::String readTextStream(Common::SeekableSubReadStreamEndian *textStream, TextCast *textCast);
	void drawBackgndTransSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawMatteSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawGhostSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawReverseSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
	void inkBasedBlit(Graphics::ManagedSurface &targetSurface, const Graphics::Surface &spriteSurface, uint16 spriteId, Common::Rect drawRect);
	void addDrawRect(uint16 entityId, Common::Rect &rect);

public:
	byte _channelData[kChannelDataSize];
	uint8 _actionId;
	uint8 _transDuration;
	uint8 _transArea; // 1 - Whole Stage, 0 - Changing Area
	uint8 _transChunkSize;
	TransitionType _transType;
	PaletteInfo *_palette;
	uint8 _tempo;

	uint16 _sound1;
	uint8 _soundType1;
	uint16 _sound2;
	uint8 _soundType2;

	uint8 _skipFrameFlag;
	uint8 _blend;
	Common::Array<Sprite *> _sprites;
	Common::Array<FrameEntity *> _drawRects;
	DirectorEngine *_vm;
};

} // End of namespace Director

#endif
