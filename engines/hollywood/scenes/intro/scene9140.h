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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9140_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9140_H

#include "hollywood/music.h"
#include "hollywood/scenes/presentation_scene.h"
#include "hollywood/scenes/scene_resources.h"
#include "hollywood/scenes/scene_text_store.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9140 : public PresentationScene {
public:
	Scene9140(HollywoodEngine *vm);

	bool play();

	enum StepType {
		kSpeechLeft,
		kSpeechRightLow,
		kSpeechRightHigh,
		kOpenRightPose,
		kCloseRightPose,
		kEnableLeftLoop,
		kDisableLeftLoop
	};

	struct SequenceStep {
		StepType type;
		byte rowIndex;
		byte frameIndex;
	};

private:
	bool load();
	void runVariantSequence(byte variantIndex);
	void runStep(const SequenceStep &step);
	void runSpeechLine(byte rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue, bool leftSpeaker);
	void runSpeechCue(uint16 textRecordId, byte continuationCount, uint16 voiceSampleId,
		uint16 centerX, uint16 topY, bool leftSpeaker);
	void animateRightPose(byte firstFrame, byte lastFrame);
	void presentAnimationFrame() override;
	void drawComposite();
	void fadeOutPalette();

	SpeechPlayer _speech;
	SceneTextStore _text;
	Common::Array<byte> _paletteResource;
	IndexedSurfaceBuffer _baseFramebuffer;
	byte _rightBodyFrame;
	byte _mouthFrame;
	bool _leftLoopEnabled;
	uint _speechAnimationStep;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9140_H
