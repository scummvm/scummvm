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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9170_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9170_H

#include "common/random.h"

#include "hollywood/music.h"
#include "hollywood/scenes/intro/intro_resource_set.h"
#include "hollywood/scenes/intro/intro_scene.h"
#include "hollywood/scenes/intro/intro_text.h"

namespace Hollywood {

class HollywoodEngine;

// Renders the ending as a viewport over a tall canvas whose animated channels
// restore clean rectangles before redrawing.
class Scene9170 : public IntroSceneBase {
public:
	Scene9170(HollywoodEngine *vm);

	bool play();

private:
	struct SubtitleOverlay {
		bool visible;
		byte colorIndex;
		uint16 centerX;
		uint16 topY;
		Common::Array<Common::String> lines;
	};

	bool load();
	bool loadChunk(uint index, Common::Array<byte> &destination, uint fixedSize);
	bool loadArenaChunk(uint index);
	void runSequence();
	void buildInitialStaticFrame();
	void switchToLowerRoomFrame();
	void addBlockListToCanvas(uint chunkIndex, int yOffset);
	void copyBaseToCanvasAtYOffset(int yOffset);
	void clearCanvasRows(int yOffset, int rowCount);
	void composeFrame();
	void restoreSpriteChannel(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, byte frameIndex);
	void drawSpriteChannel(uint chunkIndex, uint descriptorCount, const byte *frameMap,
		uint frameMapSize, byte frameIndex);
	void scrollByTable(const byte *table, uint tableSize, bool add);
	void scrollTo(uint targetRowOffset, int step);
	void waitWithAnimations(uint32 millis, byte speakerGroup, bool animateAmbient,
		bool presentChanges = true);
	void fadeInPalette();
	void startLowerRoomAmbience();
	void stopLowerRoomAmbience();
	void advanceLowerRoomAmbience();
	void runSpeechLine(byte rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, byte speakerGroup);
	void runSpeechCue(uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, byte speakerGroup);
	void advanceSpeechAnimation(byte speakerGroup);
	void advanceSpeakerIdleAnimation(byte speakerGroup);
	void advanceUpperIdleAnimations(byte speakerGroup);
	void resetSpeakerFrame(byte speakerGroup);
	byte nextTalkingFrame();
	void runEventOverlayFrames();
	void runShake();
	uint presentRowOffset() const override;
	void stopAudio() override;
	void clearSubtitle();
	void drawFrameOverlays() override;
	void wrapSubtitleText(const Common::String &text, uint16 anchorSceneX,
		Common::Array<Common::String> &lines) const;
	void calculateSubtitleBounds(uint16 anchorCenterX, uint16 anchorTopY);
	uint subtitleTextWidth(const Common::String &text) const;

	IntroResourceSet _resources;
	MusicPlayer *_music;
	SpeechPlayer _speech;
	SpeechPlayer _ambientSpeech;
	SoundBank0Player _sound;
	SoundBank0Player _ambientSound;
	IntroTextStore _text;
	Common::RandomSource _random;
	Common::Array<byte> _paletteResource;
	IndexedSurfaceBuffer _baseFramebuffer;
	IndexedSurfaceBuffer _staticFramebuffer;
	SubtitleOverlay _subtitle;
	uint _rowOffset;
	bool _upperActorsEnabled;
	bool _lowerActorsEnabled;
	bool _upperDirty[3];
	bool _lowerDirty;
	bool _effectDirty;
	bool _eventDirty;
	byte _upperFrames[3];
	byte _lowerFrame;
	byte _effectFrame;
	byte _eventFrame;
	byte _lastTalkingFrame;
	int _channelCanvasOffset;
	bool _ambientEffectsEnabled;
	uint16 _ambientSpeechSampleId;
	bool _shakeActive;
	uint _shakeRowOffset;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9170_H
