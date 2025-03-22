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
	virtual Common::Error run();
	bool _debug_drawRects = false;
	bool _debug_godMode = false;
	bool _debug_unlimitedAmmo = false;
	bool debug_dumpLibFile();
	void runCursorTimer();

protected:
	virtual void init() = 0;

	AlgEngine *_vm;
	AlgVideoDecoder *_videoDecoder;
	SceneInfo *_sceneInfo;
	Common::RandomSource *_rnd;

	Common::Path _libFileName;
	Common::File _libFile;
	Common::HashMap<Common::String, uint32> _libFileEntries;

	uint8 *_palette;
	bool _paletteDirty;

	Graphics::Surface *_background;
	Graphics::Surface *_screen;
	Common::Array<Graphics::Surface> *_gun;
	Common::Array<Graphics::Surface> *_numbers;

	Audio::SeekableAudioStream *_saveSound = nullptr;
	Audio::SeekableAudioStream *_loadSound = nullptr;
	Audio::SeekableAudioStream *_easySound = nullptr;
	Audio::SeekableAudioStream *_avgSound = nullptr;
	Audio::SeekableAudioStream *_hardSound = nullptr;
	Audio::SeekableAudioStream *_skullSound = nullptr;
	Audio::SeekableAudioStream *_shotSound = nullptr;
	Audio::SeekableAudioStream *_emptySound = nullptr;

	Audio::SoundHandle _sfxAudioHandle;
	Audio::SoundHandle _sceneAudioHandle;

	Zone *_menuzone;
	Zone *_submenzone;

	bool _leftDown;
	bool _rightDown;
	Common::Point _mousePos;

	const uint32 _pausdifscal[3] = {0x10000, 0x8000, 0x4000};
	const uint32 _rectdifscal[3] = {0x10000, 0x0C000, 0x8000};

	bool pollEvents();
	void loadLibArchive(const Common::Path &path);
	Audio::SeekableAudioStream *_LoadSoundFile(const Common::Path &path);
	void _PlaySound(Audio::SeekableAudioStream *stream);
	bool loadScene(Scene *scene);
	void updateScreen();
	uint32 _GetMsTime();
	bool __Fired(Common::Point *point);
	Rect *_CheckZone(Zone *zone, Common::Point *point);
	Zone *_CheckZonesV1(Scene *scene, Rect *&hitRect, Common::Point *point);
	Zone *_CheckZonesV2(Scene *scene, Rect *&hitRect, Common::Point *point);
	uint32 _GetFrame(Scene *scene);
	void _AdjustDifficulty(uint8 newDifficulty, uint8 oldDifficulty);
	void _RestoreCursor();
	void _SetFrame();
	int8 _SkipToNewScene(Scene *scene);
	void debug_drawZoneRects();

	// Sounds
	void _DoDiffSound(uint8 difficulty);
	void _DoSaveSound();
	void _DoLoadSound();
	void _DoSkullSound();
	void _DoShot();

	// Timer
	void _SetupCursorTimer();
	void _RemoveCursorTimer();

	// Script functions: Zone
	void _zone_globalhit(Common::Point *point);
	// Script functions: RectHit
	void _rect_hit_donothing(Rect *rect);
	void _rect_newscene(Rect *rect);
	void _rect_exit(Rect *rect);
	void _rect_easy(Rect *rect);
	void _rect_average(Rect *rect);
	void _rect_hard(Rect *rect);
	// Script functions: Scene PreOps
	void _scene_pso_drawrct(Scene *scene);
	void _scene_pso_pause(Scene *scene);
	void _scene_pso_drawrct_fadein(Scene *scene);
	void _scene_pso_fadein(Scene *scene);
	void _scene_pso_pause_fadein(Scene *scene);
	void _scene_pso_preread(Scene *scene);
	void _scene_pso_pause_preread(Scene *scene);
	// Script functions: Scene Scene InsOps
	void _scene_iso_donothing(Scene *scene);
	void _scene_iso_startgame(Scene *scene);
	void _scene_iso_pause(Scene *scene);
	// Script functions: Scene Scene NxtScn
	void _scene_nxtscn_donothing(Scene *scene);
	void _scene_default_nxtscn(Scene *scene);
	// Script functions: ShowMsg
	void _scene_sm_donothing(Scene *scene);
	// Script functions: ScnScr
	void _scene_default_score(Scene *scene);
	// Script functions: ScnNxtFrm
	void _scene_nxtfrm(Scene *scene);

	bool _buttonDown = false;
	uint8 _difficulty = 1;
	uint8 _emptyCount = 0;
	bool _fired = 0;
	uint32 _currentFrame;
	bool _gameInProgress = false;
	uint32 _gameTimer = 0;
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

	Common::String _cur_scene;
	Common::String _sub_scene;
	Common::String _ret_scene;
	Common::String _last_scene;
	Common::String _startscene;
};

} // End of namespace Alg

#endif
