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

#ifndef ULTIMA4_CONTROLLERS_READ_STRING_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_READ_STRING_CONTROLLER_H

#include "ultima/ultima4/controllers/controller.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/views/textview.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A controller to read a Common::String, terminated by the enter key.
 */
class ReadStringController : public WaitableController<Common::String> {
public:
	/**
	 * @param maxlen the maximum length of the Common::String
	 * @param screenX the screen column where to begin input
	 * @param screenY the screen row where to begin input
	 * @param accepted_chars a Common::String characters to be accepted for input
	 */
	ReadStringController(int maxlen, int screenX, int screenY, const Common::String &accepted_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 \n\r\010");
	ReadStringController(int maxlen, TextView *view, const Common::String &accepted_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890 \n\r\010");
	bool keyPressed(int key) override;

	static Common::String get(int maxlen, int screenX, int screenY, EventHandler *eh = nullptr);
	static Common::String get(int maxlen, TextView *view, EventHandler *eh = nullptr);
#ifdef IOS_ULTIMA4
	void setValue(const Common::String &utf8StringValue) {
		value = utf8StringValue;
	}
#endif

protected:
	int _maxLen, _screenX, _screenY;
	TextView *_view;
	Common::String _accepted;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
