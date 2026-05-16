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

#ifndef ACCESS_ACCESS_H
#define ACCESS_ACCESS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/util.h"

#include "engines/engine.h"

#include "graphics/surface.h"

#include "access/animation.h"
#include "access/bubble_box.h"
#include "access/char.h"
#include "access/data.h"
#include "access/events.h"
#include "access/files.h"
#include "access/font.h"
#include "access/inventory.h"
#include "access/player.h"
#include "access/resources.h"
#include "access/room.h"
#include "access/screen.h"
#include "access/scripts.h"
#include "access/sound.h"
#include "access/video.h"
#include "access/detection.h"

/**
 * This is the namespace of the Access engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Amazon: Guardians of Eden
 */
namespace Access {

enum AccessDebugChannels {
	kDebugPath = 1,
	kDebugScripts,
	kDebugGraphics,
	kDebugSound,
};

/* typed enum to match unsignedness of Common::CustomEventType */
enum ACCESSActions : Common::CustomEventType {
	kActionNone,
	kActionMoveUp,
	kActionMoveDown,
	kActionMoveLeft,
	kActionMoveRight,
	kActionMoveUpLeft,
	kActionMoveUpRight,
	kActionMoveDownLeft,
	kActionMoveDownRight,
	kActionLook,
	kActionUse,
	kActionTake,  // aka GET
	kActionInventory,
	kActionClimb,
	kActionTalk,
	kActionWalk, // aka GOTO
	kActionHelp,
	kActionOpen,
	kActionMove,
	kActionTravel,
	kActionSkip,
	kActionSaveLoad,
	kActionOptions,
};

/* These are the commands for MM. Noctropolis uses different numbers */
enum MartianCommands {
	kMartianCmdLook = 0,
	kMartianCmdOpen = 1,
	kMartianCmdMove = 2,
	kMartianCmdGetTake = 3,
	kMartianCmdUse = 4,
	kMartianCmdGoto = 5, // aka walk-to
	kMartianCmdTalk = 6,
};


/* These are the commands for MM and Amazon. Noctropolis uses different numbers */
enum AmazonCommands {
	kAmazonCmdLook = 0,
	kAmazonCmdOpen = 1,
	kAmazonCmdMove = 2,
	kAmazonCmdGetTake = 3,
	kAmazonCmdUse = 4,
	kAmazonCmdGoto = 5, // aka walk-to
	kAmazonCmdTalk = 6,
	kAmazonCmdWalkToCursor = 7,
	kAmazonCmdHelp = 8,
};


struct AccessActionCode {
	ACCESSActions _action;
	int8 _code;
};

#define ACCESS_SAVEGAME_VERSION 1

struct AccessSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
	int _totalPlayTime;
};

class AccessEngine : public Engine {
private:
	uint32 _lastTime, _curTime;

	/**
	 * A cache for the ICONS.LZ sprite data
	 */
	SpriteResource *_icons;

	/**
	 * Handles basic initialization
	 */
	void initialize();

	/**
	 * Set VGA mode
	 */
	void setVGA();

	/**
	 * Add hotspots to buffer2 for debugging purposes
	 */
	void addHotspotHighlights();

protected:
	const AccessGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	int _loadSaveSlot;

	/**
	 * Main handler for showing game rooms
	 */
	void doRoom();

	/**
	 * Play back an entire video
	 */
	void playVideo(int videoNum, const Common::Point &pt);

	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
protected:
	/**
	 * Play the game
	 */
	virtual void playGame() = 0;

	/**
	* Synchronize savegame data
	*/
	virtual Common::Error synchronize(Common::Serializer &s);

	/**
	 * Create game-specific objects, called just before setupGame.
	 */
	virtual void initObjects() = 0;

	/**
	 * Set up the game game-specific objects, called just before playGame.
	 */
	virtual void setupGame() = 0;

public:
	AnimationManager *_animation;
	BubbleBox *_bubbleBox;
	BubbleBox *_helpBox;
	BubbleBox *_travelBox;
	BubbleBox *_invBox;
	BubbleBox *_aboutBox;
	CharManager *_char;
	EventsManager *_events;
	FileManager *_files;
	InventoryManager *_inventory;
	Player *_player;
	Player *_curPlayer; // current player being animated (only changes in Noctropolis)
	Resources *_res;
	Room *_room;
	Screen *_screen;
	Scripts *_scripts;
	SoundManager *_sound;
	MusicManager *_midi;
	VideoPlayer *_video;

