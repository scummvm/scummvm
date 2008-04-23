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

#include "made/made.h"
#include "made/screen.h"
#include "made/resource.h"

namespace Made {

Screen::Screen(MadeEngine *vm) : _vm(vm) {

	_screen1 = new Graphics::Surface();
	_screen1->create(320, 200, 1);
	_screen2 = new Graphics::Surface();
	_screen2->create(320, 200, 1);

	_clipInfo1.x = _clipInfo2.x = 0;
	_clipInfo1.y = _clipInfo2.y = 0;
	_clipInfo1.w = _clipInfo2.w = 320;
	_clipInfo1.h = _clipInfo2.h = 200;

	_clipInfo1.destSurface = _screen1;
	_clipInfo2.destSurface = _screen2;
	_clipArea.destSurface = _screen2;
	
	for (int i = 0; i <= 3; i++)
		_excludeClipAreaEnabled[i] = false;

	_screenLock = false;
	_paletteLock = false;

	_paletteInitialized = false;
	_needPalette = false;
	_oldPaletteColorCount = 256;
	_paletteColorCount = 256;
	memset(_newPalette, 0, 768);
	memset(_palette, 0, 768);

	_ground = 1;
	_clip = 0;
	_exclude = 0;
	
	_visualEffectNum = 0;

	clearChannels();
}

Screen::~Screen() {
	delete _screen1;
	delete _screen2;
}

void Screen::clearScreen() {
	_screen1->fillRect(Common::Rect(0, 0, 320, 200), 0);
	_screen2->fillRect(Common::Rect(0, 0, 320, 200), 0);
	_needPalette = true;
	//_vm->_system->clearScreen();
}

void Screen::drawSurface(Graphics::Surface *sourceSurface, int x, int y, const ClipInfo &clipInfo) {

	byte *source = (byte*)sourceSurface->getBasePtr(0, 0);
	byte *dest = (byte*)clipInfo.destSurface->getBasePtr(x, y);

	// FIXME: Implement actual clipping
	if (x + sourceSurface->w > clipInfo.destSurface->w || y + sourceSurface->h > clipInfo.destSurface->h) {
		debug(2, "CLIPPING PROBLEM: x = %d; y = %d; w = %d; h = %d; x+w = %d; y+h = %d\n",
			x, y, sourceSurface->w, sourceSurface->h, x + sourceSurface->w, y + sourceSurface->h);
		return;
	}

	for (int16 yc = 0; yc < sourceSurface->h; yc++) {
		for (int16 xc = 0; xc < sourceSurface->w; xc++) {
			if (source[xc])
				dest[xc] = source[xc];
		}
		source += sourceSurface->pitch;
		dest += clipInfo.destSurface->pitch;
	}

}

void Screen::loadRGBPalette(byte *palRGB, int count) {
	for (int i = 0; i < count; i++) {
		_screenPalette[i * 4 + 0] = palRGB[i * 3 + 0];
		_screenPalette[i * 4 + 1] = palRGB[i * 3 + 1];
		_screenPalette[i * 4 + 2] = palRGB[i * 3 + 2];
		_screenPalette[i * 4 + 3] = 0;
	}
}

void Screen::setRGBPalette(byte *palRGB, int start, int count) {
	loadRGBPalette(palRGB, count);
	_vm->_system->setPalette(_screenPalette, start, count);
}

uint16 Screen::updateChannel(uint16 channelIndex) {
	return 0;
}

void Screen::deleteChannel(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100)
		return;
	_channels[channelIndex - 1].type = 0;
	_channels[channelIndex - 1].state = 0;
	_channels[channelIndex - 1].index = 0;
}

int16 Screen::getChannelType(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100)
		return -1;
	return _channels[channelIndex - 1].type;
}

int16 Screen::getChannelState(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100)
		return -1;
	return _channels[channelIndex - 1].state;
}

void Screen::setChannelState(uint16 channelIndex, int16 state) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return;
	_channels[channelIndex - 1].state = state;
}

uint16 Screen::setChannelLocation(uint16 channelIndex, int16 x, int16 y) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return 0;
	_channels[channelIndex - 1].x = x;
	_channels[channelIndex - 1].y = y;
	return updateChannel(channelIndex - 1) + 1;
}

uint16 Screen::setChannelContent(uint16 channelIndex, uint16 index) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return 0;
	//debug(2, "setChannelContent(%d, %04X)\n", channelIndex, index); fflush(stdout); g_system->delayMillis(5000);
	_channels[channelIndex - 1].index = index;
	return updateChannel(channelIndex - 1) + 1;
}

