/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MADE_SCREEN_H
#define MADE_SCREEN_H

#include "made/resource.h"

#include "common/rect.h"

#include "graphics/cursor.h"

namespace Made {

struct SpriteChannel {
	int16 type;
	int16 state;
	uint16 index;
	int16 x, y;
	uint16 fontNum;
	int16 textColor, outlineColor;
	int16 frameNum;
	int16 mask;
	Common::String previousText;
};

struct ClipInfo {
	Common::Rect clipRect;
	Graphics::Surface *destSurface;
};

struct SpriteListItem {
	int16 index, xofs, yofs;
};

class MadeEngine;
class ScreenEffects;

static const byte defaultMouseCursor[256] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  1, 15, 15,  1,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  1, 15, 15,  1,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  1, 15, 15,  1,  0,  0,  0,  0,  0,
	0,  1,  1,  1,  1,  1,  1,  1, 15, 15,  1,  0,  0,  0,  0,  0,
	1,  1, 15,  1, 15,  1, 15,  1, 15, 15,  1,  0,  0,  0,  0,  0,
	1, 15, 15,  1, 15,  1, 15,  1, 15, 15,  1,  0,  0,  0,  0,  0,
	1, 15, 15, 15, 15, 15, 15, 15, 15, 15,  1,  0,  1,  1,  1,  0,
	1, 15, 15, 15, 15, 15, 15, 15, 15, 15,  1,  1, 15, 15, 15,  1,
	1, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  1,  1,  1,
	1, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  1,  1,  0,  0,
	1,  1, 15, 15, 15, 15, 15, 15, 15, 15, 15,  1,  1,  0,  0,  0,
	0,  1,  1, 15, 15, 15, 15, 15, 15, 15,  1,  1,  0,  0,  0,  0,
	0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

class Screen {
public:
	Screen(MadeEngine *vm);
	~Screen();

	void clearScreen();

	void drawSurface(Graphics::Surface *sourceSurface, int x, int y, int16 flipX, int16 flipY, int16 mask, const ClipInfo &clipInfo);

	void setRGBPalette(byte *palRGB, int start = 0, int count = 256);
	bool isPaletteLocked() { return _paletteLock; }
	void setPaletteLock(bool lock) { _paletteLock = lock; }
	bool isScreenLocked() { return _screenLock; }
	void setScreenLock(bool lock) { _screenLock = lock; }
	void setVisualEffectNum(int visualEffectNum) { _visualEffectNum = visualEffectNum; }

	void setClipArea(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
		_clipArea.clipRect = Common::Rect(x1, y1, x2, y2);
	}

	void setExcludeArea(uint16 x1, uint16 y1, uint16 x2, uint16 y2);

	void setClip(int16 clip) { _clip = clip; }
	void setExclude(int16 exclude) { _exclude = exclude; }
	void setGround(int16 ground) { _ground = ground; }
	void setMask(int16 mask) { _mask = mask; }

	void setTextColor(int16 color) { _textColor = color; }

	void setTextRect(const Common::Rect &textRect) {
		_textRect = textRect;
		_textX = _textRect.left;
		_textY = _textRect.top;
	}

	void getTextRect(Common::Rect &textRect) {
		textRect = _textRect;
	}

	void setOutlineColor(int16 color) {
		_outlineColor = color;
		_dropShadowColor = -1;
	}

	void setDropShadowColor(int16 color) {
		_outlineColor = -1;
		_dropShadowColor = color;
	}

	void setTextXY(int16 x, int16 y) {
		_textX = x;
		_textY = y;
	}

	void homeText() {
		_textX = _textRect.left;
		_textY = _textRect.top;
	}

	void setQueueNextText(bool value) { _queueNextText = value; }
	void setVoiceTimeText(bool value) { _voiceTimeText = value; }

	uint16 updateChannel(uint16 channelIndex);
	void deleteChannel(uint16 channelIndex);
	int16 getChannelType(uint16 channelIndex);
	int16 getChannelState(uint16 channelIndex);
	void setChannelState(uint16 channelIndex, int16 state);
	uint16 setChannelLocation(uint16 channelIndex, int16 x, int16 y);
	uint16 setChannelContent(uint16 channelIndex, uint16 index);
	void setChannelUseMask(uint16 channelIndex);
	void drawSpriteChannels(const ClipInfo &clipInfo, int16 includeStateMask, int16 excludeStateMask);
	void updateSprites();
	void clearChannels();

	uint16 drawFlex(uint16 flexIndex, int16 x, int16 y, int16 flipX, int16 flipY, int16 mask, const ClipInfo &clipInfo);

	void drawAnimFrame(uint16 animIndex, int16 x, int16 y, int16 frameNum, int16 flipX, int16 flipY, const ClipInfo &clipInfo);

	uint16 drawPic(uint16 index, int16 x, int16 y, int16 flipX, int16 flipY);
	uint16 drawMask(uint16 index, int16 x, int16 y);

