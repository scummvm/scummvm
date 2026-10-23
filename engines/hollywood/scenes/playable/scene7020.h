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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE7020_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE7020_H

#include "hollywood/scenes/playable/playable_scene.h"

namespace Hollywood {

class HollywoodEngine;

class Scene7020 : public PlayableScene {
public:
	Scene7020(HollywoodEngine *vm);

	bool play();

private:
	void initializeCustomPreviewState() override;
	void drawCustomActorForegroundComposite(int activeWorldX, int activeWorldY,
		byte actorDrawOrderMode) override;
	void advanceCustomGameplayLoop(uint32 delta) override;
	void advancePrimarySpeechAnimation(uint32 delta) override;
	void advanceAmbientAudio(uint32 delta) override;
	byte primarySpeechAnimationBaseFrame(byte animationGroup) const override;
	void setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) override;

	bool loadOwner0StaticSpeechTables();
	bool getOwner0StaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const;
	Common::String getOwner0LargeTextRecord(uint16 recordId) const;
	void runOwner0SpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
		uint16 voiceSampleId, uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop,
		bool animatePrimaryDialogue);
	void beginOwner0SecondarySpeechLine(uint16 rowIndex, byte frameIndex);
	void beginOwner0PrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue);
	void runScriptedSequence();
	void runOpeningSueEntryAndIdleWaits();
	bool finishOpeningSueEntryAfterSkip();
	void runChunk6FrameRange(byte firstFrame, byte lastFrame, byte finalPoseMode);
	void runChunk7RevealFramesThenHold();
	void blackOutScenePalette();
	void resetTransientOverlayLayers();
	void setChunk6Visible(bool visible);
	void setChunk6Frame(byte frameMapIndex);
	void setChunk7Visible(bool visible);
	void setChunk7Frame(byte frameIndex);
	byte chunk7Frame() const;

	Common::Array<byte> _owner0SpeechCueDescriptors;
	Common::Array<byte> _owner0SmallRows;
	Common::Array<byte> _owner0LargeRows;
	byte _primaryPoseMode;
	bool _drawChunk7OverlayInsteadOfActor;
	uint32 _chunk7TimerAccumulator;
	uint32 _primaryTimerAccumulator;
};

} // End of namespace Hollywood

#endif
