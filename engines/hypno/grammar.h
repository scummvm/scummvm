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

class HypnoSmackerDecoder : public Video::SmackerDecoder {
public:
	bool loadStream(Common::SeekableReadStream *stream) override;
};

class MVideo {
public:
	MVideo(Filename, Common::Point, bool, bool, bool);
	Filename path;
	Common::Point position;
	bool scaled;
	bool transparent;
	bool loop;
	HypnoSmackerDecoder *decoder;
	const Graphics::Surface *currentFrame;
};

typedef Common::Array<MVideo> Videos;

enum HotspotType {
	MakeMenu,
	MakeHotspot
};

enum ActionType {
	MiceAction,
	TimerAction,	
	PaletteAction,
	BackgroundAction,
	OverlayAction,
	EscapeAction,
	SaveAction,
	LoadAction,
	LoadCheckpointAction,
	QuitAction,
	CutsceneAction,
	PlayAction,
	IntroAction,
	AmbientAction,
	WalNAction,
	GlobalAction,
	TalkAction,
	ChangeLevelAction
};

class Action {
public:
	virtual ~Action() {} // needed to make Action polymorphic
	ActionType type;
};

typedef Common::Array<Action *> Actions;

class Hotspot;

typedef Common::Array<Hotspot> Hotspots;
typedef Common::Array<Hotspots *> HotspotsStack;

class Hotspot {
public:
	Hotspot(HotspotType type_, Common::Rect rect_ = Common::Rect(0, 0, 0, 0)) {
		type = type_;
		rect = rect_;
		smenu = nullptr;
	}
	HotspotType type;
	Common::String flags[3];
	Common::Rect rect;
	Common::String setting;
	Actions actions;
	Hotspots *smenu;
};

class Mice : public Action {
public:
	Mice(Filename path_, uint32 index_) {
		type = MiceAction;
		path = path_;
		index = index_;
	}
	Filename path;
	uint32 index;
};

class Timer : public Action {
public:
	Timer(uint32 delay_) {
		type = TimerAction;
		delay = delay_;
	}
	uint32 delay;
};

class Palette : public Action {
public:
	Palette(Filename path_) {
		type = PaletteAction;
		path = path_;
	}
	Filename path;
};

class Background : public Action {
public:
	Background(Filename path_, Common::Point origin_, Common::String condition_, Common::String flag1_, Common::String flag2_) {
		type = BackgroundAction;
		path = path_;
		origin = origin_;
		condition = condition_;
		flag1 = flag1_;
		flag2 = flag2_;
	}
	Filename path;
	Common::Point origin;
	Common::String condition;
	Common::String flag1;
	Common::String flag2;
};

class Overlay : public Action {
public:
	Overlay(Filename path_, Common::Point origin_, Common::String flag_) {
		type = OverlayAction;
		path = path_;
		origin = origin_;
		flag = flag_;
	}
	Filename path;
	Common::Point origin;
	Common::String flag;
};

class Escape : public Action {
public:
	Escape() {
		type = EscapeAction;
	}
};

class Save : public Action {
public:
	Save() {
		type = SaveAction;
	}
};

class Load : public Action {
public:
	Load() {
		type = LoadAction;
	}
};

class LoadCheckpoint : public Action {
public:
	LoadCheckpoint() {
		type = LoadCheckpointAction;
	}
};

class Quit : public Action {
public:
	Quit() {
		type = QuitAction;
	}
};

class Cutscene : public Action {
public:
	Cutscene(Filename path_) {
		type = CutsceneAction;
		path = path_;
	}
	Filename path;
};

class Intro : public Action {
public:
	Intro(Filename path_) {
		type = IntroAction;
		path = path_;
	}
	Filename path;
};

class Play : public Action {
public:
	Play(Filename path_, Common::Point origin_, Common::String condition_, Common::String flag_) {
		type = PlayAction;
		path = path_;
		origin = origin_;
		condition = condition_;
		flag = flag_;
	}
	Filename path;
	Common::Point origin;
	Common::String condition;
	Common::String flag;
};

class Ambient : public Action {
public:
	Ambient(Filename path_, Common::Point origin_, Common::String flag_) {
		type = AmbientAction;
		path = path_;
		origin = origin_;
		flag = flag_;
		fullscreen = false;
		frameNumber = 0;
	}
	Filename path;
	Common::Point origin;
	Common::String flag;
	uint32 frameNumber;
	bool fullscreen;
};

