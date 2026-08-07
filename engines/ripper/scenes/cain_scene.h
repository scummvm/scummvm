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
 */

#ifndef RIPPER_SCENES_CAIN_SCENE_H
#define RIPPER_SCENES_CAIN_SCENE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/media.h"
#include "ripper/scenes/scene.h"

namespace Ripper {

class CainScene : public Scene, public MediaSequenceCallback {
public:
	explicit CainScene(RipperEngine *engine);

	Result run(uint sceneArgument) override;
	uint16 service(uint frame) override;

private:
	struct Choice {
		uint16 id;
		uint flag;
		uint textResource;
		const char *audioPath;
	};

	bool initialize();
	bool startVoice(const char *path, const char *source);
	bool serviceVoiceCompletion();
	bool finishConversation();
	void rebuildChoices();
	static const Choice *choiceTable();
	static uint choiceCount();
	const Choice *findAvailableChoice(uint16 id) const;
	void presentDialogueOverlay(uint frame);
	uint16 serviceInput();
	void updateCursor(const Common::Point &point);
	void stopAllAudio();

	Common::Array<Common::String> _gameText;
	Common::Array<const Choice *> _choices;
	Audio::SoundHandle _loopAudioHandle;
	Audio::SoundHandle _voiceHandle;
	int _hoveredControl;
	bool _conversationStarted;
	bool _voicePending;
	bool _choiceListRebuildPending;
};

} // End of namespace Ripper

#endif // RIPPER_SCENES_CAIN_SCENE_H
