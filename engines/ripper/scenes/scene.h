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

#ifndef RIPPER_SCENES_SCENE_H
#define RIPPER_SCENES_SCENE_H

#include "audio/mixer.h"
#include "common/array.h"

namespace Ripper {

class DialogueChooser;
class RipperEngine;

class Scene {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit Scene(RipperEngine *engine);
	virtual ~Scene() {}

	virtual Result run(uint sceneArgument) = 0;

protected:
	void prepare(const char *reason, uint cursorIndex, bool cursorVisible);
	void finish(const char *reason, int cursorIndex, bool cursorVisible);
	void stopAudio(Audio::SoundHandle &handle);

	RipperEngine *_engine;
	DialogueChooser &_chooser;

private:
	Common::Array<byte> _savedPalette;
};

} // End of namespace Ripper

#endif // RIPPER_SCENES_SCENE_H
