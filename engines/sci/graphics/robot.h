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

#ifndef SCI_GRAPHICS_ROBOT_H
#define SCI_GRAPHICS_ROBOT_H

namespace Sci {

#define ROBOT_FILE_STARTOFDATA 58

#ifdef ENABLE_SCI32
class GfxRobot {
public:
	GfxRobot(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette);
	~GfxRobot();

	void init(GuiResourceId resourceId, uint16 x, uint16 y);
	void drawNextFrame();
	uint16 getCurFrame() { return _curFrame; }
	uint16 getFrameCount() { return _frameCount; }
	bool isPlaying() { return _resourceId != -1; }
	void playAudio();

private:
	void initData(GuiResourceId resourceId);
	void getFrameOffsets();
	byte *assembleVideoFrame(int frame);
	void getFrameDimensions(int frame, int &width, int &height);
	void getFrameRect(int frame, Common::Rect &rect); // Not sure what to use this for yet
	int getFrameScale(int frame); // Scale factor (multiplied by 100). More like custom height, but why use a percentage for it?
	void setPalette();

	ResourceManager *_resMan;
	GfxScreen *_screen;
	GfxPalette *_palette;

	GuiResourceId _resourceId;
	byte *_resourceData;
	byte _savedPal[256 * 4];

	uint16 _x;
	uint16 _y;
	//uint16 _width;
	//uint16 _height;
	uint16 _frameCount;
	uint32 _frameSize; // is width * height (pixelCount)
	uint16 _audioSize;
	bool _hasSound;
	uint32 _palOffset;
	uint32 *_imageStart;
	uint32 *_audioStart;
	uint32 *_audioLen;
	uint16 _curFrame;
};
#endif

} // End of namespace Sci

#endif
