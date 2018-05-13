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
 */

#ifndef STARTREK_H
#define STARTREK_H

#include "common/events.h"
#include "common/list.h"
#include "common/ptr.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/util.h"

#include "engines/engine.h"

#include "startrek/filestream.h"
#include "startrek/graphics.h"
#include "startrek/object.h"
#include "startrek/room.h"
#include "startrek/sound.h"


using Common::SharedPtr;

namespace Common {
	class MacResManager;
}

namespace StarTrek {

enum StarTrekGameType {
	GType_ST25 = 1,
	GType_STJR = 2
};

enum StarTrekGameFeatures {
	GF_DEMO =    (1 << 0)
};

enum kDebugLevels {
	kDebugSound =     1 << 0,
	kDebugGraphics =  1 << 1
};


enum GameMode {
	GAMEMODE_START = 0,
	GAMEMODE_BRIDGE,
	GAMEMODE_AWAYMISSION,
	GAMEMODE_BEAMDOWN,
	GAMEMODE_BEAMUP
};

enum TextDisplayMode {
	TEXTDISPLAY_WAIT = 0,  // Wait for input before closing text
	TEXTDISPLAY_SUBTITLES, // Automatically continue when speech is done
	TEXTDISPLAY_NONE       // No text displayed
};

enum TrekEventType {
	TREKEVENT_TICK = 0, // DOS clock changes (see updateClockTicks)
	TREKEVENT_LBUTTONDOWN = 1,
	TREKEVENT_MOUSEMOVE = 2,
	TREKEVENT_LBUTTONUP = 3,
	TREKEVENT_RBUTTONDOWN = 4,
	TREKEVENT_RBUTTONUP = 5,
	TREKEVENT_KEYDOWN = 6
};

struct TrekEvent {
	TrekEventType type;
	Common::KeyState kbd;
	Common::Point mouse;
	uint32 tick;
};

const int MAX_OBJECTS = 0x20;

struct StarTrekGameDescription;
class Graphics;
class Sound;

class StarTrekEngine : public ::Engine {
protected:
	Common::Error run();

private:
	// Game modes
	Common::Error runGameMode(int mode);
	void runTransportSequence(const Common::String &name);

public:
	StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc);
	virtual ~StarTrekEngine();

	// Running the game
	Room *getRoom();

	void playSoundEffectIndex(int index);
	void playSpeech(const Common::String &filename);
	void stopPlayingSpeech();

	// Objects
	void initObjects();
	int loadAnimationForObject(int objectIndex, const Common::String &animName, uint16 x, uint16 y, uint16 arg8);
	void updateObjectAnimations();
	void removeObjectFromScreen(int objectIndex);
	void objectFunc1();
	void drawObjectToScreen(Object *object, const Common::String &animName, uint16 field5e, uint16 field60, uint16 arg8, bool addSprite);
	void releaseAnim(Object *object);

	SharedPtr<Bitmap> loadAnimationFrame(const Common::String &filename, uint16 arg2);
	Common::String getCrewmanAnimFilename(int objectIndex, const Common::String &basename);

	// Events
public:
	void pollSystemEvents();
	void initializeEventsAndMouse();
	bool getNextEvent(TrekEvent *e);
	void removeNextEvent();
	bool popNextEvent(TrekEvent *e);
	void addEventToQueue(const TrekEvent &e);
	void clearEventBuffer();
	uint32 getClockTicks();
	void updateEvents();
	void updateTimerEvent();
	void updateMouseEvents();
	void updateKeyboardEvents();
	void updateClockTicks();
	bool checkKeyPressed();

	Common::EventManager *getEventMan() { return _eventMan; }

private:
	Common::List<TrekEvent> _eventQueue;
	bool _mouseMoveEventInQueue;
	bool _tickEventInQueue;
	uint32 _frameStartMillis;

public:
	// Detection related functions
	const StarTrekGameDescription *_gameDescription;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;
	uint8 getGameType();
	Common::Language getLanguage();

	// Resource related functions
	SharedPtr<FileStream> loadFile(Common::String filename, int fileIndex=0);

	// Movie related functions
	void playMovie(Common::String filename);
	void playMovieMac(Common::String filename);


public:
	int _gameMode;
	int _lastGameMode;
	bool _redshirtDead;
	Common::String _missionToLoad;

	Object _objectList[MAX_OBJECTS];
	Object * const _kirkObject;
	Object * const _spockObject;
	Object * const _mccoyObject;
	Object * const _redshirtObject;

	SharedPtr<FileStream> _objectBanFiles[MAX_OBJECTS / 2];
	uint16 _objectBanVar2[MAX_OBJECTS / 2]; // TODO: initialize?

	uint32 _clockTicks;
	uint32 _frameIndex;

	bool _musicEnabled;
	bool _sfxEnabled;
	uint16 _word_467a6;
	uint16 _musicWorking;
	bool _sfxWorking;
	bool _finishedPlayingSpeech;

	bool _mouseControllingShip;

	// TODO: make this work.
	// When false, the keyboard generally acts in a more specific way (ie. move mouse
	// between items in a menu).
	bool _keyboardControlsMouse;

	bool _inQuitGameMenu;

	
	Graphics *_gfx;
	Sound *_sound;

private:
	Common::MacResManager *_macResFork;
	Room *_room;
};

} // End of namespace StarTrek

#endif
