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
	virtual bool saveState(Common::OutSaveFile *outSaveFile) = 0;
	virtual bool loadState(Common::InSaveFile *inSaveFile) = 0;
	bool debug_dumpLibFile();
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

	Audio::SoundHandle _sfxAudioHandle;

	Zone *_menuZone;
	Zone *_subMenuZone;

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
	virtual void updateScreen();
	uint32 getMsTime();
	bool fired(Common::Point *point);
	Rect *checkZone(Zone *zone, Common::Point *point);
	uint32 getFrame(Scene *scene);
	int8 skipToNewScene(Scene *scene);
	virtual void debug_drawZoneRects();

	// Script functions: Zone
	void zoneGlobalHit(Common::Point *point);
	// Script functions: RectHit
	void rectHitDoNothing(Rect *rect);
	void rectNewScene(Rect *rect);
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
	// Script functions: ScnNxtFrm
	void sceneNxtfrm(Scene *scene);

	bool _buttonDown = false;
	bool _fired = 0;
	uint32 _currentFrame;
	bool _gameInProgress = false;
	bool _hadPause = false;
	bool _inMenu = false;
	uint32 _pauseTime = 0;
	bool _sceneSkipped = false;
	uint32 _videoFrameSkip = 3;
	uint32 _nextFrameTime = 0;
	uint16 _videoPosX;
	uint16 _videoPosY;

	Common::String _curScene;
	Common::String _startScene;
};

} // End of namespace Alg

#endif
