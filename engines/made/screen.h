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

#ifndef MADE_SCREEN_H
#define MADE_SCREEN_H

#include "common/endian.h"
#include "common/util.h"

#include "graphics/surface.h"

namespace Made {

struct SpriteChannel {
	int16 type;
	int16 state;
	int16 needRefresh;
	uint16 index;
	int16 x, y;
	int16 x1, y1, x2, y2;
	uint32 area;
	uint16 fontNum;
	int16 textColor, outlineColor;
	int16 frameNum;
};

struct ClipInfo {
	uint16 x, y, w, h;
	Graphics::Surface *destSurface;
};

class MadeEngine;

class Screen {
public:
    Screen(MadeEngine *vm);
    ~Screen();

    void clearScreen();
    
    void drawSurface(Graphics::Surface *source, int x, int y);
    void setPalette(byte *palette, int start, int count);

    uint16 updateChannel(uint16 channelIndex);
    void deleteChannel(uint16 channelIndex);
    int16 getChannelType(uint16 channelIndex);
	int16 getChannelState(uint16 channelIndex);
	void setChannelState(uint16 channelIndex, int16 state);
    uint16 setChannelLocation(uint16 channelIndex, int16 x, int16 y);
    uint16 setChannelContent(uint16 channelIndex, uint16 index);
    void drawSpriteChannels(const ClipInfo &clipInfo, int16 includeStateMask, int16 excludeStateMask);
    void updateSprites();
	void clearChannels();

    uint16 drawFlex(uint16 flexIndex, int16 x, int16 y, uint16 flag1, uint16 flag2, const ClipInfo &clipInfo);
	void drawAnimFrame(uint16 animIndex, int16 x, int16 y, int16 frameNum, uint16 flag1, uint16 flag2, const ClipInfo &clipInfo);

    uint16 drawPic(uint16 index, int16 x, int16 y, uint16 flag1, uint16 flag2);
	uint16 drawAnimPic(uint16 animIndex, int16 x, int16 y, int16 frameNum, uint16 flag1, uint16 flag2);
	
	uint16 addSprite(uint16 spriteIndex);
	
    uint16 drawSprite(uint16 flexIndex, int16 x, int16 y);
    uint16 placeSprite(uint16 channelIndex, uint16 flexIndex, int16 x, int16 y);

    uint16 placeAnim(uint16 channelIndex, uint16 animIndex, int16 x, int16 y, int16 frameNum);
	int16 setAnimFrame(uint16 channelIndex, int16 frameNum);
    int16 getAnimFrame(uint16 channelIndex);
    // TODO: Move to script function
	int16 getAnimFrameCount(uint16 animIndex);

    uint16 placeText(uint16 channelIndex, uint16 textObjectIndex, int16 x, int16 y, uint16 fontNum, int16 textColor, int16 outlineColor);
    
    void show();
    
    void setClip(uint16 clip);
    void setExclude(uint16 exclude);
    void setGround(uint16 ground);

    byte _palette[256 * 4];

protected:
    MadeEngine *_vm;
    
    bool _screenLock;
    
    uint16 _clip, _exclude, _ground;
    
    Graphics::Surface *_screen1, *_screen2;
    ClipInfo _clipArea, _clipInfo1, _clipInfo2;
    
    ClipInfo _excludeClipArea[4];
    bool _excludeClipAreaEnabled[4];
    
    uint16 _channelsUsedCount;
	SpriteChannel _channels[100];
};

} // End of namespace Made

#endif /* MADE_H */
