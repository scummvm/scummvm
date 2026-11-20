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

#ifndef HOLLYWOOD_RUNAWAY_SCENE9020_H
#define HOLLYWOOD_RUNAWAY_SCENE9020_H

#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene2010.h"

namespace Hollywood {
namespace Runaway {

// The bank robbery and the Sandrettis' revenge alternate films with Brian's narration.
class Scene9020 : public RunawayScene {
public:
	Scene9020(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	bool load(uint16 entryScene) override;
	void start(uint32 now) override;
	void resume() override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	void draw() override;
	const Common::U32String &subtitle() const override;
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
	enum Step { kBankNarration, kBankFilm, kGangstersNarration, kGangstersFilm, kDepartureNarration, kFinished };
	bool narrating() const;
	void startNarration(byte cue);
	void startGangstersFilm();

	RunawaySequence _sequence;
	RunawayOpening _narrator;
	SceneTextStore _text;
	Graphics::ManagedSurface _screen;
	Common::Array<RunawaySequence::Command> _commands;
	Common::U32String _empty;
	Step _step = kBankNarration;
	uint32 _now = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE9020_H
