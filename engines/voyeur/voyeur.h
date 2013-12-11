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

#ifndef VOYEUR_VOYEUR_H
#define VOYEUR_VOYEUR_H

#include "voyeur/debugger.h"
#include "voyeur/events.h"
#include "voyeur/files.h"
#include "voyeur/graphics.h"
#include "voyeur/sound.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"
#include "graphics/surface.h"

/**
 * This is the namespace of the Voyeur engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Voyeur
 */
namespace Voyeur {

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

#define MAX_RESOLVE 1000

enum VoyeurDebugChannels {
	kDebugPath      = 1 <<  0
};

struct VoyeurGameDescription;


class VoyeurEngine : public Engine {
private:
	const VoyeurGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Common::Array<int> _resolves;
	FontInfoResource _defaultFontInfo;

	void ESP_Init();
	void initialiseManagers();
	void globalInitBolt();
	void initBolt();
	void vInitInterrupts();
	void initInput();
	void addVideoEventStart();

	bool doHeadTitle();
	void showConversionScreen();
	bool doLock();
	void showTitleScreen();
	void doOpening();

	void playStamp();
	void initStamp();
	void closeStamp();
	void reviewTape();
	bool doGossip();
	int doApt();
	void doTapePlaying();
	bool checkForMurder();
	void checkForIncriminate();
	void playAVideoEvent(int eventId);
	int getChooseButton();
protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	BoltFile *_bVoy;
	Debugger _debugger;
	EventsManager _eventsManager;
	FilesManager _filesManager;
	GraphicsManager _graphicsManager;
	SoundManager _soundManager;
	SVoy _voy;

	BoltFile *_stampLibPtr;
	BoltGroup *_controlGroupPtr;
	ControlResource *_controlPtr;
	byte *_stampData;
	BoltGroup *_stackGroupPtr;
	int _glGoScene;
	int _glGoStack;
	bool _bob;
	int _playStamp1;
	int _playStamp2;
	const int *_resolvePtr;
	int _iForceDeath;
public:
	VoyeurEngine(OSystem *syst, const VoyeurGameDescription *gameDesc);
	virtual ~VoyeurEngine();
	void GUIError(const Common::String &msg);

	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint16 getVersion() const;
	bool getIsDemo() const;

	int getRandomNumber(int maxNumber);
	Common::String generateSaveName(int slotNumber);
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	void playRL2Video(const Common::String &filename);
	void doTransitionCard(const Common::String &time, const Common::String &location);
	void playAVideo(int id);
	void saveLastInplay();
};

} // End of namespace Voyeur

#endif /* VOYEUR_VOYEUR_H */
