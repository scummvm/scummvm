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
	kDebugPath      = 1 << 0,
	kDebugScripts	= 1 << 1,
	kDebugGraphics	= 1 << 2,
	kDebugSound     = 1 << 3
};

extern const char *const _estTable[];

#define ACCESS_SAVEGAME_VERSION 1

struct AccessSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class AccessEngine : public Engine {
private:
	uint32 _lastTime, _curTime;

	/**
	 * Handles basic initialization
	 */
	void initialize();

	/**
	 * Set VGA mode
	 */
	void setVGA();

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
	virtual void synchronize(Common::Serializer &s);
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
	Common::Array<CharEntry *> _charTable;
	SpriteResource *_objectsTable[100];
	bool _establishTable[100];
	bool _establishFlag;
	int _establishMode;
	int _establishGroup;
	int _establishCtrlTblOfs;
	int _numAnimTimers;
	TimerList _timers;
	DeathList _deaths;
	FontManager _fonts;
	Common::Array<Common::Rect> _newRects;
	Common::Array<Common::Rect> _oldRects;
	Common::Array<ExtraCell> _extraCells;
	ImageEntryList _images;
	int _mouseMode;

	int _playerDataCount;
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
	bool _imgUnscaled;
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

	// Fields used by MM
	// TODO: Refactor
	int _travel[60];
	int _ask[40];
	int _startTravelItem;
	int _startTravelBox;
	int _startAboutItem;
	int _startAboutBox;
	int _boxDataStart;
	bool _boxDataEnd;
	int _boxSelectY;
	int _boxSelectYOld;
	int _numLines;
	byte _byte26CB5;
	int _bcnt;
	byte *_tempList;
	int _pictureTaken;
	//

	bool _vidEnd;
	bool _clearSummaryFlag;
	bool _cheatFl;
	bool _restartFl;
	// Fields mapped into the flags array
	int &_useItem;
	int &_startup;
	int &_manScaleOff;

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
	uint32 getGameID() const;
	uint32 getGameFeatures() const;
	bool shouldQuitOrRestart();

	int getRandomNumber(int maxNumber);

	void loadCells(Common::Array<CellIdent> &cells);

	/**
	* Free the sprites list
	*/
	void freeCells();

	virtual void establish(int esatabIndex, int sub) = 0;

	void plotList();
	void plotList1();

	void copyBlocks();

	void copyRects();

	void copyBF1BF2();

	void copyBF2Vid();

	void freeChar();

	/**
	 * Draw a string on a given surface and update text positioning
	 */
	void printText(BaseSurface *s, const Common::String &msg);
	void speakText(BaseSurface *s, const Common::String &msg);

	/**
	 * Load a savegame
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Save the game
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Returns true if a savegame can currently be loaded
	 */
	bool canLoadGameStateCurrently() override;

	/**
	* Returns true if the game can currently be saved
	*/
	bool canSaveGameStateCurrently() override;

	/**
	 * Read in a savegame header
	 */
	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, AccessSavegameHeader &header, bool skipThumbnail = true);

	/**
	 * Write out a savegame header
	 */
	void writeSavegameHeader(Common::OutSaveFile *out, AccessSavegameHeader &header);

	void SPRINTCHR(char c, int fontNum);
	void PRINTCHR(Common::String msg, int fontNum);

	bool playMovie(const Common::String &filename, const Common::Point &pos);
};

} // End of namespace Access

#endif /* ACCESS_ACCESS_H */
