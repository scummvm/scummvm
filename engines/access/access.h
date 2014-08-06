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
 */

#ifndef ACCESS_ACCESS_H
#define ACCESS_ACCESS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"
#include "access/data.h"
#include "access/debugger.h"
#include "access/events.h"
#include "access/files.h"
#include "access/player.h"
#include "access/room.h"
#include "access/screen.h"
#include "access/sound.h"

/**
 * This is the namespace of the Access engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Amazon: Guardians of Eden
 */
namespace Access {

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

enum {
	GType_Amazon = 1,
	GType_MeanStreets = 2
};

enum AccessDebugChannels {
	kDebugPath      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2
};

struct AccessGameDescription;

class AccessEngine : public Engine {
private:
	/**
	 * Handles basic initialisation
	 */
	void initialize();

	/**
	 * Set VGA mode
	 */
	void setVGA();

	void dummyLoop();
protected:
	const AccessGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	/**
	 * Clear the cell table
	 */
	void clearCellTable();

	/**
	 * Main handler for showing game rooms
	 */
	void doRoom();

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
protected:
	/**
	 * Play the game
	 */
	virtual void playGame() = 0;
public:
	Debugger *_debugger;
	EventsManager *_events;
	FileManager *_files;
	Player *_player;
	Room *_room;
	Screen *_screen;
	SoundManager *_sound;

	byte *_destIn;
	Graphics::Surface _buffer1;
	Graphics::Surface _buffer2;
	byte *_objectsTable[100];
	int _numAnimTimers;
	Common::Array<TimerEntry> _timers;
	Common::Array<Common::Rect> _newRect;
	Common::Array<Common::Rect> _oldRect;
	int _pCount;
	int _selectCommand;
	bool _normalMouse;
	int _mouseMode;
	int _numImages;
	int _nextImage;

	int _currentManOld;
	byte *_man1;
	byte *_manPal1;
	byte *_music;
	byte *_playField;
	byte *_script;
	byte *_anim;
	byte *_title;
	int _converseMode;
	int _startInvItem;
	int _startAboutItem;
	int _startTravelItem;
	int _startInvBox;
	int _startAboutBox;
	int _startTravelBox;

	// Fields that are included in savegames
	int _roomNumber;
	int _rawPlayerXLow;
	int _rawPlayerX;
	int _rawPlayerYLow;
	int _rawPlayerY;
	int _conversation;
	int _currentMan;
	uint32 _newTime;
	uint32 _newDate;
	int _intTim[3];
	int _timer[3];
	bool _timerFlag;
	byte _flags[99];
	byte _useItem[23];
	int _guardLoc;
	int _guardFind;
	int _helpLevel;
	int _jasMayaFlag;
	int _moreHelp;
	int _startup;
	bool _flashbackFlag;
	int _manScaleOff;
	bool _riverFlag;
	bool _antOutFlag;
	int _badEnd;
	bool _noHints;
	bool _antFlag;
	bool _allenFlag;
	bool _noSound;
	int inv[85];
	byte _help1[366];
	byte _help2[366];
	byte _help3[366];
	int _travel;
	int _ask;
	int _rScrollRow;
	int _rScrollCol;
	int _rSrcollX;
	int _rScrollY;
	int _rOldRectCount;
	int _rNewRectCount;
	int _rKeyFlag;
	int _mapOffset;
	int _screenVirtX;
public:
	AccessEngine(OSystem *syst, const AccessGameDescription *gameDesc);
	virtual ~AccessEngine();

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	uint32 getGameID() const;
	uint32 getGameFeatures() const;

	int getRandomNumber(int maxNumber);
};

} // End of namespace Access

#endif /* ACCESS_ACCESS_H */