class WalN : public Action {
public:
	WalN(Common::String wn_, Filename path_, Common::Point origin_, Common::String condition_, Common::String flag_) {
		wn = wn_;
		type = WalNAction;
		path = path_;
		origin = origin_;
		condition = condition_;
		flag = flag_;
	}
	Common::String wn;
	Filename path;
	Common::Point origin;
	Common::String condition;
	Common::String flag;
};

class Global : public Action {
public:
	Global(Common::String variable_, Common::String command_) {
		type = GlobalAction;
		variable = variable_;
		command = command_;
	}
	Common::String variable;
	Common::String command;
};

class TalkCommand {
public:
	Common::String command;
	Common::String variable;
	Filename path;
	uint32 num;
	Common::Point position;
};

typedef Common::Array<TalkCommand> TalkCommands;

class Talk : public Action {
public:
	Talk()  {
		type = TalkAction;
		boxPos = Common::Point(0, 0);
		escape = false;
	}
	TalkCommands commands;
	bool active;
	bool escape;
	Common::Point introPos;
	Filename intro;
	Common::Point boxPos;
	Filename background;
	Common::Point backgroundPos;
	Common::Rect rect;
	Filename second;
	Common::Point secondPos;
};

class ChangeLevel : public Action {
public:
	ChangeLevel(Filename level_) {
		type = ChangeLevelAction;
		level = level_;
	}
	Filename level;
};

enum LevelType {
	TransitionLevel,
	SceneLevel,
	ArcadeLevel,
	CodeLevel
};

class Level {
public:
	virtual ~Level() {} // needed to make Level polymorphic
	LevelType type;
	Filenames intros;
	Filename prefix;
	Filename levelIfWin;
	Filename levelIfLose;
	Filename music;
};

class Scene : public Level {
public:
	Scene()  {
		type = SceneLevel;
	}
	Hotspots hots;
};

class Shoot {
public:
	Shoot() {
		destroyed = false;
		video = nullptr;
		timesToShoot = 0;
		pointsToShoot = 0;
		attackWeight = 0;
		paletteOffset = 0;
		paletteSize = 0;
		attackFrame = 0;
		explosionFrame = 0;
	}
	Common::String name;
	Filename animation;
	Filename startSound;
	Common::Point position;

	uint32 timesToShoot;
	uint32 pointsToShoot;
	uint32 attackWeight;

	// Palette
	uint32 paletteOffset;
	uint32 paletteSize;

	// Sounds
	Filename deathSound;
	Filename hitSound;

	MVideo *video;
	uint32 attackFrame;
	uint32 explosionFrame;
	
	bool destroyed;
};

typedef Common::Array<Shoot> Shoots;

class ShootInfo {
public:
	Common::String name;
	uint32 timestamp;
};

typedef Common::List<ShootInfo> ShootSequence;
typedef Common::Array<Common::String> Sounds;

class ArcadeShooting : public Level {
public:
	ArcadeShooting()  {
		type = ArcadeLevel;
		health = 100;
		transitionTime = 0;
	}
	uint32 id;
	Common::String mode;
	uint32 transitionTime;

	// Videos
	Filename transitionVideo;
	Filename nextLevelVideo;
	Filename defeatNoEnergyVideo;
	Filename defeatMissBossVideo;

	Filename background;
	Filename player;
	Filename palette;
	int health;
	Shoots shoots;
	ShootSequence shootSequence;

	// Sounds 
	Filename backgroundSound;
	Filename targetSound;
	Filename shootSound;
	Filename enemySound;
	Filename hitSound;
};

class Transition : public Level {
public:
	Transition(Common::String level)  {
		type = TransitionLevel;
		nextLevel = level;
		levelEasy = "";
		levelHard = "";
		frameNumber = 0;
	}
	
	Transition(Common::String easy, Common::String hard)  {
		type = TransitionLevel;
		levelEasy = easy;
		levelHard = hard;
		frameNumber = 0;
	}
	Common::String nextLevel;
	Common::String levelEasy;
	Common::String levelHard;
	Filename frameImage;
	uint32 frameNumber;
};

class Code : public Level {
public:
	Code()  {
		type = CodeLevel;
	}
	Common::String name;
};

typedef Common::HashMap<Filename, Level*> Levels;
extern Hotspots *g_parsedHots;
extern ArcadeShooting *g_parsedArc;

} // End of namespace Hypno

#endif