	BaseSurface *_destIn;
	BaseSurface *_current;
	ASurface _buffer1;
	ASurface _buffer2;
	ASurface _vidBuf;
	int _vidX, _vidY;
	SpriteResource *_objectsTable[128];
	bool _establishTable[128];
	bool _establishFlag;
	int _establishMode;
	int _establishGroup;
	int _establishCtrlTblOfs;
	TimerList _timers;
	DeathList _deaths;
	FontManager _fonts;
	Common::Array<Common::Rect> _newRects;
	Common::Array<Common::Rect> _oldRects;
	Common::Array<ExtraCell> _extraCells;
	ImageEntryList _images;
	int _mouseMode;

	uint8 _playerDataCount;
	int _currentManOld;
	int _converseMode;
	bool _currentCharFlag;
	bool _boxSelect;
	int _scale;
	int _scaleH1, _scaleH2;
	int _scaleN1;
	int _scaleT1;
	int _scaleMaxY;
	int _scaleI;
	int _scrollX, _scrollY;
	int _scrollCol, _scrollRow;
	bool _canSaveLoad;

	Resource *_establish;
	int _printEnd;
	int _txtPages;
	int _narateFile;
	int _sndSubFile;
	int _countTbl[6];

	// Fields that are included in savegames
	int _conversation;
	int _currentMan;
	uint32 _newTime;
	uint32 _newDate;
	int _flags[256];

	// Fields used by MM and sometimes Noctropolis
	// TODO: Refactor
	byte _travel[60]; // only first ~15 used in MM
	byte _ask[40];
	byte _asked[40]; // Noctropolis only
	int _startTravelItem;
	int _startTravelBox;
	int _startAboutItem;
	int _startAboutBox;

	Common::Point _askBase; // Noctropolis only
	bool _keepAskPosition;  // Noctropolis only
	int _boxDataStart;
	bool _boxDataEnd;
	int _boxSelectY;
	int _boxSelectYOld;
	int _numLines;
	int _bcnt;

	bool _clearSummaryFlag; // amazon only
	bool _cheatFl; 	 // cheats are enabled
	bool _restartFl; // game should restart
	bool _textFlag;  // whether subtitles are enabled
	bool _hotspotFl; // whether hotspot highlighting is enabled (for debug)
	bool _exitBox;   // whether the current hotspot is an exit (Noctropolis only)
	uint16 _stilScaleOff;

	// Fields mapped into the flags array
	int &_useItem;
	int &_startup;
	int &_manScaleOff;
	int &_pictureTaken;

public:
	AccessEngine(OSystem *syst, const AccessGameDescription *gameDesc);
	~AccessEngine() override;

	virtual void dead(int deathId) = 0;

	uint32 getFeatures() const;
	bool isCD() const;
	bool isDemo() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	AccessGameType getGameID() const;
	uint32 getGameFeatures() const;
	bool shouldQuitOrRestart();

	int getRandomNumber(int maxNumber);

	const SpriteResource *getIcons();

	void loadCells(const Common::Array<CellIdent> &cells);

	/**
	* Free the sprites list
	*/
	void freeCells();

	virtual void establish(int esatabIndex, int sub) = 0;

	void plotList();
	void plotList1();
	void clearPlotImagesIn(int16 x, int16 y, int16 w, int16 h);
	void clearPlotVidsIn(int16 x, int16 y, int16 w, int16 h);

	void copyBlocks();

	void copyRects();

	void copyBF1BF2();

	void copyBF2Vid();

	void freeChar();

	virtual int16 getScreenWidth() const { return 320; }
	virtual int16 getScreenHeight() const { return 200; }

	/**
	 * Draw a string on a given surface and update text positioning
	 */
	void printText(BaseSurface *s, const Common::String &msg);
	void speakText(BaseSurface *s, const Common::String &msg);

	void syncSoundSettings() override;

	/**
	 * Load a savegame
	 */
	Common::Error loadGameState(int slot) override;
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	/**
	 * Save the game
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	/**
	 * Returns true if a savegame can currently be loaded
	 */
	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	* Returns true if the game can currently be saved
	*/
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Read in a savegame header
	 */
	static bool readSavegameHeader(Common::InSaveFile *in, AccessSavegameHeader &header, bool skipThumbnail = true);

	bool playMovie(const Common::Path &filename, const Common::Point &pos);

	const AccessActionCode *getActionCodes();

	/**
	 * Draw additional things on the screen after main rendering
     */
	virtual void drawOverlays();

};

} // End of namespace Access

#endif /* ACCESS_ACCESS_H */
