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

#ifndef DIRECTOR_CHANNEL_H
#define DIRECTOR_CHANNEL_H

#include "director/cursor.h"

namespace Graphics {
	struct Surface;
	class ManagedSurface;
	class MacWidget;
}

namespace Director {

class Sprite;
class Cursor;

class Channel {
public:
	Channel(Sprite *sp, int priority = 0);
	~Channel();

	DirectorPlotData getPlotData();
	const Graphics::Surface *getMask(bool forceMatte = false);
	Common::Rect getBbox(bool unstretched = false);

	bool isStretched();
	bool isDirty(Sprite *nextSprite = nullptr);
	bool isEmpty();
	bool isActiveText();
	bool isMouseIn(const Common::Point &pos);
	bool isMatteIntersect(Channel *channel);
	bool isMatteWithin(Channel *channel);
	bool isActiveVideo();
	bool isVideoDirectToStage();

	void setWidth(int w);
	void setHeight(int h);
	void setBbox(int l, int t, int r, int b);
	void setCast(uint16 castId);
	void setClean(Sprite *nextSprite, int spriteId, bool partial = false);
	void setEditable(bool editable);
	void replaceSprite(Sprite *nextSprite);
	void replaceWidget();
	bool updateWidget();

	void addDelta(Common::Point pos);

public:
	Sprite *_sprite;
	Cursor _cursor;
	Graphics::MacWidget *_widget;

	bool _dirty;
	bool _visible;
	uint _constraint;
	Common::Point _currentPoint;
	Common::Point _delta;
	Graphics::ManagedSurface *_mask;

	int _priority;
	int _width;
	int _height;

	// Using in digital movie sprites
	double _movieRate;
	uint16 _movieTime;
	uint16 _startTime;
	uint16 _stopTime;

private:
	Graphics::ManagedSurface *getSurface();
	MacShape *getShape();
	Common::Point getPosition();
	uint32 getForeColor();
	uint32 getBackColor();

	void addRegistrationOffset(Common::Point &pos, bool subtract = false);
};

} // End of namespace Director

#endif
