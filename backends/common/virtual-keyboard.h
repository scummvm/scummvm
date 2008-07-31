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

namespace Common {

class VirtualKeyboardGUI;
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
		void *data;
		
		Event() : data(0) {}
		~Event() {
			if (data) {
				switch (type) {
				case kEventKey:
					delete (KeyState*)data;
					break;
				case kEventModifier:
					delete (byte*)data;
					break;
				case kEventSwitchMode:
					delete (String*)data;
					break;
				case kEventClose:
					break;
				}
			}
		}
	};
	
	typedef Common::HashMap<Common::String, Event*> EventMap; 

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

	class KeyPressQueue {
	public:
		KeyPressQueue();
		void toggleFlags(byte fl);
		void clearFlags();
		void insertKey(KeyState key);
		void deleteKey();
		void moveLeft();
		void moveRight();
		KeyState pop();
		void clear();
		bool empty();
		String getString();
		bool hasStringChanged();

	private:
		byte _keyFlags;

		List<VirtualKeyPress> _keys;
		String _str;

		bool _strChanged;

		List<VirtualKeyPress>::iterator _keyPos;
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
	bool isDisplaying();

	/**
	  * Returns true if the keyboard is loaded and ready to be shown
	  */
	bool isLoaded() {
		return _loaded;
	}

protected:	// TODO : clean up all this stuff

	OSystem *_system;

	friend class VirtualKeyboardGUI;
	VirtualKeyboardGUI	*_kbdGUI;

	KeyPressQueue _keyQueue;
	
	friend class VirtualKeyboardParser;
	VirtualKeyboardParser *_parser;

	void reset();
	void deleteEvents();
	bool checkModeResolutions();
	void switchMode(Mode *newMode);
	void switchMode(const Common::String& newMode);
	void handleMouseDown(int16 x, int16 y);
	void handleMouseUp(int16 x, int16 y);
	String findArea(int16 x, int16 y);
	void processAreaClick(const Common::String &area);

	bool _loaded;

	ModeMap _modes;
	Mode *_initialMode;
	Mode *_currentMode;

	HorizontalAlignment  _hAlignment;
	VerticalAlignment    _vAlignment;

	String _areaDown;

};


} // End of namespace GUI


#endif
