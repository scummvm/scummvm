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

#ifndef DS_KEYBOARD_H
#define DS_KEYBOARD_H

#include "common/events.h"

namespace DS {

class Keyboard : public Common::EventSource {
public:
	Keyboard(Common::EventDispatcher *eventDispatcher);
	~Keyboard();

	void init(int layer, int mapBase, int tileBase, bool mainDisplay);

	void show();
	void hide();
	inline bool isVisible() const { return _visible; }

	// Implementation of the EventSource interface
	virtual bool pollEvent(Common::Event &event);

protected:
	Common::EventDispatcher *_eventDispatcher;
	int _lastKey;
	bool _visible;

	bool mapKey(int key, Common::KeyState &ks);
};

} // End of namespace DS

#endif // #ifndef DS_KEYBOARD_H
