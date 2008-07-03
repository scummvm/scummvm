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

#ifndef GUI_VIRTUAL_KEYBOARD_H
#define GUI_VIRTUAL_KEYBOARD_H

class OSystem;

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/imagemap.h"
#include "common/singleton.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace GUI {

class VirtualKeyboardParser;



class VirtualKeyboard : public Common::Singleton<VirtualKeyboard> {
private:
	/** Type of key event */
	enum EventType {
		kEventKey,
		kEventSwitchMode,

		kEventMax
	};

	struct Event {
		Common::String name;
		EventType type;
		void *data;
	};
	
	typedef Common::HashMap<Common::String, Event, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> EventMap; 

	struct Mode {
		Common::String     name;
		Common::String     resolution;
		Common::String     bitmapName;
		Graphics::Surface *image;
		Common::ImageMap   imageMap;
		EventMap           events;
	};

public:
	VirtualKeyboard();
	virtual ~VirtualKeyboard();

	bool loadKeyboardPack(Common::String packName);
	void show();

private:
	OSystem	*_system;
	
	friend class VirtualKeyboardParser;
	VirtualKeyboardParser *_parser;

	void runLoop();
	void draw();

	Common::HashMap<Common::String, Mode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _modes;
};


} // End of namespace GUI


#endif
