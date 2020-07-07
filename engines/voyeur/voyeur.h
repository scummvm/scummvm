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

#ifndef VOYEUR_VOYEUR_H
#define VOYEUR_VOYEUR_H

#include "voyeur/debugger.h"
#include "voyeur/data.h"
#include "voyeur/events.h"
#include "voyeur/files.h"
#include "voyeur/screen.h"
#include "voyeur/sound.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"

/**
 * This is the namespace of the Voyeur engine.
 *
 * Status of this engine: Complete
 *
 * Games using this engine:
 * - Voyeur (Dos)
 */
namespace Voyeur {

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

// Constants used for doInterface display of the mansion
#define MANSION_MAX_X 784
#define MANSION_MAX_Y 150
#define MANSION_VIEW_X 40
#define MANSION_VIEW_Y 27
#define MANSION_VIEW_WIDTH 240
#define MANSION_VIEW_HEIGHT 148
#define MANSION_SCROLL_AREA_X 20
#define MANSION_SCROLL_AREA_Y 20
#define MANSION_SCROLL_INC_X 4
#define MANSION_SCROLL_INC_Y 4

enum VoyeurDebugChannels {
	kDebugScripts	= 1 << 0
};

enum VoyeurArea { AREA_NONE, AREA_APARTMENT, AREA_INTERFACE, AREA_ROOM, AREA_EVIDENCE };

struct VoyeurGameDescription;

class VoyeurEngine : public Engine {
private:
	const VoyeurGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	FontInfoResource _defaultFontInfo;

	void ESP_Init();
	void globalInitBolt();
	void initBolt();
	void vInitInterrupts();
	void initInput();

	bool doHeadTitle();
	void showConversionScreen();
	bool doLock();
	void showTitleScreen();
	void doOpening();

	void playStamp();
	void initStamp();
	void closeStamp();

	/**
	 * Shows the game ending title animation
	 */
	void doTailTitle();

	/**
	 * Shows the game ending credits
	 */
	void doClosingCredits();

	/**
	 * Shows the final anti-piracy message before exiting the game
	 */
	void doPiracy();

	/**
	 * Review previous tape recordings on the TV
	 */
	void reviewTape();

	/**
	 * Shows the TV gossip animation
	 */
	void doGossip();

	/**
	 * Shows the animation of the VCR tape during the 'Call the Police' sequence
	 */
	void doTapePlaying();

	/**
	 * Does a check as to whether a murder has been witnessed
	 */
	bool checkForMurder();

	/**
	 * Does a check for whether incriminating evidence has been revealed
	 */
	bool checkForIncriminate();

	/**
	 * Plays a video event previously witnessed
	 */
	void playAVideoEvent(int eventIndex);

	/**
	 * Shows the user a screen to select one of four characters to send the
	 * video tape to
	 */
	int getChooseButton();

	/**
	 * Synchronizes the game data
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Resets the mansion view position
	 */
	void centerMansionView();
protected:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:
	BoltFile *_bVoy;
	Debugger *_debugger;
	EventsManager *_eventsManager;
	FilesManager *_filesManager;
	Screen *_screen;
	SoundManager *_soundManager;
	SVoy *_voy;

	BoltFile *_stampLibPtr;
	BoltGroup *_controlGroupPtr;
	ControlResource *_controlPtr;
	byte *_stampData;
	BoltGroup *_stackGroupPtr;
	int _glGoState;
	int _glGoStack;
	int _stampFlags;
	int _playStampGroupId;
	int _currentVocId;

	int _audioVideoId;
	const int *_resolvePtr;
	int _iForceDeath;
	int _checkTransitionId;
	int _gameHour;
	int _gameMinute;
	int _flashTimeVal;
	bool _flashTimeFlag;
	int _timeBarVal;
	int _checkPhoneVal;
	Common::Point _mansionViewPos;
	ThreadResource *_mainThread;
	VoyeurArea _voyeurArea;
	int _loadGameSlot;
public:
	VoyeurEngine(OSystem *syst, const VoyeurGameDescription *gameDesc);
	~VoyeurEngine() override;
	void GUIError(const Common::String &msg);

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	bool getIsDemo() const;

	int getRandomNumber(int maxNumber);
	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	void loadGame(int slot);

	void playRL2Video(const Common::String &filename);
	void doTransitionCard(const Common::String &time, const Common::String &location);

	/**
	 * Play a given video
	 */
	void playAVideo(int videoId);

	/**
	 * Play a given video for a given amount of time. This is particularly used
	 * for later tape playback, where it will only play back as much of the video
	 * as the user originally watched (since they can break out of watching a video).
	 */
	void playAVideoDuration(int videoId, int duration);

	/**
	 * Play an audio sequence
	 */
	void playAudio(int audioId);

	void makeViewFinder();
	void makeViewFinderP();
	void initIFace();
	void checkTransition();
	int doComputerText(int maxLen);
	void getComputerBrush();

	/**
	 * Displays the time/charge remaining on the video camera screen
	 */
	void doTimeBar();

	/**
	 * If necessary, flashes the time remaining bar on the video camera screen
	 */
	void flashTimeBar();

	/**
	 * Handle scrolling of the mansion view in the camera sights
	 */
	void doScroll(const Common::Point &pt);

	/**
	 * Check for phone call
	 */
	void checkPhoneCall();

	/**
	 * Display evidence sequence from within a room
	 * Suspension of disbelief needed to believe that recording from a distance,
	 * you could still flip through the often pages of evidence for a single hotspot.
	 */
	void doEvidDisplay(int evidId, int eventId);

	/**
	 * Flips the active page and waits until it's drawn
	 */
	void flipPageAndWait();

	/**
	 * Flips the active page and waits until it's drawn and faded in
	 */
	void flipPageAndWaitForFade();

	/**
	 * Returns the string for the current in-game day of the week
	 */
	Common::String getDayName();

	/**
	 * Returns the string for the current in-game time of day
	 */
	Common::String getTimeOfDay();

	/**
	 * Show the ending sequence of the arrest
	 */
	void showEndingNews();
};

#define VOYEUR_SAVEGAME_VERSION 3

/**
 * Header for Voyeur savegame files
 */
struct VoyeurSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _saveYear, _saveMonth, _saveDay;
	int _saveHour, _saveMinutes;
	int _totalFrames;

	/**
	 * Read in the header from the specified file
	 */
	bool read(Common::InSaveFile *f, bool skipThumbnail = true);

	/**
	 * Write out header information to the specified file
	 */
	void write(Common::OutSaveFile *f, VoyeurEngine *vm, const Common::String &saveName);
};

} // End of namespace Voyeur

#endif /* VOYEUR_VOYEUR_H */
