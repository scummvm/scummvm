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

#ifndef TESORO_WORLD_H
#define TESORO_WORLD_H

#include "common/scummsys.h"

#include "tesoro/core/clickcatcher.h"
#include "tesoro/core/cursor.h"
#include "tesoro/core/actionstrip.h"
#include "tesoro/core/inventoryitem.h"
#include "tesoro/core/objectgroup.h"
#include "tesoro/core/jim.h"
#include "tesoro/core/character.h"
#include "tesoro/core/objectlist.h"
#include "tesoro/core/sounddevice.h"
#include "tesoro/core/speechtable.h"
#include "tesoro/core/walkarea.h"

namespace Tesoro {

class Jim;
class Level;
class Object;
class Scripts;
class TesoroEngine;

/**
 * The game's world as the objects see it: the table of levels (loaded when first needed), the main object list, the
 * spoken lines, Jim, the pointer and what it carries, the sound.
 */
class World {
public:
	enum { kLevelCount = 47 };

	explicit World(TesoroEngine &engine);
	~World();

	bool loadGameData();

	/** Once a frame: the sounds that have finished are let go. */
	void updateSound();

	/** Starts a script block (by its name) at a label for a level; the variables are what the script reads and sets. */
	int startScriptAt(Level *level, const char *block, const char *label, byte *variables);
	int startScriptInJimLevel(const char *block, const char *label, byte *variables);
	int startScriptInLevel(int level, const char *block, const char *label, byte *variables);
	int startScript(Character &who, const char *block, const char *label, byte *variables);
	int startScript(InteractiveObject &who, const char *block, const char *label, byte *variables);


	bool jimEntered() { return _jimEntered; }
	void setJimEntered(bool entered) { _jimEntered = entered; }

	/** Reads a level file and builds its objects, hosted by this world. */
	Level *loadLevelFile(const char *name, int mode, int index = -1);
	/** The level at a place of the game's list, loaded on first use (null when it cannot be read). */
	Level *level(int index);
	/** The place of a level file's name in the game's list, -1 if it is not one of them. */
	int levelIndexOf(const char *name) const;

	MainList &mainList() { return _main; }
	/** The second step of a level in the frame (its cameras, and for the player's level the picture shown). */
	int levelSecondStep(Level &level);
	SpeechTables &speech() { return _speech; }
	Scripts *scripts() { return _scripts; }

	/** The protagonist */
	Jim *jim();

	bool putJimInLevel(int index, int32 x, int32 y);
	/** Puts Jim into the first room of a level; false if there is no such level. */
	bool putJimInLevel(Level *level, int32 x, int32 y);

	/** Put Jim in the port and the opening script. */
	Level *startGame();

	int32 now();
	const SpeechTable::Line *phrase(const char *name);
	bool jimSpeaking();
	bool isJim(const Object *object);
	int jimFacing();
	void jimFace(int facing);
	void jimStopTalking();
	void jimForgetLine();
	void jimSayAnswer(const SpeechTable::Line &line);
	void jimWalkTo(int32 x, int32 y, WalkTarget *target);
	void jimUpdate();
	int jimPhase() { return _jimPhase; }
	int jimStep() { return _jimStep; }
	int jimExtra() { return _jimExtra; }
	void setPointerActive(bool active);
	void setHoverText(const char *text);

	/** The main list takes each object once: never insert the same object twice. */
	void insertInMainList(Object *object, int priority);
	void insertInMainListFront(Object *object, int priority);
	void removeFromMainList(Object *object);
	void clearMainList() { _main.clear(); }
	bool combinationReply(uint64 objectKey, const SpeechTable::Line *&line);
	/** The item's picture shows in the strip again, then the pointer lets go of it. */
	void returnPointerItem();

	Common::SeekableReadStream &archiveStream();

	int useItemOnInterface();
	bool hasWalkArea();
	/** The size of things at a point (0x100 is life size; 0x100 too without a floor). */
	int32 scaleAt(int32 x, int32 y);
	bool findPath(int32 destX, int32 destY, WalkTarget *target);
	bool hasWalkAreaOf(Object *other);
	int jimClicked(int verb);
	int jimRunVerb();

