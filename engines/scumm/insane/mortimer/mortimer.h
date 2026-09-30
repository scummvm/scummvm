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
#include "audio/mixer.h"
#include "graphics/surface.h"
#include "scumm/insane/mortimer/flight.h"

namespace Scumm {

// Registered by the SCUMM plugin, but deliberately independent of ScummEngine
// and both Rebel Assault engines.
class MortimerEngine : public Engine {
public:
	explicit MortimerEngine(OSystem *system);
	~MortimerEngine() override;
	Common::Error run() override;
	bool hasFeature(EngineFeature feature) const override;

protected:
	void pauseEngineIntern(bool pause) override;

private:
	bool loadResources();
	bool loadFlight(uint segment);
	bool startFlight();
	bool nextFlight(bool skip = false);
	bool restart(bool all = false);
	void setMouse(const Common::Point &position);
	void draw();
	void drawSprite(const MortimerSprite &sprite, int x, int y);
	void playSound(const MortimerSound &sound, Audio::SoundHandle &handle, Audio::Mixer::SoundType type);
	void playFlightSounds();
	void stopSounds();

	MortimerFlight _flight;
	Graphics::Surface _screen;
	Common::Array<MortimerSprite> _cockpit, _dashboard, _reticles, _icons;
	MortimerSound _scopeSound, _rescueSound, _collisionSound, _slimeSound, _slimeHitSound;
	Common::Array<Audio::SoundHandle> _flightHandles;
	Audio::SoundHandle _scopeHandle, _rescueHandle, _collisionHandle;
	Common::String _loadError;
	Common::Point _mouse, _view;
	uint32 _nextFrameTime, _pauseTime;
	uint _segment, _completedRescues, _completedSpecies, _shellShocks;
	bool _finished;
};

} // End of namespace Scumm

#endif
