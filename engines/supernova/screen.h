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

#ifndef SUPERNOVA_SCREEN_H
#define SUPERNOVA_SCREEN_H

#include "common/scummsys.h"

#include "supernova/imageid.h"
#include "supernova/msn_def.h"

namespace Supernova {

class SupernovaEngine;
class ResourceManager;
class GuiElement;
class Room;

class Screen {
public:
	struct ImageInfo {
		int filenumber;
		int section;
	};

public:
	Screen(SupernovaEngine *vm, ResourceManager *resMan);

	void paletteFadeIn();
	void paletteFadeOut();
	void paletteBrightness();
	void renderImage(int section);
	void renderImageSection(int section);
	bool setCurrentImage(int filenumber);
	void saveScreen(int x, int y, int width, int height);
	void saveScreen(const GuiElement &guiElement);
	void restoreScreen();
	void renderRoom(Room &room);
	void renderMessage(const char *text, MessagePosition position = kMessageNormal);
	void removeMessage();
	void renderText(const char *text, int x, int y, byte color);
	void renderText(const uint16 character, int x, int y, byte color);
	void renderText(const char *text);
	void renderText(const uint16 character);
	void renderText(const GuiElement &guiElement);
	void renderBox(int x, int y, int width, int height, byte color);
	void renderBox(const GuiElement &guiElement);
	void setColor63(byte value);
	void update();

private:

};

}

#endif
