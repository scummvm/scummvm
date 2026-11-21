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

#ifndef HOLLYWOOD_RUNAWAY_SCENE9030_H
#define HOLLYWOOD_RUNAWAY_SCENE9030_H

#include "hollywood/runaway/presentation_scene.h"

namespace Hollywood {
namespace Runaway {

class Scene9030 : public RunawayPresentationScene {
public:
	Scene9030(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game);
	bool load(uint16 entryScene) override;
	void start(uint32 now) override;
	void resume() override;
	void update(uint32 now, int scrollDirection, bool inventoryOpen) override;
	void draw() override;
	const Common::U32String &subtitle() const override { return _speech.subtitle(); }
	void skipSpeech() override;
	void skipVideo() override;
	bool escaped() const override { return _step == kFinished; }
	void captureState(RunawaySaveState &state) const override;
	bool restoreState(const RunawaySaveState &state, uint32 now, bool entering = false) override;

private:
	enum Step {
		kExplanation, kCloseupExit, kFadeOut, kBeachFadeIn, kExclamation, kPause,
		kGinaRequest, kReply, kTurnBack, kFarewell, kLastWords, kMusicTail, kFinished
	};
	void enter(Step step);
	void say(byte variation);
	void nextLine();
	void updateBeach();

	RunawayAudio &_audio;
	Common::RandomSource _random;
	RunawaySpeech _speech;
	SceneTextStore _text;
	SceneSpeechCue _cue;
	RunawayAnimation _layers[11];
	Graphics::ManagedSurface _background;
	Step _step = kExplanation;
	uint32 _now = 0, _started = 0, _beachStarted = 0, _ginaTick = 0, _ambientTick = 0;
	uint _frame = 0, _cameraY = 0;
	byte _line = 0, _request = 0, _ginaStep = 0;
};

} // End of namespace Runaway
} // End of namespace Hollywood

#endif // HOLLYWOOD_RUNAWAY_SCENE9030_H
