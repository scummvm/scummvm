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
	HighlightAction,
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
	SwapPointerAction,
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
typedef Common::Array<Graphics::Surface *> Frames;

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
	Filename background;
	Frames backgroundFrames;
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

class SwapPointer : public Action {
public:
	SwapPointer(uint32 index_) {
		type = SwapPointerAction;
		index = index_;
	}
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

class Highlight : public Action {
public:
	Highlight(Common::String condition_) {
		type = HighlightAction;
		condition = condition_;
	}
	Common::String condition;
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
		active = true;
	}

	Talk(Talk *t)  {
		*this = *t;
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
	Level() {
		type = CodeLevel;
		musicRate = 22050;
		playMusicDuringIntro = false;
	}
	virtual ~Level() {} // needed to make Level polymorphic
	LevelType type;
	Filenames intros;
	Filename prefix;
	Filename levelIfWin;
	Filename levelIfLose;
	bool playMusicDuringIntro;
	Filename music;
	uint32 musicRate;
};

class Scene : public Level {
public:
	Scene()  {
		type = SceneLevel;
		resolution = "640x480";
	}
	Common::String resolution;
	Hotspots hots;
};

class FrameInfo {
public:
	FrameInfo(uint32 start_, uint32 length_) {
		start = start_;
		length = length_;
	}

	uint32 lastFrame() {
		return start + length;
	}
	uint32 start;
	uint32 length;
};

enum ScriptMode {
	Interactive = 1,
	NonInteractive,
};

class ScriptInfo {
public:
	ScriptInfo(uint32 time_, uint32 mode_, uint32 actor_, uint32 cursor_) {
		time = time_;
		mode = ScriptMode(mode_);
		actor = actor_;
		cursor = cursor_;
	}
	uint32 time;
	ScriptMode mode;
	uint32 actor;
	uint32 cursor;
};

typedef Common::List<ScriptInfo> Script;

class Shoot {
public:
	Shoot() {
		destroyed = false;
		video = nullptr;
		timesToShoot = 1;
		pointsToShoot = 0;
		attackWeight = 0;
		paletteOffset = 0;
		paletteSize = 0;
		missedAnimation = 0;
		objKillsCount = 0;
		objMissesCount = 0;
		animation = "NONE";
		explosionAnimation = "";
		startFrame = 0;
		lastFrame = 1024;
		interactionFrame = 0;
		noEnemySound = false;
		isAnimal = false;
		nonHostile = false;
		playInteractionAudio = false;
		animalSound = "";
		jumpToTimeAfterKilled = 0;
		warningVideoIdx = 0;
	}
	Common::String name;
	Filename animation;
	Filename startSound;
	Common::Point position;
	Common::Point deathPosition;


	uint32 timesToShoot;
	uint32 pointsToShoot;
	uint32 attackWeight;

	// Objectives
	uint32 objKillsCount;
	uint32 objMissesCount;

	// Palette
	uint32 paletteOffset;
	uint32 paletteSize;

	// Missed animation
	uint32 missedAnimation;

	// Sounds
	Filename enemySound;
	Filename deathSound;
	Filename hitSound;
	Filename animalSound;

	MVideo *video;
	Common::List<uint32> attackFrames;
	Common::Array<FrameInfo> bodyFrames;
	Common::Array<FrameInfo> explosionFrames;
	uint32 startFrame;
	uint32 lastFrame;
	uint32 interactionFrame;
	Filename explosionAnimation;
	Filename additionalVideo;
	bool playInteractionAudio;
	bool destroyed;
	bool noEnemySound;

	// Soldier Boyz specific
	bool nonHostile;
	bool isAnimal;
	Common::String checkIfDestroyed;
	int jumpToTimeAfterKilled;
	uint32 warningVideoIdx;
};

typedef Common::Array<Shoot> Shoots;

class ShootInfo {
public:
	Common::String name;
	uint32 timestamp;
};

typedef Common::List<ShootInfo> ShootSequence;

class SegmentShoots {
public:
	SegmentShoots() {
		segmentRepetition = 0;
	}
	ShootSequence shootSequence;
	uint32 segmentRepetition;
};

typedef Common::Array<SegmentShoots> SegmentShootsSequence;
typedef Common::Array<Common::String> Sounds;

enum SegmentType {
	Straight,
	Select3,
	TurnLeft3,
	Straight3,
	TurnRight3,
	Select2,
	TurnLeft2,
	TurnRight2,
};

class Segment {
public:
	Segment(byte type_, uint32 start_, uint32 size_)  {
		type = type_;
		start = start_;
		size = size_;
		end = false;
	}

