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

#ifndef HOLLYWOOD_RUNAWAY_SCENE9010_H
#define HOLLYWOOD_RUNAWAY_SCENE9010_H

#include "hollywood/runaway/presentation.h"
#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace Runaway {

class Scene9010 : public RunawayScene {
public:
	Scene9010(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	bool load(uint16 entryScene) override;
	void start(uint32 now) override;
	void resume() override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	void draw() override;
	const Common::U32String &subtitle() const override { return _sequence.subtitle(); }
	void skipSpeech() override { skipVideo(); }
	void skipVideo() override;
	bool speaking() const override { return !subtitle().empty(); }
	bool busy() const override { return true; }
	bool playingVideo() const override { return true; }
	void captureState(RunawaySaveState &state) const override;
	bool restoreState(const RunawaySaveState &state, uint32 now, bool entering = false) override;
	Graphics::ManagedSurface &screen() override { return _screen; }
	int roomWidth() const override { return kRunawayScreenWidth; }
	byte hotspotAt(Common::Point mouse) const override { return 0; }
	Common::U32String actionCaption(byte hotspot, RunawayVerb verb, byte item) const override { return Common::U32String(); }
	const Common::U32String &committedCaption() const override { return _empty; }
	void click(Common::Point mouse, RunawayVerb verb, byte item) override {}
	void sayInventory(const RunawayInventory::Reply &reply) override {}
	void setCameraX(int x) override {}
	void setOptions(bool antialias, bool fastScrolling) override {}

private:
	enum Step { kArrival, kCarDoor, kFarewell, kDeparture, kFlight, kCredits, kMusicTail, kFinished };
	void enter(Step step);
	void startFarewell();

	RunawayAudio &_audio;
	RunawaySequence _sequence;
	RunawayCredits _credits;
	SceneTextStore _text;
	RunawayAnimation _car;
	Graphics::ManagedSurface _background, _screen;
	Common::Array<byte> _parallax;
	Common::Array<RunawaySequence::Command> _commands;
	Common::U32String _empty;
	Step _step = kArrival;
	uint32 _now = 0, _started = 0;
	Common::Point _camera;
	uint _frame = 0;
	bool _gog = false;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE9010_H
