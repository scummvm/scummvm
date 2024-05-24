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

#ifndef DIRECTOR_CHANNEL_H
#define DIRECTOR_CHANNEL_H

#include "director/cursor.h"
#include "director/sprite.h"

namespace Graphics {
	struct Surface;
	class ManagedSurface;
	class MacWidget;
}

namespace Director {

class Sprite;
class Cursor;
class Score;

class Channel {
public:
	Channel(Score *sc, Sprite *sp, int priority = 0);
	Channel(const Channel &channel);
	Channel& operator=(const Channel &channel);
	~Channel();

	DirectorPlotData getPlotData();
	const Graphics::Surface *getMask(bool forceMatte = false);

	inline int getWidth() { return _sprite->_width; };
	inline int getHeight() { return _sprite->_height; };
	inline Common::Point getPosition() { return _sprite->getPosition(); };
	// Return the area of screen to be used for drawing content.
	inline Common::Rect getBbox(bool unstretched = false) { return _sprite->getBbox(unstretched); };

	bool isStretched();
	bool isDirty(Sprite *nextSprite = nullptr);
	bool isEmpty();
	bool isActiveText();
	bool isMouseIn(const Common::Point &pos);
	bool isMatteIntersect(Channel *channel);
	bool isMatteWithin(Channel *channel);
	bool isActiveVideo();
	bool isVideoDirectToStage();

	inline void setWidth(int w) { _sprite->setWidth(w); replaceWidget(); _dirty = true; };
	inline void setHeight(int h) { _sprite->setHeight(h); replaceWidget(); _dirty = true; };
	inline void setBbox(int l, int t, int r, int b) { _sprite->setBbox(l, t, r, b); replaceWidget(); _dirty = true; };
	void setPosition(int x, int y, bool force = false);
	void setCast(CastMemberID memberID);
	void setClean(Sprite *nextSprite, bool partial = false);
	void setStretch(bool enabled);
	bool getEditable();
	void setEditable(bool editable);
	void replaceSprite(Sprite *nextSprite);
	void replaceWidget(CastMemberID previousCastId = CastMemberID(0, 0), bool force = false);
	bool updateWidget();
	void updateTextCast();

	bool isTrail();

	void updateGlobalAttr();

	bool canKeepWidget(CastMemberID castId);
	bool canKeepWidget(Sprite *currentSprite, Sprite *nextSprite);

	int getMouseChar(int x, int y);
	int getMouseWord(int x, int y);
	int getMouseItem(int x, int y);
	int getMouseLine(int x, int y);

	void updateVideoTime();

	// used for film loops
	bool hasSubChannels();
	Common::Array<Channel> *getSubChannels();

public:
	Sprite *_sprite;
	Cursor _cursor;
	Graphics::MacWidget *_widget;

	bool _dirty;
	bool _visible;
	uint _constraint;
	Graphics::ManagedSurface *_mask;

	int _priority;

	// Used in digital movie sprites
	double _movieRate;
	int _movieTime;
	int _startTime;
	int _stopTime;

	// Used in film loops
	uint _filmLoopFrame;

private:
	Graphics::ManagedSurface *getSurface();
	Score *_score;
};

} // End of namespace Director

#endif
