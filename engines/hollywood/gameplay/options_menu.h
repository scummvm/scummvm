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

#ifndef HOLLYWOOD_GAMEPLAY_OPTIONS_MENU_H
#define HOLLYWOOD_GAMEPLAY_OPTIONS_MENU_H

#include "common/array.h"
#include "common/path.h"
#include "common/str.h"
#include "common/types.h"
#include "graphics/managed_surface.h"
#include "hollywood/graphics.h"
#include "hollywood/music.h"

namespace Graphics {
struct Surface;
}

namespace Hollywood {

class HollywoodEngine;

class GameplayOptionsMenu {
public:
	GameplayOptionsMenu(HollywoodEngine *vm);

	void setAudioContext(const Common::Path &soundBank0ArchiveName, bool hasSpeechPreviewSample,
		uint16 speechPreviewSampleId);
	bool run(const Common::Array<byte> &basePalette);

private:
	enum HitAction {
		kHitNone,
		kHitSave,
		kHitLoad,
		kHitQuit,
		kHitPlay,
		kHitMusicToggle,
		kHitSoundToggle,
		kHitTestToggle,
		kHitMusicDown,
		kHitMusicUp,
		kHitSoundDown,
		kHitSoundUp,
		kHitVoiceDown,
		kHitVoiceUp,
		kHitTextSpeedDown,
		kHitTextSpeedUp,
		kHitSpeechText,
		kHitSpeechVoice,
		kHitSpeechBoth,
		kHitConfirmQuit,
		kHitCancelQuit
	};

	bool load();
	bool loadMenuFramebuffer();
	bool loadObjectPalette();
	bool loadObjectPaletteFromResource000();
	void preparePalette(const Common::Array<byte> &basePalette);
	void pollEvents(bool &done);
	void handleKeyDown(uint16 keycode, bool &done);
	void handleLeftClick(uint16 cursorX, uint16 cursorY, bool &done);
	HitAction hitActionAt(uint16 cursorX, uint16 cursorY) const;
	bool isVolumeHitAction(HitAction action) const;
	bool applyVolumeHitAction(HitAction action);
	void startHeldVolume(HitAction action);
	void stopHeldVolume();
	void advanceHeldVolume(uint32 delta);
	bool pointInGlobalRect(uint16 cursorX, uint16 cursorY, uint16 left, uint16 top,
		uint16 right, uint16 bottom) const;
	void changeVolume(byte &volume, int delta);
	void setSpeechMode(byte mode);
	void beginAudioSession();
	void endAudioSession();
	void disableTestAudio();
	void updateTestAudio();
	void stopTestAudio();
	uint16 testMusicCueId() const;
	uint16 testSoundCueId() const;
	void captureGameplayPreview();
	byte nearestPreviewPaletteColor(uint red, uint green, uint blue) const;
	void composeScreen();
	void drawGameplayPreview();
	void drawControls(Graphics::Surface &surface);
	void drawQuitConfirmation(Graphics::Surface &surface);
	void drawText(Graphics::Surface &surface, const Common::String &text, int globalX, int y,
		byte color, bool centered);
	void drawQuitConfirmationBacking();
	void drawQuitButtonFrame(int globalLeft, int globalRight);
	void shiftGlobalRectColors(int globalLeft, int top, int globalRight, int bottom, int delta);
	void drawStatusDot(int globalX, int y, byte color);
	void drawStatusDots();
	void drawValueBar(int rowIndex, byte value, byte color);
	void drawToggleSquare(int columnIndex, int rowIndex, byte color);
	void present();

	HollywoodEngine *_vm;
	IndexedSurfaceBuffer _menuFramebuffer;
	Common::Array<byte> _objectPaletteTriples;
	Common::Array<byte> _palette;
	Graphics::ManagedSurface _screen;
	Graphics::ManagedSurface _gameplayPreview;
	Palette6Bit _displayPalette;
	MusicPlayer _testMusic;
	SoundBank0Player _testSound;
	SpeechPlayer _testSpeech;
	Common::Path _soundBank0ArchiveName;
	uint16 _speechPreviewSampleId;
	bool _loaded;
	bool _confirmQuit;
	bool _loadedGame;
	bool _sceneMusicWasPlaying;
	bool _hasSpeechPreviewSample;
	HitAction _heldVolumeAction;
	uint32 _heldVolumeAccumulator;
	uint32 _heldVolumeRepeatMillis;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_OPTIONS_MENU_H
