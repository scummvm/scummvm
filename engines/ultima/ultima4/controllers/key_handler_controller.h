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

#ifndef ULTIMA4_CONTROLLERS_KEY_HANDLER_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_KEY_HANDLER_CONTROLLER_H

#include "ultima/ultima4/controllers/controller.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A class for handling keystrokes.
 */
class KeyHandler {
public:
	virtual ~KeyHandler() {}

	/* Typedefs */
	typedef bool (*Callback)(int, void *);

	/** Additional information to be passed as data param for read buffer key handler */
	typedef struct ReadBuffer {
		int (*_handleBuffer)(Common::String *);
		Common::String *_buffer;
		int _bufferLen;
		int _screenX, _screenY;
	} ReadBuffer;

	/** Additional information to be passed as data param for get choice key handler */
	typedef struct GetChoice {
		Common::String _choices;
		int (*_handleChoice)(int);
	} GetChoice;

	/* Constructors */
	KeyHandler(Callback func, void *data = nullptr, bool asyncronous = true);

	/**
	 * Handles any and all keystrokes.
	 * Generally used to exit the application, switch applications,
	 * minimize, maximize, etc.
	 */
	static bool globalHandler(int key);

	/* Static default key handler functions */
	/**
	 * A default key handler that should be valid everywhere
	 */
	static bool defaultHandler(int key, void *data);

	/**
	 * A key handler that ignores keypresses
	 */
	static bool ignoreKeys(int key, void *data);

	/* Operators */
	bool operator==(Callback cb) const;

	/* Member functions */
	/**
	 * Handles a keypress.
	 * First it makes sure the key combination is not ignored
	 * by the current key handler. Then, it passes the keypress
	 * through the global key handler. If the global handler
	 * does not process the keystroke, then the key handler
	 * handles it itself by calling its handler callback function.
	 */
	bool handle(int key);

	/**
	 * Returns true if the key or key combination is always ignored by xu4
	 */
	virtual bool isKeyIgnored(int key);

protected:
	Callback _handler;
	bool _async;
	void *_data;
};

/**
 * A controller that wraps a keyhander function.  Keyhandlers are
 * deprecated -- please use a controller instead.
 */
class KeyHandlerController : public Controller {
public:
	KeyHandlerController(KeyHandler *handler);
	~KeyHandlerController();

	bool keyPressed(int key) override;
	KeyHandler *getKeyHandler();

private:
	KeyHandler *_handler;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
