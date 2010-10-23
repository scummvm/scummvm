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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
 * BS_InputEngine
 * -------------
 * This is the input interface engine that contains all the methods that an
 * input source must implement.
 * All input engines must be derived from this class.
 *
 * Autor: Alex Arnst
 */

#ifndef SWORD25_INPUTENGINE_H
#define SWORD25_INPUTENGINE_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/persistable.h"
#include "sword25/kernel/callbackregistry.h"

namespace Sword25 {

/// Class definitions

class InputEngine : public Service, public Persistable {
public:
	InputEngine(Kernel *pKernel);
	~InputEngine();

	// NOTE: These codes are registered in inputengine_script.cpp
	// Any changes to these enums must also adjust the above file.
	enum KEY_CODES  {
		KEY_BACKSPACE   = 0x08,
		KEY_TAB         = 0x09,
		KEY_CLEAR       = 0x0C,
		KEY_RETURN      = 0x0D,
		KEY_PAUSE       = 0x13,
		KEY_CAPSLOCK    = 0x14,
		KEY_ESCAPE      = 0x1B,
		KEY_SPACE       = 0x20,
		KEY_PAGEUP      = 0x21,
		KEY_PAGEDOWN    = 0x22,
		KEY_END         = 0x23,
		KEY_HOME        = 0x24,
		KEY_LEFT        = 0x25,
		KEY_UP          = 0x26,
		KEY_RIGHT       = 0x27,
		KEY_DOWN        = 0x28,
		KEY_PRINTSCREEN = 0x2C,
		KEY_INSERT      = 0x2D,
		KEY_DELETE      = 0x2E,
		KEY_0           = 0x30,
		KEY_1           = 0x31,
		KEY_2           = 0x32,
		KEY_3           = 0x33,
		KEY_4           = 0x34,
		KEY_5           = 0x35,
		KEY_6           = 0x36,
		KEY_7           = 0x37,
		KEY_8           = 0x38,
		KEY_9           = 0x39,
		KEY_A           = 0x41,
		KEY_B           = 0x42,
		KEY_C           = 0x43,
		KEY_D           = 0x44,
		KEY_E           = 0x45,
		KEY_F           = 0x46,
		KEY_G           = 0x47,
		KEY_H           = 0x48,
		KEY_I           = 0x49,
		KEY_J           = 0x4A,
		KEY_K           = 0x4B,
		KEY_L           = 0x4C,
		KEY_M           = 0x4D,
		KEY_N           = 0x4E,
		KEY_O           = 0x4F,
		KEY_P           = 0x50,
		KEY_Q           = 0x51,
		KEY_R           = 0x52,
		KEY_S           = 0x53,
		KEY_T           = 0x54,
		KEY_U           = 0x55,
		KEY_V           = 0x56,
		KEY_W           = 0x57,
		KEY_X           = 0x58,
		KEY_Y           = 0x59,
		KEY_Z           = 0x5A,
		KEY_NUMPAD0     = 0x60,
		KEY_NUMPAD1     = 0x61,
		KEY_NUMPAD2     = 0x62,
		KEY_NUMPAD3     = 0x63,
		KEY_NUMPAD4     = 0x64,
		KEY_NUMPAD5     = 0x65,
		KEY_NUMPAD6     = 0x66,
		KEY_NUMPAD7     = 0x67,
		KEY_NUMPAD8     = 0x68,
		KEY_NUMPAD9     = 0x69,
		KEY_MULTIPLY    = 0x6A,
		KEY_ADD         = 0x6B,
		KEY_SEPARATOR   = 0x6C,
		KEY_SUBTRACT    = 0x6D,
		KEY_DECIMAL     = 0x6E,
		KEY_DIVIDE      = 0x6F,
		KEY_F1          = 0x70,
		KEY_F2          = 0x71,
		KEY_F3          = 0x72,
		KEY_F4          = 0x73,
		KEY_F5          = 0x74,
		KEY_F6          = 0x75,
		KEY_F7          = 0x76,
		KEY_F8          = 0x77,
		KEY_F9          = 0x78,
		KEY_F10         = 0x79,
		KEY_F11         = 0x7A,
		KEY_F12         = 0x7B,
		KEY_NUMLOCK     = 0x90,
		KEY_SCROLL      = 0x91,
		KEY_LSHIFT      = 0xA0,
		KEY_RSHIFT      = 0xA1,
		KEY_LCONTROL    = 0xA2,
		KEY_RCONTROL    = 0xA3
	};

	// NOTE: These codes are registered in inputengine_script.cpp.
	// Any changes to these enums must also adjust the above file.
	enum KEY_COMMANDS {
		KEY_COMMAND_ENTER = 1,
		KEY_COMMAND_LEFT = 2,
		KEY_COMMAND_RIGHT = 3,
		KEY_COMMAND_HOME = 4,
		KEY_COMMAND_END = 5,
		KEY_COMMAND_BACKSPACE = 6,
		KEY_COMMAND_TAB = 7,
		KEY_COMMAND_INSERT = 8,
		KEY_COMMAND_DELETE = 9
	};

