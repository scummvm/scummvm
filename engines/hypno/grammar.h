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

#include "common/array.h"
#include "common/hash-ptr.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/str.h"

#include "video/smk_decoder.h"

namespace Hypno {

typedef Common::String Filename;
typedef Common::List<Filename> Filenames;

enum HotspotType {
	MakeMenu,
	MakeHotspot
};

class Action {
public:
	virtual ~Action(){}; // needed to make Action polymorphic
};

typedef Common::Array<Action *> Actions;

class Hotspot;

typedef Common::Array<Hotspot> Hotspots;
typedef Common::Array<Hotspots *> HotspotsStack;

class MVideo {
public:
	MVideo(Filename, Common::Point, bool, bool, bool);
	Filename path;
	Common::Point position;
	bool scaled;
	bool transparent;
	bool loop;
	Video::SmackerDecoder *decoder = nullptr;
	const Graphics::Surface *currentFrame = nullptr;
};

typedef Common::Array<MVideo> Videos;

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
	Filename path;
	uint32 index;
};

class Palette : public Action {
public:
	Filename path;
};

class Background : public Action {
public:
	Filename path;
	Common::Point origin;
	Common::String condition;
};

class Overlay : public Action {
public:
	Filename path;
	Common::Point origin;
	Common::String flag;
};

class Escape : public Action {
};

class Quit : public Action {
};

class Cutscene : public Action {
public:
	Filename path;
};

class Play : public Action {
public:
	Filename path;
	Common::Point origin;
	Common::String condition;
	Common::String flag;
};

class Ambient : public Action {
public:
	Filename path;
	Common::Point origin;
	Common::String flag;
	bool fullscreen;
};

class WalN : public Action {
public:
	Filename path;
	Common::Point origin;
	Common::String condition;
	Common::String flag;
};

class Global : public Action {
public:
	Common::String variable;
	Common::String command;
};

class TalkCommand {
public:
	Common::String command;
	Filename path;
	uint32 num;
	Common::Point position;
};

typedef Common::Array<TalkCommand> TalkCommands;

class Talk : public Action {
public:
	TalkCommands commands;
	bool active;
	Filename background;
	Common::Point position;
	Common::Rect rect;
};

class ChangeLevel : public Action {
public:
	Filename level;
};

class Shoot {
public:
	Common::String name;
	Filename animation;
	Filename startSound;
	Filename endSound;
	Common::Point position;
	int damage;
	MVideo *video = nullptr;
	uint32 explosionFrame;
	bool destroyed = false;
};

typedef Common::Array<Shoot> Shoots;

class ShootInfo {
public:
	Common::String name;
	uint32 timestamp;
};

typedef Common::List<ShootInfo> ShootSequence;
typedef Common::Array<Common::String> Sounds;

class Transition {
public:
	Common::String level;
	Filenames intros;
};

class Puzzle {
public:
	Common::String name;
	Filenames intros;
	Common::String levelIfWin;
	Common::String levelIfLose;
};

class Scene {
public:
	Filename intro;
	Common::String prefix;
	Hotspots hots;
	Filename sound;
	Common::String levelIfWin;
	Common::String levelIfLose;
};

class ArcadeShooting {
public:
	uint32 id;
	Common::String levelIfWin;
	Common::String levelIfLose;
	Filename transitionVideo;
	uint32 transitionTime;
	Filenames defeatVideos;
	Filenames winVideos;
	Filename intro;
	Filename prefix;
	Filename background;
	Filename player;
	int health;
	Shoots shoots;
	ShootSequence shootSequence;
	Filename shootSound;
	Filename enemySound;
	Filename hitSound;
	Filename music;
};

class Level {
public:
	Transition trans;
	Scene scene;
	ArcadeShooting arcade;
	Puzzle puzzle;
};

typedef Common::HashMap<Filename, Level> Levels;
extern Hotspots *g_parsedHots;
extern ArcadeShooting g_parsedArc;

} // End of namespace Hypno

#endif
