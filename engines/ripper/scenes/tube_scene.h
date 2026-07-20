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

#ifndef RIPPER_SCENES_TUBE_SCENE_H
#define RIPPER_SCENES_TUBE_SCENE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/media.h"
#include "ripper/resources.h"
#include "ripper/scenes/scene.h"

namespace Ripper {

class TubeScene : public Scene, public MediaSequenceCallback {
public:
	explicit TubeScene(RipperEngine *engine);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;
	bool ownsInput() const override { return true; }

private:
	bool initialize();
	void drawSwitchFrame(uint frameIndex) const;
	void drawSwitch() const;
	void animateSwitch(bool enabled);
	uint countInstalledTubes() const;
	bool playCue(uint cue);
	bool playSegment(uint firstFrame, uint lastFrame, uint16 &command,
		uint loopStartFrame = 0xffffffff);
	uint16 serviceInput();
	uint16 serviceInventory();
	void updateCursor(const Common::Point &point);
	void stopAudio();

	Common::Array<BitmapAssetFrame> _switchFrames;
	Common::Array<Common::String> _gameText;
	Common::Array<byte> _switchBacking;
	Audio::SoundHandle _audioHandles[4];
	uint _tubeCount;
	int _hoveredControl;
	bool _switchOn;
	bool _secondaryCuePending;
};

} // End of namespace Ripper

#endif // RIPPER_SCENES_TUBE_SCENE_H
