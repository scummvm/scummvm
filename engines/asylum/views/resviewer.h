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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_VIEWS_RESVIEWER_H
#define ASYLUM_VIEWS_RESVIEWER_H

#include "asylum/system/graphics.h"

#include "asylum/eventhandler.h"
#include "asylum/shared.h"

namespace Asylum {

class AsylumEngine;

class ResourceViewer : public EventHandler {
public:
	ResourceViewer(AsylumEngine *engine);
	~ResourceViewer() {};

	void setEventHandler(EventHandler *handler) { _handler = handler; }
	bool setResourceId(ResourceId resourceId);
	bool handleEvent(const AsylumEvent &evt);

private:
	AsylumEngine *_vm;
	EventHandler *_handler;
	ResourceId _resourceId;
	GraphicResource _resource;
	int  _frameIndex;
	uint _frameCount;
	int  _frameIncrement;
	int16 _x, _y;
	uint16 _width, _height;
	bool _scroll;
	int _resPack;
	int _paletteIndex;
	bool _animate;

	bool isPalette(ResourceId resourceId);
	void drawPalette();
	void drawResource();

	void key(const AsylumEvent &evt);
	void update();
};

} // End of namespace Asylum

#endif // ASYLUM_VIEWS_RESVIEWER_H