	Object *findObject(int level, const char *name);
	Object *findObjectByKey(int level, uint64 key);
	/** The object is switched on (for the interface items: the player has them). A missing object warns and counts as off. */
	bool isActive(int level, const char *name);
	InventoryItem *pointerItem();
	void setPointerItem(InventoryItem *item);
	void cursorFrame(int first, int last);
	int32 interfaceCameraX();
	void hideInventory();
	bool pointerHasItem();

	Level *levelAt(int index) { return level(index); }
	/** The level Jim is in, or null. */
	Level *jimLevel();
	void playMusicTrack(byte track);
	void setJimStep(byte step) { _jimPhase = 0; _jimStep = step; }
	void jimShow(bool visible);
	/** The pointer lets go of its item and shows the arrow */
	void clearPointerItem();

	int jimLevelIndex();
	void jimMoveTo(int32 x, int32 y);
	void jimRemoveFromMainList();
	void resetRenderQueue();
	void drawRooms(Level *level);
	void composeFrame();
	void clearScreens();
	void fadeOut(int effect, int speed);
	void wait(int milliseconds);
	void setJimStepKeep(byte step) { _jimStep = step; }
	/** Plays frames first to last of an object's first animation. */
	void playFrames(Object &object, int first, int last);
	/** The parts of a level's walk area: the floor, and the route round obstacles that the floor names. */
	enum WalkPart { kFloor, kRoute };
	/** Switches a level's walk area to other frames, which changes where Jim can walk. */
	void setWalkAreaFrames(Level *level, WalkPart part, int first, int last);
	void startMovie(int number);

	void walkJimToPointer(int32 mouseX, int32 mouseY);

	uint32 playVoice(uint32 archiveOffset);
	bool voiceActive(uint32 id);
	void stopVoice(uint32 id);

	/** Makes the pointer and the interface level known (they are loaded by the engine). */
	void setCursor(Cursor *cursor) {
		_cursor = cursor;
		if (_cursor) {
			_cursor->setHost(this);
			if (_actionStrip)
				_cursor->setActionStrip(static_cast<ActionStrip *>(_actionStrip));
		}
	}
	void setInterface(Level *level) { _interface = level; }
	bool actionStripAvailable();
	void actionStripReady(Object *strip) {
		_actionStrip = strip;
		if (_cursor)
			_cursor->setActionStrip(static_cast<ActionStrip *>(strip));
	}
	uint32 randomNumber(uint32 lo, uint32 hi);
	bool skipPressed();
	bool subtitlesOn();

	Camera &updateCamera(int32 divisor);
	void queueText(const byte *text, uint32 colour, int32 centreX, int32 y, int32 maxWidth);
	void drawSubtitle(Jim &jim, const SpeechTable::Line &line);
	int interactiveUpdate(Jim &jim);

	bool menuKeyPressed();
	void acknowledgeKeys();
	Cursor::Place place();
	void enterMainMenu();
	int levelIndex() { return jimLevelIndex(); }
	Object *interfaceObject(const char *name);
	int menuPage() { return _subtitles ? 1 : 0; }
	void jimResumeFacing() { jim()->resumeFacing(); }

private:
	friend class WorldLevel;
	friend class Scripts;
	friend struct SceneAdapter;

	/** The objects the game starts with switched off, once a level has been loaded. */
	void applyStartState(Level &where, int index);

	Object *createObject(Level &level, const ObjectRecord &record, Room &room, int32 loadTime);

	TesoroEngine &_engine;
	SpeechTables _speech;
	Common::Array<byte> _speechText[SpeechTables::kCount];
	AnimPlayer _font;
	Level *_levels[kLevelCount] = {};
	Jim *_jim = nullptr;
	Cursor *_cursor = nullptr;
	Level *_interface = nullptr;
	Object *_actionStrip = nullptr;
	bool _subtitles = true;
	int _jimVerb = 0;					// what the pointer carried when it clicked Jim
	bool _jimClickPending = false;		// Jim is saying his answer to that click
	bool _jimClickAnswered = false;
	MainList _main;
	SoundDevice _sound;	// speech and sound effects
	CdAudio _music;
	Scripts *_scripts = nullptr;
	byte _jimPhase = 0, _jimStep = 0, _jimExtra = 0;
	bool _jimEntered = false;
};

} // End of namespace Tesoro

#endif