void Screen::drawSpriteChannels(const ClipInfo &clipInfo, int16 includeStateMask, int16 excludeStateMask) {

	for (int i = 0; i <= 3; i++)
		_excludeClipArea[i].destSurface = clipInfo.destSurface;

	_clipArea.destSurface = clipInfo.destSurface;
	
	for (uint16 i = 0; i < _channelsUsedCount; i++) {
	
		debug(2, "drawSpriteChannels() i = %d\n", i);
	
		if (((_channels[i].state & includeStateMask) == includeStateMask) && (_channels[i].state & excludeStateMask) == 0) {
			uint16 flag1 = _channels[i].state & 0x10;
			uint16 flag2 = _channels[i].state & 0x20;
			
			debug(2, "drawSpriteChannels() type = %d; index = %04X\n", _channels[i].type, _channels[i].index);
			
			switch (_channels[i].type) {

			case 1: // drawFlex
				if (_channels[i].state & 4) {
					drawFlex(_channels[i].index, _channels[i].x, _channels[i].y, flag1, flag2, _clipArea);
				} else if (_channels[i].state & 8) {
					for (int excludeIndex = 0; excludeIndex < 4; excludeIndex++) {
						if (_excludeClipAreaEnabled[excludeIndex]) {
							drawFlex(_channels[i].index, _channels[i].x, _channels[i].y, flag1, flag2, _excludeClipArea[excludeIndex]);
						}
					}
				} else {
					drawFlex(_channels[i].index, _channels[i].x, _channels[i].y, flag1, flag2, clipInfo);
				}
				break;

			case 2: // drawObjectText
				// TODO
				break;

			case 3: // drawAnimFrame
				if (_channels[i].state & 4) {
					drawAnimFrame(_channels[i].index, _channels[i].x, _channels[i].y, _channels[i].frameNum, flag1, flag2, _clipArea);
				} else if (_channels[i].state & 8) {
					for (int excludeIndex = 0; excludeIndex < 4; excludeIndex++) {
						if (_excludeClipAreaEnabled[excludeIndex]) {
							drawAnimFrame(_channels[i].index, _channels[i].x, _channels[i].y, _channels[i].frameNum, flag1, flag2, _excludeClipArea[excludeIndex]);
						}
					}
				} else {
					drawAnimFrame(_channels[i].index, _channels[i].x, _channels[i].y, _channels[i].frameNum, flag1, flag2, clipInfo);
				}
				break;

			case 4: // drawMenuText
				// TODO
				break;
				
			default:
				break;

			}

		}
	
	}

}

void Screen::updateSprites() {
	// TODO: This needs some more work, dirty rectangles are currently not used

	memcpy(_screen2->pixels, _screen1->pixels, 64000);

	drawSpriteChannels(_clipInfo1, 3, 0);
	drawSpriteChannels(_clipInfo2, 1, 2);

	_vm->_system->copyRectToScreen((const byte*)_screen2->pixels, _screen2->pitch, 0, 0, _screen2->w, _screen2->h);
	
}

void Screen::clearChannels() {
	for (uint16 i = 0; i < ARRAYSIZE(_channels); i++) {
		_channels[i].type = 0;
		_channels[i].index = 0;
	}
	_channelsUsedCount = 0;
}

uint16 Screen::drawFlex(uint16 flexIndex, int16 x, int16 y, uint16 flag1, uint16 flag2, const ClipInfo &clipInfo) {

	if (flexIndex == 0)
		return 0;

	PictureResource *flex = _vm->_res->getPicture(flexIndex);
	Graphics::Surface *sourceSurface = flex->getPicture();

	drawSurface(sourceSurface, x, y, clipInfo);

	// Palette is set in showPage
	if (flex->hasPalette() && !_paletteLock && _needPalette) {
		byte *flexPalette = flex->getPalette();
		_oldPaletteColorCount = _paletteColorCount;
		_paletteColorCount = flex->getPaletteColorCount();
		memcpy(_newPalette, _palette, _oldPaletteColorCount * 3);
		memcpy(_palette, flexPalette, _paletteColorCount * 3);
		_needPalette = false;
	}

	_vm->_res->freeResource(flex);

	return 0;
}

void Screen::drawAnimFrame(uint16 animIndex, int16 x, int16 y, int16 frameNum, uint16 flag1, uint16 flag2, const ClipInfo &clipInfo) {

	if (frameNum < 0)
		return;

	AnimationResource *anim = _vm->_res->getAnimation(animIndex);
	Graphics::Surface *sourceSurface = anim->getFrame(frameNum);

	drawSurface(sourceSurface, x, y, clipInfo);

	_vm->_res->freeResource(anim);
}

uint16 Screen::drawPic(uint16 index, int16 x, int16 y, uint16 flag1, uint16 flag2) {

	//HACK (until clipping is implemented)
	if (y > 200) y = 0;

	drawFlex(index, x, y, flag1, flag2, _clipInfo1);
	return 0;
}

uint16 Screen::drawAnimPic(uint16 animIndex, int16 x, int16 y, int16 frameNum, uint16 flag1, uint16 flag2) {
	drawAnimFrame(animIndex, x, y, frameNum, flag1, flag2, _clipInfo1);
	return 0;
}

void Screen::addSprite(uint16 spriteIndex) {
	bool oldScreenLock = _screenLock;
	drawFlex(spriteIndex, 0, 0, 0, 0, _clipInfo1);
	_screenLock = oldScreenLock;
}

uint16 Screen::drawSprite(uint16 flexIndex, int16 x, int16 y) {
	return placeSprite(_channelsUsedCount + 1, flexIndex, x, y);
}

