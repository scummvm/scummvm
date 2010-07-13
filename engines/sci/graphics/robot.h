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
	GfxRobot(ResourceManager *resMan, GfxScreen *screen, GuiResourceId resourceId);
	~GfxRobot();

	void draw();

private:
	void initData(GuiResourceId resourceId);

	ResourceManager *_resMan;
	GfxScreen *_screen;

	GuiResourceId _resourceId;
	Resource *_resource;
	byte *_resourceData;

	uint16 _width;
	uint16 _height;
	uint16 _frameCount;
	uint32 _frameSize; // is width * height (pixelCount)
};
#endif

} // End of namespace Sci

#endif
