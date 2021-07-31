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

#ifndef HYPNO_GRAMMAR_H
#define HYPNO_GRAMMAR_H

#include "common/str.h"
#include "common/hash-str.h"
#include "common/hash-ptr.h"
#include "common/queue.h"
#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"

namespace Hypno {

enum HotspotType {
    MakeMenu,
 	MakeHotspot
};

class Action {
	public:
	virtual ~Action() {}; // needed to make Action polymorphic
};

typedef Common::List<Action*> Actions;

class Hotspot;
typedef Common::Array<Hotspot> Hotspots;
typedef Common::Array<Hotspots *> HotspotsStack;
typedef Common::List<Common::String> Movies;

class Hotspot { 
  public:
    HotspotType type;
    Common::String stype;
	Common::String stypeFlag;
	Common::Rect rect;
	Common::String setting;
	Actions actions;
	Hotspots *smenu;
};

class Mice : public Action {
  public:
	Common::String path;
	uint32 index;
};

class Palette : public Action {
  public:
	Common::String path;
};

class Background : public Action {
  public:
	Common::String path;
	Common::Point origin;
	Common::String condition;
};

class Overlay : public Action {
  public:
	Common::String path;
	Common::Point origin;
	Common::String flag;
};

class Escape : public Action {
};

class Quit : public Action {
};

class Cutscene : public Action {
  public:
	Common::String path;
};

class Play : public Action {
  public:
	Common::String path;
	Common::Point origin;
	Common::String condition;
	Common::String flag;
};

class WalN : public Action {
  public:
	Common::String path;
	Common::Point origin;
	Common::String condition;
	Common::String flag;
};

class Global : public Action {
  public:
	Common::String variable;
	Common::String command;
};

class Level {
  public:
	Hotspots hots;
	Movies intros;
};

typedef Common::HashMap<Common::String, Level> Levels;
extern Hotspots *g_parsedHots;
//extern Settings g_settings;

} // End of namespace Hypno

#endif
