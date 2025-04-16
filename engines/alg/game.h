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

#ifndef ALG_GAME_H
#define ALG_GAME_H

#include "common/random.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "alg/alg.h"
#include "alg/scene.h"
#include "alg/video.h"

namespace Alg {

class Game {

public:
	Game(AlgEngine *vm);
	virtual ~Game();
	virtual void init();
	virtual Common::Error run();
	bool debug_dumpLibFile();
	void runCursorTimer();
	bool _debug_drawRects = false;
	bool _debug_godMode = false;
	bool _debug_unlimitedAmmo = false;

protected:
	AlgEngine *_vm;
	AlgVideoDecoder *_videoDecoder;
	SceneInfo *_sceneInfo;
	Common::RandomSource *_rnd;

	Common::File _libFile;
	Common::HashMap<Common::String, uint32> _libFileEntries;

	uint8 *_palette;
	bool _paletteDirty;

	Graphics::Surface *_background;
	Graphics::Surface *_screen;
	Common::Array<Graphics::Surface *> *_gun;
	Common::Array<Graphics::Surface *> *_numbers;

	Audio::SeekableAudioStream *_saveSound = nullptr;
	Audio::SeekableAudioStream *_loadSound = nullptr;
	Audio::SeekableAudioStream *_easySound = nullptr;
	Audio::SeekableAudioStream *_avgSound = nullptr;
	Audio::SeekableAudioStream *_hardSound = nullptr;
	Audio::SeekableAudioStream *_skullSound = nullptr;
	Audio::SeekableAudioStream *_shotSound = nullptr;
	Audio::SeekableAudioStream *_emptySound = nullptr;

	Audio::SoundHandle _sfxAudioHandle;

	Zone *_menuzone;
	Zone *_submenzone;

	bool _leftDown = false;
	bool _rightDown = false;
	Common::Point _mousePos;

	const uint32 _pauseDiffScale[3] = {0x10000, 0x8000, 0x4000};
	const uint32 _rectDiffScale[3] = {0x10000, 0x0C000, 0x8000};

	void shutdown();
	bool pollEvents();
	void loadLibArchive(const Common::Path &path);
	Audio::SeekableAudioStream *loadSoundFile(const Common::Path &path);
	void playSound(Audio::SeekableAudioStream *stream);
	bool loadScene(Scene *scene);
	void updateScreen();
	uint32 getMsTime();
	bool fired(Common::Point *point);
	Rect *checkZone(Zone *zone, Common::Point *point);
	Zone *checkZonesV1(Scene *scene, Rect *&hitRect, Common::Point *point);
	Zone *checkZonesV2(Scene *scene, Rect *&hitRect, Common::Point *point);
	uint32 getFrame(Scene *scene);
	void adjustDifficulty(uint8 newDifficulty, uint8 oldDifficulty);
	int8 skipToNewScene(Scene *scene);
	uint16 randomUnusedInt(uint8 max, uint16 *mask, uint16 exclude);
	void debug_drawZoneRects();

	// Sounds
	void doDiffSound(uint8 difficulty);
	void doSaveSound();
	void doLoadSound();
	void doSkullSound();
	void doShot();

	// Timer
	void setupCursorTimer();
	void removeCursorTimer();

	// Script functions: Zone
	void zoneGlobalHit(Common::Point *point);
	// Script functions: RectHit
	void rectHitDoNothing(Rect *rect);
	void rectNewScene(Rect *rect);
	void rectExit(Rect *rect);
	void rectEasy(Rect *rect);
	void rectAverage(Rect *rect);
	void rectHard(Rect *rect);
	// Script functions: Scene PreOps
	void scenePsoDrawRct(Scene *scene);
	void scenePsoPause(Scene *scene);
	void scenePsoDrawRctFadeIn(Scene *scene);
	void scenePsoFadeIn(Scene *scene);
	void scenePsoPauseFadeIn(Scene *scene);
	void scenePsoPreRead(Scene *scene);
	void scenePsoPausePreRead(Scene *scene);
	// Script functions: Scene Scene InsOps
	void sceneIsoDoNothing(Scene *scene);
	void sceneIsoStartGame(Scene *scene);
	void sceneIsoPause(Scene *scene);
	// Script functions: Scene Scene NxtScn
	void sceneNxtscnDoNothing(Scene *scene);
	void sceneDefaultNxtscn(Scene *scene);
	// Script functions: ShowMsg
	void sceneSmDonothing(Scene *scene);
	// Script functions: ScnScr
	void sceneDefaultScore(Scene *scene);
	// Script functions: ScnNxtFrm
	void sceneNxtfrm(Scene *scene);

	bool _buttonDown = false;
	uint8 _difficulty = 1;
	uint8 _emptyCount = 0;
	bool _fired = false;
	uint32 _currentFrame;
	bool _gameInProgress = false;
	uint32 _thisGameTimer = 0;
	bool _hadPause = false;
	bool _holster = false;
	bool _inMenu = false;
	uint8 _inHolster = 0;
	int8 _lives = 0;
	long int _minF;
	long int _maxF;
	uint8 _oldWhichGun = 0xFF;
	uint8 _oldDifficulty = 1;
	int8 _oldLives = 0;
	int32 _oldScore = -1;
	uint8 _oldShots = 0;
	uint32 _pauseTime = 0;
	bool _sceneSkipped = false;
	int32 _score = 0;
	bool _shotFired = false;
	uint16 _shots = 0;
	uint32 _videoFrameSkip = 3;
	uint32 _nextFrameTime = 0;
	uint16 _videoPosX;
	uint16 _videoPosY;
	uint8 _whichGun = 0;

	Common::String _curScene;
	Common::String _subScene;
	Common::String _retScene;
	Common::String _lastScene;
	Common::String _startScene;
};

} // End of namespace Alg

#endif