uint16 Screen::placeSprite(uint16 channelIndex, uint16 flexIndex, int16 x, int16 y) {

	debug(2, "placeSprite(%d, %04X, %d, %d)\n", channelIndex, flexIndex, x, y); fflush(stdout);

	if (channelIndex < 1 || channelIndex >= 100)
		return 0;
		
	channelIndex--;
		
	PictureResource *flex = _vm->_res->getPicture(flexIndex);
	
	if (flex) {
		Graphics::Surface *surf = flex->getPicture();
	
		int16 state = 1;
		int16 x1, y1, x2, y2;
		
		x1 = x;
		y1 = y;
		x2 = x + surf->w + 1;
		y2 = y + surf->h + 1;
		//TODO: clipRect(x1, y1, x2, y2);
		
		if (_ground == 0)
			state |= 2;
		if (_clip != 0)
			state |= 4;
		if (_exclude != 0)
			state |= 8;
			
		_channels[channelIndex].state = state;
		_channels[channelIndex].type = 1;
		_channels[channelIndex].index = flexIndex;
		_channels[channelIndex].x = x;
		_channels[channelIndex].y = y;
		_channels[channelIndex].x1 = x1;
		_channels[channelIndex].y1 = y1;
		_channels[channelIndex].x2 = x2;
		_channels[channelIndex].y2 = y2;
		_channels[channelIndex].area = (x2 - x2) * (y2 - y1);
		
		if (_channelsUsedCount <= channelIndex)
			_channelsUsedCount = channelIndex + 1;

		_vm->_res->freeResource(flex);
	} else {
		_channels[channelIndex].type = 0;
		_channels[channelIndex].state = 0;
	}

	return channelIndex + 1;

}

uint16 Screen::placeAnim(uint16 channelIndex, uint16 animIndex, int16 x, int16 y, int16 frameNum) {

	if (channelIndex < 1 || channelIndex >= 100)
		return 0;

	channelIndex--;

	AnimationResource *anim = _vm->_res->getAnimation(animIndex);

	if (anim) {

		int16 state = 1;
		int16 x1, y1, x2, y2;

		x1 = x;
		y1 = y;
		x2 = x + anim->getWidth();
		y2 = y + anim->getHeight();
		//TODO: clipRect(x1, y1, x2, y2);

		if (anim->getFlags() == 1 || _ground == 0)
			state |= 2;
		if (_clip != 0)
			state |= 4;
		if (_exclude != 0)
			state |= 8;

		_channels[channelIndex].state = state;
		_channels[channelIndex].type = 3;
		_channels[channelIndex].index = animIndex;
		_channels[channelIndex].frameNum = frameNum;
		_channels[channelIndex].needRefresh = 1;
		_channels[channelIndex].x = x;
		_channels[channelIndex].y = y;
		_channels[channelIndex].x1 = x1;
		_channels[channelIndex].y1 = y1;
		_channels[channelIndex].x2 = x2;
		_channels[channelIndex].y2 = y2;
		_channels[channelIndex].area = (x2 - x2) * (y2 - y1);

		if (_channelsUsedCount <= channelIndex)
			_channelsUsedCount = channelIndex + 1;

		_vm->_res->freeResource(anim);
	} else {
		_channels[channelIndex].type = 0;
		_channels[channelIndex].state = 0;
	}

	return channelIndex + 1;

}

int16 Screen::setAnimFrame(uint16 channelIndex, int16 frameNum) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return 0;
	 channelIndex--;
	_channels[channelIndex].frameNum = frameNum;
	_channels[channelIndex].needRefresh = 1;
	return updateChannel(channelIndex) + 1;
}

int16 Screen::getAnimFrame(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return -1;
	return _channels[channelIndex - 1].frameNum;
}

int16 Screen::getAnimFrameCount(uint16 animIndex) {
	int16 frameCount = 0;
	AnimationResource *anim = _vm->_res->getAnimation(animIndex);
	if (anim) {
		frameCount = anim->getCount();
		_vm->_res->freeResource(anim);
	}
	return frameCount;
}


uint16 Screen::placeText(uint16 channelIndex, uint16 textObjectIndex, int16 x, int16 y, uint16 fontNum, int16 textColor, int16 outlineColor) {
	return 0;
}

void Screen::show() {

	// TODO
	
	if (_screenLock)
		return;

	drawSpriteChannels(_clipInfo1, 3, 0);
	memcpy(_screen2->pixels, _screen1->pixels, 64000);
	drawSpriteChannels(_clipInfo2, 1, 2);

	// TODO: Implement visual effects (palette fading etc.)
	if (!_paletteLock)
		setRGBPalette(_palette, 0, _paletteColorCount);
	_vm->_system->copyRectToScreen((const byte*)_screen2->pixels, _screen2->pitch, 0, 0, _screen2->w, _screen2->h);
	_vm->_system->updateScreen();

	if (!_paletteInitialized) {
		memcpy(_newPalette, _palette, _paletteColorCount * 3);
		_oldPaletteColorCount = _paletteColorCount;
		_paletteInitialized = true;
	}

}

} // End of namespace Made
