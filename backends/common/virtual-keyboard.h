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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_VIRTUAL_KEYBOARD_H
#define COMMON_VIRTUAL_KEYBOARD_H

class OSystem;

#include "common/events.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/image-map.h"
#include "common/keyboard.h"
#include "common/list.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace Common {

class VirtualKeyboardParser;

class VirtualKeyboard {
protected:
	enum EventType {
		kEventKey,
		kEventModifier,
		kEventSwitchMode,
		kEventClose
	};

	struct Event {
		Common::String name;
		EventType type;
		Graphics::Surface *rollOverImage;
		void *data;
	};
	
	typedef Common::HashMap<Common::String, Event, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> EventMap; 

	struct Mode {
		Common::String		name;
		Common::String		resolution;
		Common::String		bitmapName;
		Graphics::Surface	*image;
		OverlayColor		transparentColor;
		Common::ImageMap	imageMap;
		EventMap			events;
		Common::Rect		*previewArea;

		Mode() : image(0), previewArea(0) {}
	};
	
	typedef Common::HashMap<Common::String, Mode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ModeMap;

	enum HorizontalAlignment {
		kAlignLeft,
		kAlignCentre,
		kAlignRight
	};

	enum VerticalAlignment {
		kAlignTop,
		kAlignMiddle,
		kAlignBottom
	};

	struct VirtualKeyPress {
		Common::KeyState key;
		uint strLen;
	};

	class Queue {
	public:
		Queue();
		void insertKey(KeyState key);
		void deleteKey();
		void moveLeft();
		void moveRight();
		KeyState pop();
		void clear();
		bool empty();
		String getString();

	private:
		List<VirtualKeyPress> _keys;
		List<VirtualKeyPress>::iterator _keyPos;

		String _str;
		uint _strPos;
	};

public:
	VirtualKeyboard();
	virtual ~VirtualKeyboard();
	
	/**
	  * Loads the keyboard pack with the given name.
	  * The system first looks for an uncompressed keyboard pack by searching 
	  * for packName.xml in the filesystem, if this does not exist then it 
	  * searches for a compressed keyboard pack by looking for packName.zip.
	  * @param packName name of the keyboard pack
	  */
	bool loadKeyboardPack(Common::String packName);

	/**
	  * Shows the keyboard, starting an event loop that will intercept all
	  * user input (like a modal GUI dialog).
	  * It is assumed that the game has been paused, before this is called
	  */
	void show();

	/**
	  * Hides the keyboard, ending the event loop.
	  */
	void hide();

	/**
	  * Returns true if the keyboard is currently being shown
	  */
	bool isDisplaying() { 
		return _displaying;
	}

	/**
	  * Returns true if the keyboard is loaded and ready to be shown
	  */
	bool isLoaded() {
		return _loaded;
	}

protected:	// TODO : clean up all this stuff
	OSystem	*_system;

	byte _keyFlags;
	Queue _keyQueue;
	KeyState *_keyDown;


	static const int SNAP_WIDTH = 10;
	
	friend class VirtualKeyboardParser;
	VirtualKeyboardParser *_parser;

	void reset();
	void deleteEventData();
	void screenChanged();
	bool checkModeResolutions();
	void setDefaultPosition();
	void move(int16 x, int16 y);
	void switchMode(Mode *newMode);
	void switchMode(const Common::String& newMode);
	String findArea(int16 x, int16 y);
	void processClick(const Common::String &area);
	void runLoop();
	void redraw();

	Graphics::Surface _overlayBackup;

	bool _loaded;
	bool _displaying;
	bool _needRedraw;
	bool _firstRun;

	ModeMap _modes;
	Mode *_initialMode;
	Mode *_currentMode;

	int _lastScreenChanged;
	Rect _kbdBound;

	HorizontalAlignment  _hAlignment;
	VerticalAlignment    _vAlignment;

	String _areaDown;
	Point _dragPoint;
	bool _drag;

	static const int kCursorAnimateDelay = 250;
	int		_cursorAnimateCounter;
	int		_cursorAnimateTimer;
	byte		_cursor[2048];
	void setupCursor();
	void removeCursor();
	void animateCursor();

};


} // End of namespace GUI


#endif
