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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_SCENES_GYM_SCENE_H
#define RIPPER_SCENES_GYM_SCENE_H

#include "audio/mixer.h"
#include "common/rect.h"

#include "ripper/media.h"
#include "ripper/resources.h"
#include "ripper/scenes/scene.h"

namespace Ripper {

class GymScene : public Scene, public MediaSequenceCallback {
public:
	explicit GymScene(RipperEngine *engine);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;
	bool ownsInput() const override { return true; }

private:
	bool initialize();
	bool playControlCue(uint cue);
	bool playStateMedia(uint state, uint16 &command);
	bool pulseKeyboardControl(uint control);
	void drawPressedControl() const;
	void updateCursor(const Common::Point &point);
	void stopAllAudio();

	BitmapAssetFrame _buttonFrames[2];
	Audio::SoundHandle _audioHandles[3];
	int _hoveredControl;
	int _pressedControl;
	uint _currentState;
};

} // End of namespace Ripper

#endif // RIPPER_SCENES_GYM_SCENE_H
