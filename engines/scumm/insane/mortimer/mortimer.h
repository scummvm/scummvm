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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCUMM_INSANE_MORTIMER_MORTIMER_H
#define SCUMM_INSANE_MORTIMER_MORTIMER_H

#include "engines/engine.h"
#include "backends/keymapper/keymap.h"
#include "audio/mixer.h"
#include "common/random.h"
#include "graphics/surface.h"
#include "scumm/insane/mortimer/flight.h"
#include "scumm/insane/mortimer/interview.h"
#include "scumm/insane/mortimer/campaign.h"
#include "scumm/insane/mortimer/castle.h"
#include "scumm/smush/saud_stream.h"

namespace Scumm {

// Registered by the SCUMM plugin, but deliberately independent of ScummEngine
// and both Rebel Assault engines.
class MortimerEngine : public Engine {
public:
	MortimerEngine(OSystem *system, bool isDemo);
	~MortimerEngine() override;
	Common::Error run() override;
	bool hasFeature(EngineFeature feature) const override;
	static Common::KeymapArray initKeymaps();

protected:
	void pauseEngineIntern(bool pause) override;

private:
	enum CrashStage { kNoCrash, kCrashMovie, kRepairMovie };

	bool loadResources();
	bool selectDifficulty();
	void setDifficulty(uint flight, uint riddle);
	uint introCount() const;
	bool loadIntro(uint index);
	bool loadFlight(uint segment);
	bool enterCampaignScene();
	bool loadRegion(uint region);
	bool loadRetailFlight(const Common::String &name, bool cached = false);
	bool followFlightBranch();
	bool loadSubtitles(const char *name);
	void updateMovieContext();
	bool startMovie(bool preserveScreen = false);
	bool startFlight();
	bool startCrash();
	bool loadCrashMovie(bool repair);
	bool finishCrash();
	void creditRescues();
	bool processInterview();
	bool processCastle();
	bool updateCastle();
	bool updateScene();
	bool nextScene(bool skip = false);
	bool restart(bool all = false);
	MortimerMovie &currentMovie();
	const MortimerMovie &currentMovie() const;
	MortimerFlight &currentFlight() { return _castleActive ? _castle : _flight; }
	bool interactive() const { return _castleActive ? _castle.courseView() :
		_crashStage == kNoCrash && !_interviewActive && _introIndex < 0 && _flight.interactive(); }
	void setMouse(const Common::Point &position);
	void updateSteering(bool centerView = false);
	void updateView(int x, int y);
	void updateInterface();
	void fire();
	Common::Point moviePoint(const Common::Point &point) const;
	Common::Point screenPoint(const Common::Point &point) const;
	void draw();
	void drawSprite(const MortimerSprite &sprite, int x, int y, bool centered = false, const Common::Rect *clip = nullptr);
	void drawProjectile();
	void drawTargets();
	void drawDashboard();
	bool speechPlaying() const;
	void playSound(const MortimerSound &sound, Audio::SoundHandle &handle, Audio::Mixer::SoundType type, uint64 skipFrames = 0);
	void playMovieSounds(bool seek = false);
	void playCollisionSound();
	void stopSounds();
	bool handleCheatKey(const Common::KeyState &key);
	bool skipRegion();
	bool addShellShock();
	void clearShellShock();

	const bool _isDemo, _restoredContent;
	uint _flightDifficulty, _riddleDifficulty;
	Common::RandomSource _random;
	MortimerCutscene _intro;
	MortimerFlight _flight;
	MortimerInterview _interview;
	MortimerCastle _castle;
	MortimerSubtitles _subtitles;
	MortimerCampaign _campaign;
	Graphics::Surface _screen;
	Common::Array<MortimerSprite> _cockpit, _dashboard, _reticles, _icons, _head;
	Common::Array<MortimerSprite> _scopeHit, _slimeHit, _scopeBeam, _slimeBall, _numbers, _corners;
	MortimerSound _scopeSound, _rescueSound, _collisionSound, _slimeSound, _slimeHitSound, _glowSound, _balloonSound;
	MortimerSound _buttonSound, _payoutSound;
	SaudStreamState _soundState;
	Common::Array<Audio::SoundHandle> _movieHandles;
	Common::Array<Audio::SoundHandle> _collisionHandles;
	Audio::SoundHandle _scopeHandle, _rescueHandle, _glowHandle, _castleHandle;
	Common::String _loadError;
	Common::String _flightName, _pendingFlight, _subtitleName;
	Common::Rect _viewport;
	Common::Point _mouse, _view, _aim, _shot;
	Common::Point _steeringHistory[10];
	int _steering[4];
	MortimerFlight::Weapon _shotWeapon;
	CrashStage _crashStage;
	uint32 _nextFrameTime, _pauseTime;
	int _introIndex;
	int _loadedRegion;
	uint _creditedRescues;
	uint _segment, _carriedStones, _completedSpecies, _shellShocks, _shockFrame, _crashCount;
	uint _uiFrame, _shotFrame, _glowFrame, _glowStones, _pendingGlowStones;
	uint _iconFlash[7];
	bool _finished, _controllerMode, _interviewActive, _castleActive;
	bool _resourcesLoaded, _playingCached;
	bool _needsRepair, _hitJolt;
	bool _cheats, _programmer, _invulnerable;
};

} // End of namespace Scumm

#endif
