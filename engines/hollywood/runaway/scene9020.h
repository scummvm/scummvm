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

#include "hollywood/runaway/presentation_scene.h"
#include "hollywood/runaway/scene2010.h"

namespace Hollywood {
namespace Runaway {

// The bank robbery and the Sandrettis' revenge alternate films with Brian's narration.
class Scene9020 : public RunawayPresentationScene {
public:
	Scene9020(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	bool load(uint16 entryScene) override;
	void start(uint32 now) override;
	void resume() override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	void draw() override;
	const Common::U32String &subtitle() const override;
	void skipSpeech() override;
	void skipVideo() override;
	void captureState(RunawaySaveState &state) const override;
	bool restoreState(const RunawaySaveState &state, uint32 now, bool entering = false) override;

private:
	enum Step { kBankNarration, kBankFilm, kGangstersNarration, kGangstersFilm, kDepartureNarration, kFinished };
	bool narrating() const;
	void startNarration(byte cue);
	void startGangstersFilm();

	RunawaySequence _sequence;
	RunawayOpening _narrator;
	SceneTextStore _text;
	Common::Array<RunawaySequence::Command> _commands;
	Step _step = kBankNarration;
	uint32 _now = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE9020_H
