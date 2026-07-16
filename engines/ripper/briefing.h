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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_BRIEFING_H
#define RIPPER_BRIEFING_H

#include "audio/mixer.h"
#include "common/rect.h"
#include "ripper/resources.h"

namespace Ripper {

class ResourceManager;
class RipperEngine;
struct MouseState;

enum BriefingServiceResult {
	kBriefingIdle,
	kBriefingHovered,
	kBriefingActivated,
	kBriefingFailed
};

class BriefingManager {
public:
	explicit BriefingManager(RipperEngine *engine);
	~BriefingManager();

	bool initialize(ResourceManager &resources);
	bool arm(uint selector, bool playNotification = true);
	void restore(bool armed, uint selector);
	void clear();
	BriefingServiceResult service(const MouseState &mouse);
	void draw();

	bool isArmed() const { return _armed; }
	uint getSelector() const { return _selector; }

private:
	void advanceAnimation(uint32 now);
	void captureBacking();
	void restoreBacking();
	bool activate();

	RipperEngine *_engine;
	BitmapAssetSequence _frames;
	Common::Array<byte> _backing;
	Common::Rect _bounds;
	Audio::SoundHandle _alertHandle;
	uint32 _lastFrameMillis;
	uint _frameIndex;
	uint _selector;
	bool _armed;
	bool _initialized;
	bool _hovered;
	bool _announcedSelectors[9];
};

} // End of namespace Ripper

#endif // RIPPER_BRIEFING_H
