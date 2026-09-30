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
#include "graphics/surface.h"
#include "scumm/insane/mortimer/flight.h"
#include "scumm/insane/mortimer/interview.h"

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
	bool loadResources();
	uint introCount() const;
	bool loadIntro(uint index);
	bool loadFlight(uint segment);
	bool startMovie(bool preserveScreen = false);
	bool startFlight();
	bool processInterview();
	bool updateScene();
	bool nextScene(bool skip = false);
	bool restart(bool all = false);
	MortimerMovie &currentMovie();
	const MortimerMovie &currentMovie() const;
	bool interactive() const { return !_interviewActive && _introIndex < 0 && _flight.interactive(); }
	void setMouse(const Common::Point &position);
	void updateSteering();
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
	void playSound(const MortimerSound &sound, Audio::SoundHandle &handle, Audio::Mixer::SoundType type, uint offset = 0);
	void playMovieSounds(bool seek = false);
	void stopSounds();

	const bool _isDemo;
	MortimerCutscene _intro;
	MortimerFlight _flight;
	MortimerInterview _interview;
	MortimerSubtitles _subtitles;
	Graphics::Surface _screen;
	Common::Array<MortimerSprite> _cockpit, _dashboard, _reticles, _icons, _head;
	Common::Array<MortimerSprite> _scopeHit, _slimeHit, _scopeBeam, _slimeBall, _numbers, _corners;
	MortimerSound _scopeSound, _rescueSound, _collisionSound, _slimeSound, _slimeHitSound, _glowSound, _balloonSound;
	MortimerSound _buttonSound, _payoutSound;
	Common::Array<Audio::SoundHandle> _movieHandles;
	Audio::SoundHandle _scopeHandle, _rescueHandle, _collisionHandle, _glowHandle;
	Common::String _loadError;
	Common::Rect _viewport;
	Common::Point _mouse, _view, _aim, _shot;
	Common::Point _steeringHistory[10];
	int _steering[4];
	MortimerFlight::Weapon _shotWeapon;
	uint32 _nextFrameTime, _pauseTime;
	int _introIndex;
	uint _segment, _carriedStones, _completedSpecies, _shellShocks;
	uint _uiFrame, _shotFrame, _glowFrame, _glowStones, _pendingGlowStones;
	uint _iconFlash[7];
	bool _finished, _controllerMode, _interviewActive;
};

} // End of namespace Scumm

#endif