	/// --------------------------------------------------------------
	/// THESE METHODS MUST BE IMPLEMENTED BY THE INPUT ENGINE
	/// --------------------------------------------------------------

	/**
	 * Initialises the input engine
	 * @return          Returns a true on success, otherwise false.
	 */
	bool init();

	/**
	 * Performs a "tick" of the input engine.
	 *
	 * This method should be called once per frame. It can be used by implementations
	 * of the input engine that are not running in their own thread, or to perform
	 * additional administrative tasks that are needed.
	 */
	void update();

	/**
	 * Returns true if the left mouse button is pressed
	 */
	bool isLeftMouseDown();

	/**
	 * Returns true if the right mouse button is pressed.
	*/
	bool isRightMouseDown();

	/**
	 * Returns true if the left mouse button was pressed and released.
	 *
	 * The difference between this and IsLeftMouseDown() is that this only returns
	 * true when the left mouse button is released.
	*/
	bool wasLeftMouseDown();

	/**
	 * Returns true if the right mouse button was pressed and released.
	 *
	 * The difference between this and IsRightMouseDown() is that this only returns
	 * true when the right mouse button is released.
	*/
	bool wasRightMouseDown();

	/**
	 * Returns true if the left mouse button double click was done
	 */
	bool isLeftDoubleClick();

	/**
	 * Returns the X position of the cursor in pixels
	*/
	int getMouseX();

	/**
	 * Returns the Y position of the cursor in pixels
	 */
	int getMouseY();

	/**
	 * Sets the X position of the cursor in pixels
	 */
	void setMouseX(int posX);

	/**
	 * Sets the Y position of the cursor in pixels
	 */
	void setMouseY(int posY);

	/**
	 * Returns true if a given key was pressed
	 * @param KeyCode       The key code to be checked
	 * @return              Returns true if the given key is done, otherwise false.
	 */
	bool isKeyDown(uint keyCode);

	/**
	 * Returns true if a certain key was pushed and released.
	 *
	 * The difference between IsKeyDown() is that this only returns true after the key
	 * has been released. This method facilitates the retrieval of keys, and reading
	 * strings that users type.
	 * @param KeyCode       The key code to be checked
	 */
	bool wasKeyDown(uint keyCode);

	typedef CallbackPtr CharacterCallback;

	/**
	 * Registers a callback function for keyboard input.
	 *
	 * The callbacks that are registered with this function will be called whenever an
	 * input key is pressed. A letter entry is different from the query using the
	 * methods IsKeyDown () and WasKeyDown () in the sense that are treated instead
	 * of actual scan-coded letters. These were taken into account, among other things:
	 * the keyboard layout, the condition the Shift and Caps Lock keys and the repetition
	 * of longer holding the key.
	 * The input of strings by the user through use of callbacks should be implemented.
	 * @return              Returns true if the function was registered, otherwise false.
	*/
	bool registerCharacterCallback(CallbackPtr callback);

	/**
	 * De-registeres a previously registered callback function.
	 * @return              Returns true if the function could be de-registered, otherwise false.
	 */
	bool unregisterCharacterCallback(CallbackPtr callback);

	typedef CallbackPtr CommandCallback;

	/**
	 * Registers a callback function for the input of commands that can have influence on the string input
	 *
	 * The callbacks that are registered with this function will be called whenever the input service
	 * has a key that affects the character string input. This could be the following keys:
	 * Enter, End, Left, Right, ...
	 * The input of strings by the user through the use of callbacks should be implemented.
	 * @return              Returns true if the function was registered, otherwise false.
	 */
	bool registerCommandCallback(CallbackPtr callback);

	/**
	 * Un-register a callback function for the input of commands that can have an influence on the string input.
	 * @return              Returns true if the function could be de-registered, otherwise false.
	 */
	bool unregisterCommandCallback(CommandCallback callback);

	void reportCharacter(byte character);
	void reportCommand(KEY_COMMANDS command);

	bool persist(OutputPersistenceBlock &writer);
	bool unpersist(InputPersistenceBlock &reader);

private:
	bool registerScriptBindings();
	void unregisterScriptBindings();

private:
	void testForLeftDoubleClick();
	void alterKeyboardState(int keycode, byte newState);

	byte _keyboardState[2][256];
	bool _leftMouseState[2];
	bool _rightMouseState[2];
	uint _currentState;
	int _mouseX;
	int _mouseY;
	bool _leftMouseDown;
	bool _rightMouseDown;
	bool _leftDoubleClick;
	uint _doubleClickTime;
	int _doubleClickRectWidth;
	int _doubleClickRectHeight;
	uint _lastLeftClickTime;
	int _lastLeftClickMouseX;
	int _lastLeftClickMouseY;
	Common::List<CommandCallback> _commandCallbacks;
	Common::List<CharacterCallback> _characterCallbacks;
};

} // End of namespace Sword25

#endif
