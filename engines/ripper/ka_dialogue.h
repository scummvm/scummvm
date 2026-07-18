/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef RIPPER_KA_DIALOGUE_H
#define RIPPER_KA_DIALOGUE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/media.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class KaDialogueScene : public MediaSequenceCallback {
public:
	enum Result {
		kExited,
		kLoadFailed
	};

	explicit KaDialogueScene(RipperEngine *engine);

	Result run(uint sceneArgument);
	uint16 service(uint frame) override;

private:
	struct Choice {
		uint flag;
		uint textResource;
		const char *audioPath;
	};

	bool initialize();
	bool startVoice(const char *path, const char *source);
	bool serviceVoiceCompletion();
	void rebuildChoices();
	void serviceLoopAudio(uint frame);
	uint16 serviceInput();
	void updateCursor(const Common::Point &point);
	void stopAllAudio();

	RipperEngine *_engine;
	Common::RandomSource _random;
	Common::Array<Common::String> _gameText;
	Common::Array<Choice> _choices;
	Audio::SoundHandle _ambientHandle;
	Audio::SoundHandle _deckCueHandle;
	Audio::SoundHandle _loopCueHandles[3];
	Audio::SoundHandle _voiceHandle;
	uint _sceneArgument;
	int _hoveredControl;
	bool _conversationStarted;
	bool _voicePending;
	bool _acceptInput;
};

} // End of namespace Ripper

#endif // RIPPER_KA_DIALOGUE_H
