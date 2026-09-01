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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE5130_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE5130_H

#include "common/array.h"
#include "common/random.h"
#include "common/str.h"
#include "common/types.h"

#include "hollywood/music.h"
#include "hollywood/scenes/presentation_scene.h"
#include "hollywood/scenes/scene_text_store.h"

namespace Hollywood {

class HollywoodEngine;

// Runs the modal cocktail mixer reached from scene 5120 and returns its result
// through GameplayState before handing control back to the salon.
class Scene5130 : public PresentationScene {
public:
	Scene5130(HollywoodEngine *vm);
	~Scene5130() override;

	bool play();

private:
	bool load();
	bool loadInventoryOwnerPalette();
	void expandFillRunsToSavedFramebuffer();
	void runIntroAnimation();
	void runMixerLoop();
	void handleMixerAction(byte actionId);
	void runChangeDrinkAction();
	void runOpenTapAction();
	void runMixResultAction(bool correctRecipe);
	bool selectedRecipeIsCorrect() const;
	void applySuccessDrinkPalette();
	void applyFailureDrinkPalette();
	void applyDrinkPalette(byte red, byte green, byte blue);
	void drawFrame();
	void drawDrinkStrip();
	void drawFrameOverlays() override;
	void drawCaption();
	void startSpeechLine(uint16 rowIndex, byte frameIndex);
	void startSpeechPart();
	void advanceSpeech(uint32 millis);
	void waitForSpeechLine();
	void stopSpeechLine();
	void beginSpeechLine(uint16 rowIndex, byte frameIndex);
	void advanceRuntime(uint32 millis);
	void updateAmbientMusic(uint32 millis);
	bool fadePaletteFromBlack();
	bool fadePaletteToBlack();
	bool waitAndRender(uint32 millis);
	bool waitForRuntime(uint32 millis);
	bool pollEvents(bool allowSkip) override;
	byte actionAtCursor() const;
	Common::String captionForAction(byte actionId) const;
	void setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue);
	void stopAudio() override;
	bool animationPlaybackShouldStop() const override;
	void presentAnimationFrame() override;
	bool waitForAnimationFrame(uint32 millis, bool allowSkip) override;

	IndexedSurfaceBuffer _baseFramebuffer;
	Common::Array<byte> _fillRuns;
	Common::Array<byte> _paletteMask;
	SceneTextStore _textStore;
	SpeechPlayer _speech;
	SoundBank0Player _soundBank0;
	Common::RandomSource _random;
	byte _selectedDrinks[3];
	byte _selectedDrinkCount;
	byte _currentDrinkId;
	uint _drinkStripRow;
	byte _pendingActionId;
	byte _hoverActionId;
	uint16 _speechTextRecordId;
	uint16 _speechVoiceSampleId;
	byte _speechPartIndex;
	byte _speechPartCount;
	uint32 _speechRemainingMillis;
	uint32 _ambientMusicTimerMillis;
	bool _speechActive;
	bool _mixerActionsEnabled;
	bool _deferredExitRequested;
	bool _exitRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5130_H
