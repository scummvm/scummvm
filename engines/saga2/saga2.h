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

#ifndef SAGA2_H
#define SAGA2_H

#include "common/debug.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/system.h"

#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "saga2/console.h"
#include "saga2/gfx.h"
#include "saga2/idtypes.h"
#include "saga2/weapons.h"
#include "saga2/vdraw.h"

namespace Video {
class SmackerDecoder;
}

namespace Common {
class MemoryWriteStreamDynamic;
}

namespace Saga2 {

class ContainerManager;
class Timer;
class TimerList;
class BandList;
class ActorAppearance;
class PathRequest;
class MotionTask;
class MotionTaskList;
class CMassWeightIndicator;
class GrabInfo;
class CImageCache;
class SensorList;
class Sensor;
class EffectDisplayPrototypeList;
class SpellDisplayPrototypeList;
class DisplayNodeList;
class SpellDisplayList;
class gMousePointer;
class ActiveRegion;
class gToolBase;
class Properties;
class PlayerActor;
class ProtoObj;
class ActorProto;
class HandleArray;
class TileActivityTaskList;
class TaskStackList;
class TaskList;
class Deejay;
class frameSmoother;
class frameCounter;
class CMapFeature;
class AudioInterface;
class PaletteManager;
class ActorManager;
class CalenderTime;
class TileModeManager;
struct SAGA2GameDescription;

enum {
	kDebugResources = 1 << 0,
	kDebugActors    = 1 << 1,
	kDebugScripts   = 1 << 2,
	kDebugEventLoop = 1 << 3,
	kDebugInit      = 1 << 4,
	kDebugTiles     = 1 << 5,
	kDebugPalettes  = 1 << 6,
	kDebugLoading   = 1 << 7,
	kDebugTimers    = 1 << 8,
	kDebugPath      = 1 << 9,
	kDebugTasks     = 1 << 10,
	kDebugSound     = 1 << 11,
	kDebugSaveload  = 1 << 12,
	kDebugSensors   = 1 << 13
};

#define TICKSPERSECOND (728L/10L)

class Saga2Engine : public Engine {
public:
	Saga2Engine(OSystem *syst, const SAGA2GameDescription *desc);
	~Saga2Engine();

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
	const ADGameFileDescription *getFilesDescriptions() const;
	int getGameId() const;
	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override { return true; }
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) override;
	Common::Error loadGameState(int slot) override;
	void syncSoundSettings() override;

	Common::String getSavegameFile(int num);

	void syncGameStream(Common::Serializer &s);

	void loadExeResources();
	void freeExeResources();

	void startVideo(const char *fileName, int x, int y);
	bool checkVideo();
	void endVideo();
	void abortVideo();

	void readConfig();
	void saveConfig();

public:
	// We need random numbers
	Common::RandomSource *_rnd;
	Console *_console;
	Renderer *_renderer;
	AudioInterface *_audio;
	PaletteManager *_pal;
	ActorManager *_act;
	CalenderTime *_calender;
	TileModeManager *_tmm;
	ContainerManager *_cnm;

	WeaponStuff _weaponRack[kMaxWeapons];
	weaponID _loadedWeapons;
	Common::Array<char *> _nameList;
	Common::Array<PlayerActor *> _playerList;
	Common::Array<ProtoObj *> _objectProtos;
	Common::Array<ActorProto *> _actorProtos;
	Common::Array<CMapFeature *> _mapFeatures;
	Common::List<TimerList *> _timerLists;
	Common::List<Timer *> _timers;
	Common::List<ActorAppearance *> _appearanceLRU;
	Common::List<PathRequest *> _pathQueue;
	Common::List<SensorList *> _sensorListList;
	Common::List<Sensor *> _sensorList;
	Common::List<CMassWeightIndicator *> _indList;
	Common::List<int> _platformLRU;
	BandList *_bandList;
	MotionTaskList *_mTaskList;
	CImageCache *_imageCache;
	GrabInfo *_mouseInfo;
	EffectDisplayPrototypeList *_edpList;
	SpellDisplayPrototypeList *_sdpList;
	DisplayNodeList *_mainDisplayList;
	SpellDisplayList *_activeSpells;
	gMousePointer *_pointer;
	ActiveRegion *_activeRegionList;
	gToolBase *_toolBase;
	Properties *_properties;
	HandleArray *_tileImageBanks;
	TileActivityTaskList *_aTaskList;
	TaskStackList *_stackList;
	TaskList *_taskList;
	Deejay *_grandMasterFTA;
	frameSmoother *_frate;
	frameCounter *_lrate;

	gDisplayPort _mainPort;
	gPort _backPort;
	gPixelMap _tileDrawMap;

	bool _gameRunning;
	bool _autoAggression;
	bool _autoWeapon;
	bool _showNight;
	bool _speechText;
	bool _speechVoice;

	bool _teleportOnClick;
	bool _teleportOnMap;
	bool _showPosition;
	bool _showStats;
	bool _showStatusMsg;

	bool _indivControlsFlag;
	bool _userControlsSetup;
	int _fadeDepth;
	int _currentMapNum;


private:
	const SAGA2GameDescription *_gameDescription;
	Video::SmackerDecoder *_smkDecoder;
	int _videoX, _videoY;
};

extern Saga2Engine *g_vm;

} // End of namespace Saga2

#endif
