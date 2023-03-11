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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_UI_ANIMATEDBUTTON_H
#define NANCY_UI_ANIMATEDBUTTON_H

#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;

namespace UI {

class AnimatedButton : public RenderObject {
public:
	AnimatedButton(uint zOrder);
	virtual ~AnimatedButton() = default;

	void init() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	void setActive(bool active) { _isActive = active; }
	void setOpen(bool open) { _isOpen = open; }
	void setFrame(int frame);
	bool isPlaying() const;
	virtual void onClick() = 0;
	virtual void onTrigger() = 0;

protected:
	Common::Array<Common::Rect> _srcRects;
	uint32 _frameTime;
	Common::String _sound;
	bool _alwaysHighlightCursor;

	bool _isActive;
	int _currentFrame;
	uint32 _nextFrameTime;
	bool _isOpen;
	Common::Rect _hotspot;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_ANIMATEDBUTTON_H
