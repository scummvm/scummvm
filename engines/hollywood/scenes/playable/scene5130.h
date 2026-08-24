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
#include "common/str.h"
#include "common/types.h"

#include "hollywood/music.h"
#include "hollywood/scenes/playable/animation_layers.h"
#include "hollywood/scenes/playable/scene_resources.h"
#include "hollywood/scenes/playable/scene_surface_state.h"
#include "hollywood/scenes/playable/scene_text_store.h"
#include "hollywood/scenes/playable/speech_overlay.h"

namespace Hollywood {

class HollywoodEngine;

class Scene5130 {
public:
	Scene5130(HollywoodEngine *vm);
	~Scene5130();

	bool play();

private:
	bool load();
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
	void resetAnimationLayers();
	void updateAnimationLayerFrames();
	void drawFrame();
	void drawTransientLayers(const TransientLayerCompositor &compositor);
	void drawSpriteLayer(const ResourceSpriteLayer &layer);
	void drawDrinkStrip();
	void presentFrame();
	void drawSpeechOverlay();
	void drawCaption();
	void beginSpeechLine(uint16 rowIndex, byte frameIndex);
	void wrapSpeechText(const Common::String &text, uint16 centerX, Common::Array<Common::String> &lines) const;
	void calculateSpeechOverlayBounds(SpeechOverlay &overlay, int centerX, int topY) const;
	uint speechTextWidth(const Common::String &text) const;
	uint speechOverlayTextWidth(const SpeechOverlay &overlay) const;
	bool waitAndRender(uint32 millis);
	bool pollEvents(byte &selectedAction);
	byte actionAtCursor() const;
	Common::String captionForAction(byte actionId) const;
	void setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue);
	void stopAudio();

	HollywoodEngine *_vm;
	SceneResources _resources;
	SceneSurfaceState _surface;
	SceneTextStore _textStore;
	SpeechOverlay _speechOverlay;
	SpeechPlayer _speech;
	SoundBank0Player _soundBank0;
	TransientLayerCompositor _animationLayers;
	byte _selectedDrinks[3];
	byte _selectedDrinkCount;
	byte _currentDrinkId;
	byte _introFrame;
	byte _tapFrame;
	byte _changeFrame;
	byte _liquidFrame;
	byte _mixFrame;
	byte _pourFrame;
	uint _drinkStripRow;
	byte _hoverActionId;
	bool _pourVisible;
	bool _exitRequested;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE5130_H