	uint16 drawAnimPic(uint16 animIndex, int16 x, int16 y, int16 frameNum, int16 flipX, int16 flipY);

	void addSprite(uint16 spriteIndex);

	uint16 drawSprite(uint16 flexIndex, int16 x, int16 y);
	uint16 placeSprite(uint16 channelIndex, uint16 flexIndex, int16 x, int16 y);

	uint16 placeAnim(uint16 channelIndex, uint16 animIndex, int16 x, int16 y, int16 frameNum);
	int16 setAnimFrame(uint16 channelIndex, int16 frameNum);
	int16 getAnimFrame(uint16 channelIndex);

	uint16 placeText(uint16 channelIndex, uint16 textObjectIndex, int16 x, int16 y, uint16 fontNum, int16 textColor, int16 outlineColor);
#ifdef USE_TTS
	void voiceChannelText(const char *text, uint16 channelIndex);
#endif

	void show();
	void flash(int count);

	void setFont(int16 fontNum);
	void printChar(uint c, int16 x, int16 y, byte color);
	void printText(const char *text);
	void printTextEx(const char *text, int16 x, int16 y, int16 fontNum, int16 textColor, int16 outlineColor, const ClipInfo &clipInfo);
	void printObjectText(int16 objectIndex, int16 x, int16 y, int16 fontNum, int16 textColor, int16 outlineColor, const ClipInfo &clipInfo);
	int16 getTextWidth(int16 fontNum, const char *text);

	// Interface functions for the screen effects class
	Graphics::Surface *lockScreen();
	void unlockScreen();
	void showWorkScreen();
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	void updateScreenAndWait(int delay);

	int16 addToSpriteList(int16 index, int16 xofs, int16 yofs);
	SpriteListItem getFromSpriteList(int16 index);
	void clearSpriteList();

	void setMouseCursor(const Graphics::Cursor *cursor);
	void setDefaultMouseCursor();

	/**
	 * Pushes the work screen to the backend. For the ReelMagic release the
	 * screen is a true colour one and the MPEG picture is mixed in underneath.
	 */
	void presentWorkScreen();

	/** True when the graphics layer is stretched to a 240 line screen. */
	bool isGraphicsStretched() const { return _stretchGraphics; }
	int getOutputHeight() const { return _outputHeight; }
	int screenToGameY(int y) const { return y * 200 / _outputHeight; }
	int gameToScreenY(int y) const { return (y * _outputHeight + 199) / 200; }

	/**
	 * Set the MPEG picture which the ReelMagic release mixes underneath the
	 * paletted graphics.
	 */
	void setVideoLayer(const Graphics::Surface *videoLayer) { _videoLayer = videoLayer; }

protected:
	MadeEngine *_vm;
	ScreenEffects *_fx;

	bool _screenLock;
	bool _paletteLock;

	byte *_palette, *_newPalette;
	int _paletteColorCount, _oldPaletteColorCount;
	bool _paletteInitialized, _needPalette;
	int16 _textColor;
	int16 _outlineColor;
	int16 _dropShadowColor;

	int16 _textX, _textY;
	Common::Rect _textRect;
	int16 _currentFontNum;
	FontResource *_font;
	ClipInfo _fontDrawCtx;

	int16 _clip, _exclude, _ground, _mask;
	int _visualEffectNum;

	Graphics::Surface *_backgroundScreen, *_workScreen, *_screenMask;

	// The ReelMagic release runs on a true colour screen so that the MPEG
	// picture can be mixed in under the paletted graphics; the palette is then
	// applied here rather than by the backend.
	bool _trueColor;
	bool _stretchGraphics;
	int _outputHeight;
	byte _outputPalette[256 * 3];
	uint32 _outputColors[256];	// _outputPalette in the screen format
	Graphics::Surface *_outputScreen;
	const Graphics::Surface *_videoLayer;
	// What the screen effects draw on. They expect to reach the visible screen
	// as 8 bit paletted pixels, which a true colour backend cannot give them, so
	// they get this mirror of the graphics layer instead and it is converted on
	// the way out.
	Graphics::Surface *_fxScreen;

	/**
	 * Converts a rect of 8 bit graphics into the true colour output screen,
	 * mixing the MPEG picture in where the graphics are transparent, and hands
	 * the result to the backend. @p src points at the top left of the rect.
	 */
	void blitTrueColorRect(const byte *src, int srcPitch, int x, int y, int w, int h);
	ClipInfo _clipArea, _backgroundScreenDrawCtx, _workScreenDrawCtx, _maskDrawCtx;

	ClipInfo _excludeClipArea[4];
	bool _excludeClipAreaEnabled[4];

	uint16 _channelsUsedCount;
	SpriteChannel _channels[100];

	bool _queueNextText;
	bool _voiceTimeText;

	Common::Array<SpriteListItem> _spriteList;

};

} // End of namespace Made

#endif /* MADE_H */
