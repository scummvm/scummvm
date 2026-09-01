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

#ifndef HOLLYWOOD_SCENES_INTRO_SCENE9130_H
#define HOLLYWOOD_SCENES_INTRO_SCENE9130_H

#include "hollywood/music.h"
#include "hollywood/scenes/presentation_scene.h"
#include "hollywood/scenes/scene_resources.h"
#include "hollywood/scenes/scene_text_store.h"

namespace Hollywood {

class HollywoodEngine;

class Scene9130 : public PresentationScene {
public:
	Scene9130(HollywoodEngine *vm);

	bool play();

private:
	bool load();
	void runClipAndDialogue();
	void drawClipFrame(byte frameIndex);
	void maybeStartNextSpeechLine();
	bool prepareSpeechLine(byte frameIndex);
	bool startCurrentSpeechSegment();
	void stopAudio() override;

	MusicPlayer *_music;
	SpeechPlayer _speech;
	SceneTextStore _text;
	Common::Array<byte> _paletteResource;
	uint16 _activeTextRecordId;
	uint16 _activeVoiceSampleId;
	byte _activeContinuationCount;
	byte _activeContinuationIndex;
	byte _activeSpeechStyleIndex;
	bool _activeSpeechCue;
	byte _nextSpeechFrameIndex;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_INTRO_SCENE9130_H