	byte type;
	uint32 start;
	uint32 size;
	bool end;
};

typedef Common::Array<Segment> Segments;

class ArcadeTransition {
public:
	ArcadeTransition(Filename video_, Filename palette_, Filename sound_, uint32 soundRate_, uint32 time_)  {
		video = video_;
		palette = palette_;
		sound = sound_;
		soundRate = soundRate_;
		loseLevel = false;
		jumpToTime = 0;
		time = time_;
	}

	Filename video;
	Filename palette;
	Filename sound;
	uint32 soundRate;
	bool loseLevel;
	uint32 jumpToTime;
	uint32 time;
};

typedef Common::List<ArcadeTransition> ArcadeTransitions;

class ArcadeShooting : public Level {
public:
	ArcadeShooting()  {
		type = ArcadeLevel;
		health = 100;
		id = 0;
		objKillsRequired[0] = 0;
		objKillsRequired[1] = 0;
		objMissesAllowed[0] = 0;
		objMissesAllowed[1] = 0;
		mouseBox = Common::Rect(0, 0, 320, 200);
		frameDelay = 0;
		targetSoundRate = 0; // TODO: unused
		shootSoundRate = 0;
		enemySoundRate = 0;
		hitSoundRate = 0;
		additionalSoundRate = 0;
	}
	void clear() {
		nextLevelVideo.clear();
		postStatsVideo.clear();
		backgroundVideo.clear();
		transitions.clear();
		maskVideo.clear();
		player.clear();
		shoots.clear();
		intros.clear();
		defeatNoEnergyFirstVideo.clear();
		defeatMissBossVideo.clear();
		defeatNoEnergySecondVideo.clear();
		missBoss1Video.clear();
		missBoss2Video.clear();
		hitBoss1Video.clear();
		hitBoss2Video.clear();
		beforeVideo.clear();
		briefingVideo.clear();
		additionalVideo.clear();
		additionalSound.clear();
		noAmmoSound.clear();
		segments.clear();
		script.clear();
		objKillsRequired[0] = 0;
		objKillsRequired[1] = 0;
		objMissesAllowed[0] = 0;
		objMissesAllowed[1] = 0;
		mouseBox = Common::Rect(0, 0, 320, 200);
		targetSoundRate = 0;
		shootSoundRate = 0;
		enemySoundRate = 0;
		hitSoundRate = 0;
		noAmmoSoundRate = 0;
	}

	uint32 id;
	uint32 frameDelay;
	Common::String mode;
	Common::Rect mouseBox;
	ArcadeTransitions transitions;
	Segments segments;

	// Objectives
	uint32 objKillsRequired [2];
	uint32 objMissesAllowed [2];

	// Script
	Script script;

	// Videos
	Filename nextLevelVideo;
	Filename postStatsVideo;
	Filename defeatNoEnergyFirstVideo;
	Filename defeatNoEnergySecondVideo;
	Filename defeatMissBossVideo;
	Filename hitBoss1Video;
	Filename missBoss1Video;
	Filename hitBoss2Video;
	Filename missBoss2Video;
	Filename beforeVideo;
	Filename additionalVideo;
	Filename briefingVideo;

	Filename backgroundVideo;
	Filename backgroundPalette;
	Filename maskVideo;
	Filename player;
	int health;
	Shoots shoots;
	SegmentShootsSequence shootSequence;

	// Sounds
	Filename targetSound;
	uint32 targetSoundRate;
	Filename shootSound;
	uint32 shootSoundRate;
	Filename enemySound;
	uint32 enemySoundRate;
	Filename hitSound;
	uint32 hitSoundRate;
	Filename additionalSound;
	uint32 additionalSoundRate;
	Filename noAmmoSound;
	uint32 noAmmoSoundRate;
};

class Transition : public Level {
public:
	Transition(Common::String level)  {
		type = TransitionLevel;
		nextLevel = level;
		levelEasy = "";
		levelHard = "";
		frameNumber = 0;
		frameImage = "";
	}

	Transition(Common::String easy, Common::String hard)  {
		type = TransitionLevel;
		levelEasy = easy;
		levelHard = hard;
		frameNumber = 0;
		frameImage = "";
	}
	Common::String nextLevel;
	Common::String levelEasy;
	Common::String levelHard;
	Filename frameImage;
	uint32 frameNumber;
};

class Code : public Level {
public:
	Code(Common::String name_)  {
		type = CodeLevel;
		name = name_;
	}
	Common::String name;
};

typedef Common::HashMap<Filename, Level*> Levels;
extern Hotspots *g_parsedHots;
extern ArcadeShooting *g_parsedArc;

} // End of namespace Hypno

#endif

